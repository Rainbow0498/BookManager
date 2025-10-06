#pragma once

#include <QWidget>
#include <string>

namespace Ui {
    class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

private slots:
    void on_btnRegister_clicked();

    void on_btnReturn_clicked();

private:
    Ui::Register *ui;
};
