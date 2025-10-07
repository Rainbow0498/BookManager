#include "novel.h"
#include "ui_novel.h"
#include "BookItemWidget.h"
#include "DatabaseManager.h"
#include "UserSession.h"

Novel::Novel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Novel)
{
    ui->setupUi(this);
    this->resize(1060, 850);
    this->LoadUserData();
}

Novel::~Novel()
{
    delete ui;
}

void Novel::on_btnMyShalf_clicked()
{
    currentPage = 0;
    this->UpdateShalfGrid();
}

// 当主界面打开的时候调用该函数，获取用户的基本信息
void Novel::LoadUserData()
{
    QString username = UserSession::instance().username();
    auto db = DatabaseManager::instance().db();
    auto users = db["users"];
    auto novels = db["novels"];

    auto result = users.find_one(make_document(kvp("username", username.toStdString())));
    if (!result) return;

    auto userDoc = result->view();
    ui->UserName->setText(username);

    auto arr = userDoc["bookshelf"].get_array().value;
    int bookCount = std::distance(arr.begin(), arr.end());
    ui->ShalfNumber->setText(QString("书架数量：%1 本").arg(bookCount));

    std::vector<std::tuple<QString, QString, int>> books;
    for (auto &&book : userDoc["bookshelf"].get_array().value) {
        auto doc = book.get_document().view();
        bsoncxx::oid id = doc["book_id"].get_oid().value;
        int progress = doc["progress"].get_int32();

        auto novel = novels.find_one(make_document(kvp("_id", id)));
        if (novel) {
            auto n = novel->view();
            QString title = QString::fromStdString(std::string(n["title"].get_string().value));
            QString author = QString::fromStdString(std::string(n["author"].get_string().value));
            books.emplace_back(title, author, progress);
        }
    }
    allBooks = books;
    this->UpdateShalfGrid();
}

void Novel::UpdateShalfGrid(){
    // 清空旧的书籍项
    QLayoutItem *item;
    while ((item = ui->ShalfGrid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 当前页数据范围
    int start = currentPage * pageSize;
    int end = std::min(start + pageSize, (int)allBooks.size());

    int row = 0, col = 0;
    for (int i = start; i < end; ++i) {
        auto [title, author, progress] = allBooks[i];
        BookItemWidget *itemWidget = new BookItemWidget(title, author, progress, this);

        ui->ShalfGrid->addWidget(itemWidget, row, col);

        col++;
        if (col >= 3) { col = 0; row++; }
    }

    ui->currentPageLabel->setText(QString("第 %1 页 / 共 %2 页")
                           .arg(currentPage + 1)
                           .arg((allBooks.size() + pageSize - 1) / pageSize));

    ui->btnLastPage->setEnabled(currentPage > 0);
    ui->btnNextPage->setEnabled(end < (int)allBooks.size());
}
