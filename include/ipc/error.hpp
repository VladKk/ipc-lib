#ifndef ERROR_HPP
#define ERROR_HPP

#include <cstdint>

enum class IpcError : std::uint8_t {
    connection_refused,
    connection_closed,
    would_block,
    message_too_large,
    serialization_error,
    system_error
};

#endif // ERROR_HPP
