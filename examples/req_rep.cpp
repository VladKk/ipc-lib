#include <ipc/unix_socket.hpp>
#include <print>

int main(int argc, char* argv[])
{
    const auto args = std::span(argv, argc).subspan(1);

    if (args.size() < 2) {
        std::println("Pass argument to proceed (--server or --client)");
        return EXIT_FAILURE;
    }

    const std::string path = "/tmp/ipc_sock_demo.sock";

    if (std::string_view(args.back()) == "--server") {
        auto listener = ipc::UDSListener::bind(path);
        if (!listener) {
            const std::error_code ec = listener.error();
            std::println(R"(Listener setup error: value={} category="{}" message="{}")",
                         ec.value(), ec.category().name(), ec.message());
        }

        auto conn = listener->accept();
        if (!conn) {
            const std::error_code ec = conn.error();
            std::println(R"(Connection setup error: value={} category="{}" message="{}")",
                         ec.value(), ec.category().name(), ec.message());
        }

        conn->send(std::as_bytes(std::span("Hello world!")))
                .transform([](std::size_t n) { std::println("Sent {} bytes!", n); })
                .or_else([](std::error_code e) -> std::expected<void, std::error_code> {
                    std::println(R"(Send error: value={} category="{}" message="{}")",
                                        e.value(), e.category().name(), e.message());
                    return std::unexpected(e);
                });
    } else if (std::string_view(args.back()) == "--client") {
        std::span<std::byte> msg;
        auto conn = ipc::connect(path);
        if (!conn) {
            const std::error_code ec = conn.error();
            std::println(R"(Connection setup error: value={} category="{}" message="{}")",
                         ec.value(), ec.category().name(), ec.message());
        }

        conn->recv(msg)
                .transform([](std::size_t n) { std::println("Received {} bytes!", n); })
                .or_else([](std::error_code e) -> std::expected<void, std::error_code> {
                    std::println(R"(Recv error: value={} category="{}" message="{}")",
                                        e.value(), e.category().name(), e.message());
                    return std::unexpected(e);
                });

        std::println("{}", std::string_view(reinterpret_cast<const char*>(msg.data()), msg.size()));
    }

    return EXIT_SUCCESS;
}
