#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>

#include "Connection.hpp"

namespace massy::common::communication
{

class Server
{
public:
  Server(const std::string &address, const std::string &port);
  Server(const Server &) = delete;
  Server(const Server &&) = delete;
  Server &operator=(const Server &) = delete;
  Server &operator=(const Server &&) = delete;
  ~Server() { std::cout << "Server::~Server" << std::endl; }

  bool startAndRun();

private:
  const std::string port_;
  const std::string address_;
  boost::asio::io_context ioContext_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::signal_set signals_;

  ConnectionManager connectionManager_;

  bool start();
  void run();
  void doAccept();
  void gracefullyStop();
};
} // namespace massy::common::communication