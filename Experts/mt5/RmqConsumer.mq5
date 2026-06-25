//+------------------------------------------------------------------+
//| RmqConsumer.mq5                                                  |
//| RabbitMq4Metatrader — consume from RabbitMQ via RmqBridge.dll    |
//+------------------------------------------------------------------+
#property copyright "RabbitMq4Metatrader"
#property version   "1.00"

#import "RmqBridge.dll"
   int RmqConnect(string host, int port, string user, string pass);
   void RmqDisconnect();
   int RmqPoll(string &buffer, int buffer_size);
   int RmqGetLastError(string &err, int size);
#import

input string RmqHost = "127.0.0.1";
input int    RmqPort = 5672;
input string RmqUser = "rmq4mt";
input string RmqPass = "rmq4mt";

int OnInit()
{
   if(RmqConnect(RmqHost, RmqPort, RmqUser, RmqPass) != 1)
   {
      string err = "";
      RmqGetLastError(err, 256);
      Print("RmqConsumer: connect failed: ", err);
      return(INIT_FAILED);
   }
   EventSetTimer(1);
   return(INIT_SUCCEEDED);
}

void OnDeinit(const int reason)
{
   EventKillTimer();
   RmqDisconnect();
}

void OnTimer()
{
   string buffer = "";
   int received = RmqPoll(buffer, 4096);
   if(received > 0)
      Print("RmqConsumer: ", buffer);
}
