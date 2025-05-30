#include "logs.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

Logs::Logs(QNetworkAccessManager *manager, QListWidget *target, QLabel *ItemsPerMinute, QObject *parent)
    : QObject(parent), network(manager), listWidget(target), ItemsPerMinuteWidget(ItemsPerMinute)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        this->fetch(currentToken);
    });
    timer->start(7000);
}

void Logs::fetch(const QString &token)
{
    QUrl url(BackendAddress AccessLogsUrl);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    if(network)
    {
        reply = network->get(request);
        connect(reply, &QNetworkReply::finished, this, &Logs::onLogsReceived);
    }
}

#include "Globals.h"  // Ensure LogsPerMinute is declared here

void Logs::onLogsReceived()
{
    if (!reply)
        return;

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("result") || !root["result"].isArray())
        return;

    QJsonArray logs = root["result"].toArray();

    if (listWidget)
        listWidget->clear();

    int totalCount = logs.size();
    if (countLabel)
        countLabel->setText(QString::number(totalCount));

    QDateTime minTime, maxTime;
    bool first = true;

    for (const QJsonValue &entry : logs)
    {
        if (!entry.isObject())
            continue;

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
            }
            else
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

        if (listWidget)
        {
            QListWidgetItem *item = new QListWidgetItem(logEntry);
            item->setBackground(obj["isSuccess"].toBool() ? Qt::darkGreen : Qt::darkRed);
            listWidget->addItem(item);
        }
    }

    if (listWidget)
    {
        listWidget->setVisible(true);
        listWidget->update();
        listWidget->scrollToBottom();
    }

    qint64 elapsedMinutes = minTime.secsTo(maxTime) / 60;
    if (elapsedMinutes == 0) elapsedMinutes = 1;

    int itemsPerMinute = totalCount / elapsedMinutes;

    // Update UI label
    if (ItemsPerMinuteWidget)
        ItemsPerMinuteWidget->setText(QString::number(itemsPerMinute));
}



void Logs::setCountLabel(QLabel *label)
{
    countLabel = label;
}

void Logs::setSpeedLabel(QLabel *label)
{
    speedLabel = label;
}



