#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/process/async_pipe.hpp>
#include <cstddef>
#include <future>
#include <memory>
#include <optional>
#include <unistd.h>
#include <wait.h>

namespace lab1 {

/**
 * @brief Single session with a user.
 */
class Session final: public std::enable_shared_from_this<Session>
{
public:
    /**
     * @brief Construct session from already
     *  opened socket.
     */
    Session(boost::asio::io_context& context,
            boost::asio::ip::tcp::socket socket);

    /**
     * @brief Start serving client.
     */
    void start();

private:
    template<typename T>
    class Result
    {
    public:
        using future_type = std::shared_future<std::optional<T>>;

        Result(const pid_t pid, 
               future_type future,
               std::shared_ptr<boost::process::async_pipe> pipe) noexcept :
            _pid{pid},
            _future{std::move(future)},
            _pipe{std::move(pipe)}
        { }

        ~Result() noexcept
        {
            /// Terminate child process
            ::kill(_pid, SIGKILL);
            /// Collect its status code to omit orphans
            int status;
            ::waitpid(_pid, &status, 0);
            /// Close pipe
            boost::system::error_code ec;
            _pipe->close(ec);
        }

        [[nodiscard]]
        auto& future() noexcept
        {
            return _future;
        }

        [[nodiscard]]
        const auto& future() const noexcept
        {
            return _future;
        }
    
    private:
        pid_t _pid;
        future_type _future;
        std::shared_ptr<boost::process::async_pipe> _pipe;
    };

    template<typename Op, auto F>
    [[nodiscard]]
    auto _submit(size_t index) -> Result<typename Op::value_type>;

private:
    boost::asio::io_context& _context;
    boost::asio::ip::tcp::socket _socket;
};

} // namespace lab1
