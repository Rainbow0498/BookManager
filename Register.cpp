#include "Register.h"
#include "Login.h"
#include "DatabaseManager.h"
#include <QMessageBox>
#include "ui_Register.h"

Register::Register(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Register)
{
    ui->setupUi(this);
    ui->etUserPassword->setEchoMode(QLineEdit::Password);
    ui->etUserPassword_2->setEchoMode(QLineEdit::Password);
}

Register::~Register()
{
    delete ui;
}

void Register::on_btnRegister_clicked()
{
    auto db = DatabaseManager::instance().db();
    auto users = db["users"];

    std::string username = ui->etUserName->text().toStdString();
    std::string password = ui->etUserPassword->text().toStdString();
    std::string confirmPassword = ui->etUserPassword_2->text().toStdString();
    if (username.empty() || password.empty() || confirmPassword.empty()) {
        ui->statusLabel->setText("⚠️ 用户名或密码不能为空");
        return;
    }

    // 检查两次密码是否一致
    if (password != confirmPassword) {
        ui->statusLabel->setText("❌ 两次密码不一致");
        QMessageBox::warning(this, "错误", "两次输入的密码不一致！");
        return;
    }
    // 检查是否重复注册
    auto exists = users.find_one(make_document(kvp("username", username)));
    if (exists) {
        ui->statusLabel->setText("⚠️ 用户名已存在");
        QMessageBox::warning(this, "注册失败", "该用户名已被注册！");
        return;
    }

    // 2. 创建新用户
    users.insert_one(make_document(kvp("username", username),
                                   kvp("password", password),
                                   kvp("bookshelf", bsoncxx::builder::basic::make_array())));

    ui->statusLabel->setText("✅ 注册成功，请登录");
    QMessageBox::information(this, "注册成功", "用户创建成功，请登录！");
}

void Register::on_btnReturn_clicked()
{
    auto *rec = new Login();
    rec->show();
    this->close();
}
