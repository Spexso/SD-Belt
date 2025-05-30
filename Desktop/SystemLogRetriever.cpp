#include "SystemLogRetriever.h"
#include "Globals.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>


SystemLogRetriever::SystemLogRetriever(QNetworkAccessManager* manager, QListWidget* LogWidget, QObject* parent)
    : QObject(parent), network(manager), LogListWidget(LogWidget)
{

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        this->fetch(currentToken);
    });
    timer->start(2000);
}

void SystemLogRetriever::fetch(const QString& token)
{
    currentToken = token;

    QUrl url(QString("%1%2").arg(BackendAddress).arg(SystemLogs));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    reply = network->get(request);
    connect(reply, &QNetworkReply::finished, this, &SystemLogRetriever::OnSystemLogReceived);
}

void SystemLogRetriever::OnSystemLogReceived()
{
    if (!reply)
        return;

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "System log request failed:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "Failed to parse system log JSON:" << parseError.errorString();
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("result") || !root["result"].isArray())
    {
        qWarning() << "Missing or invalid 'result' in system log response";
        return;
    }

    QJsonArray logArray = root["result"].toArray();

    if (LogListWidget)
        LogListWidget->clear();

    for (const QJsonValue& entry : std::as_const(logArray))
    {
        if (!entry.isObject())
            continue;

        QJsonObject obj = entry.toObject();
        QString level = obj.value("level").toString();

        if (level != "ERROR")
            continue; // Only process ERROR logs

        QString timestamp = obj.value("timestamp").toString();
        QString message = obj.value("message").toString();

        QString logEntry = QString("[%1] %2: %3")
                               .arg(timestamp)
                               .arg(level)
                               .arg(message);

        qDebug() << logEntry;

        if (LogListWidget)
        {
            QListWidgetItem* item = new QListWidgetItem(logEntry);
            item->setBackground(Qt::darkCyan);
            LogListWidget->addItem(item);
        }
    }

    if (LogListWidget)
        LogListWidget->scrollToBottom();
}
