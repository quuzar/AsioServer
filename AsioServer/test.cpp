#include <iostream>
#include <boost/asio.hpp>
#include "AsioServer.h"
#include "basedata.h"

int main() {
    BaseData db;  

    boost::asio::io_context io;
    boost::asio::ip::tcp::acceptor acceptor(io,
        { boost::asio::ip::tcp::v6(), 8080 });


    ServerSocket* socket = new ServerSocket(acceptor, io, &db);  

    boost::asio::post(io, [socket]() { start_sesion(socket); });

    io.run();

    delete socket;
}