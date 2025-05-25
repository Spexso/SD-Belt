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
    timer->start(2000);
}

void SystemInfoRetriever::fetch(const QString &token)
{
    QUrl url(BackendAddress AccessSystemInfoUrl);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    reply = network->get(request);
    connect(reply, &QNetworkReply::finished, this, &SystemInfoRetriever::OnSystemInfoReceived);
}

void SystemInfoRetriever::OnSystemInfoReceived()
{
    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    if (!root.contains("result") || !root["result"].isObject()) return;

    QJsonObject SystemInfo = root["result"].toObject();

    this->SetCpuUtilization(SystemInfo["cpuUsage"].toDouble());
    this->SetRamUtilization(SystemInfo["memoryUsage"].toDouble());
    this->SetCpuTemperature(SystemInfo["cpuTemperature"].toDouble());
    this->SetIsSytemOnline(SystemInfo["status"].toString() != "INACTIVE");

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


