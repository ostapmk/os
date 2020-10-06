#include <Lab1/Server/Server.hpp>

#include <Lab1/3rdparty/lyra/lyra.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/signal_set.hpp>
#include <csignal>
#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    uint16_t port = 20'003;
    std::string host = "127.0.0.1";
    bool show_help = false;

    auto cli
        = lyra::opt(port, "port")
            ["-p"]["--port"]
            ("Port to bind to [default: 20003]")
        | lyra::opt(host, "host")
            ["-l"]["--listen"]
            ("Address to listen to [default: 127.0.0.1]")
        | lyra::help(show_help)
            ("Show help message");
    
    auto result = cli.parse({argc, argv});
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
        return 1;
    }

    if (show_help) {
        std::cout << cli << std::endl;
        return 0;
    }

    try {
        boost::asio::io_context context;
        lab1::Server server{context, boost::asio::ip::make_address(host), port};
        boost::asio::signal_set signal_set{context, SIGTERM};
        auto work_guard = boost::asio::make_work_guard(context);

        /// Asyncrhonously listen to termination signal
        signal_set.async_wait(
            [&] (const auto /*ec*/, const int /*sig*/) {
                signal_set.cancel();
                server.stop();
                work_guard.reset();
            }
        );
        /// Start server
        server.start();
        /// Start main event loop
        context.run();
    } catch(const std::exception& e) {
        std::cerr << "Application failed with error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
