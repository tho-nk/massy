#include "Connection.hpp"

#include <boost/core/ignore_unused.hpp>
#include <chrono>
#include <functional>

namespace massy::common::communication
{
Connection::Connection(boost::asio::ip::tcp::socket &&socket, boost::asio::io_context &ioContext,
                       ConnectionManager &connectionManager)
    : socket_(std::forward<boost::asio::ip::tcp::socket>(socket)), epochTimer_(ioContext),
      connectionManager_(connectionManager)
{
  std::clog << "create Connection" << std::endl;
}

void Connection::start()
{
  doRead();
  doWrite();
}

void Connection::doRead()
{
  boost::asio::async_read(
      socket_, request_, boost::asio::transfer_at_least(1),
      std::bind(&Connection::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void Connection::onRead(boost::system::error_code errorCode, size_t transferredBytes)
{
  if (errorCode)
  {
    std::cerr << "Error during read operation : " << errorCode.message() << std::endl;
    connectionManager_.stop(shared_from_this());
    return;
  }
  else
  {
    std::clog << "message from client " << this << " := " << &request_ << std::endl;
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer("hey\n"),
                             [self](boost::system::error_code errorCode, size_t transferredBytes)
                             {
                               if (errorCode)
                               {
                                 std::cerr << "Error during write operation : " << errorCode.message() << std::endl;
                                 self->connectionManager_.stop(self);
                               }
                               std::clog << "send hey to client " << self << std::endl;
                             });
  }
  doRead();
}

void Connection::doWrite()
{
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
  std::clog << "send " << ms << " to client " << this << std::endl;
  boost::asio::async_write(
      socket_, boost::asio::buffer(std::to_string(ms) + "\n"),
      std::bind(&Connection::onWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void Connection::onWrite(boost::system::error_code errorCode, size_t transferredBytes)
{
  boost::ignore_unused(transferredBytes);
  if (errorCode)
  {
    std::cerr << "Error during write operation : " << errorCode.message() << std::endl;
    connectionManager_.stop(shared_from_this());
    return;
  }

  epochTimer_.expires_after(boost::asio::chrono::seconds(1));
  epochTimer_.async_wait(std::bind(&Connection::doWrite, shared_from_this()));
}

void Connection::sendBye()
{
  auto self(shared_from_this());
  boost::asio::async_write(socket_, boost::asio::buffer("bye\n"),
                           [self](boost::system::error_code errorCode, size_t transferredBytes)
                           {
                             if (errorCode)
                             {
                               std::cerr << "Error during write operation : " << errorCode.message() << std::endl;
                               self->connectionManager_.stop(self);
                             }
                             std::clog << "send bye to client " << self << std::endl;
                           });
}

void Connection::stop()
{
  socket_.close();
  epochTimer_.cancel();
}

void ConnectionManager::start(std::shared_ptr<Connection> connection)
{
  std::clog << "ConnectionManager::start connections_.size() = " << connections_.size() << std::endl;
  connections_.insert(connection);
  connection->start();
}

void ConnectionManager::stop(std::shared_ptr<Connection> connection)
{
  std::clog << "ConnectionManager::stop" << std::endl;
  connections_.erase(connection);
  connection->stop();
}

void ConnectionManager::stopAll()
{
  std::clog << "ConnectionManager::stopAll" << std::endl;
  for (auto &connection : connections_)
  {
    connection->stop();
  }
  connections_.clear();
}

void ConnectionManager::gracefullyStopAll()
{
  std::clog << "ConnectionManager::gracefullyStopAll" << std::endl;
  for (auto &connection : connections_)
  {
    connection->sendBye();
    connection->stop();
  }
  connections_.clear();
}
} // namespace massy::common::communication