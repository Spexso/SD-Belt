#ifndef SYSTEMINFORETRIEVER_H
#define SYSTEMINFORETRIEVER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QLabel>

#define PercentSign "%"
#define BaseUrl "https://sdbelt.devodev.online/api/v1"
#define AccessSystemInfoUrl "/system/info"

class SystemInfoRetriever : public QObject
{
    Q_OBJECT

public:

    explicit SystemInfoRetriever(QNetworkAccessManager *manager, QLabel* CpuTemperatureValue, QLabel* CpuUsageValue, QLabel* RamUsageValue, QObject *parent = nullptr);
    void fetch(const QString &token);

    inline double GetCpuUtilization() { return CpuUtilization;}
    inline double GetRamUtilization() { return RamUtilization;}
    inline double GetCpuTemperature() { return CpuTemperature;}

private slots:

    void OnSystemInfoReceived();

protected:

    void SetCpuUtilization(double NewCpuUtilization);
    void SetRamUtilization(double NewRamUtilization);
    void SetCpuTemperature(double NewCpuTemperature);

    void SetOverlayCpuUtilization();
    void SetOverlayRamUtilization();
    void SetOverlayCpuTemperature();

private:

    QNetworkAccessManager *network;
    QNetworkReply *reply;

    QLabel* CpuTemperatureWidget;
    QLabel* CpuUsageWidget;
    QLabel* RamUsageWidget;

    double CpuUtilization;
    double RamUtilization;
    double CpuTemperature;

    QTimer *timer = nullptr;
    QString currentToken;

signals:
};

#endif // SYSTEMINFORETRIEVER_H
