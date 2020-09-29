#include <Lab1/Server/Session.hpp>

#include <Lab1/Server/Operations.hpp>

#include <algorithm>
#include <array>
#include <boost/asio/buffer.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <sys/types.h>
#include <sysexits.h>
#include <utility>

namespace lab1 {
namespace {

    /**
     * @brief Parse string in the following format:
     *  <operation><spaces[min:1]><index><spaces>
     * @example
     *  OR 1
     * @example
     *  AND     5
     */
    [[nodiscard]]
    constexpr auto parse(std::string_view str) noexcept -> std::optional<std::pair<Operation, size_t>>
    {
        Operation op;
        if (const auto pos = str.find_first_of(" "); pos != std::string_view::npos) {
            if (const auto parsed = from_string(str.substr(0, pos))) {
                op = std::move(*parsed);
                str.remove_prefix(pos);
            } else {
                return {};
            }
        } else {
            return {};
        }

        /// Skip spaces
        while (!str.empty() && str.back() == ' ') {
            str.remove_suffix(1);
        }

        if (str.empty()) {
            return {};
        }

        size_t value = 0;
        size_t power = 1;
        while (!str.empty() && ('0' <= str.back() && str.back() <= '9')) {
            value += power * (str.back() - '0');
            power *= 10;
            str.remove_suffix(1);
        }

        /// Skip spaces
        while (!str.empty() && str.back() == ' ') {
            str.remove_suffix(1);
        }

        if (!str.empty()) {
            return {};
        }

        return std::pair{std::move(op), value};
    }

    constexpr std::string_view kUsage = 
        "Copyright (c) 2020 Ostap Mykytiuk\n"
        "\n"
        "SYNOPSIS\n"
        "    Provide operation and index to retrieve predefined functions attributes.\n"
        "    Apply operation to functions result.\n"
        "\n"
        "OPERATIONS\n"
        "    OR\n"
        "        - logical 'OR' of operands\n"
        "    AND\n"
        "        - logical 'AND' of operands\n"
        "    MUL\n"
        "        - multiply operands\n"
        "\n"
        "INDEX RANGE\n"
        "    [0 - 5]\n"
        "\n"
        "EXAMPLE\n"
        "   OR 0\n"
        "\n"
        "\n";

    constexpr std::string_view kInvalidInput = "You have an error in your input, try again!\n";

    constexpr std::string_view kOutOfRange = "Provided index is out of allowed range!\n";

} // namespace


Session::Session(boost::asio::io_context& context,
                 boost::asio::ip::tcp::socket socket) :
    _context{context},
    _socket{std::move(socket)}
{ }

void Session::start()
{
    auto self = shared_from_this();
    boost::asio::spawn(
        _context,
        [this, self] (boost::asio::yield_context yield) {
            boost::system::error_code ec;

            /// Start from sending greeting message
            boost::asio::async_write(
                _socket,
                boost::asio::buffer(kUsage),
                yield[ec]
            );

            /// Start reading requests from client
            std::string buffer;
            buffer.reserve(1024);
            while (_socket.is_open()) {
                /// Read operation and index
                /// Allow to read only small chunk of data otherwise
                /// user is abusing us
                const auto size = boost::asio::async_read_until(
                    _socket,
                    boost::asio::dynamic_buffer(buffer, buffer.capacity()),
                    '\n',
                    yield[ec]
                );

                if (ec) {
                    /// Connection aborted
                    return;
                }

                const std::string_view input{buffer.data(), size - 1};
                if (input.empty()) {
                    /// It's not an error
                    continue;
                }

                const auto line = parse(input);
                buffer.erase(0, size);

                if (!line) {
                    /// Send error message
                    boost::asio::async_write(
                        _socket,
                        boost::asio::buffer(kInvalidInput),
                        yield[ec]
                    );

                    /// Try again
                    continue;
                }

                /// Split into separate variables
                const auto [operation, index] = *line;
                const auto error = std::visit(
                    [&, this, index = index] (const auto operation) {
                        using Op = std::remove_const_t<decltype(operation)>;

                        /// Check whether index fit into bounds
                        if (index >= Op::kSize) {
                            boost::asio::async_write(
                                _socket,
                                boost::asio::buffer(kOutOfRange),
                                yield[ec]
                            );

                            return false;
                        }

                        /// Submit functions to execution
                        auto f = _submit<Op, spos::lab1::demo::f_func<Op::kNativeOperation>>(index);
                        auto g = _submit<Op, spos::lab1::demo::g_func<Op::kNativeOperation>>(index);
                        
                        /// TODO
                        /// Timer to periodically check for completion
                        // boost::asio::deadline_timer timer{_context};
                        // const auto result = Op::compute(f.future().get().value(), g.future().get().value());
                        // const auto serialized = Op::serialize(result);
                        boost::asio::async_write(
                            _socket,
                            boost::asio::buffer("Implement me"),
                            yield[ec]
                        );

                        return true;
                    },
                    operation
                );

                if (error) {
                    /// Continue on error
                    continue;
                }
            }
        }
    );
}

template<typename Op, auto F>
[[nodiscard]]
auto Session::_submit(const size_t index) -> Result<typename Op::value_type>
{
    /// Pipe for communication with child
    auto pipe = std::make_shared<boost::process::async_pipe>(_context);
    /// Notify context that we are about to fork
    _context.notify_fork(boost::asio::io_context::fork_prepare);

    const auto pid = ::fork();
    if (pid == 0) {
        /// Notify io_context from child
        _context.notify_fork(boost::asio::io_context::fork_child);
        /// Close reading end of a pipe
        std::move(*pipe).source().close();
        /// Compute function
        const auto result = (*F)(index);
        /// Serialize result
        const auto serialized = Op::serialize(result);
        /// Write result to pipe
        boost::system::error_code ec;
        boost::asio::write(*pipe, boost::asio::buffer(serialized), ec);
        if (ec) {
            /// Exit with an error
            std::exit(EX_SOFTWARE);
        }
        /// Exit successfully
        std::exit(EX_OK);
    } else if (pid < 0) {
        /// Rare case that can happen when system has run out of resources
        std::cerr << "Fork failed" << std::endl;
        /// Panic
        std::exit(EX_OSERR);
    }

    /// Notify io_context from parent
    _context.notify_fork(boost::asio::io_context::fork_parent);
    /// Close writing part of a pipe
    std::move(*pipe).sink().close();
    /// Promise to store value from async operation
    std::promise<std::optional<typename Op::value_type>> promise;
    /// Future function's result
    auto future = promise.get_future();
    /// Read result from a pipe
    auto buffer = std::make_shared<std::string>();
    boost::asio::async_read(
        *pipe, 
        boost::asio::dynamic_buffer(*buffer),
        [promise = std::move(promise), pipe, buffer] (const auto ec, const auto) mutable {
            if (ec) {
                return promise.set_value({});
            }

            auto deserialized = Op::deserialize(*buffer);
            if (!deserialized) {
                return promise.set_value({});
            }

            promise.set_value(std::move(*deserialized));
        }
    );

    return {pid, std::move(future), pipe};
}

} // namespace lab1
