#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "qnetworkaccessmanager.h"
#include <QPushButton>
// Logs
#include "Logs.h"


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
    void SetupHeader();
    void SetupLogoOverlay();
    void SetupSystemStatusOverlay(QFile &HeaderStyle);
    void SetupDashboardOverlay(QFile &HeaderStyle);

private:

    Ui::MainWindow *ui;
    QNetworkAccessManager *NetworkManager;
    QVector<QFile*> ResourceStyles;

    Logs *logs;


    void SetupDebug();
private slots:

    void updateCameraView();
    void OnAngleTextEditChanged();
    bool eventFilter(QObject* obj, QEvent* event);
    void SetupLogs();
    void setActiveButton(QPushButton *active);
    void OnSpeedAdjusted();
    void OnSpeedChanged(int value);

    void OnDashboardButtonClicked();
    void OnCamerasButtonClicked();
    void OnLogsButtonClicked();
    void OnDebugButtonClicked();

    void OnReverseTheFlowClicked();
    void OnEmergencyStopClicked();
    void OnNotifyAdminClicked();
};
#endif // MAINWINDOW_H
