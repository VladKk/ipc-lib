#ifndef UNIX_SOCKET_HPP
#define UNIX_SOCKET_HPP

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>

#include "transport.hpp"

struct FDHandler {
    explicit FDHandler(const int fd = -1) noexcept : fd(fd) {}
    ~FDHandler() noexcept { reset(); }

    // Disable copy
    FDHandler(const FDHandler&) = delete;
    FDHandler& operator=(const FDHandler&) = delete;

    FDHandler(FDHandler&& other) noexcept : fd(other.release()) {}
    FDHandler& operator=(FDHandler&& other) noexcept
    {
        if (this != &other) {
            reset(other.release());
        }

        return *this;
    }

    [[nodiscard]] bool isValid() const noexcept { return fd >= 0; }

    explicit operator int() const noexcept { return fd; }

    int release() noexcept { return std::exchange(fd, -1); }
    void reset(const int new_fd = -1) noexcept
    {
        if (fd >= 0) {
            close(fd);
        }
        fd = new_fd;
    }

    int fd = -1;
};

class UnixSocketTransport {
public:
    UnixSocketTransport() = default;

    ~UnixSocketTransport()
    {
        close();
    }

    UnixSocketTransport(const UnixSocketTransport&) = delete;
    UnixSocketTransport& operator=(const UnixSocketTransport&) = delete;

    UnixSocketTransport(UnixSocketTransport&& other) noexcept
        : fd_(std::move(other.fd_)),
          path_(std::move(other.path_)),
          is_server_(other.is_server_) {}

    UnixSocketTransport& operator=(UnixSocketTransport&& other) noexcept
    {
        if (this != &other) {
            close();
            fd_ = std::move(other.fd_);
            path_ = std::move(other.path_);
            is_server_ = other.is_server_;
        }

        return *this;
    }

    auto setupServer(const std::string_view path, const int backlog = 5) -> std::expected<void, IpcError>
    {
        path_ = path;
        is_server_ = true;

        fd_.reset(socket(AF_UNIX, SOCK_STREAM, 0));
        if (!fd_.isValid()) {
            return std::unexpected(IpcError::connection_refused);
        }

        unlink(path_.c_str());

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(
            std::span(addr.sun_path).data(), std::string(path).c_str(), sizeof(addr.sun_path) - 1);

        if (const socklen_t len =
                offsetof(sockaddr_un, sun_path) + std::strlen(std::span(addr.sun_path).data()) + 1;
            bind(fd_.fd, reinterpret_cast<sockaddr*>(&addr), len) == -1) { // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            return std::unexpected(IpcError::connection_refused);
        }

        if (listen(fd_.fd, backlog) == -1) {
            return std::unexpected(IpcError::connection_refused);
        }

        return {};
    }

    auto setupClient(const std::string_view path) -> std::expected<void, IpcError>
    {
        fd_.reset(socket(AF_UNIX, SOCK_STREAM, 0));
        if (!fd_.isValid()) {
            return std::unexpected(IpcError::connection_refused);
        }

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(
            std::span(addr.sun_path).data(), std::string(path).c_str(), sizeof(addr.sun_path) - 1);

        if (const socklen_t len =
                offsetof(sockaddr_un, sun_path) + std::strlen(std::span(addr.sun_path).data()) + 1;
            connect(fd_.fd, reinterpret_cast<sockaddr*>(&addr), len) == -1) { // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                return std::unexpected(IpcError::connection_refused);
        }

        return {};
    }

    [[nodiscard]]
    auto accept() const -> std::expected<UnixSocketTransport, IpcError>
    {
        if (!fd_.isValid() || !is_server_) {
            return std::unexpected(IpcError::system_error);
        }

        FDHandler cfd;
        cfd.reset(::accept(fd_.fd, nullptr, nullptr));

        if (!cfd.isValid()) {
            return std::unexpected(IpcError::system_error);
        }

        return UnixSocketTransport(std::move(cfd));
    }

    [[nodiscard]]
    auto send(const std::span<const std::byte> msg) const -> std::expected<size_t, IpcError>
    {
        if (!fd_.isValid()) {
            return std::unexpected(IpcError::system_error);
        }

        ::send(fd_.fd, msg.data(), msg.size(), MSG_NOSIGNAL);

        return msg.size();
    }

    [[nodiscard]]
    auto recv(const std::span<std::byte> msg) const -> std::expected<size_t, IpcError>
    {
        if (!fd_.isValid()) {
            return std::unexpected(IpcError::system_error);
        }

        errno = 0;
        if (ssize_t const bytes_read = ::recv(fd_.fd, msg.data(), msg.size(), AF_IRDA);
            bytes_read <= 0) {
            switch (errno) {
                case ECONNREFUSED: return std::unexpected(IpcError::connection_refused);
                case EPIPE: case ECONNRESET: return std::unexpected(IpcError::connection_closed);
                case EAGAIN: return std::unexpected(IpcError::would_block);
                default: return std::unexpected(IpcError::system_error);
            }
        }

        return msg.size();
    }

    void close()
    {
        if (is_server_ && !path_.empty()) {
            unlink(path_.c_str());
            path_.clear();
        }
    }

private:
    explicit UnixSocketTransport(FDHandler fd) : fd_(std::move(fd)) {}

    FDHandler fd_;
    std::string path_;
    bool is_server_{false};
};

static_assert(ByteTransport<UnixSocketTransport>);

#endif // UNIX_SOCKET_HPP
