#ifndef SYSTEMLOGRETRIEVER_H
#define SYSTEMLOGRETRIEVER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QListWidget>
#include <QTimer>
#include <QLabel>
#include "Globals.h"

class SystemLogRetriever : public QObject
{
    Q_OBJECT

public:

    explicit SystemLogRetriever(QNetworkAccessManager* manager, QListWidget* LogWidget, QObject* parent = nullptr);
    void fetch(const QString &token);

private slots:

    void OnSystemLogReceived();

private:

    QNetworkAccessManager *network;
    QNetworkReply *reply;

    QListWidget* LogListWidget = nullptr;
    QTimer *timer = nullptr;
    QString currentToken;

signals:
};

#endif // SYSTEMLOGRETRIEVER_H
