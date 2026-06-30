#pragma once

#include <sys/socket.h>
#include <sys/un.h>

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <span>

namespace ipc::extra {

struct unix_addr {
    sockaddr_un addr;
    socklen_t len;
};

inline unix_addr buildAddr(std::span<const std::byte> path)
{
    if (path.empty()) {
        return {};
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    const std::span span_path(addr.sun_path);
    socklen_t len{
        offsetof(sockaddr_un, sun_path)}; // Specify immediately for unnamed addresses (socketpair)

    if (path.front() == std::byte{0}) { // ABSTRACT
        auto safe_view = path | std::views::take(std::min(path.size(), sizeof(addr.sun_path)));
        auto [in, out] = std::ranges::copy(
            safe_view | std::views::transform([](std::byte b) { return static_cast<char>(b); }),
            span_path.data());

        len += std::distance(span_path.data(), out);
    } else { // FILESYSTEM
        auto safe_view = path | std::views::take(std::min(path.size(), sizeof(addr.sun_path) - 1));
        auto [in, out] = std::ranges::copy(
            safe_view | std::views::transform([](std::byte b) { return static_cast<char>(b); }),
            span_path.data());

        span_path[std::distance(span_path.data(), out)] = '\0';
        len += std::distance(span_path.data(), out) + 1;
    }

    return {.addr = addr, .len = len};
}

} // namespace ipc::extra
