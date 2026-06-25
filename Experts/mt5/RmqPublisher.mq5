//+------------------------------------------------------------------+
//| RmqPublisher.mq5                                                 |
//| RabbitMq4Metatrader — publish to RabbitMQ via RmqBridge.dll      |
//+------------------------------------------------------------------+
#property copyright "RabbitMq4Metatrader"
#property version   "1.00"

#import "RmqBridge.dll"
   int RmqConnect(string host, int port, string user, string pass);
   void RmqDisconnect();
   int RmqPublish(string body);
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
      Print("RmqPublisher: connect failed: ", err);
      return(INIT_FAILED);
   }
   return(INIT_SUCCEEDED);
}

void OnDeinit(const int reason)
{
   RmqDisconnect();
}

void OnTick()
{
   MqlTick tick;
   if(!SymbolInfoTick(_Symbol, tick))
      return;

   string payload = StringFormat("{\"symbol\":\"%s\",\"bid\":%G,\"ask\":%G}",
                                 _Symbol, tick.bid, tick.ask);
   RmqPublish(payload);
}
