#pragma once

#include <concepts>
#include <expected>
#include <span>

#include "error.hpp"

namespace ipc {

template <class T>
concept ByteTransport = requires(T t, std::span<std::byte> in, std::span<const std::byte> out) {
    { t.send(out) } -> std::same_as<std::expected<std::size_t, std::error_code>>;
    { t.recv(in) } -> std::same_as<std::expected<std::size_t, std::error_code>>;
};

} // namespace ipc
