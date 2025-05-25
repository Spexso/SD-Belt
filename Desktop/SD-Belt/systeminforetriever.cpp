#include "SystemInfoRetriever.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

SystemInfoRetriever::SystemInfoRetriever(QNetworkAccessManager *manager, QLabel* CpuTemperatureValue, QLabel* CpuUsageValue, QLabel* RamUsageValue, QObject *parent)
    : QObject{parent}, network(manager), CpuTemperatureWidget(CpuTemperatureValue), CpuUsageWidget(CpuUsageValue), RamUsageWidget(RamUsageValue)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        this->fetch(currentToken);
    });
    timer->start(2000);
}

void SystemInfoRetriever::fetch(const QString &token)
{
    QUrl url(BaseUrl AccessSystemInfoUrl);

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

    for (const QJsonValue &entry : SystemInfo)
    {
        QJsonObject Obj = entry.toObject();
        this->SetCpuUtilization(Obj["cpuUsage"].toDouble());
        this->SetRamUtilization(Obj["memoryUsage"].toDouble());
        this->SetCpuTemperature(Obj["cpuTemperature"].toDouble());
    }

    SetOverlayCpuTemperature();
    SetOverlayCpuUtilization();
    SetOverlayRamUtilization();
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
        QString PercentText = PercentSign % QString::number(this->GetCpuUtilization());
        CpuUsageWidget->setText(PercentText);
    }
}

void SystemInfoRetriever::SetOverlayRamUtilization()
{
    if(RamUsageWidget)
    {
        QString PercentText = PercentSign % QString::number(this->GetRamUtilization());
        RamUsageWidget->setText(PercentText);
    }
}

void SystemInfoRetriever::SetOverlayCpuTemperature()
{
    if(CpuTemperatureWidget)
    {
        QString PercentText = PercentSign % QString::number(this->GetCpuTemperature());
        CpuTemperatureWidget->setText(PercentText);
    }
}

void SystemInfoRetriever::SetOverlayIsSystemOnline()
{

}


