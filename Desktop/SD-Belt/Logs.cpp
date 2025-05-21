#include "logs.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

Logs::Logs(QNetworkAccessManager *manager, QListWidget *target, QObject *parent)
    : QObject(parent), network(manager), listWidget(target)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        this->fetch(currentToken);
    });
    timer->start(10000);
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
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    if (!root.contains("result") || !root["result"].isArray()) return;

    QJsonArray logs = root["result"].toArray();
    listWidget->clear();

    int totalCount = logs.size();
    if (countLabel)
        countLabel->setText(QString::number(totalCount));

    QDateTime minTime, maxTime;
    bool first = true;

    for (const QJsonValue &entry : logs)
    {
        QJsonObject obj = entry.toObject();
        QString status = obj["isSuccess"].toBool() ? "✅" : "❌";
        QString productId = obj["productId"].toString();
        double health = obj["healthRatio"].toDouble();

        QString message = obj.contains("errorMessage") && !obj["errorMessage"].isNull()
                              ? obj["errorMessage"].toString()
                              : "No error";

        QString timestampStr = obj["timestamp"].toString();
        QDateTime timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
        QString timeFormatted = timestamp.toString("yyyy-MM-dd hh:mm:ss");

        if (timestamp.isValid())
        {
            if (first)
            {
                minTime = maxTime = timestamp;
                first = false;
            } else
            {
                if (timestamp < minTime) minTime = timestamp;
                if (timestamp > maxTime) maxTime = timestamp;
            }
        }

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


    qint64 elapsedMinutes = minTime.secsTo(maxTime) / 60;
    if (elapsedMinutes == 0) elapsedMinutes = 1;

    int itemsPerMinute = totalCount / elapsedMinutes;
    if (speedLabel)
        speedLabel->setText(QString::number(itemsPerMinute));
}

void Logs::setCountLabel(QLabel *label)
{
    countLabel = label;
}

void Logs::setSpeedLabel(QLabel *label)
{
    speedLabel = label;
}



