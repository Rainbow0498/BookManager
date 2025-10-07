#pragma once
#include <QWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QEvent>

class CategoryDropdown : public QWidget {
    Q_OBJECT
public:
    explicit CategoryDropdown(QWidget *parent = nullptr);

    void setCategories(const QStringList &categories);
    QString currentCategory() const { return m_currentCategory; }

signals:
    void categorySelected(const QString &category);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onMainButtonClicked();
    void onCategoryClicked();

private:
    QPushButton *mainButton;
    QFrame *popup;
    QScrollArea *scrollArea;
    QGridLayout *gridLayout;

    QString m_currentCategory;
};
