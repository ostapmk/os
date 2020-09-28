#pragma once

#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <cstdint>

namespace lab1 {

/**
 * @brief Server for lab work.
 */
class Server
{
public:
    /**
     * @brief Construct server object.
     * @param context Reference to execution context.
     * @param address Address to listen to incoming connections.
     * @param port Port to bind address to.
     */
    Server(boost::asio::io_context& context,
           const boost::asio::ip::address& address,
           uint16_t port);

    /**
     * @brief Start serving requests.
     */
    void start();

    /**
     * @brief Gracefully shutdown the server.
     */
    void stop();

private:
    boost::asio::io_context& _context;
    boost::asio::ip::tcp::acceptor _acceptor;
};

} // namespace lab1
