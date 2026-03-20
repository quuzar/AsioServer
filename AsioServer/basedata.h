#pragma once
#include <rocksdb/db.h>
#include <rocksdb/options.h>

#ifdef _WIN32
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "rpcrt4.lib")
#endif

struct BaseData {
	rocksdb::DB* db;
	rocksdb::Options options;

	BaseData() {
		options.create_if_missing = true;  
		rocksdb::Status s = rocksdb::DB::Open(options, "./data", &db);
		if (!s.ok())
			throw std::runtime_error(s.ToString());
	}

	void write(const std::string& key, const std::string& value) {
		db->Put(rocksdb::WriteOptions(), key, value);
	}

	std::string read(const std::string& key) {
		std::string value;
		rocksdb::Status s = db->Get(rocksdb::ReadOptions(), key, &value);
		if (s.ok()) {
			return value;
		}
		return "";
	}

	void del(const std::string& key) {
		db->Delete(rocksdb::WriteOptions(), key);
	}

	~BaseData() {
		delete db;
	}
};