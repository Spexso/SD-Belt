#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Globals.h"

#include <QFile>
#include <QDebug>
#include <QDir>
#include <QPixmap>

// Network
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

// QUrl getUrl(QString("%1%2").arg(BackendAddress).arg(SystemLogs));
// QNetworkRequest getRequest(getUrl);

// QNetworkReply* getReply = NetworkManager->get(getRequest);
// connect(getReply, &QNetworkReply::finished, this, [=]() {
//     if (getReply->error() != QNetworkReply::NoError)
//     {
//         qWarning() << "[GET /logs/system] Failed:" << getReply->errorString();
//         getReply->deleteLater();
//         return;
//     }

//     QByteArray response = getReply->readAll();
//     qDebug() << "[GET /logs/system] Response:" << response;
//     getReply->deleteLater();
// });

int lastDialValue = 180;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    NetworkManager = new QNetworkAccessManager(this);

    // Logs
    logs = new Logs(NetworkManager, ui->MainLogs, ui->ItemsPerMinute,  this);
    logs->setCountLabel(ui->label_19);
    logs->setSpeedLabel(ui->ItemsPerMinute);
    // logs->fetch("547b62e5-8fa8-4f33-a8b2-9cf8de4b97ba");

    // System Info
    SystemInfo = new SystemInfoRetriever(NetworkManager, ui->CpuTemperatureValue, ui->CPUUsageValue, ui->RamUsageValue, ui->SystemIndicator, this);

    // System Log
    SystemLog = new SystemLogRetriever(NetworkManager, ui->LogList, this);

    // TEST START
    speedPostTimer = new QTimer(this);
    speedPostTimer->setInterval(1000);
    speedPostTimer->setSingleShot(true);

    connect(speedPostTimer, &QTimer::timeout, this, [=]() {
        speedLocked = false;
    });

    // Load QSS style file for verticalWidget
    QFile MainWindowStyle(":/styles/MainWindow.qss");
    MainWindowStyle.setObjectName("MainWindowStyle");

    QFile CentralWindowStyle(":/styles/CentralWindow.qss");
    CentralWindowStyle.setObjectName("CentralWindowStyle");

    QFile MenuButton(":/styles/MenuButton.qss");
    MenuButton.setObjectName("MenuButton");

    QFile MenuButtonSelected(":/styles/MenuButtonSelected.qss");
    MenuButtonSelected.setObjectName("MenuButtonSelected");

    QFile Header(":/styles/Header.qss");
    Header.setObjectName("Header");

    QFile TextStyle(":/styles/TextStyles.qss");
    TextStyle.setObjectName("TextStyle");

    ResourceStyles.append(&MainWindowStyle);
    ResourceStyles.append(&CentralWindowStyle);
    ResourceStyles.append(&MenuButton);
    ResourceStyles.append(&MenuButtonSelected);
    ResourceStyles.append(&Header);
    ResourceStyles.append(&TextStyle);

    for (QFile* StyleFile : std::as_const(ResourceStyles))
    {
        if (StyleFile->open(QFile::ReadOnly | QFile::Text))
        {
            QString name = StyleFile->objectName();
            QString style = StyleFile->readAll();

            if (name == "MainWindowStyle")
            {
                ui->MainBackground->setStyleSheet(style);
            }
            else if (name == "CentralWindowStyle")
            {
                ui->centralwidget->setStyleSheet(style);
            }
            else if(name == "MenuButton")
            {
                ui->DashboardButton->setStyleSheet(style);
                ui->CamerasButton->setStyleSheet(style);
                ui->LogsButton->setStyleSheet(style);
                ui->DebugButton->setStyleSheet(style);
                MenuButtonStyle = style;
            }
            else if(name == "MenuButtonSelected")
            {
                MenuButtonSelectedStyle = style;
            }
            else if(name == "Header")
            {
                SetupLogo();
                ui->SystemIndicator->setStyleSheet(style);
                ui->Header->setStyleSheet(style);
            }
            else if(name == "TextStyle")
            {
                ui->Name->setStyleSheet(style);
                ui->Name->setText(ProjectName);
            }
            else
            {
                qDebug() << "Failed to match with any QSS File from resource:" << StyleFile->errorString();
            }

            StyleFile->close();
        }
        else
        {
            qDebug() << "Failed to load QSS from resource:" << StyleFile->errorString();
        }
    }

    SetupDebug();

    // Button Setup
    connect(ui->DashboardButton, &QPushButton::clicked, this, &MainWindow::OnDashboardButtonClicked);
    connect(ui->CamerasButton, &QPushButton::clicked, this, &MainWindow::OnCamerasButtonClicked);
    connect(ui->LogsButton, &QPushButton::clicked, this, &MainWindow::OnLogsButtonClicked);
    connect(ui->ReverseButton, &QPushButton::clicked, this, &MainWindow::OnReverseTheFlowClicked);
    // connect(ui->NotifyButton, &QPushButton::clicked, this, &MainWindow::OnNotifyAdminClicked);
    connect(ui->EmergencyStopButton, &QPushButton::clicked, this, &MainWindow::OnEmergencyStopClicked);
    connect(ui->DebugButton, &QPushButton::clicked, this, &MainWindow::OnDebugButtonClicked);

    // Click Bind
    connect(ui->DashboardButton, &QPushButton::clicked, this, [=]() {
        ui->StackedWidget->setCurrentIndex(0); // Go to first page
    });

    connect(ui->CamerasButton, &QPushButton::clicked, this, [=]() {
        ui->StackedWidget->setCurrentIndex(1); // Go to second page
    });

    connect(ui->LogsButton, &QPushButton::clicked, this, [=]() {
        ui->StackedWidget->setCurrentIndex(2); // Go to third page
    });

    connect(ui->DebugButton, &QPushButton::clicked, this, [=]() {
        ui->StackedWidget->setCurrentIndex(3); // Go to fourth page
    });

    connect(ui->SpeedAdjuster, &QSlider::sliderReleased, this, &MainWindow::OnSpeedAdjusted);
    connect(ui->SpeedAdjuster, &QSlider::valueChanged, this, &MainWindow::OnSpeedChanged);

    ui->DashboardButton->click();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setActiveButton(QPushButton *active)
{
    QList<QPushButton*> buttons = { ui->DashboardButton, ui->CamerasButton, ui->LogsButton, ui->DebugButton };
    for (QPushButton *btn : buttons)
        btn->setStyleSheet(btn == active ? MenuButtonSelectedStyle : MenuButtonStyle);
}

void MainWindow::SetupDebug()
{
    /*      connect(ui->ServoAngleText, &QTextEdit::textChanged,
            this, &MainWindow::OnAngleTextEditChanged);
    */
    ui->ServoAngleText->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->ServoAngleText && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            QString input = ui->ServoAngleText->toPlainText().trimmed();
            bool ok = false;
            int value = input.toInt(&ok);

            if (ok && value >= 0 && value <= 360)
            {
                qDebug() << "Accepted angle:" << value;
                lastDialValue = value;
            }
            else
            {
                qDebug() << "Invalid angle input:" << input;
                // optionally show error or reset
            }

            return true; // stop propagation
        }
    }

    return QMainWindow::eventFilter(obj, event); // default handling
}


void MainWindow::OnAngleTextEditChanged()
{
    QString input = ui->ServoAngleText->toPlainText().trimmed();

    bool ok = false;
    int value = input.toInt(&ok);

    if (ok && value >= 0 && value <= 360)
    {
        qDebug() << "Valid angle:" << value;
        // Proceed with applying the angle (e.g. to a motor)
    }
    else
    {
        // Optionally reject the change visually or ignore it
        qDebug() << "Invalid input:" << input;
    }
}


void MainWindow::SetupLogo()
{
    // Setup Logo
    QPixmap LogoPix(":/styles/img/Logo.png");
    QSizePolicy Policy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->Logo->setSizePolicy(Policy);
    int w = ui->Logo->width();
    int h = ui->Logo->height();
    ui->Logo->setPixmap(LogoPix.scaled(w, h, Qt::KeepAspectRatio));
}

void MainWindow::OnDashboardButtonClicked()
{
    // Switch Page/Tab Logic
    setActiveButton(ui->DashboardButton);
}

void MainWindow::OnDebugButtonClicked()
{
    // Switch Page/Tab Logic
    setActiveButton(ui->DebugButton);
}

// CAMERA
void MainWindow::OnCamerasButtonClicked()
{
    setActiveButton(ui->CamerasButton);
}

void MainWindow::OnLogsButtonClicked()
{
    // Switch Page/Tab Logic
    setActiveButton(ui->LogsButton);
}

void MainWindow::OnReverseTheFlowClicked()
{
    QUrl postUrl(QString("%1%2").arg(BackendAddress).arg(SystemReversePoint));
    QNetworkRequest postRequest(postUrl);
    postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    // Send the Reverse command to motor
    QNetworkReply *postReply = NetworkManager->post(postRequest, "REV\n");
    connect(postReply, &QNetworkReply::finished, this, [=]() {
        QString response = postReply->readAll();
        qDebug() << "[POST /echo] Response:" << response;
        postReply->deleteLater();
    });
}

void MainWindow::OnEmergencyStopClicked()
{
    QUrl postUrl(QString("%1%2").arg(BackendAddress).arg(SystemStopPoint));
    QNetworkRequest postRequest(postUrl);
    postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    // Send the STOP command for immediate emergency stop
    QNetworkReply *postReply = NetworkManager->post(postRequest, "STOP\n");
    connect(postReply, &QNetworkReply::finished, this, [=]() {
        QString response = postReply->readAll();
        qDebug() << "[POST /echo] Response:" << response;
        postReply->deleteLater();
    });

     ui->SpeedPercent->setText(QString("% %1").arg(0));
     ui->SpeedAdjuster->setValue(0);
}

void MainWindow::OnSpeedAdjusted()
{
    if(ui->SpeedAdjuster)
    {
        int value = ui->SpeedAdjuster->value();
        QString command = QString::number(value);

        QUrl postUrl(QString("%1%2").arg(BackendAddress).arg(SystemSpeedPoint));
        QNetworkRequest postRequest(postUrl);
        postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

        QNetworkReply *postReply = NetworkManager->post(postRequest, command.toUtf8());
        connect(postReply, &QNetworkReply::finished, this, [=]() {
            QString response = postReply->readAll();
            qDebug() << "[POST /speed] Response:" << response;
            postReply->deleteLater();
        });

        ui->SpeedPercent->setText(QString("% %1").arg(value));
    }
}

void MainWindow::OnSpeedChanged(int value)
{
    if (!ui->SpeedAdjuster)
        return;

    ui->SpeedPercent->setText(QString("% %1").arg(value));

    // Check if Speed change available
    if (speedLocked)
        return;

    // Start the timer, lock
    speedLocked = true;
    speedPostTimer->start();

    OnSpeedAdjusted(); // Change the speed
}
