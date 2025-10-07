#pragma once

#include <QWidget>

namespace Ui {
    class Novel;
}

class Novel : public QWidget
{
    Q_OBJECT

public:
    explicit Novel(QWidget *parent = nullptr);
    void LoadUserData();
    void UpdateShalfGrid();
    ~Novel();

private slots:
    void on_btnMyShalf_clicked();

private:
    Ui::Novel *ui;
    std::vector<std::tuple<QString, QString, int>> allBooks;
    int currentPage = 0;
    const int pageSize = 9;
};
