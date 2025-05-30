#include "CameraReceiver.h"

Receiver::Receiver(QWidget* parent)
    : QWidget(parent)
{
    socket_.bind(5000, QUdpSocket::ShareAddress |
                           QUdpSocket::ReuseAddressHint);

    connect(&socket_, &QUdpSocket::readyRead,
            this, &Receiver::ProcessPending);

    for (int i = 0; i < CAM_COUNT; ++i)
    {
        QLabel* NewEntry = new QLabel("No Frame");

        if(NewEntry)
        {
            NewEntry->setStyleSheet("color: white;");

            labels_[i] = NewEntry;
            labels_[i]->setMinimumSize(WIDGET_W, WIDGET_H);
            labels_[i]->setAlignment(Qt::AlignCenter);
            layout_.addWidget(labels_[i], 0, i);
        }
    }

    setLayout(&layout_);
    setWindowTitle("UDP Camera Viewer");
}

void Receiver::ProcessPending()
{
    if (!socket_.isValid())
        return;

    while (socket_.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(socket_.pendingDatagramSize());
        qint64 bytesRead = socket_.readDatagram(datagram.data(), datagram.size());

        // If read failed or mismatch in size
        if (bytesRead <= 0 || datagram.size() < 5)
            continue;

        QDataStream stream(datagram);
        stream.setByteOrder(QDataStream::BigEndian);

        quint8 camId = 0;
        quint32 length = 0;

        stream >> camId >> length;

        // Validate camId and data size
        if (camId >= CAM_COUNT || static_cast<int>(length) != datagram.size() - 5)
            continue;

        // Extract image data
        QByteArray imageData = datagram.mid(5, length);
        if (imageData.isEmpty())
            continue;

        QImage img = QImage::fromData(imageData, "JPG");
        if (img.isNull())
            continue;

        if (labels_[camId])
        {
            QPixmap pixmap = QPixmap::fromImage(img).scaled(
                labels_[camId]->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                );

            labels_[camId]->setPixmap(pixmap);
        }
    }
}

Receiver::~Receiver()
{
    // No need to delete labels_ manually; Qt does it because of QObject hierarchy
    disconnect(&socket_, nullptr, nullptr, nullptr);
    socket_.close();
}
