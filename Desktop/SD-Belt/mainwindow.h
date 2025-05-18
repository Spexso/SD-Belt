#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "qnetworkaccessmanager.h"
#include <QPushButton>

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

private slots:

    void updateCameraView();

    void setActiveButton(QPushButton *active);
    void OnSpeedAdjusted();
    void OnSpeedChanged(int value);

    void OnDashboardButtonClicked();
    void OnCamerasButtonClicked();
    void OnLogsButtonClicked();

    void OnReverseTheFlowClicked();
    void OnEmergencyStopClicked();
    void OnNotifyAdminClicked();
};
#endif // MAINWINDOW_H
