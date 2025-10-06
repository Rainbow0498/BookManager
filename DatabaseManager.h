#pragma once
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <QString>
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;

class DatabaseManager {
public:
    static DatabaseManager& instance() {
        static DatabaseManager instance;
        return instance;
    }

    mongocxx::database db() { return client_["noveldb"]; }

    // 禁止拷贝与赋值
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    mongocxx::instance inst_;
    mongocxx::client client_;

    DatabaseManager() : client_(mongocxx::uri{}) {
        qDebug("✅ MongoDB 已连接：noveldb");
    }
};
