#pragma once

#include <string_view>
#include <variant>
#include <utility>

namespace qf {

enum class ErrorCode {
    none = 0,
    invalid_argument,
    out_of_range,
    empty_input,
    singular,
    non_finite_input,
    numerical_failure,
    not_implemented,
};

class Status final {
public:
    constexpr Status() noexcept = default;
    constexpr Status(ErrorCode code, std::string_view message) noexcept
        : code_(code), message_(message) {}

    [[nodiscard]] constexpr bool ok() const noexcept { return code_ == ErrorCode::none; }
    [[nodiscard]] constexpr ErrorCode code() const noexcept { return code_; }
    [[nodiscard]] constexpr std::string_view message() const noexcept { return message_; }

    constexpr explicit operator bool() const noexcept { return ok(); }

private:
    ErrorCode code_ = ErrorCode::none;
    std::string_view message_{};
};

template <typename T>
class Result final {
public:
    Result(const T& value) : state_(value) {}
    Result(T&& value) : state_(std::move(value)) {}
    Result(Status error) : state_(error) {}

    [[nodiscard]] bool has_value() const noexcept { return std::holds_alternative<T>(state_); }
    [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }

    [[nodiscard]] const T& value() const & { return std::get<T>(state_); }
    [[nodiscard]] T& value() & { return std::get<T>(state_); }
    [[nodiscard]] T&& value() && { return std::get<T>(std::move(state_)); }

    [[nodiscard]] Status error() const noexcept {
        return has_value() ? Status{} : std::get<Status>(state_);
    }

private:
    std::variant<T, Status> state_;
};

[[nodiscard]] constexpr Status success() noexcept { return {}; }

} // namespace qf
