#ifndef TRANSPORT_HPP
#define TRANSPORT_HPP

#include <expected>
#include <functional>
#include <span>

#include "error.hpp"

template <class T>
concept ByteTransport = requires(T t, std::span<std::byte> in, std::span<const std::byte> out)
{
    { t.send(out) } -> std::same_as<std::expected<size_t, IpcError>>;
    { t.recv(in) } -> std::same_as<std::expected<size_t, IpcError>>;
};

#endif // TRANSPORT_HPP
