#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>

void AsioClient() {
    boost::asio::io_context io;
    boost::asio::ip::tcp::resolver resolver(io);
    auto client_ep = resolver.resolve("127.0.0.1", "8080");

    boost::asio::ip::tcp::socket socket(io);

    std::cout << ">> Start client\n";
    std::cout << ">> Connected to server... ";

    try {
        boost::asio::connect(socket, client_ep);
    }
    catch (boost::system::error_code ec) {
        std::cout << "error\n" << ec.message() << "\n";
        socket.close();
        return;
    }

    std::cout << "success\n";
    std::string str;

    while (true) {
        std::cout << ">>> ";
        std::getline(std::cin, str);

        if (str == "exit") break;

        boost::asio::write(socket, boost::asio::buffer(str + "\n"));

        char buffer[1024];
        boost::system::error_code ec;
        size_t len = socket.read_some(boost::asio::buffer(buffer), ec);

        if (!ec) {
            std::cout << "<<< " << std::string(buffer, len);
        }
        else if (ec == boost::asio::error::eof) {
            std::cout << "Server disconnected\n";
            break;
        }
        else {
            std::cout << "Error: " << ec.message() << "\n";
            break;
        }
    }

    socket.close();
    std::cout << ">> disconnected\n";
}