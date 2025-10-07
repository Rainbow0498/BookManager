#pragma once
#include <string>
#include <vector>
#include <bsoncxx/types.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;

struct UserBook {
    bsoncxx::oid book_id;  // 对应 novels 集合的 _id
    int progress;                   // 当前看到的章节索引

    bsoncxx::document::value toBson() const {
        return make_document(
            kvp("book_id", book_id),
            kvp("progress", progress)
            );
    }
};

class User {
public:
    std::string username;
    std::string password;
    std::vector<UserBook> bookshelf;

    User(const std::string& u, const std::string& p)
        : username(u), password(p) {}

    bsoncxx::document::value toBson() const {
        bsoncxx::builder::basic::array shelfArr;
        for (const auto& book : bookshelf) {
            shelfArr.append(book.toBson());
        }
        return make_document(
            kvp("username", username),
            kvp("password", password),
            kvp("bookshelf", shelfArr.extract())
            );
    }
};
