//
// Copyright (c) 2021-2022 Guilherme Schvarcz Franco (guilhermefrancosi at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, synchronous, uploading a file.
//
//------------------------------------------------------------------------------

//[example_http_client

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem/path.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

// Performs an HTTP POST with file upload and prints the response
int main(int argc, char** argv)
{
    try
    {
        // Check command line arguments.
        if(argc != 4 && argc != 5)
        {
            std::cerr <<
                "Usage: http-client-sync-file-upload <host> <port> <target> <file> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                "Example:\n" <<
                "    http-client-sync-file-upload www.example.com 80 /\n" <<
                "    http-client-sync-file-upload www.example.com 80 / 1.0\n";
            return EXIT_FAILURE;
        }
        auto const host = argv[1];
        auto const port = argv[2];
        auto const target = argv[3];
        boost::filesystem::path const fileName = argv[4];
        int version = argc == 6 && !std::strcmp("1.0", argv[5]) ? 10 : 11;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        // Set up an HTTP POST request message with file upload
        http::request<http::string_body> req{http::verb::post, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Prepare the multipart/form-data message
        std::ifstream f(
            fileName.native(), std::ios::binary); // maybe binary file
        std::string const file_buffer(
            std::istreambuf_iterator<char>(f), {});

#define MULTI_PART_BOUNDARY                                          \
    "AaB03x" // This is the boundary to limit the start/end of a
             // part. It may be any string. More info on the RFC
             // 2388 (https://datatracker.ietf.org/doc/html/rfc2388)
#define CRLF                                                         \
    "\r\n" // Line ends must be CRLF
           // https://datatracker.ietf.org/doc/html/rfc7231#section-3.1.1.4

        std::ostringstream payload;
        payload
            << "--" MULTI_PART_BOUNDARY CRLF
            << R"(Content-Disposition: form-data; name="comment")"
            << CRLF CRLF "Larry" CRLF << //
            "--" MULTI_PART_BOUNDARY CRLF
            << R"(Content-Disposition: form-data; name="files"; filename=)"
            << fileName.filename() << CRLF
            << "Content-Type: application/octet-stream" CRLF CRLF
            << file_buffer << CRLF << //
            "--" MULTI_PART_BOUNDARY << "--" CRLF;

        req.set(
            http::field::content_type,
            "multipart/form-data; boundary=" MULTI_PART_BOUNDARY);

#undef CRLF
#undef MULTI_PART_BOUNDARY

        req.body() = std::move(payload).str();
        req.prepare_payload();

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Write the message to standard out
        std::cout << res << std::endl;

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//]
