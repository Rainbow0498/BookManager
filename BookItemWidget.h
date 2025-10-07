#pragma once
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QMouseEvent>
class BookItemWidget : public QFrame {
    Q_OBJECT
public:
    explicit BookItemWidget(const QString &bookId,
                            const QString &title,
                            const QString &author,
                            int progress,
                            bool showProgress = true,
                            QWidget *parent = nullptr)
        : QFrame(parent),
        m_bookId(bookId),
        m_title(title),
        m_author(author) {

        setFrameShape(QFrame::Box);
        setLineWidth(1);
        setFixedSize(160, 150);

        auto *layout = new QVBoxLayout(this);

        // 假装封面
        QLabel *cover = new QLabel(this);
        cover->setFixedSize(80, 80);
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
        if(showProgress){
            QProgressBar *progressBar = new QProgressBar(this);
            progressBar->setRange(0, 100);
            progressBar->setValue(progress);
            progressBar->setFormat("进度 %p%");
            layout->addWidget(progressBar);
        }

        setLayout(layout);
    }
signals:
    void bookClicked(const QString &book_id);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) { emit bookClicked(m_bookId); }
        QFrame::mousePressEvent(event);
    }

private:
    QString m_bookId;
    QString m_title;
    QString m_author;
};
