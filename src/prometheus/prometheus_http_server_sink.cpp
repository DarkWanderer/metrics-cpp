#include <metrics/prometheus.h>
#include <metrics/sink.h>

//#pragma warning(push, 1)
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
//#pragma warning(pop)

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>

using namespace std;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using boost::asio::ip::tcp;

namespace Metrics {
    namespace Prometheus {
        struct Context {
            shared_ptr<IRegistry> registry;
        };

        class HttpConnection : public std::enable_shared_from_this<HttpConnection>
        {
        public:
            HttpConnection(tcp::socket socket, Context context)
                : context_(context), socket_(std::move(socket))
            {
            }

            // Initiate the asynchronous operations associated with the connection.
            void start()
            {
                read_request();
                check_deadline();
            }

        private:
            // Application context
            Context context_;

            // The socket for the currently connected client.
            tcp::socket socket_;

            // The buffer for performing reads.
            beast::flat_buffer buffer_{ 8192 };

            // The request message.
            http::request<http::dynamic_body> request_;

            // The response message.
            http::response<http::dynamic_body> response_;

            // The timer for putting a deadline on connection processing.
            net::steady_timer deadline_{
                socket_.get_executor(), std::chrono::seconds(60) };

            // Asynchronously receive a complete request message.
            void read_request()
            {
                auto self = shared_from_this();

                http::async_read(
                    socket_,
                    buffer_,
                    request_,
                    [self](beast::error_code ec,
                        std::size_t bytes_transferred)
                {
                    boost::ignore_unused(bytes_transferred);
                    if (!ec)
                        self->process_request();
                });
            }

            // Determine what needs to be done with the request message.
            void process_request()
            {
                response_.version(request_.version());
                response_.keep_alive(false);

                switch (request_.method()) {
                case http::verb::get:
                    response_.result(http::status::ok);
                    response_.set(http::field::server, "Boost::Beast");
                    create_response();
                    break;

                default:
                    // We return responses indicating an error if
                    // we do not recognize the request method.
                    response_.result(http::status::method_not_allowed);
                    response_.set(http::field::content_type, "text/plain");
                    beast::ostream(response_.body())
                        << "Invalid request-method '"
                        << std::string(request_.method_string())
                        << "'";
                    break;
                }

                write_response();
            }

            // Construct a response message based on the program state.
            void create_response()
            {
                if (request_.target() == "/metrics") {
                    response_.set(http::field::content_type, "text/plain");
                    beast::ostream(response_.body()) << serialize(context_.registry);
                }
                else {
                    response_.result(http::status::not_found);
                    response_.set(http::field::content_type, "text/plain");
                    beast::ostream(response_.body()) << "File not found\r\n";
                }
            }

            // Asynchronously transmit the response message.
            void write_response()
            {
                auto self = shared_from_this();

                response_.content_length(response_.body().size());

                http::async_write(socket_, response_, [self](beast::error_code ec, std::size_t)
                {
                    self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                    self->deadline_.cancel();
                });
            }

            // Check whether we have spent enough time on this connection.
            void check_deadline()
            {
                auto self = shared_from_this();

                deadline_.async_wait([self](beast::error_code ec)
                {
                    if (!ec) {
                        // Close socket to cancel any outstanding operation.
                        self->socket_.close(ec);
                    }
                });
            }
        };

        // "Loop" forever accepting new connections.
        static void accept(tcp::acceptor& acceptor, tcp::socket& socket, Context& context)
        {
            acceptor.async_accept(socket, [&](beast::error_code ec) {
                if (!ec)
                    std::make_shared<HttpConnection>(std::move(socket), context)->start();
                accept(acceptor, socket, context);
            });
        }

        class PrometheusHttpServerSink : public IRegistrySink {
        private:
            shared_ptr<IRegistry> m_registry;
            const string m_address;
            const string m_port;
            net::io_context m_io_context;
            thread m_thread;

            void run() {
                // TODO: resolve in constructor to make sure this doesn't throw exception in background
                tcp::resolver resolver(m_io_context);
                tcp::resolver::query query(m_address, m_port);
                auto endpoint_iterator = resolver.resolve(query);

                tcp::acceptor acceptor(m_io_context, *endpoint_iterator.begin());
                tcp::socket socket{ m_io_context };

                Context context{ m_registry };

                accept(acceptor, socket, context);
                m_io_context.run();
            }

        public:
            PrometheusHttpServerSink(shared_ptr<IRegistry> registry, const string& address, const string& port) :
                m_registry(registry),
                m_address(address),
                m_port(port),
                m_io_context(1)
            {
                m_thread = thread(&PrometheusHttpServerSink::run, this);
            };

            virtual ~PrometheusHttpServerSink() {
                m_io_context.stop();
                m_thread.join();
            }

            virtual shared_ptr<IRegistry> registry() const override {
                return m_registry;
            };
        };

        shared_ptr<IRegistrySink> createPrometheusHttpServerSink(shared_ptr<IRegistry> registry, const string& address, const string& port)
        {
            if (!registry)
                registry = createRegistry();

            return make_shared<PrometheusHttpServerSink>(registry, address, port);
        }
    }
}
