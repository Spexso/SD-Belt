#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "qnetworkaccessmanager.h"
#include <QPushButton>
#include "Logs.h"
#include "SystemInfoRetriever.h"
#include "SystemLogRetriever.h"
#include "Globals.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:

    // Style
    void SetupLogo();

    /* Common */
    void setActiveButton(QPushButton *active);
    bool eventFilter(QObject* obj, QEvent* event);

    /* Debug */
    void SetupDebug();
    void OnAngleTextEditChanged();

    /* Cameras */
    void updateCameraView();

    /* Logs */
    void SetupLogs();

    /* Tab Widget */
    void OnDashboardButtonClicked();
    void OnCamerasButtonClicked();
    void OnLogsButtonClicked();
    void OnDebugButtonClicked();

    /* Dashboard */
    void OnReverseTheFlowClicked();
    void OnEmergencyStopClicked();
    void OnNotifyAdminClicked();
    void OnSpeedAdjusted();
    void OnSpeedChanged(int value);

private:

    QVector<QFile*> ResourceStyles;

private:

    QString MenuButtonSelectedStyle;
    QString MenuButtonStyle;
    Ui::MainWindow *ui;

private:

    QNetworkAccessManager *NetworkManager;
    SystemInfoRetriever *SystemInfo;
    SystemLogRetriever *SystemLog;
    Logs *logs;

    QTimer *speedPostTimer;
    bool speedLocked = false;
};
#endif // MAINWINDOW_H
