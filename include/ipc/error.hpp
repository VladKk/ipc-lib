#pragma once

#include <cstdint>
#include <system_error>

namespace ipc {

enum class IPC_ERROR : std::uint8_t {
    // 0 - success
    CLOSED = 1,
    PATH_TOO_LONG,
    PATH_EMPTY,
    INVALID_FD,
    SERIALIZATION_ERROR
};

struct ipc_category : std::error_category {
    [[nodiscard]] const char* name() const noexcept override
    {
        return "ipc";
    }
    [[nodiscard]] std::string message(int ev) const override
    {
        switch (static_cast<IPC_ERROR>(ev)) {
            case IPC_ERROR::CLOSED:
                return "connection closed by peer";
            case IPC_ERROR::PATH_TOO_LONG:
                return "socket path too long (> 108 char)";
            case IPC_ERROR::PATH_EMPTY:
                return "socket path should not be empty";
            case IPC_ERROR::INVALID_FD:
                return "invalid file descriptor";
            case IPC_ERROR::SERIALIZATION_ERROR:
                return "serialization failed";
        }

        return "unknown IPC error";
    }
};

inline const std::error_category& ipcCategory() noexcept
{
    static const ipc_category c;
    return c;
}

inline std::error_code makeErrorCode(IPC_ERROR e)
{
    return {static_cast<int>(e), ipcCategory()};
}

} // namespace ipc

template <>
struct std::is_error_code_enum<ipc::IPC_ERROR> : std::true_type {};
