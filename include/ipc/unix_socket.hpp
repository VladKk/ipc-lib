#pragma once

#include <sys/socket.h>

#include <memory>

#include "addr.hpp"
#include "fd.hpp"
#include "transport.hpp"

namespace ipc {

class Listener {
public:
    Listener() = default;
    ~Listener()
    {
        close();
    }

    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;

    Listener(Listener&& other) noexcept
        : fd_(std::move(other.fd_)), path_(std::move(other.path_)), is_server_(other.is_server_)
    {}
    Listener& operator=(Listener&& other) noexcept
    {
        if (this != &other) {
            close();
            fd_ = std::move(other.fd_);
            path_ = std::move(other.path_);
            is_server_ = other.is_server_;
        }

        return *this;
    }

    std::expected<void, std::error_code> setupServer(const std::string_view path,
                                                     const int backlog = 5)
    {
        path_ = path;
        is_server_ = true;

        auto addr = createSocket(std::span<const std::byte, extra::max_path_len>(
                                     std::as_bytes(std::span(path.data(), path.size()))))
                        .transform([](const extra::unix_addr& a) { return a; })
                        .transform_error([](const std::error_code e) { return e.message(); });
        if (!addr) {
            return std::unexpected(makeErrorCode(IPC_ERROR::SOCKET_ERROR));
        }

        if (bind(fd_.get(), reinterpret_cast<sockaddr*>(&addr->addr), addr->len) < 0) { // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        if (listen(fd_.get(), backlog) < 0) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return {};
    }

    std::expected<void, std::error_code> setupClient(const std::string_view path)
    {
        auto addr = createSocket(std::span<const std::byte, extra::max_path_len>(
                                     std::as_bytes(std::span(path.data(), path.size()))))
                        .transform([](const extra::unix_addr& a) { return a; })
                        .transform_error([](const std::error_code e) { return e.message(); });
        if (!addr) {
            return std::unexpected(makeErrorCode(IPC_ERROR::SOCKET_ERROR));
        }

        if (connect(fd_.get(), reinterpret_cast<sockaddr*>(&addr->addr), addr->len) < 0) { // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return {};
    }

    [[nodiscard]] std::expected<Listener, std::error_code> accept() const
    {
        if (!fd_.isValid() || !is_server_) {
            return std::unexpected(makeErrorCode(IPC_ERROR::ACCEPT_ERROR));
        }

        extra::FileDescriptor cfd;
        cfd.reset(::accept(fd_.get(), nullptr, nullptr));

        if (!cfd.isValid()) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return Listener(std::move(cfd));
    }

    [[nodiscard]] const extra::FileDescriptor& getFd() const
    {
        return fd_;
    }

    void close()
    {
        if (fd_.isValid()) {
            fd_.release();
        }

        if (is_server_ && !path_.empty()) {
            unlink(path_.c_str());
            path_.clear();
        }
    }

private:
    explicit Listener(extra::FileDescriptor fd) : fd_(std::move(fd)) {}

    std::expected<extra::unix_addr, std::error_code>
    createSocket(const std::span<const std::byte, extra::max_path_len> path)
    {
        fd_.reset(socket(AF_UNIX, SOCK_STREAM, 0));
        if (!fd_.isValid()) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }


        unlink(reinterpret_cast<const char*>(path.data())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

        return extra::buildAddr(path);
    }

    extra::FileDescriptor fd_;
    std::string path_;
    bool is_server_{false};
};

class Connection {
public:
    Connection() : listener_(std::make_unique<Listener>()) {}

    [[nodiscard]] std::expected<size_t, std::error_code>
    send(const std::span<const std::byte> msg) const
    {
        if (!listener_->getFd().isValid()) {
            return std::unexpected(makeErrorCode(IPC_ERROR::SEND_ERROR));
        }

        ssize_t n{};
        do { // NOLINT(cppcoreguidelines-avoid-do-while)
            n = ::send(listener_->getFd().get(), msg.data(), msg.size(), MSG_NOSIGNAL);
        } while (n < 0 && errno == EINTR);

        if (n == 0) {
            return std::unexpected(makeErrorCode(IPC_ERROR::CLOSED));
        }
        if (n < 0) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return msg.size();
    }

    [[nodiscard]] std::expected<size_t, std::error_code> recv(const std::span<std::byte> msg) const
    {
        if (!listener_->getFd().isValid()) {
            return std::unexpected(makeErrorCode(IPC_ERROR::RECV_ERROR));
        }

        ssize_t n{};
        do { // NOLINT(cppcoreguidelines-avoid-do-while)
            n = ::recv(listener_->getFd().get(), msg.data(), msg.size(), 0);
        } while (n < 0 && errno == EINTR);

        if (n == 0) {
            return std::unexpected(makeErrorCode(IPC_ERROR::CLOSED));
        }
        if (n < 0) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }

        return msg.size();
    }

private:
    std::unique_ptr<Listener> listener_;
};

static_assert(ByteTransport<Connection>);

} // namespace ipc
