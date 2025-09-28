#pragma once
#include <string>
#include <vector>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;

// ========== 章节类 ==========
class Chapter {
public:
    std::string title;   // 章节名称
    int wordCount;       // 字数
    std::string content; // 正文内容

    Chapter(const std::string& t, int w, const std::string& c)
        : title(t), wordCount(w), content(c) {}

    bsoncxx::document::value toBson() const {
        return make_document(
            kvp("title", title),
            kvp("word_count", wordCount),
            kvp("content", content)
            );
    }

    static Chapter fromBson(const bsoncxx::document::view& doc) {
        return Chapter(
            std::string(doc["title"].get_string().value),
            doc["word_count"].get_int32(),
            std::string(doc["content"].get_string().value)
            );
    }
};

// ========== 书籍类 ==========
class Book {
public:
    std::string title;        // 书名
    std::string coverUrl;     // 封面地址
    std::string author;       // 作者
    std::string status;       // 状态: 连载 / 断更 / 完结
    std::vector<std::string> categories; // 分类: 玄幻 / 游戏 / 都市...
    int wordCount;            // 总字数
    std::string intro;        // 简介
    std::vector<Chapter> chapters; // 章节集合

    Book(const std::string& t, const std::string& c, const std::string& a,
         const std::string& s, const std::vector<std::string>& cats,
         int wc, const std::string& i, const std::vector<Chapter>& ch)
        : title(t), coverUrl(c), author(a), status(s),
        categories(cats), wordCount(wc), intro(i), chapters(ch) {}

    // 转 BSON
    bsoncxx::document::value toBson() const {
        // 分类数组
        bsoncxx::builder::basic::array catArr;
        for (const auto& cat : categories) {
            catArr.append(cat);
        }

        // 章节数组
        bsoncxx::builder::basic::array chapArr;
        for (const auto& ch : chapters) {
            chapArr.append(ch.toBson());
        }

        return make_document(
            kvp("title", title),
            kvp("cover_url", coverUrl),
            kvp("author", author),
            kvp("status", status),
            kvp("categories", catArr),
            kvp("word_count", wordCount),
            kvp("intro", intro),
            kvp("chapters", chapArr)
            );
    }

    // 从 BSON 转对象
    static Book fromBson(const bsoncxx::document::view& doc) {
        std::string title = std::string(doc["title"].get_string().value);
        std::string cover = std::string(doc["cover_url"].get_string().value);
        std::string author = std::string(doc["author"].get_string().value);
        std::string status = std::string(doc["status"].get_string().value);
        int wordCount = doc["word_count"].get_int32();
        std::string intro = std::string(doc["intro"].get_string().value);

        // 分类
        std::vector<std::string> categories;
        for (auto&& val : doc["categories"].get_array().value) {
            categories.push_back(std::string(val.get_string().value));
        }

        // 章节
        std::vector<Chapter> chapters;
        for (auto&& val : doc["chapters"].get_array().value) {
            chapters.push_back(Chapter::fromBson(val.get_document().view()));
        }

        return Book(title, cover, author, status, categories, wordCount, intro, chapters);
    }
};
