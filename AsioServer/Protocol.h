#pragma once
#include <string>
#include <vector>
#include <sstream>

struct ServerSocket;
struct BaseData;

static size_t parser(const std::string str, std::vector<std::string>& tokens) {
	std::stringstream ss(str);
	std::string token;
	size_t size{ 0 };

	if (str.empty()) {
		return 0;
	}

	while (ss >> token) {
		tokens.push_back(token);
		++size;
	}

	return size;
}

static void set(ServerSocket* socket, BaseData& db, const std::string& key, const std::string& value) {
	db.write(socket->login + ":" + key, value);
}

static std::string get(ServerSocket* socket, BaseData& db, const std::string& key) {
	return db.read(socket->login + ":" + key);
}

static void del(ServerSocket* socket, BaseData& db, const std::string& key) {
	db.del(socket->login + ":" + key);
}

static void login(ServerSocket* socket, const std::string& login) {
	socket->login = login;
}