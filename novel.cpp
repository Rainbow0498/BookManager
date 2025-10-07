#include "novel.h"
#include "ui_novel.h"
#include "BookItemWidget.h"
#include "DatabaseManager.h"
#include "UserSession.h"
#include <QDebug>
#include <QMessageBox>
Novel::Novel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Novel)
{
    ui->setupUi(this);
    this->resize(1060, 850);
    this->LoadUserData();

    QStringList statuses = {"全部", "连载中", "已完结"};
    for (const QString &s : statuses) {
        QPushButton *btn = new QPushButton(s);
        btn->setCheckable(true);
        btn->setStyleSheet("QPushButton:checked { background-color: #0078D7; color: white; }");
        connect(btn, &QPushButton::clicked, this, &Novel::onStatusClicked);
        ui->statusLayout->addWidget(btn);
        statusButtons.push_back(btn);
    }
    statusButtons[0]->setChecked(true); // 默认选中“全部”
    themeDropdown = new CategoryDropdown(this);
    LoadCategoriesFromDB();
    ui->themeLayout->addWidget(themeDropdown);

    ui->etSearch->setPlaceholderText("输入书名或作者搜索...");
    ui->chapterLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    // ui->chapterLayout->SetFixedSize(450, 600);
}

Novel::~Novel()
{
    delete ui;
}

void Novel::on_btnMyShalf_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageShalf);
    currentPage = 0;
    this->UpdateShalfGrid();
}

void Novel::on_btnBookStore_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->pageBookStore);
    currentPage = 0;
    LoadBooksFromDB();
}

// 当主界面打开的时候调用该函数，获取用户的基本信息
void Novel::LoadUserData()
{
    username = UserSession::instance().username();
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

    std::vector<std::tuple<QString,QString, QString, int>> books;
    for (auto &&book : userDoc["bookshelf"].get_array().value) {
        auto doc = book.get_document().view();
        bsoncxx::oid id = doc["book_id"].get_oid().value;
        int progress = doc["progress"].get_int32();

        auto novel = novels.find_one(make_document(kvp("_id", id)));
        if (novel) {
            auto n = novel->view();
            QString id = QString::fromStdString(n["_id"].get_oid().value.to_string());
            QString title = QString::fromStdString(std::string(n["title"].get_string().value));
            QString author = QString::fromStdString(std::string(n["author"].get_string().value));
            books.emplace_back(id,title, author, progress);
        }
    }
    allBooks = books;

    this->UpdateShalfGrid();
}

void Novel::UpdateShalfGrid(){
    auto db = DatabaseManager::instance().db();
    auto users = db["users"];
    auto novels = db["novels"];
    // 清空旧的书籍项
    QLayoutItem *item;
    while ((item = ui->ShalfGrid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    auto result = users.find_one(make_document(kvp("username", username.toStdString())));
    auto userDoc = result->view();
    auto arr = userDoc["bookshelf"].get_array().value;
    std::vector<std::tuple<QString,QString, QString, int>> books;
    for (auto &&book : userDoc["bookshelf"].get_array().value) {
        auto doc = book.get_document().view();
        bsoncxx::oid id = doc["book_id"].get_oid().value;
        int progress = doc["progress"].get_int32();

        auto novel = novels.find_one(make_document(kvp("_id", id)));
        if (novel) {
            auto n = novel->view();
            QString id = QString::fromStdString(n["_id"].get_oid().value.to_string());
            QString title = QString::fromStdString(std::string(n["title"].get_string().value));
            QString author = QString::fromStdString(std::string(n["author"].get_string().value));
            books.emplace_back(id,title, author, progress);
        }
    }
    allBooks = books;

    // 当前页数据范围
    int start = currentPage * pageSize;
    int end = std::min(start + pageSize, (int)allBooks.size());

    int row = 0, col = 0;
    for (int i = start; i < end; ++i) {
        auto [id,title, author, progress] = allBooks[i];
        BookItemWidget *itemWidget = new BookItemWidget(id,title, author, progress,true, this);
        connect(itemWidget, &BookItemWidget::bookClicked, this, [=](const QString &id) {
            emit OpenBookDetail(id);
        });
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

void Novel::LoadCategoriesFromDB()
{
    auto db = DatabaseManager::instance().db();
    auto novels = db["novels"];

    std::set<QString> categorySet;
    for (auto &&doc : novels.find({})) {
        if (doc["categories"]) {
            for (auto &&val : doc["categories"].get_array().value) {
                QString cat = QString::fromStdString(std::string(val.get_string().value));
                categorySet.insert(cat);
            }
        }
    }
    qDebug() << categorySet;
    QStringList themeList;
    for (const auto &cat : categorySet)
        themeList << cat;

    themeList.sort();
    themeList.prepend("全部");
    themeDropdown->setCategories(themeList);

    // 绑定信号
    connect(themeDropdown, &CategoryDropdown::categorySelected, this, [=](const QString &cat) {
        currentTheme = (cat == "全部") ? "" : cat;
        LoadBooksFromDB();
    });
}

void Novel::LoadBooksFromDB()
{
    auto db = DatabaseManager::instance().db();
    auto novels = db["novels"];

    bsoncxx::builder::basic::document query;

    // 1️⃣ 状态筛选
    if (!currentStatus.isEmpty()) { query.append(kvp("status", currentStatus.toStdString())); }

    // 2️⃣ 主题筛选
    if (!currentTheme.isEmpty()) { query.append(kvp("categories", currentTheme.toStdString())); }

    // 3️⃣ 搜索关键字
    QString keyword = ui->etSearch->text();
    if (!keyword.isEmpty()) {
        query.append(kvp("$or",
                         bsoncxx::builder::basic::make_array(
                             make_document(kvp("title", make_document(kvp("$regex", keyword.toStdString())))),
                             make_document(kvp("author", make_document(kvp("$regex", keyword.toStdString())))))));
    }

    auto cursor = novels.find(query.view());
    std::vector<std::tuple<QString, QString, QString>> books;
    for (auto &&doc : cursor) {
        QString id = QString::fromStdString(doc["_id"].get_oid().value.to_string());
        QString title = QString::fromStdString(std::string(doc["title"].get_string().value));
        QString author = QString::fromStdString(std::string(doc["author"].get_string().value));
        books.emplace_back(id, title, author);
    }

    QLayoutItem *item;
    while ((item = ui->StoreGrid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 当前页数据范围
    int start = currentPage * pageSize;
    int end = std::min(start + pageSize, (int) books.size());

    int row = 0, col = 0;
    for (int i = start; i < end; ++i) {
        auto [id, title, author] = books[i];
        BookItemWidget *itemWidget = new BookItemWidget(id, title, author, 0, false, this);
        connect(itemWidget, &BookItemWidget::bookClicked, this, [=](const QString &id) { emit OpenBookDetail(id); });
        ui->StoreGrid->addWidget(itemWidget, row, col);
        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }

    ui->currentPageLabel_2->setText(
        QString("第 %1 页 / 共 %2 页").arg(currentPage + 1).arg((books.size() + pageSize - 1) / pageSize));

    ui->btnLastPage_2->setEnabled(currentPage > 0);
    ui->btnNextPage_2->setEnabled(end < (int) books.size());
}

void Novel::OpenBookDetail(QString id)
{
    ui->stackedWidget->setCurrentWidget(ui->pageBookDetail);
    auto db = DatabaseManager::instance().db();
    auto novels = db["novels"];
    auto doc = novels.find_one(make_document(kvp("_id", bsoncxx::oid(id.toStdString()))));
    book_id = id;
    if (!doc) return;

    auto view = doc->view();
    ui->bookTitle->setText(QString::fromStdString(std::string(view["title"].get_string().value)));
    ui->bookAuthor->setText("作者：" + QString::fromStdString(std::string(view["author"].get_string().value)));
    ui->bookIntro->setText(QString::fromStdString(std::string(view["intro"].get_string().value)));
    auto chapters = view["chapters"].get_array().value;
    int row = 0, col = 0;

    for (auto &&c : chapters) {
        QString chTitle = QString::fromStdString(std::string(c.get_document().view()["title"].get_string().value));
        qDebug() << chTitle;
        QPushButton *chBtn = new QPushButton(chTitle, this);
        chBtn->setFixedWidth(100);
        chBtn->setCursor(Qt::PointingHandCursor);
        chBtn->setStyleSheet(R"(
            QPushButton {
                border: 1px solid #ccc;
                border-radius: 4px;
                padding: 6px 8px;
                text-align: left;
                background-color: #000000;
                color:blue
            }
            QPushButton:hover {
                background-color: #e9f2ff;
                border: 1px solid #0078d7;
            }
        )");
        ui->chapterLayout->addWidget(chBtn, row, col);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }
    qDebug() << "chapterScroll visible?" << ui->scrollArea->isVisible();
    qDebug() << "chapterLayout count:" << ui->chapterLayout->count();
    bool inShelf = IsBookInUserShelf();
    if (inShelf) {
        ui->btnAddOrOpen->setText("开始阅读");
        ui->btnDelete->show();
    } else {
        ui->btnAddOrOpen->setText("加入书架");
        ui->btnDelete->hide();
    }
    qDebug() << "触发";
}

bool Novel::IsBookInUserShelf()
{
    auto db = DatabaseManager::instance().db();
    auto users = db["users"];
    auto user = users.find_one(make_document(kvp("username", username.toStdString())));
    if (!user) return false;

    auto view = user->view();
    if (!view["bookshelf"]) return false;

    auto shelf = view["bookshelf"].get_array().value;
    bsoncxx::oid oidBook(book_id.toStdString());

    for (auto &&b : shelf) {
        auto bookObj = b.get_document().view();
        if (bookObj["book_id"].type() == bsoncxx::type::k_oid) {
            if (bookObj["book_id"].get_oid().value == oidBook) return true;
        }
    }
    return false;
}

void Novel::on_btnSearch_clicked()
{
    this->LoadBooksFromDB();
}

void Novel::onStatusClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;

    for (auto *b : statusButtons)
        b->setChecked(false);
    btn->setChecked(true);

    currentStatus = btn->text();
    if (currentStatus == "全部") currentStatus = "";
    this->LoadBooksFromDB();
}

// void Novel::on_theme_clicked(){
//     QPushButton *btn = qobject_cast<QPushButton*>(sender());
//     if (!btn) return;

//     for (auto *b : themeButtons) b->setChecked(false);
//     btn->setChecked(true);

//     currentTheme = btn->text();
//     if (currentTheme == "全部") currentTheme = "";
//     this->LoadBooksFromDB();
// }

void Novel::on_btnNextPage_2_clicked()
{
    currentPage++;
    LoadBooksFromDB();
}

void Novel::on_btnLastPage_2_clicked()
{
    currentPage--;
    LoadBooksFromDB();
}

void Novel::on_btnNextPage_clicked()
{
    currentPage++;
    UpdateShalfGrid();
}

void Novel::on_btnLastPage_clicked()
{
    currentPage--;
    LoadBooksFromDB();
}

void Novel::on_btnAddOrOpen_clicked()
{
    bool inShelf = IsBookInUserShelf();
    auto db = DatabaseManager::instance().db();
    auto users = db["users"];

    bsoncxx::oid oidBook(book_id.toStdString());
    if (!inShelf) {
        // 加入书架
        users.update_one(make_document(kvp("username", username.toStdString())),
                         make_document(
                             kvp("$push",
                                 make_document(
                                     kvp("bookshelf", make_document(kvp("book_id", oidBook), kvp("progress", 0)))))));
        QMessageBox::information(this, "成功", "已加入书架！");
        ui->btnAddOrOpen->setText("开始阅读");
        ui->btnDelete->show();
        auto result = users.find_one(make_document(kvp("username", username.toStdString())));
        if (!result) return;

        auto userDoc = result->view();
        auto arr = userDoc["bookshelf"].get_array().value;
        int bookCount = std::distance(arr.begin(), arr.end());
        ui->ShalfNumber->setText(QString("书架数量：%1 本").arg(bookCount));
    } else {
        QMessageBox::information(this, "提示", "进入阅读界面（暂未实现）");
    }
}

void Novel::on_btnDelete_clicked()
{
    auto db = DatabaseManager::instance().db();
    auto users = db["users"];

    bsoncxx::oid oidBook(book_id.toStdString());

    users.update_one(make_document(kvp("username", username.toStdString())),
                     make_document(
                         kvp("$pull", make_document(kvp("bookshelf", make_document(kvp("book_id", oidBook)))))));

    QMessageBox::information(this, "提示", "已从书架中移除！");
    auto result = users.find_one(make_document(kvp("username", username.toStdString())));
    if (!result) return;

    auto userDoc = result->view();
    auto arr = userDoc["bookshelf"].get_array().value;
    int bookCount = std::distance(arr.begin(), arr.end());
    ui->ShalfNumber->setText(QString("书架数量：%1 本").arg(bookCount));
    ui->btnAddOrOpen->setText("加入书架");
    ui->btnDelete->hide();
}

void Novel::on_btnBack_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageBookStore);
    currentPage = 0;
    LoadBooksFromDB();
}
