#include "rmq4mt/client.hpp"
#include "rmq4mt/error.hpp"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/framing.h>
#include <rabbitmq-c/tcp_socket.h>

namespace rmq4mt {

namespace {

constexpr int kChannel = 1;

bool decode_server_error(std::string& message, const amqp_rpc_reply_t& reply) {
    if (reply.reply_type != AMQP_RESPONSE_SERVER_EXCEPTION) {
        return false;
    }
    if (reply.reply.id == AMQP_CHANNEL_CLOSE_METHOD) {
        const auto* close = static_cast<amqp_channel_close_t*>(reply.reply.decoded);
        if (close != nullptr && close->reply_text.bytes != nullptr) {
            message += ": ";
            message.append(static_cast<const char*>(close->reply_text.bytes), close->reply_text.len);
            return true;
        }
    }
    if (reply.reply.id == AMQP_CONNECTION_CLOSE_METHOD) {
        const auto* close = static_cast<amqp_connection_close_t*>(reply.reply.decoded);
        if (close != nullptr && close->reply_text.bytes != nullptr) {
            message += ": ";
            message.append(static_cast<const char*>(close->reply_text.bytes), close->reply_text.len);
            return true;
        }
    }
    return false;
}

bool check_login_reply(std::string& last_error, const amqp_rpc_reply_t& reply) {
    if (reply.reply_type == AMQP_RESPONSE_NORMAL) {
        return true;
    }
    last_error = "amqp_login failed (reply type " + std::to_string(reply.reply_type) + ")";
    decode_server_error(last_error, reply);
    return false;
}

}  // namespace

Client::Client(Config config) : config_(std::move(config)) {}

Client::~Client() {
    disconnect();
}

const std::string& Client::last_error() const {
    return last_error_;
}

bool Client::check_reply(const char* step) {
    const amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn_);
    if (reply.reply_type == AMQP_RESPONSE_NORMAL) {
        return true;
    }
    last_error_ = std::string(step) + " failed (reply type " + std::to_string(reply.reply_type) + ")";
    decode_server_error(last_error_, reply);
    return false;
}

bool Client::connect() {
    disconnect();
    last_error_.clear();

    conn_ = amqp_new_connection();
    if (conn_ == nullptr) {
        last_error_ = "amqp_new_connection failed";
        return false;
    }

    amqp_socket_t* socket = amqp_tcp_socket_new(conn_);
    if (socket == nullptr) {
        last_error_ = "amqp_tcp_socket_new failed";
        disconnect();
        return false;
    }

    const int open_status = amqp_socket_open(socket, config_.host.c_str(), config_.port);
    if (open_status != AMQP_STATUS_OK) {
        last_error_ = format_wsa_error("amqp_socket_open", open_status);
        disconnect();
        return false;
    }

    amqp_rpc_reply_t login = amqp_login(
        conn_,
        config_.vhost.c_str(),
        0,
        131072,
        30,
        AMQP_SASL_METHOD_PLAIN,
        config_.user.c_str(),
        config_.pass.c_str());
    if (!check_login_reply(last_error_, login)) {
        disconnect();
        return false;
    }

    amqp_channel_open(conn_, kChannel);
    if (!check_reply("amqp_channel_open")) {
        disconnect();
        return false;
    }

    if (!setup_topology()) {
        disconnect();
        return false;
    }

    connected_ = true;
    return true;
}

bool Client::setup_topology() {
    const amqp_bytes_t exchange = amqp_cstring_bytes(config_.exchange.c_str());
    amqp_exchange_declare(
        conn_, kChannel, exchange, amqp_cstring_bytes("topic"),
        0, 1, 0, 0, amqp_empty_table);
    if (!check_reply("amqp_exchange_declare")) {
        return false;
    }

    const amqp_bytes_t queue = amqp_cstring_bytes(config_.queue.c_str());
    amqp_queue_declare(conn_, kChannel, queue, 0, 1, 0, 0, amqp_empty_table);
    if (!check_reply("amqp_queue_declare")) {
        return false;
    }

    amqp_queue_bind(
        conn_, kChannel, queue, exchange, amqp_cstring_bytes("#"), amqp_empty_table);
    if (!check_reply("amqp_queue_bind")) {
        return false;
    }

    amqp_basic_consume(
        conn_, kChannel, queue, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    if (!check_reply("amqp_basic_consume")) {
        return false;
    }

    consuming_ = true;
    return true;
}

void Client::disconnect() {
    if (conn_ != nullptr) {
        if (connected_) {
            amqp_channel_close(conn_, kChannel, AMQP_REPLY_SUCCESS);
            amqp_connection_close(conn_, AMQP_REPLY_SUCCESS);
        }
        amqp_destroy_connection(conn_);
        conn_ = nullptr;
    }
    connected_ = false;
    consuming_ = false;
}

bool Client::is_connected() const {
    return connected_;
}

bool Client::publish(const char* body) {
    if (!connected_ || conn_ == nullptr) {
        last_error_ = "not connected";
        return false;
    }

    const std::string payload = body != nullptr ? body : "";
    amqp_bytes_t message = amqp_cstring_bytes(payload.c_str());
    amqp_bytes_t exchange = amqp_cstring_bytes(config_.exchange.c_str());

    amqp_basic_properties_t props;
    std::memset(&props, 0, sizeof(props));
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("application/json");
    props.delivery_mode = 2;

    const int status = amqp_basic_publish(
        conn_, kChannel, exchange, amqp_empty_bytes,
        0, 0, &props, message);
    if (status != AMQP_STATUS_OK) {
        last_error_ = format_wsa_error("amqp_basic_publish", status);
        return false;
    }
    return true;
}

int Client::poll(char* buffer, std::size_t buffer_size) {
    if (!connected_ || conn_ == nullptr || !consuming_ || buffer_size == 0) {
        return 0;
    }

    amqp_envelope_t envelope;
    struct timeval timeout = {0, 0};
    const amqp_rpc_reply_t reply = amqp_consume_message(conn_, &envelope, &timeout, 0);
    if (reply.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION &&
        reply.library_error == AMQP_STATUS_TIMEOUT) {
        return 0;
    }
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        return 0;
    }

    const std::size_t length = envelope.message.body.len;
    if (length == 0 || length + 1 > buffer_size) {
        amqp_destroy_envelope(&envelope);
        return 0;
    }

    std::memcpy(buffer, envelope.message.body.bytes, length);
    buffer[length] = '\0';
    amqp_destroy_envelope(&envelope);
    return static_cast<int>(length);
}

}  // namespace rmq4mt
