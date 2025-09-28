#include "mainwindow.h"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <QApplication>

#include "Book.h"
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;

void initDatabase(mongocxx::client& client) {
    auto db = client["noveldb"];

    // 获取所有集合名称
    auto collections = db.list_collection_names();

    bool hasNovels = false;
    bool hasUsers = false;
    for (const auto& name : collections) {
        if (name == "novels") hasNovels = true;
        if (name == "users") hasUsers = true;
    }

    // 如果没有 novels 集合，就创建并插入一条测试数据
    if (!hasNovels) {
        std::cout << "novels 集合不存在，正在创建..." << std::endl;
        auto coll = db["novels"];
        coll.insert_one(make_document(
            kvp("title" , "测试小说"),
            kvp("author", "未知"),
            kvp("intro" , "这是简介")
            )
                        );
    }

    // 如果没有 users 集合，也创建一个
    if (!hasUsers) {
        std::cout << "users 集合不存在，正在创建..." << std::endl;
        auto coll = db["users"];
        coll.insert_one(
            make_document(
                kvp("username", "admin"),
                kvp("bookshelf", make_array())   // 空数组
                )
            );
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};
    initDatabase(conn);
    auto db = conn["noveldb"];
    auto coll = db["novels"];

    std::vector<Chapter> chaps = {
        Chapter("第一章 陨落的天才", 1200, "正文内容..."),
        Chapter("第二章 修炼的起点", 1300, "正文内容...")
    };

    // 构造书籍
    Book book(
        "斗破苍穹",
        "http://xxx.com/doupo.jpg",
        "天蚕土豆",
        "连载",
        {"玄幻", "热血"},
        2500000,
        "少年萧炎，天才陨落，浴火重生……",
        chaps
        );

    coll.insert_one(book.toBson().view());


    w.show();
    return a.exec();
}



