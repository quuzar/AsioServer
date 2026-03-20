#include <iostream>
#include <array>
#include <string>
#include <boost/asio.hpp>
#include <rocksdb/db.h>
#include <rocksdb/options.h>

struct ServerSocket;
void start_sesion(ServerSocket* socket);
void initial_sesion(ServerSocket* socket);
void read_socket(ServerSocket* socket);

struct ServerSocket {
    boost::asio::ip::tcp::socket sk;
    boost::asio::ip::tcp::acceptor* accp;
    boost::asio::io_context* io;

    ServerSocket() = delete;
    ServerSocket(boost::asio::ip::tcp::acceptor& accptor, boost::asio::io_context& ioc)
        : accp{ &accptor }, io{ &ioc }, sk{ ioc } {
    };

    inline boost::system::error_code accept() {
        boost::system::error_code ec;
        accp->accept(sk, ec);
        return ec;
    }

    inline size_t read(char* buffer, boost::system::error_code& ec) {
        return sk.read_some(boost::asio::buffer(buffer, 1024), ec);
    }

    inline size_t write(std::string str, boost::system::error_code& ec) {
        return boost::asio::write(sk, boost::asio::buffer(str), ec);
    }

    inline boost::system::error_code resete() {
        boost::system::error_code ec;
        sk.close(ec);
        sk = boost::asio::ip::tcp::socket(*io);
        return ec;
    }

    ~ServerSocket(){
        boost::system::error_code ec;
        sk.close(ec);
    }
};

void initial_sesion(ServerSocket* socket) {
    if (auto ec = socket->accept()) {
        std::cout << "error\n";
        std::cout << ec.message() << "\n";
        socket->resete();
    }
}

void read_socket(ServerSocket* socket) {
    std::array<char, 1024> buffer{};
    boost::system::error_code ec{};

    size_t size = socket->read(buffer.data(), ec);
    if (!ec) {
        std::cout << ">>> " << std::string(buffer.data(), size) << "\n";
        boost::asio::post(*socket->io, [socket]() { read_socket(socket); });
        return;
    }
    else if (ec == boost::asio::error::eof) {
        std::cout << ">> client disconnected\n";
    }
    else {
        std::cout << ">> critical error: " << ec.message() << "\n>> disconnected\n";
    }
    socket->resete();
    boost::asio::post(*socket->io, [socket]() { start_sesion(socket); });
    return;
}

void start_sesion(ServerSocket* socket) {
    initial_sesion(socket);
    boost::asio::post(*socket->io, [socket]() { read_socket(socket); });
}

int main() {
    boost::asio::io_context io;

    boost::asio::ip::tcp::endpoint server_ep{
        boost::asio::ip::tcp::v6(),
        8080
    };

    boost::asio::ip::tcp::acceptor acceptors(io, server_ep);
    ServerSocket* socket = new ServerSocket(acceptors, io);

    boost::asio::post(io, [socket]() { start_sesion(socket); });

    io.run();

    return 0;
}