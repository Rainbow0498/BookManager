#include "categorydropdown.h"
#include <QApplication>
#include <QStyle>
#include <QPoint>

CategoryDropdown::CategoryDropdown(QWidget *parent)
    : QWidget(parent) {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainButton = new QPushButton("主题 ▼", this);
    mainButton->setFixedHeight(24);
    mainButton->setStyleSheet(
        "QPushButton { border: 1px solid gray; border-radius: 4px; background: white; }"
        "QPushButton:hover { background: #f0f0f0; }");
    mainLayout->addWidget(mainButton);
    setLayout(mainLayout);

    // ===== 弹出层构造 =====
    popup = new QFrame(nullptr, Qt::Popup);
    popup->setFrameShape(QFrame::Box);
    popup->setLineWidth(1);

    scrollArea = new QScrollArea(popup);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QWidget *container = new QWidget();
    gridLayout = new QGridLayout(container);
    gridLayout->setSpacing(8);
    gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scrollArea->setWidget(container);

    QVBoxLayout *popupLayout = new QVBoxLayout(popup);
    popupLayout->addWidget(scrollArea);
    popup->setLayout(popupLayout);

    connect(mainButton, &QPushButton::clicked, this, &CategoryDropdown::onMainButtonClicked);
}

void CategoryDropdown::setCategories(const QStringList &categories) {
    // 清空旧标签
    QLayoutItem *item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    int row = 0, col = 0;
    for (const QString &cat : categories) {
        QPushButton *btn = new QPushButton(cat);
        btn->setCheckable(true);
        btn->setStyleSheet(
            "QPushButton { padding: 4px 8px; border: 1px solid #aaa; border-radius: 3px; }"
            "QPushButton:checked { background-color: #0078D7; color: white; border: none; }");
        connect(btn, &QPushButton::clicked, this, &CategoryDropdown::onCategoryClicked);

        gridLayout->addWidget(btn, row, col);
        col++;
        if (col >= 4) { col = 0; row++; }
    }
}

void CategoryDropdown::onMainButtonClicked() {
    // 计算弹出位置
    QPoint pos = mapToGlobal(QPoint(0, height()));

    popup->setFixedWidth(width());
    popup->setFixedHeight(180); // 可调
    popup->move(pos);
    popup->show();
}

void CategoryDropdown::onCategoryClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    // 取消其他选中
    for (int i = 0; i < gridLayout->count(); ++i) {
        if (auto *b = qobject_cast<QPushButton*>(gridLayout->itemAt(i)->widget()))
            b->setChecked(false);
    }

    btn->setChecked(true);
    m_currentCategory = btn->text();
    mainButton->setText(m_currentCategory + " ▼");

    popup->hide();
    emit categorySelected(m_currentCategory);
}

// 监听点击外部关闭弹窗
bool CategoryDropdown::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress && popup->isVisible()) {
        popup->hide();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}
