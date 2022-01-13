enum RavenousLogLevel {
   LOG_FATAL = 0,
   LOG_INFO = 1,
   LOG_WARNING = 2,
   LOG_ERROR = 3
};

#define Quit_fatal(x)  {      std::string mmsx; \
                              mmsx += "---------------------------------------------------\n"; \
                              mmsx += "> FATAL error occured. Error description:          \n"; \
                              mmsx += "---------------------------------------------------\n"; \
                              std::cout << mmsx << x << "\n"; \
                              assert(false); }


void log(RavenousLogLevel level, std::string message)
{
   std::string message_header = "\n";
   switch(level)
   {
      case LOG_FATAL:
      {
         message_header += "---------------------------------------------------\n";
         message_header += "> FATAL error occured. Error description:          \n";
         message_header += "---------------------------------------------------\n";
      }
      case LOG_INFO:
      {
         message_header += "\n> INFO message:          \n";
      }
      case LOG_WARNING:
      {
         message_header += "\n> WARNING message:          \n";
      }
      case LOG_ERROR:
      {
         message_header += "\n> ERROR message:          \n";
      }
   }

   std::cout << message_header << message << "\n";

   if(level == LOG_FATAL)
      assert(false);
}



