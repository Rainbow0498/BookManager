#include "Login.h"
#include "DatabaseManager.h"
#include "Register.h"
#include <QMessageBox>
#include "ui_Login.h"

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
    ui->etUserPassword->setEchoMode(QLineEdit::Password);
}

Login::~Login()
{
    delete ui;
}

void Login::on_btnLogin_clicked() {
    auto db = DatabaseManager::instance().db();
    auto users = db["users"];

    std::string username = ui->etUserName->text().toStdString();
    std::string password = ui->etUserPassword->text().toStdString();

    auto result = users.find_one(make_document(kvp("username", username)));

    if (!result) {
        ui->statusLabel->setText("❌ 用户不存在");
        return;
    }
    auto doc = result->view();
    std::string dbPwd = std::string(doc["password"].get_string().value);

    if (dbPwd == password) {
        ui->statusLabel->setText("✅ 登录成功");
        QMessageBox::information(this, "登录成功", "欢迎回来，" + ui->etUserName->text());
        // TODO: 进入主界面
    } else {
        ui->statusLabel->setText("❌ 密码错误");
    }
}
void Login::on_btnRegister_clicked()
{
    auto *rec = new Register();
    rec->show();
    this->close();
}
