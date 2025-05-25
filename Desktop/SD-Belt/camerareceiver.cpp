#include "CameraReceiver.h"

#define WIDGET_H 240
#define WIDGET_W 320
#define CAM_COUNT 3

Receiver::Receiver(QWidget* parent)
    : QWidget(parent)
{
    socket_.bind(5000, QUdpSocket::ShareAddress |
                           QUdpSocket::ReuseAddressHint);

    connect(&socket_, &QUdpSocket::readyRead,
            this, &Receiver::ProcessPending);

    for (int i = 0; i < CAM_COUNT; ++i)
    {
        labels_[i] = new QLabel("No Frame");
        labels_[i]->setMinimumSize(WIDGET_W, WIDGET_H);
        labels_[i]->setAlignment(Qt::AlignCenter);
        layout_.addWidget(labels_[i], 0, i);
    }

    setLayout(&layout_);
    setWindowTitle("UDP Camera Viewer");
}

void Receiver::ProcessPending()
{
    while (socket_.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(socket_.pendingDatagramSize());
        socket_.readDatagram(datagram.data(), datagram.size());

        if (datagram.size() < 5)
            continue;

        QDataStream stream(datagram);
        stream.setByteOrder(QDataStream::BigEndian);

        quint8 camId;
        quint32 length;

        stream >> camId >> length;

        if (camId >= CAM_COUNT || length != datagram.size() - 5)
            continue;

        QByteArray imageData = datagram.mid(5, length);
        QImage img = QImage::fromData(imageData, "JPG");

        if (!img.isNull() && labels_[camId])
        {
            labels_[camId]->setPixmap(
                QPixmap::fromImage(img).scaled(labels_[camId]->size(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation));
        }
    }
}

Receiver::~Receiver()
{
    // No need to delete labels_ manually; Qt does it because of QObject hierarchy
    disconnect(&socket_, nullptr, nullptr, nullptr);
    socket_.close();
}
