#include "Server.hpp"

namespace massy::common::communication
{
Server::Server(const std::string &address, const std::string &port)
    : address_(address), port_(port), ioContext_(), acceptor_(ioContext_), signals_(ioContext_, SIGINT),
      connectionManager_()
{
  std::clog << "create Server" << std::endl;
}

bool Server::start()
{
  boost::system::error_code errorCode;
  boost::asio::ip::tcp::resolver resolver(ioContext_);
  boost::asio::ip::tcp::endpoint endPoint = *resolver.resolve(address_, port_).begin();

  // open
  acceptor_.open(endPoint.protocol(), errorCode);
  if (errorCode)
  {
    std::cerr << "Error opening endPoint : " << errorCode.message() << std::endl;
    return false;
  }

  // reuse
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), errorCode);
  if (errorCode)
  {
    std::cerr << "Error setting option : " << errorCode.message() << std::endl;
    return false;
  }

  // bind
  acceptor_.bind(endPoint, errorCode);
  if (errorCode)
  {
    std::cerr << "Error binding to port : " << errorCode.message() << std::endl;
    return false;
  }

  // listen
  acceptor_.listen(boost::asio::socket_base::max_listen_connections, errorCode);
  if (errorCode)
  {
    std::cerr << "Error listening to port : " << errorCode.message() << std::endl;
    return false;
  }

  doAccept();
  return true;
}

void Server::doAccept()
{
  acceptor_.async_accept(
      [this](boost::system::error_code errorCode, boost::asio::ip::tcp::socket socket)
      {
        // signal check
        if (!acceptor_.is_open())
        {
          std::cerr << "Server is stopped" << std::endl;
          return;
        }
        std::clog << "async_accept : " << std::endl;
        if (!errorCode)
        {
          connectionManager_.start(Connection::createConnection(std::move(socket), ioContext_, connectionManager_));
        }
        else
        {
          std::cerr << "Error acception connection" << errorCode.message() << std::endl;
        }
        doAccept();
      });
}

void Server::gracefullyStop()
{
  connectionManager_.gracefullyStopAll();
  ioContext_.stop();
  acceptor_.close();
}

void Server::run() { ioContext_.run(); }

bool Server::startAndRun()
{
  if (start())
  {
    signals_.async_wait(
        [this](const boost::system::error_code &error, int signal_number)
        {
          std::clog << "handling signal " << signal_number << std::endl;
          gracefullyStop();
        });
    run();
    return true;
  }
  else
  {
    return false;
  }
}
} // namespace massy::common::communication