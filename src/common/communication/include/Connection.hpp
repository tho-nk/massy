#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <set>
#include <string>

namespace massy::common::communication
{

class Connection;

class ConnectionManager
{
public:
  ConnectionManager() { std::clog << "create ConnectionManager" << std::endl; }
  ConnectionManager(const ConnectionManager &) = delete;
  ConnectionManager(const ConnectionManager &&) = delete;
  ConnectionManager &operator=(const ConnectionManager &) = delete;
  ConnectionManager &operator=(const ConnectionManager &&) = delete;
  ~ConnectionManager() { std::clog << "destroy ConnectionManager" << std::endl; }

  void start(std::shared_ptr<Connection> connection);
  void stop(std::shared_ptr<Connection> connection);
  void stopAll();
  void gracefullyStopAll();

private:
  std::set<std::shared_ptr<Connection>> connections_;
};

class Connection : public std::enable_shared_from_this<Connection>
{
public:
  Connection(const Connection &) = delete;
  Connection(const Connection &&) = delete;
  Connection &operator=(const Connection &) = delete;
  Connection &operator=(const Connection &&) = delete;
  ~Connection() { std::clog << "destroy Connection" << std::endl; }

  static std::shared_ptr<Connection> createConnection(boost::asio::ip::tcp::socket &&socket,
                                                      boost::asio::io_context &ioContext,
                                                      ConnectionManager &connectionManager)
  {
    return std::shared_ptr<Connection>(
        new Connection(std::forward<boost::asio::ip::tcp::socket>(socket), ioContext, connectionManager));
  }

  void start();
  void stop();
  void sendBye();

private:
  explicit Connection(boost::asio::ip::tcp::socket &&socket, boost::asio::io_context &ioContext,
                      ConnectionManager &connectionManager);
  void onRead(boost::system::error_code errorCode, size_t transferredBytes);
  void doRead();

  void onWrite(boost::system::error_code errorCode, size_t transferredBytes);
  void doWrite();

  boost::asio::ip::tcp::socket socket_;

  boost::asio::streambuf request_;
  boost::asio::steady_timer epochTimer_;

  // std::shared_ptr<ConnectionManager>
  ConnectionManager &connectionManager_;
};

} // namespace massy::common::communication