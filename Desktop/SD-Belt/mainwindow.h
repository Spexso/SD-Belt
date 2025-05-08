#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "webcamwidget.h"
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
    QVector<QFile*> ResourceStyles;

    // Camera
    WebcamWidget *webcam = nullptr;
    QStringList cameraUrls;
    int currentCameraIndex = 0;
    QPushButton *prevButton = nullptr;
    QPushButton *nextButton = nullptr;
    void updateCameraView();


private slots:

    void OnDashboardButtonClicked();
    void OnCamerasButtonClicked();
    void OnLogsButtonClicked();
};
#endif // MAINWINDOW_H
