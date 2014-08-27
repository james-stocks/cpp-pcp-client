#include "test_client.h"

#include <iostream>

// NB: (from websocketpp wiki) "Per the websocket spec, a client can
// only have one connection in the connecting state at a time."

namespace Cthun {
namespace Client {

TestClient::TestClient(std::vector<Message> & messages)
    : BaseClient(),
      messages_ { messages } {
}

TestClient::~TestClient() {
    BaseClient::shutdown();
}

//
// Event loop callbacks
//

void TestClient::onOpen_(websocketpp::connection_hdl hdl) {
    std::cout << "### Triggered onOpen_()\n";

    // TODO(ale): use client.get_alog().write() as in telemetry_client

    for (auto msg : messages_) {
        // Send a message once the connection is open
        websocketpp::lib::error_code ec;

        // NB: as for WebSocket API, send() will give an error in case
        // the connection state in not OPEN (or CLOSING).

        // NB: send() will put msg on a queue... The only call that
        // provides flow control info is connection::get_buffered_amount()
        // (i.e., it could be used to throttle the message tx rate).

        client_.send(hdl, msg, websocketpp::frame::opcode::text, ec);
        if (ec) {
            throw message_error { "Failed to send message: " + ec.message() };
        }
        std::cout << "### Message sent (ASYNCHRONOUS - onOpen_) " << msg << "\n";
    }
}

void TestClient::onClose_(websocketpp::connection_hdl hdl) {
    std::cout << "### Triggered onClose_()\n";
}

void TestClient::onFail_(websocketpp::connection_hdl hdl) {
    std::cout << "### Triggered onFail_()\n";
}

Context_Ptr TestClient::onTlsInit_(websocketpp::connection_hdl hdl) {
    Context_Ptr ctx {
        new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1) };

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cout << "### ERROR (tls): " << e.what() << std::endl;
    }
    return ctx;
}

void TestClient::onMessage_(websocketpp::connection_hdl hdl,
                            Client_Configuration::message_ptr msg) {
        std::cout << "### Got a MESSAGE!!! " << msg->get_payload() << std::endl;
}

}  // namespace Client
}  // namespace Cthun