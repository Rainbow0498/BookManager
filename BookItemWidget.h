#pragma once
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QProgressBar>

class BookItemWidget : public QFrame {
    Q_OBJECT
public:
    explicit BookItemWidget(const QString &title,
                            const QString &author,
                            int progress,
                            QWidget *parent = nullptr)
        : QFrame(parent) {

        setFrameShape(QFrame::Box);
        setLineWidth(1);
        setFixedSize(160, 200);

        auto *layout = new QVBoxLayout(this);

        // 假装封面
        QLabel *cover = new QLabel(this);
        cover->setFixedSize(120, 100);
        cover->setStyleSheet("background-color: lightgray; border-radius: 5px;");
        cover->setAlignment(Qt::AlignCenter);
        cover->setText("封面");
        layout->addWidget(cover, 0, Qt::AlignCenter);

        QLabel *titleLabel = new QLabel(title, this);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-weight: bold;");
        layout->addWidget(titleLabel);

        QLabel *authorLabel = new QLabel("作者：" + author, this);
        authorLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(authorLabel);

        // QLabel *progressLabel = new QLabel(progress+'/', this);
        // authorLabel->setAlignment(Qt::AlignCenter);
        // layout->addWidget(authorLabel);
        QProgressBar *progressBar = new QProgressBar(this);
        progressBar->setRange(0, 100);
        progressBar->setValue(progress);
        progressBar->setFormat("进度 %p%");
        layout->addWidget(progressBar);

        setLayout(layout);
    }
};
