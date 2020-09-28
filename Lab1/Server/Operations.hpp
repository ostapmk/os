#pragma once

#include <Lab1/3rdparty/demofuncs.hpp>

#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace lab1 {
namespace detail {

    [[nodiscard]]
    constexpr auto to_string(const bool value) noexcept -> std::string_view
    {
        return value ? "true" : "false";
    }

    [[nodiscard]]
    constexpr auto bool_from_string(const std::string_view str) noexcept -> std::optional<bool>
    {
        if (str == "true") {
            return true;
        }

        if (str == "false") {
            return false;
        }

        return {};
    }

} // namespace detail

/**
 * @brief Basic type of operation to implement common logic and
 *  some stuff injection.
 */
template<typename T, auto N>
struct BasicOperation
{
    /**
     * @brief Type of operands.
     */
    using value_type = T;

    /**
     * @brief Underlying native operation.
     */
    static constexpr auto kNativeOperation = N;

    /**
     * @brief Size of predefined functions parameters.
     */
    static constexpr auto kSize = std::size(spos::lab1::demo::op_group_traits<N>::cases);

};

/**
 * @brief Logical "or" operation.
 */
struct Or final: BasicOperation<bool, spos::lab1::demo::op_group::OR>
{
    /**
     * @brief Result of short circuit.
     */
    static constexpr value_type kShortCircuitResult = true;

    /**
     * @brief Convert operand to string.
     */
    [[nodiscard]]
    static constexpr auto serialize(const value_type value) noexcept -> std::string_view
    {
        return detail::to_string(value);
    }

    /**
     * @brief Deserialize operand from string.
     */
    [[nodiscard]]
    static constexpr auto deserialize(const std::string_view str) noexcept -> std::optional<value_type>
    {
        return detail::bool_from_string(str);
    }

    /**
     * @brief Compute logical 'OR' of two values.
     */
    [[nodiscard]]
    static constexpr auto compute(const value_type lhs, const value_type rhs) noexcept -> value_type
    {
        return lhs || rhs;
    }

    /**
     * @brief Check whether operation can return immediately due to @a value
     *  causing short circuit.
     */
    [[nodiscard]]
    static constexpr bool check_short_circuit(const value_type value) noexcept
    {
        return value;
    }
};

/**
 * @brief Logical "and" operation.
 */
struct And final: BasicOperation<bool, spos::lab1::demo::op_group::AND>
{
    /**
     * @brief Result of short circuit.
     */
    static constexpr value_type kShortCircuitResult = false;

    /**
     * @brief Convert operand to string.
     */
    [[nodiscard]]
    static constexpr auto serialize(const value_type value) noexcept -> std::string_view
    {
        return detail::to_string(value);
    }

    /**
     * @brief Deserialize operand from string.
     */
    [[nodiscard]]
    static constexpr auto deserialize(const std::string_view str) noexcept -> std::optional<value_type>
    {
        return detail::bool_from_string(str);
    }

    /**
     * @brief Compute logical 'AND' of two values.
     */
    [[nodiscard]]
    static constexpr auto compute(const value_type lhs, const value_type rhs) noexcept -> value_type
    {
        return lhs && rhs;
    }

    /**
     * @brief Check whether operation can return immediately due to @a value
     *  causing short circuit.
     */
    [[nodiscard]]
    static constexpr bool check_short_circuit(const value_type value) noexcept
    {
        return !value;
    }
};

/**
 * @brief Integer multiplication operation.
 */
struct Mul final: BasicOperation<int, spos::lab1::demo::op_group::INT>
{
    /**
     * @brief Result of short circuit.
     */
    static constexpr value_type kShortCircuitResult = 0;

    /**
     * @brief Convert operand to string.
     */
    [[nodiscard]]
    static auto serialize(const value_type value) -> std::string
    {
        return std::to_string(value);
    }

    /**
     * @brief Deserialize operand from string.
     */
    [[nodiscard]]
    static auto deserialize(const std::string_view str) noexcept -> std::optional<value_type>
    {
        value_type result{0};
        if (std::from_chars(str.begin(), str.end(), result).ec == std::errc{}) {
            return result;
        }
    
        return {};
    }

    /**
     * @brief Compute multiplication of two values.
     */
    [[nodiscard]]
    static constexpr auto compute(const value_type lhs, const value_type rhs) noexcept -> value_type
    {
        return lhs * rhs;
    }

    /**
     * @brief Check whether operation can return immediately due to @a value
     *  causing short circuit.
     */
    [[nodiscard]]
    static constexpr bool check_short_circuit(const value_type value) noexcept
    {
        return value == 0;
    }
};

/**
 * @brief Type of possible operation.
 */
using Operation = std::variant<
    Or,
    And,
    Mul
>;

/**
 * @brief Convert string identifier to actual operation.
 * @note Case sensitive.
 */
[[nodiscard]]
constexpr auto from_string(const std::string_view str) noexcept -> std::optional<Operation>
{
    if (str == "OR") {
        return Or{};
    }

    if (str == "AND") {
        return And{};
    }

    if (str == "MUL") {
        return Mul{};
    }

    return {};
}

} // namespace lab1
