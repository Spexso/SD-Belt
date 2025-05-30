#include "SystemLogRetriever.h"
#include "Globals.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

SystemLogRetriever::SystemLogRetriever(QNetworkAccessManager* manager, QListWidget* logWidget, QObject* parent)
    : QObject(parent), network(manager), LogListWidget(logWidget)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        this->fetch(currentToken);
    });
    timer->start(4000); // Every 2 seconds
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
    QNetworkReply* finishedReply = reply;
    reply = nullptr;

    if (!finishedReply)
        return;

    QByteArray responseData = finishedReply->readAll();
    finishedReply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "Failed to parse system log JSON:" << parseError.errorString();
        return;
    }

    if (!doc.isObject())
    {
        qWarning() << "Expected a JSON object but got something else.";
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("logs") || !root["logs"].isArray())
    {
        qWarning() << "JSON does not contain a valid 'logs' array.";
        return;
    }

    QJsonArray logsArray = root["logs"].toArray();
    for (const QJsonValue& entry : logsArray)
    {
        QString text = entry.toString();

        // Limit log entries to avoid bad_alloc
        if (LogListWidget->count() >= MaxLogItems)
        {
            delete LogListWidget->takeItem(0); // Remove oldest
        }

        LogListWidget->addItem(new QListWidgetItem(text));
    }
}
