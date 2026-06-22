#pragma once

#include <cstdint>
#include <system_error>

namespace ipc {

enum class IPC_ERROR : std::uint8_t {
    // 0 - success
    CLOSED = 1,
    SOCKET_ERROR,
    ACCEPT_ERROR,
    SEND_ERROR,
    RECV_ERROR,
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
            case IPC_ERROR::SOCKET_ERROR:
                return "could not create socket for listener";
            case IPC_ERROR::ACCEPT_ERROR:
                return "could not accept incoming connection";
            case IPC_ERROR::SEND_ERROR:
                return "could not send message";
            case IPC_ERROR::RECV_ERROR:
                return "could not receive message";
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
