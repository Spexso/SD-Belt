#ifndef WEBCAMWIDGET_H
#define WEBCAMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QWebSocket>

class WebcamWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WebcamWidget(const QString &url, QWidget *parent = nullptr);

private slots:
    void onFrameReceived(const QString &base64Data);

private:
    QLabel *videoLabel;
    QWebSocket socket;
};

#endif // WEBCAMWIDGET_H
