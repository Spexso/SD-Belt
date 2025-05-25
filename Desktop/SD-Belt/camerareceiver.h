#ifndef CAMERARECEIVER_H
#define CAMERARECEIVER_H

#include <QWidget>
#include <QtCore>
#include <QtNetwork>
#include <QtWidgets>
#include "Globals.h"

class Receiver : public QWidget
{
    Q_OBJECT
public:
    explicit Receiver(QWidget* parent = nullptr);
    ~Receiver();

private slots:
    void ProcessPending();

private:
    QUdpSocket   socket_;
    QLabel*      labels_[3]{};
    QGridLayout  layout_;
};


#endif // CAMERARECEIVER_H
