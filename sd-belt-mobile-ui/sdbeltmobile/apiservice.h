#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QVariantMap>
#include <QVariantList>

class ApiService : public QObject
{
    Q_OBJECT
public:
    explicit ApiService(QObject *parent = nullptr);

    // --- Authentication ---
    Q_INVOKABLE void login(const QString &username, const QString &password);

    // --- Other API Methods ---
    Q_INVOKABLE void fetchProducts();
    Q_INVOKABLE void fetchSystemInfo();
    Q_INVOKABLE void startSystem();
    Q_INVOKABLE void stopSystem();
    Q_INVOKABLE void reverseSystem();
    Q_INVOKABLE void fetchScans(int limit = 20);
    Q_INVOKABLE void fetchScansForRange(const QString &startDate, const QString &endDate, const QString &productId = "", const QString &requestContext = "");
    Q_INVOKABLE void fetchScanStatistics(const QString &startDateIso, const QString &endDateIso, const QString &requestContext = "", const QString &productId = "");
    Q_INVOKABLE void fetchProductDetails(const QString &productId);
    Q_INVOKABLE void setSystemAccuracy(int accuracy);
    Q_INVOKABLE QString getProductIcon(const QString &productIdOrImageId);

signals:
    // --- Authentication Signals ---
    void loginSuccess(const QString &token, const QString &message);
    void loginFailed(const QString &message);

    // --- Other Signals ---
    void productsFetched(const QVariantList &products);
    void systemInfoFetched(const QVariantMap &systemInfo);
    void systemCommandSuccess(const QString &command, const QString &message);
    void systemCommandError(const QString &command, const QString &errorMsg);
    void scansFetched(const QVariantList &scans);
    void scansForRangeFetched(const QVariantList &scans, const QString &requestContext);
    void scanStatisticsFetched(const QVariantMap &statistics, const QString &requestContext);
    void productDetailsFetched(const QVariantMap &productDetails);
    void networkError(const QString &endpoint, const QString &errorString);
    void generalMessage(const QString &type, const QString &message);

private slots:
    // --- Authentication Slot ---
    void onLoginReplyFinished(QNetworkReply *reply);

    // --- Other Reply Slots ---
    void onProductsReplyFinished(QNetworkReply *reply);
    void onSystemInfoReplyFinished(QNetworkReply *reply);
    void onStartSystemReplyFinished(QNetworkReply *reply);
    void onStopSystemReplyFinished(QNetworkReply *reply);
    void onReverseSystemReplyFinished(QNetworkReply *reply);
    void onScansReplyFinished(QNetworkReply *reply);
    void onScanStatisticsReplyFinished(QNetworkReply *reply);
    void onProductDetailsReplyFinished(QNetworkReply *reply);
    void onSetSystemAccuracyReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_baseUrl;
    QVariantList m_productListCache;
    QVariantMap getProductById(const QString &productId);
    // handleNetworkError is primarily for logging, parseReply is for status/result structure
    void handleNetworkError(QNetworkReply *reply, const QString &endpointContext);
    QJsonObject parseReply(QNetworkReply *reply, const QString &endpointContext);
    bool m_isRestarting = false;
    // QString m_authToken; // to store the auth token
};

#endif // APISERVICE_H
