#include "mainwindow.h"
#include "Login.h"
#include "DatabaseManager.h"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <QApplication>
// #include "ImportBookData.h"
#include "json.hpp"
#include <fstream>
#include <vector>
#include "Book.h"
using json = nlohmann::json;
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

void ImportData(){
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};
    auto db = conn["noveldb"];
    auto collection = db["novels"];

    std::ifstream file("fanqie_books.json");
    if (!file.is_open()) {
        std::cerr << "❌ 无法打开文件 books_data.json" << std::endl;
        return;
    }

    json data;
    file >> data;
    file.close();

    if (!data.is_array()) {
        std::cerr << "❌ JSON 格式错误：根节点不是数组" << std::endl;
        return;
    }

    int count = 0;
    for (auto& item : data) {
        try {
            // ====== 1️⃣ 解析字段 ======
            std::string title = item.value("title", "");
            std::string author = item.value("author", "");
            std::string cover = ""; // 暂无封面
            std::string status = item.value("status","");
            std::string wordCount = item.value("word_count", "");
            std::string intro = item.value("intro", "");

            // 分类数组
            std::vector<std::string> categories;
            if (item.contains("category") && item["category"].is_array()) {
                for (auto& c : item["category"]) {
                    categories.push_back(c.get<std::string>());
                }
            }

            // 章节数组
            std::vector<Chapter> chapters;
            if (item.contains("chapters") && item["chapters"].is_array()) {
                for (auto& ch : item["chapters"]) {
                    chapters.emplace_back(ch.get<std::string>());
                }
            }

            // ====== 2️⃣ 构造 Book 对象 ======
            Book book(title, cover, author,status, categories, wordCount, intro, chapters);

            // ====== 3️⃣ 转 BSON 并插入数据库 ======
            auto doc = book.toBson();
            collection.insert_one(doc.view());

            count++;
            std::cout << "✅ 已插入：" << title << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "⚠️ 插入失败：" << e.what() << std::endl;
        }
    }

    std::cout << "🎉 成功导入 " << count << " 本书籍到 MongoDB!" << std::endl;
}

void addUserBook()
{
    auto db = DatabaseManager::instance().db();
    auto users = db["users"];
    auto novels = db["novels"];

    // 构建要插入的数组
    bsoncxx::builder::basic::array bookshelfArray;

    std::vector<std::tuple<QString, QString, int>> testBooks = {{"斗破苍穹", "天蚕土豆", 30}};
    std::string username = "Rainbow";
    for (const auto& book : testBooks) {
        QString title = std::get<0>(book);
        QString author = std::get<1>(book);
        int progress = std::get<2>(book);

        // 先找书的 _id
        auto novel = novels.find_one(make_document(
            kvp("title", title.toStdString()),
            kvp("author", author.toStdString())
            ));

        if (!novel) {
            qWarning() << "未找到书籍：" << title;
            continue;
        }

        bsoncxx::oid bookId = novel->view()["_id"].get_oid().value;

        // 生成书架项
        bookshelfArray.append(make_document(
            kvp("book_id", bookId),
            kvp("progress", progress)
            ));
    }

    // 更新用户文档（一次性替换书架）
    users.update_one(
        make_document(kvp("username", username)),
        // make_document(kvp("$set", kvp("bookshelf", bookshelfArray.extract())))
        make_document(kvp("$push", make_document(
                kvp("bookshelf", make_document(kvp("$each", bookshelfArray.extract())))
            )))
        );

    qDebug() << "✅ 已为用户" << username << "更新书架";
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Login w;
    // mongocxx::instance inst{};
    // mongocxx::client conn{mongocxx::uri{}};
    // initDatabase(conn);
    // auto db = conn["noveldb"];
    // auto coll = db["novels"];

    // std::vector<Chapter> chaps = {
    //     Chapter("第一章 陨落的天才", 1200, "正文内容..."),
    //     Chapter("第二章 修炼的起点", 1300, "正文内容...")
    // };

    // // 构造书籍
    // Book book(
    //     "斗破苍穹",
    //     "http://xxx.com/doupo.jpg",
    //     "天蚕土豆",
    //     "连载",
    //     {"玄幻", "热血"},
    //     2500000,
    //     "少年萧炎，天才陨落，浴火重生……",
    //     chaps
    //     );

    // coll.insert_one(book.toBson().view());
    // addUserBook();
    // ImportData();
    w.show();
    return a.exec();
}



