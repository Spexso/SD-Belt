#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QPixmap>


#define ProjectName "SD-Belt"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    // Button Setup
    connect(ui->DashboardButton, &QPushButton::clicked, this, &MainWindow::OnDashboardButtonClicked);
    connect(ui->CamerasButton, &QPushButton::clicked, this, &MainWindow::OnCamerasButtonClicked);
    connect(ui->LogsButton, &QPushButton::clicked, this, &MainWindow::OnLogsButtonClicked);


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
    ui->SystemMessage->setStyleSheet(Style);
}

void MainWindow::OnDashboardButtonClicked()
{
    // Switch Page/Tab Logic
}


// ---- CAMERA SECTION ----
void MainWindow::OnCamerasButtonClicked()
{
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
}
