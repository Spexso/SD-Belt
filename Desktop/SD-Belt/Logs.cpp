#include "logs.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

Logs::Logs(QNetworkAccessManager *manager, QListWidget *target, QObject *parent)
    : QObject(parent), network(manager), listWidget(target)
{
}

void Logs::fetch(const QString &token)
{
    QUrl url(BaseUrl "/scans");

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    reply = network->get(request);
    connect(reply, &QNetworkReply::finished, this, &Logs::onLogsReceived);
}

void Logs::onLogsReceived()
{
    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);

    if (!doc.isObject()) {
        reply->deleteLater();
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("result") || !root["result"].isArray()) {
        reply->deleteLater();
        return;
    }

    QJsonArray logs = root["result"].toArray();
    listWidget->clear();

    for (const QJsonValue &entry : logs)
    {
        QJsonObject obj = entry.toObject();
        QString status = obj["isSuccess"].toBool() ? "✅" : "❌";
        QString productId = obj["productId"].toString();
        double health = obj["healthRatio"].toDouble();

        QString message = obj["errorMessage"].isNull()
                              ? "No error"
                              : obj["errorMessage"].toString();

        QString timestampStr = obj["timestamp"].toString();
        QDateTime timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
        QString timeFormatted = timestamp.toString("yyyy-MM-dd hh:mm:ss");

        QString logEntry = QString("[%1] %2 (%3%) %4: %5")
                               .arg(timeFormatted)
                               .arg(productId)
                               .arg(QString::number(health, 'f', 1))
                               .arg(status)
                               .arg(message);

        QListWidgetItem *item = new QListWidgetItem(logEntry);
        item->setBackground(obj["isSuccess"].toBool() ? Qt::darkGreen : Qt::darkRed);
        listWidget->addItem(item);
    }

    listWidget->setVisible(true);
    listWidget->update();

    reply->deleteLater();
}


