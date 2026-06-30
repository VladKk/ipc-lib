#pragma once

#include <sys/socket.h>

#include <utility>

#include "addr.hpp"
#include "fd.hpp"
#include "transport.hpp"

namespace ipc {

constexpr int default_backlog = 5;

inline std::expected<extra::FileDescriptor, std::error_code> makeSocket()
{
    extra::FileDescriptor fd;
    fd.reset(socket(AF_UNIX, SOCK_STREAM, 0));
    if (!fd.isValid()) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    return fd;
}

class Connection {
public:
    explicit Connection(extra::FileDescriptor fd) : fd_(std::move(fd)) {}

    [[nodiscard]] std::expected<std::size_t, std::error_code>
    send(std::span<const std::byte> msg) const
    {
        if (!fd_.isValid()) {
            return std::unexpected(makeErrorCode(IPC_ERROR::INVALID_FD));
        }

        ssize_t n{};
        do { // NOLINT(cppcoreguidelines-avoid-do-while)
            n = ::send(fd_.get(), msg.data(), msg.size(), MSG_NOSIGNAL);
        } while (n < 0 && errno == EINTR);

        if (n == 0) {
            return std::unexpected(makeErrorCode(IPC_ERROR::CLOSED));
        }
        if (n < 0) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return static_cast<std::size_t>(n);
    }

    [[nodiscard]] std::expected<std::size_t, std::error_code> recv(std::span<std::byte> msg) const
    {
        if (!fd_.isValid()) {
            return std::unexpected(makeErrorCode(IPC_ERROR::INVALID_FD));
        }

        ssize_t n{};
        do { // NOLINT(cppcoreguidelines-avoid-do-while)
            n = ::recv(fd_.get(), msg.data(), msg.size(), 0);
        } while (n < 0 && errno == EINTR);

        if (n == 0) {
            return std::unexpected(makeErrorCode(IPC_ERROR::CLOSED));
        }
        if (n < 0) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return static_cast<std::size_t>(n);
    }

private:
    extra::FileDescriptor fd_;
};

class UDSListener {
public:
    UDSListener(extra::FileDescriptor fd, std::string path)
        : path_(std::move(path)), fd_(std::move(fd))
    {}
    ~UDSListener()
    {
        close();
    }

    UDSListener(const UDSListener&) = delete;
    UDSListener& operator=(const UDSListener&) = delete;

    UDSListener(UDSListener&& other) noexcept = default;
    UDSListener& operator=(UDSListener&& other) noexcept = default;

    static std::expected<UDSListener, std::error_code> bind(std::string_view path,
                                                            int backlog = default_backlog)
    {
        if (path.empty()) {
            return std::unexpected(makeErrorCode(IPC_ERROR::PATH_EMPTY));
        }

        if (path.size() >= sizeof(sockaddr_un::sun_path)) {
            return std::unexpected(makeErrorCode(IPC_ERROR::PATH_TOO_LONG));
        }

        auto fd = makeSocket();
        if (!fd) {
            return std::unexpected(fd.error());
        }

        unlink(std::string(path).c_str());

        auto [addr, len] = extra::buildAddr(std::as_bytes(std::span(path.data(), path.size())));

        if (::bind(
                fd.value().get(),
                reinterpret_cast<sockaddr*>( // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                    &addr),
                len) < 0) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        if (listen(fd.value().get(), backlog) < 0) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return UDSListener{std::move(fd).value(), std::string(path)};
    }

    [[nodiscard]] std::expected<Connection, std::error_code> accept() const
    {
        if (!fd_.isValid()) {
            return std::unexpected(makeErrorCode(IPC_ERROR::INVALID_FD));
        }

        extra::FileDescriptor cfd;
        do { // NOLINT(cppcoreguidelines-avoid-do-while)
            cfd.reset(::accept(fd_.get(), nullptr, nullptr));
        } while (cfd.get() == -1 && errno == EINTR);

        if (!cfd.isValid()) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return Connection{std::move(cfd)};
    }

    void close()
    {
        if (fd_.isValid()) {
            fd_.reset();

            if (!path_.empty()) {
                unlink(path_.c_str());
                path_.clear();
            }
        }
    }

private:
    std::string path_;
    extra::FileDescriptor fd_;
};

inline std::expected<Connection, std::error_code> connect(std::string_view path)
{
    if (path.empty()) {
        return std::unexpected(makeErrorCode(IPC_ERROR::PATH_EMPTY));
    }

    if (path.size() >= sizeof(sockaddr_un::sun_path)) {
        return std::unexpected(makeErrorCode(IPC_ERROR::PATH_TOO_LONG));
    }

    auto fd = makeSocket();
    if (!fd) {
        return std::unexpected(fd.error());
    }

    auto [addr, len] = extra::buildAddr(std::as_bytes(std::span(path.data(), path.size())));

    if (::connect(
            fd.value().get(),
            reinterpret_cast<sockaddr*>( // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                &addr),
            len) < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    return Connection{std::move(fd).value()};
}

static_assert(ByteTransport<Connection>);

} // namespace ipc
