#ifndef LOGS_H
#define LOGS_H

#include <QObject>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#define BaseUrl "https://sdbelt.devodev.online/api/v1"

class Logs : public QObject
{
    Q_OBJECT
public:
    explicit Logs(QNetworkAccessManager *manager, QListWidget *target, QObject *parent = nullptr);
    void fetch(const QString &token);

private slots:
    void onLogsReceived();

private:
    QNetworkAccessManager *network;
    QListWidget *listWidget;
    QNetworkReply *reply;

signals:
};

#endif // LOGS_H
