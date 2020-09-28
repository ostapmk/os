#include <Lab1/Server/Server.hpp>

#include <Lab1/Server/Session.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <memory>
#include <utility>

namespace lab1 {

Server::Server(boost::asio::io_context& context,
               const boost::asio::ip::address& address,
               const uint16_t port) :
    _context{context},
    _acceptor{_context, {address, port}}
{ }

void Server::start()
{
    /// Start listening
    _acceptor.listen();

    std::cout << "Server started listening on " << _acceptor.local_endpoint() << std::endl;

    /// Start main loop of connections accepting
    boost::asio::spawn(
        _context,
        [this] (boost::asio::yield_context yield) {
            boost::system::error_code ec;
            boost::asio::ip::tcp::socket socket{_context};
            while (_acceptor.is_open()) {
                _acceptor.async_accept(socket, yield[ec]);
                if (ec) {
                    std::cerr << "Acceptor failed with message: " << ec.message() << std::endl;
                }

                /// Start serving client
                std::make_shared<Session>(_context, std::move(socket))->start();                                                                                         
            }            
        }
    );
}

void Server::stop()
{
    std::cout << "Server asked to stop" << std::endl;
    /// Stop accepting incoming connections
    _acceptor.cancel();
}

} // namespace lab1
