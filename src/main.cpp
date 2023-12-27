#include <iostream>

#include "common/communication/include/Server.hpp"

namespace mcc = massy::common::communication;

int main(int argc, char** argv)
{
   try {
      if (argc != 3) {
         std::cerr << "Usage: client <host> <port>\n";
         return 1;
      }
      mcc::Server server(argv[1], argv[2]);
      server.startAndRun();
   }
   catch (...) {
      std::cerr << "handling" << std::endl;
   }
}
