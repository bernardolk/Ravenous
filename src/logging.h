enum RavenousLogLevel {
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
      case LOG_INFO:
      {
         message_header += "\n> INFO message: ";
         break;
      }
      case LOG_WARNING:
      {
         message_header += "\n> WARNING message: ";
         break;
      }
      case LOG_ERROR:
      {
         message_header += "\n> ERROR message: ";
         break;
      }
   }

   std::cout << message_header << message << "\n";
}



