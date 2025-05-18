#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QPixmap>

#include "StyleLoader.h"

// Network
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>


#define ProjectName "SD-Belt"
#define ServerAddr "http://10.1.249.58"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    NetworkManager = new QNetworkAccessManager(this);

    // qDebug() << "Current working dir:" << QDir::currentPath();

    // Load QSS style file for verticalWidget
    QFile MainWindowStyle(":/styles/MainWindow.qss");
    MainWindowStyle.setObjectName("MainWindowStyle");

    QFile CentralWindowStyle(":/styles/CentralWindow.qss");
    CentralWindowStyle.setObjectName("CentralWindowStyle");

    ResourceStyles.append(&MainWindowStyle);
    ResourceStyles.append(&CentralWindowStyle);

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

    SetupHeader();
    SetupLogoOverlay();
    loadGlobalStyles();

    QFile menuStyle(":/styles/MenuButton.qss");
    if (menuStyle.open(QFile::ReadOnly | QFile::Text))
    {
        QString style = menuStyle.readAll();

        ui->DashboardButton->setStyleSheet(style);
        ui->CamerasButton->setStyleSheet(style);
        ui->LogsButton->setStyleSheet(style);
    }

    // Button Setup
    connect(ui->DashboardButton, &QPushButton::clicked, this, &MainWindow::OnDashboardButtonClicked);
    connect(ui->CamerasButton, &QPushButton::clicked, this, &MainWindow::OnCamerasButtonClicked);
    connect(ui->LogsButton, &QPushButton::clicked, this, &MainWindow::OnLogsButtonClicked);
    connect(ui->ReverseButton, &QPushButton::clicked, this, &MainWindow::OnReverseTheFlowClicked);
    connect(ui->NotifyButton, &QPushButton::clicked, this, &MainWindow::OnNotifyAdminClicked);
    connect(ui->EmergencyStopButton, &QPushButton::clicked, this, &MainWindow::OnEmergencyStopClicked);

    QLinearGradient gradient(0, 0, 100, 0); // horizontal gradient
    gradient.setColorAt(0.0, QColor(255, 0, 0, 0));    // Fully transparent red
    gradient.setColorAt(1.0, QColor(255, 0, 0, 255));  // Fully opaque red

    QBrush transparentBrush(gradient);
    // Populating Log, dummy
    for(int i = 0; i < 20; i++)
    {
        QListWidgetItem *item = new QListWidgetItem(ui->LogList);
        item->setText("Raspberry pi cpu load exceed %93, overheat alert!");
        item->setBackground(transparentBrush);
        ui->LogList->addItem(item);
    }

    connect(ui->DashboardButton, &QPushButton::clicked, this, [=]() {
        ui->StackedWidget->setCurrentIndex(0); // Go to first page
    });

    connect(ui->CamerasButton, &QPushButton::clicked, this, [=]() {
        ui->StackedWidget->setCurrentIndex(1); // Go to second page
    });

    connect(ui->LogsButton, &QPushButton::clicked, this, [=]() {
        ui->StackedWidget->setCurrentIndex(2); // Go to third page
    });

    connect(ui->SpeedAdjuster, &QSlider::sliderReleased, this, &MainWindow::OnSpeedAdjusted);
    connect(ui->SpeedAdjuster, &QSlider::valueChanged, this, &MainWindow::OnSpeedChanged);

    ui->DashboardButton->click();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupHeader()
{
    QFile HeaderStyle(":/styles/Header.qss");
    if (HeaderStyle.open(QFile::ReadOnly | QFile::Text))
    {
        QString style = HeaderStyle.readAll();
        SetupLogoOverlay();
        SetupSystemStatusOverlay(HeaderStyle);
        ui->Header->setStyleSheet(style);
    }

    HeaderStyle.close();
}

void MainWindow::setActiveButton(QPushButton *active)
{
    QList<QPushButton*> buttons = { ui->DashboardButton, ui->CamerasButton, ui->LogsButton };
    for (QPushButton *btn : buttons)
        btn->setStyleSheet(btn == active ? MenuButtonSelectedStyle : MenuButtonStyle);
}

void MainWindow::SetupLogoOverlay()
{
    // Setup Logo
    QPixmap LogoPix(":/styles/img/Logo.png");
    QSizePolicy Policy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->Logo->setSizePolicy(Policy);
    int w = ui->Logo->width();
    int h = ui->Logo->height();
    ui->Logo->setPixmap(LogoPix.scaled(w, h, Qt::KeepAspectRatio));

    // Setup Name
    QFile TextStyle(":/styles/TextStyles.qss");
    if (TextStyle.open(QFile::ReadOnly | QFile::Text))
    {
        QString Style = TextStyle.readAll();
        ui->Name->setStyleSheet(Style);
        ui->Name->setText(ProjectName);
    }

    TextStyle.close();
}

void MainWindow::SetupSystemStatusOverlay(QFile& HeaderStyle)
{
    // Setup System Status
    QString Style = HeaderStyle.readAll();
    ui->SystemIndicator->setStyleSheet(Style);
    //ui->SystemMessage->setStyleSheet(Style);
}

void MainWindow::SetupDashboardOverlay(QFile& HeaderStyle)
{
    // Setup System Status
    QString Style = HeaderStyle.readAll();
    ui->SystemIndicator->setStyleSheet(Style);
    ui->SystemMessage->setStyleSheet(Style);
}

void MainWindow::OnDashboardButtonClicked()
{
    // Switch Page/Tab Logic
    setActiveButton(ui->DashboardButton);
}


// ---- CAMERA SECTION ----
void MainWindow::OnCamerasButtonClicked()
{
    setActiveButton(ui->CamerasButton);

    // Camera URL's
    cameraUrls = {
        "ws://localhost:8765",
        "ws://192.168.1.101:8765"
    };

    QLayout *layoutBase = ui->cameraView->layout();
    QVBoxLayout *layout = nullptr;

    if (!layoutBase) {
        layout = new QVBoxLayout(ui->cameraView);
        ui->cameraView->setLayout(layout);
    } else {
        layout = qobject_cast<QVBoxLayout*>(layoutBase);
        if (!layout) {
            delete layoutBase;
            layout = new QVBoxLayout(ui->cameraView);
            ui->cameraView->setLayout(layout);
        }
    }

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    QWidget *rowContainer = new QWidget(this);
    QHBoxLayout *rowLayout = new QHBoxLayout(rowContainer);

    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(0);

    prevButton = new QPushButton("◀", this);
    nextButton = new QPushButton("▶", this);
    webcam = new WebcamWidget(cameraUrls[currentCameraIndex], this);

    connect(prevButton, &QPushButton::clicked, this, [=]() {
        currentCameraIndex = (currentCameraIndex - 1 + cameraUrls.size()) % cameraUrls.size();
        updateCameraView();
    });

    connect(nextButton, &QPushButton::clicked, this, [=]() {
        currentCameraIndex = (currentCameraIndex + 1) % cameraUrls.size();
        updateCameraView();
    });

    rowLayout->addWidget(prevButton);
    rowLayout->addWidget(webcam);
    rowLayout->addWidget(nextButton);

    QHBoxLayout *containerLayout = new QHBoxLayout();
    containerLayout->addStretch(1);
    containerLayout->addWidget(rowContainer);
    containerLayout->addStretch(1);

    layout->addLayout(containerLayout);
}

void MainWindow::updateCameraView()
{
    if (!webcam || cameraUrls.isEmpty()) return;

    QWidget *rowContainer = webcam->parentWidget();
    QHBoxLayout *rowLayout = qobject_cast<QHBoxLayout*>(rowContainer->layout());
    if (!rowLayout) return;

    int index = rowLayout->indexOf(webcam);

    rowLayout->removeWidget(webcam);
    webcam->deleteLater();

    webcam = new WebcamWidget(cameraUrls[currentCameraIndex], this);

    rowLayout->insertWidget(index, webcam);
}

void MainWindow::OnLogsButtonClicked()
{
    // Switch Page/Tab Logic
    setActiveButton(ui->LogsButton);
}

void MainWindow::OnReverseTheFlowClicked()
{
    QUrl postUrl(QString("%1:8080/rev").arg(ServerAddr));
    QNetworkRequest postRequest(postUrl);
    postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    // Send the REV command to reverse motor direction
    QNetworkReply *postReply = NetworkManager->post(postRequest, "REV\n");
    connect(postReply, &QNetworkReply::finished, this, [=]() {
        QString response = postReply->readAll();
        qDebug() << "[POST /echo] Response:" << response;
        postReply->deleteLater();
    });

    ui->SpeedPercent->setText(QString("% %1").arg(0));
    ui->SpeedAdjuster->setValue(0);
}

void MainWindow::OnEmergencyStopClicked()
{
     QUrl postUrl(QString("%1:8080/stop").arg(ServerAddr));
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

        QUrl postUrl(QString("%1:8080/speed").arg(ServerAddr));
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
    if(ui->SpeedAdjuster)
    {
        ui->SpeedPercent->setText(QString("% %1").arg(value));
    }
}

void MainWindow::OnNotifyAdminClicked()
{
    // Send a POST request
    //QNetworkRequest postRequest(postUrl);
    //postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    //QNetworkReply *postReply = NetworkManager->post(postRequest, "Hello from Qt client");
    //connect(postReply, &QNetworkReply::finished, this, [=]() {
    //    QString response = postReply->readAll();
    //    qDebug() << "[POST /echo] Response:" << response;
    //    postReply->deleteLater();
    //});
}
