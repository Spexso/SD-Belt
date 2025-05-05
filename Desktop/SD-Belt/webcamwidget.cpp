#include "webcamwidget.h"
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QDebug>

WebcamWidget::WebcamWidget(const QString &url, QWidget *parent)
    : QWidget(parent)
{
    videoLabel = new QLabel(this);
    videoLabel->setFixedSize(640, 480);
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setStyleSheet("background-color: black;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(videoLabel);
    setLayout(layout);

    connect(&socket, &QWebSocket::connected, this, [] {
        qDebug() << "[WebcamWidget] WebSocket connection established.";
    });

    connect(&socket, &QWebSocket::textMessageReceived,
            this, &WebcamWidget::onFrameReceived);

    socket.open(QUrl(url));
}

void WebcamWidget::onFrameReceived(const QString &base64Data)
{
    QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());
    QImage image;
    image.loadFromData(imageData, "JPG");
    if (!image.isNull()) {
        videoLabel->setPixmap(QPixmap::fromImage(image).scaled(videoLabel->size()));
    } else {
        qDebug() << "[WebcamWidget] Invalid image.";
    }
}


