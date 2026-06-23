#pragma once

#include <sys/socket.h>
#include <unistd.h>

#include <expected>
#include <system_error>
#include <utility>

namespace ipc::extra {

class FileDescriptor {
public:
    explicit FileDescriptor(int fd = -1) noexcept : fd_(fd) {}
    ~FileDescriptor() noexcept
    {
        reset();
    }

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.release()) {}
    FileDescriptor& operator=(FileDescriptor&& other) noexcept
    {
        if (this != &other) {
            reset(other.release());
        }

        return *this;
    }

    [[nodiscard]] int get() const noexcept
    {
        return fd_;
    }
    [[nodiscard]] bool isValid() const noexcept
    {
        return fd_ >= 0;
    }

    int release() noexcept
    {
        return std::exchange(fd_, -1);
    }
    void reset(const int new_fd = -1) noexcept
    {
        if (isValid()) {
            close(fd_);
        }
        fd_ = new_fd;
    }

private:
    int fd_ = -1;
};

inline std::expected<FileDescriptor, std::error_code> make_socket()
{
    FileDescriptor fd;
    fd.reset(socket(AF_UNIX, SOCK_STREAM, 0));
    if (!fd.isValid()) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    return fd;
}

} // namespace ipc::extra
