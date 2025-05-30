#include "SystemInfoRetriever.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

SystemInfoRetriever::SystemInfoRetriever(QNetworkAccessManager *manager, QLabel* CpuTemperatureValue, QLabel* CpuUsageValue, QLabel* RamUsageValue, QLabel* SystemIndicatorLabel, QObject *parent)
    : QObject{parent}, network(manager), CpuTemperatureWidget(CpuTemperatureValue), CpuUsageWidget(CpuUsageValue), RamUsageWidget(RamUsageValue), SystemIndicatorWidget(SystemIndicatorLabel)
{
    SystemStatusGoodPix.load(":/styles/img/good.png");
    SystemStatusBadPix.load(":/styles/img/bad.png");

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        this->fetch(currentToken);
    });
    timer->start(4000);
}

void SystemInfoRetriever::fetch(const QString &token)
{
    QUrl url(BackendAddress AccessSystemInfoUrl);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    if(network)
    {
        reply = network->get(request);
        connect(reply, &QNetworkReply::finished, this, &SystemInfoRetriever::OnSystemInfoReceived);
    }
}

void SystemInfoRetriever::OnSystemInfoReceived()
{
    if (!reply)
        return;

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "System info request failed:" << reply->errorString();
        reply->deleteLater();
        reply = nullptr; // Prevent crash on reuse
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "Failed to parse system info JSON:" << parseError.errorString();
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("result") || !root["result"].isObject())
    {
        qWarning() << "Missing or invalid 'result' in system info response";
        return;
    }

    QJsonObject systemInfo = root["result"].toObject();

    // Safely extract and apply system info values
    double cpuUsage = systemInfo.value("cpuUsage").toDouble();
    double ramUsage = systemInfo.value("memoryUsage").toDouble();
    double cpuTemp = systemInfo.value("cpuTemperature").toDouble();
    QString status = systemInfo.value("status").toString();

    this->SetCpuUtilization(cpuUsage);
    this->SetRamUtilization(ramUsage);
    this->SetCpuTemperature(cpuTemp);
    this->SetIsSytemOnline(status != "INACTIVE");

    SetOverlayCpuTemperature();
    SetOverlayCpuUtilization();
    SetOverlayRamUtilization();
    SetOverlayIsSystemOnline();
}

void SystemInfoRetriever::SetCpuUtilization(double NewCpuUtilization)
{
    this->CpuUtilization = NewCpuUtilization;
}

void SystemInfoRetriever::SetRamUtilization(double NewRamUtilization)
{
    this->RamUtilization = NewRamUtilization;
}

void SystemInfoRetriever::SetCpuTemperature(double NewCpuTemperature)
{
    this->CpuTemperature = NewCpuTemperature;
}

void SystemInfoRetriever::SetIsSytemOnline(bool NewIsSystemOnline)
{
    this->IsSystemOnline = NewIsSystemOnline;
}

void SystemInfoRetriever::SetOverlayCpuUtilization()
{
    if(CpuUsageWidget)
    {
        QString PercentText = PercentSign % QString::number(this->GetCpuUtilization(), 'f', 1);
        CpuUsageWidget->setText(PercentText);
    }
}

void SystemInfoRetriever::SetOverlayRamUtilization()
{
    if(RamUsageWidget)
    {
        QString PercentText = PercentSign % QString::number(this->GetRamUtilization(), 'f', 1);
        RamUsageWidget->setText(PercentText);
    }
}

void SystemInfoRetriever::SetOverlayCpuTemperature()
{
    if(CpuTemperatureWidget)
    {
        QString PercentText = QString::number(this->GetCpuTemperature(), 'f', 1) % CelciusSign;
        CpuTemperatureWidget->setText(PercentText);
    }
}

void SystemInfoRetriever::SetOverlayIsSystemOnline()
{
    if(SystemIndicatorWidget && !SystemStatusGoodPix.isNull() && !SystemStatusBadPix.isNull())
    {
        if(this->GetIsSystemOnline())
            SystemIndicatorWidget->setPixmap(SystemStatusGoodPix);
        else
            SystemIndicatorWidget->setPixmap(SystemStatusBadPix);
    }
}


