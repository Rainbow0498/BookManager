#pragma once

#include <QWidget>
#include <QPushButton>
#include "categorydropdown.h"
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
    void LoadCategoriesFromDB();
    void LoadBooksFromDB();
    void OpenBookDetail(QString id);
    bool IsBookInUserShelf();
    ~Novel();

private slots:
    void on_btnMyShalf_clicked();

    void on_btnBookStore_clicked();

    // void on_theme_clicked();
    void onStatusClicked();
    void on_btnSearch_clicked();

    void on_btnNextPage_2_clicked();

    void on_btnLastPage_2_clicked();

    void on_btnNextPage_clicked();

    void on_btnLastPage_clicked();

    void on_btnAddOrOpen_clicked();

    void on_btnDelete_clicked();

    void on_btnBack_clicked();

private:
    Ui::Novel *ui;
    CategoryDropdown *themeDropdown;
    std::vector<std::tuple<QString, QString, QString, int>> allBooks;
    std::vector<QPushButton *> statusButtons;
    // std::vector<QPushButton*> themeButtons;
    QString book_id;
    QString username;
    QString currentStatus;
    QString currentTheme;
    int currentPage = 0;
    const int pageSize = 9;
};
