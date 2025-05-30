#ifndef LOGS_H
#define LOGS_H

#include "Globals.h"
#include <QObject>
#include <QLabel>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

class Logs : public QObject
{
    Q_OBJECT
public:
    explicit Logs(QNetworkAccessManager *manager, QListWidget *target, QLabel* ItemsPerMinute, QObject *parent = nullptr);
    void fetch(const QString &token);
    void setCountLabel(QLabel *label);
    void setSpeedLabel(QLabel *label);

private slots:
    void onLogsReceived();

private:
    QNetworkAccessManager *network;
    QListWidget *listWidget;
    QNetworkReply *reply;
    QLabel *countLabel = nullptr;
    QLabel *speedLabel = nullptr;
    QLabel* ItemsPerMinuteWidget = nullptr;

    QTimer *timer = nullptr;
    QString currentToken;

signals:
};

#endif // LOGS_H
