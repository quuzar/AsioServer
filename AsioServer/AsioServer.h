#pragma once
#include <iostream>
#include <array>
#include <string>
#include <boost/asio.hpp>
#include <vector>
#include "basedata.h"
#include "Logger.h"

struct BaseData;

struct ServerSocket {
    boost::asio::ip::tcp::socket sk;
    boost::asio::ip::tcp::acceptor* accp;
    boost::asio::io_context* io;
    std::string login;
    BaseData* db;

    ServerSocket() = delete;
    ServerSocket(boost::asio::ip::tcp::acceptor& accptor,
        boost::asio::io_context& ioc,
        BaseData* database) 
        : accp{ &accptor }, io{ &ioc }, sk{ ioc }, db{ database } {
    };

    boost::system::error_code accept() {
        boost::system::error_code ec;
        accp->accept(sk, ec);
        return ec;
    }

    size_t read(char* buffer, boost::system::error_code& ec) {
        return sk.read_some(boost::asio::buffer(buffer, 1024), ec);
    }

    size_t write(std::string str, boost::system::error_code& ec) {
        return boost::asio::write(sk, boost::asio::buffer(str), ec);
    }

    boost::system::error_code resete() {
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

#include "Protocol.h"

static void initial_sesion(ServerSocket* socket);
static void read_socket(ServerSocket* socket);
static void start_sesion(ServerSocket* socket);

static void initial_sesion(ServerSocket* socket) {
    Logger::log("Waiting for connection...");
    if (auto ec = socket->accept()) {
        Logger::log("Accept error: " + ec.message());
        socket->resete();
    }
    else {
        Logger::log("Client connected");
    }
}

static void read_socket(ServerSocket* socket) {
    std::array<char, 1024> buffer{};
    boost::system::error_code ec{};
    std::vector<std::string> tokens{};
    size_t size = socket->read(buffer.data(), ec);
    if (!ec) {
        size = parser(std::string(buffer.data(), size), tokens);
        if (size == 0) {
            boost::asio::post(*socket->io, [socket]() { read_socket(socket); });
            return;
        }

        if (tokens[0] == "LOGIN" && size >= 2) {
            Logger::log("LOGIN: " + tokens[1]);
            login(socket, tokens[1]);
            socket->write("OK\n", ec);
        }
        else if (tokens[0] == "SET" && size >= 3) {
            Logger::log("SET: " + tokens[1] + " = " + tokens[2]);
            set(socket, *socket->db, tokens[1], tokens[2]);
            socket->write("OK\n", ec);
        }
        else if (tokens[0] == "GET" && size >= 2) {
            Logger::log("GET: " + tokens[1]);
            std::string value = get(socket, *socket->db, tokens[1]);
            socket->write((value + "\n"), ec);
        }
        else if (tokens[0] == "DEL" && size >= 2) {
            Logger::log("DEL: " + tokens[1]);
            del(socket, *socket->db, tokens[1]);
            socket->write("OK\n", ec);
        }
        else {
            socket->write("ERROR\n", ec);
        }


        boost::asio::post(*socket->io, [socket]() { read_socket(socket); });
        return;
    }
    else if (ec == boost::asio::error::eof) {
        Logger::log("Client disconnected: " + socket->login);
    }
    else {
        Logger::log("Error: " + ec.message());
    }
    socket->resete();
    boost::asio::post(*socket->io, [socket]() { start_sesion(socket); });
    return;
}

static void start_sesion(ServerSocket* socket) {
    initial_sesion(socket);
    boost::asio::post(*socket->io, [socket]() { read_socket(socket); });
}