// apiservice.cpp
#include "apiservice.h"
#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimeZone>
#include <QUrlQuery>

// Constructor
ApiService::ApiService(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_baseUrl = "https://sdbelt.devodev.online/api/v1";
    //m_baseUrl = "http://172.20.10.6:6060/api/v1";
    qDebug() << "ApiService initialized. Base URL:" << m_baseUrl;
}

// --- Authentication ---
void ApiService::login(const QString &username, const QString &password) {
    qDebug() << "ApiService: login called for username:" << username;
    QJsonObject jsonPayload;
    jsonPayload["username"] = username;
    jsonPayload["password"] = password;
    QJsonDocument jsonDoc(jsonPayload);
    QByteArray jsonData = jsonDoc.toJson();

    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/login")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));

    QNetworkReply *reply = m_networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onLoginReplyFinished(reply);
    });
}


// --- Public Invokable Methods ---
void ApiService::fetchProducts() {
    qDebug() << "ApiService: fetchProducts called";
    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/products")));
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onProductsReplyFinished(reply);
    });
}

void ApiService::fetchSystemInfo() {
    qDebug() << "ApiService: fetchSystemInfo called";
    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/system/info")));
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSystemInfoReplyFinished(reply);
    });
}

void ApiService::startSystem() {
    qDebug() << "ApiService: startSystem called (POST)";
    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/system/start")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, QByteArrayLiteral("POST"), QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onStartSystemReplyFinished(reply);
    });
}

void ApiService::stopSystem() {
    qDebug() << "ApiService: stopSystem called (POST)";
    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/system/stop")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, QByteArrayLiteral("POST"), QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onStopSystemReplyFinished(reply);
    });
}

void ApiService::reverseSystem() {
    qDebug() << "ApiService: reverseSystem called (POST)";
    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/system/reverse")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, QByteArrayLiteral("POST"), QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReverseSystemReplyFinished(reply);
    });
}

void ApiService::fetchScans(int limit) {
    qDebug() << "ApiService: fetchScans called with limit:" << limit;
    QUrl url(m_baseUrl + QStringLiteral("/scans"));
    if (limit > 0) {
        QUrlQuery query;
        query.addQueryItem("limit", QString::number(limit));
        url.setQuery(query);
    }
    qDebug() << "ApiService: fetchScans URL:" << url.toString();
    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onScansReplyFinished(reply);
    });
}

QString formatDateTimeForApi(const QDateTime& dt) {
    if (!dt.isValid()) return QString();
    return dt.toString("yyyy-MM-ddTHH:mm:ss");
}


void ApiService::fetchScansForRange(const QString &startDateStr, const QString &endDateStr, const QString &productId, const QString &requestContext) {
    qDebug() << "ApiService: fetchScansForRange called. Start:" << startDateStr << "End:" << endDateStr << "ProductID:" << productId << "Context:" << requestContext;
    QUrl url(m_baseUrl + QStringLiteral("/scans"));
    QUrlQuery query;

    if (!startDateStr.isEmpty()) query.addQueryItem(QStringLiteral("startDate"), QUrl::toPercentEncoding(startDateStr));
    if (!endDateStr.isEmpty()) query.addQueryItem(QStringLiteral("endDate"), QUrl::toPercentEncoding(endDateStr));

    url.setQuery(query);
    qDebug() << "ApiService: fetchScansForRange URL:" << url.toString();

    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);

    QString effectiveContext = requestContext;
    if (effectiveContext.isEmpty()) {
        effectiveContext = QStringLiteral("scansForRange_") + QString::number(QDateTime::currentMSecsSinceEpoch());
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, productId, effectiveContext]() {
        QJsonObject jsonObj = parseReply(reply, "/scans (for range " + effectiveContext + ")");
        if (jsonObj.isEmpty() && reply->error() != QNetworkReply::NoError) {
            qWarning() << "ApiService: fetchScansForRange - jsonObj is empty and there was a network error for context:" << effectiveContext;
            reply->deleteLater();
            return;
        }
        if (jsonObj.isEmpty() || !jsonObj.contains(QStringLiteral("result"))) {
            qWarning() << "ApiService: fetchScansForRange - jsonObj is empty or missing 'result' field for context:" << effectiveContext;
            emit scansForRangeFetched(QVariantList(), effectiveContext);
            reply->deleteLater();
            return;
        }

        QVariantList allScans = jsonObj.value(QStringLiteral("result")).toArray().toVariantList();
        QVariantList resultScans;

        for (const QVariant& scanVariant : std::as_const(allScans)) {
            QVariantMap scanMap = scanVariant.toMap();
            if (productId.isEmpty() || scanMap.value(QStringLiteral("productId")).toString() == productId) {
                QVariantMap product = getProductById(scanMap.value(QStringLiteral("productId")).toString());
                scanMap["productName"] = product.value(QStringLiteral("name"), scanMap.value(QStringLiteral("productId")).toString());
                QString imageIdForIcon = product.contains(QStringLiteral("imageId")) ? product.value(QStringLiteral("imageId")).toString() : scanMap.value(QStringLiteral("productId")).toString();
                scanMap["productIcon"] = getProductIcon(imageIdForIcon);
                resultScans.append(scanMap);
            }
        }
        emit scansForRangeFetched(resultScans, effectiveContext);
        reply->deleteLater();
    });
}

void ApiService::fetchScanStatistics(const QString &startDateIso, const QString &endDateIso, const QString &requestContext, const QString &productId) {
    qDebug() << "ApiService: fetchScanStatistics called. StartISO:" << startDateIso << "EndISO:" << endDateIso << "Context:" << requestContext << "ProductID:" << productId;

    if (startDateIso.isEmpty() || endDateIso.isEmpty()){
        qWarning() << "ApiService: fetchScanStatistics - startDate or endDate is empty. Aborting.";
        emit scanStatisticsFetched(QVariantMap(), requestContext.isEmpty() ? QStringLiteral("scanStats_empty_date") : requestContext);
        return;
    }

    QDateTime startDt = QDateTime::fromString(startDateIso, Qt::ISODate);
    QDateTime endDt = QDateTime::fromString(endDateIso, Qt::ISODate);

    if (!startDt.isValid() || !endDt.isValid()) {
        qWarning() << "ApiService: fetchScanStatistics - Invalid date string provided. Start:" << startDateIso << "End:" << endDateIso;
        emit scanStatisticsFetched(QVariantMap(), requestContext.isEmpty() ? QStringLiteral("scanStats_invalid_date") : requestContext);
        return;
    }

    QString formattedStartDate = formatDateTimeForApi(startDt);
    QString formattedEndDate = formatDateTimeForApi(endDt);

    QUrl url(m_baseUrl + QStringLiteral("/scans/statistics"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("startDate"), QUrl::toPercentEncoding(formattedStartDate));
    query.addQueryItem(QStringLiteral("endDate"), QUrl::toPercentEncoding(formattedEndDate));
    if (!productId.isEmpty()) {
        query.addQueryItem(QStringLiteral("productId"), QUrl::toPercentEncoding(productId));
    }
    url.setQuery(query);
    qDebug() << "ApiService: fetchScanStatistics Formatted URL:" << url.toString();

    QString effectiveContext = requestContext;
    if (effectiveContext.isEmpty()) {
        effectiveContext = QStringLiteral("scanStats_") + (productId.isEmpty() ? "" : productId + "_") + QString::number(QDateTime::currentMSecsSinceEpoch());
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("requestContext", effectiveContext);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onScanStatisticsReplyFinished(reply);
    });
}

void ApiService::fetchProductDetails(const QString &productId) {
    qDebug() << "ApiService: fetchProductDetails called for productId:" << productId;
    if (productId.isEmpty()) {
        qWarning() << "ApiService: fetchProductDetails called with empty productId.";
        emit productDetailsFetched(QVariantMap());
        return;
    }
    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/product/") + productId));
    qDebug() << "ApiService: fetchProductDetails URL:" << request.url().toString();
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onProductDetailsReplyFinished(reply);
    });
}

void ApiService::setSystemAccuracy(int accuracy) {
    qDebug() << "ApiService: setSystemAccuracy called with accuracy:" << accuracy;


    QUrl url(m_baseUrl + QStringLiteral("/system/threshold"));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("text/plain"));

    // Convert accuracy to double and send as raw body
    double accuracyDouble = static_cast<double>(accuracy);
    QByteArray bodyData = QByteArray::number(accuracyDouble, 'f', 1);

    QNetworkReply *reply = m_networkManager->post(request, bodyData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSetSystemAccuracyReplyFinished(reply);
    });
}

QString ApiService::getProductIcon(const QString &productIdOrImageId) {
    // 1. Direct ID check
    QString upperIdOrImageId = productIdOrImageId.toUpper();
    if (upperIdOrImageId == QLatin1String("APPLE")) return QStringLiteral("🍎");
    //if (upperIdOrImageId == QLatin1String("BANANA")) return QStringLiteral("🍌");
    if (upperIdOrImageId == QLatin1String("POTATO")) return QStringLiteral("🥔");
    if (upperIdOrImageId == QLatin1String("ORANGE")) return QStringLiteral("🍊");

    // 2. Check against productListCache using the 'id' field
    for (const QVariant& productVar : std::as_const(m_productListCache)) {
        QVariantMap productMap = productVar.toMap();
        QString currentProductId = productMap.value(QStringLiteral("id")).toString();
        QString currentImageId = productMap.value(QStringLiteral("imageId")).toString();

        if (currentProductId.compare(productIdOrImageId, Qt::CaseInsensitive) == 0 ||
            currentImageId.compare(productIdOrImageId, Qt::CaseInsensitive) == 0) {
            QString actualMatchedIdUpper = currentProductId.toUpper();
            if (actualMatchedIdUpper == QLatin1String("APPLE")) return QStringLiteral("🍎");
            //if (actualMatchedIdUpper == QLatin1String("BANANA")) return QStringLiteral("🍌");
            if (actualMatchedIdUpper == QLatin1String("POTATO")) return QStringLiteral("🥔");
            if (actualMatchedIdUpper == QLatin1String("ORANGE")) return QStringLiteral("🍊");
        }
    }

    // 3. Fallback based on keywords in imageId if it wasn't a direct ID match
    if (productIdOrImageId.contains(QLatin1String("apple"), Qt::CaseInsensitive)) return QStringLiteral("🍎");
    //if (productIdOrImageId.contains(QLatin1String("banana"), Qt::CaseInsensitive)) return QStringLiteral("🍌");
    if (productIdOrImageId.contains(QLatin1String("banana"), Qt::CaseInsensitive)) return QStringLiteral("🥔");
    if (productIdOrImageId.contains(QLatin1String("orange"), Qt::CaseInsensitive)) return QStringLiteral("🍊");
    // Default fallback icon if no specific match found
    return QStringLiteral("📦");
}

// --- Private Slots (Reply Handlers) ---

void ApiService::onLoginReplyFinished(QNetworkReply *reply) {
    QString endpointContext = "/login";
    qDebug() << "ApiService: onLoginReplyFinished for" << endpointContext;

    if (!reply) {
        qWarning() << "ApiService: onLoginReplyFinished - Null QNetworkReply for" << endpointContext;
        emit networkError(endpointContext, "Internal error: Null network reply object.");
        emit loginFailed("Application error during login."); // Generic app error
        return;
    }

    QByteArray responseData = reply->readAll();
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug().noquote() << "ApiService: Raw response for" << endpointContext << "HTTP Status:" << httpStatusCode << "\n" << QString::fromUtf8(responseData);

    if (reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::AuthenticationRequiredError) {
        QString errorMessage = reply->errorString() + " (HTTP " + QString::number(httpStatusCode) + ")";
        qWarning() << "ApiService: Network error for" << endpointContext << "-"
                   << "Qt Error:" << reply->error() << "(" << reply->errorString() << ")";

        emit networkError(endpointContext, errorMessage);
        emit loginFailed("Network error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj;
    QString messageFromApi;

    if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        jsonObj = jsonDoc.object();
        qDebug().noquote() << "ApiService: Parsed JSON for" << endpointContext << ":\n" << QJsonDocument(jsonObj).toJson(QJsonDocument::Indented);
        if (jsonObj.contains(QStringLiteral("message")) && jsonObj.value(QStringLiteral("message")).isString()) {
            messageFromApi = jsonObj.value(QStringLiteral("message")).toString();
        }
    } else if (httpStatusCode != 200) {
        qWarning() << "ApiService: Failed to parse JSON response for" << endpointContext << "and HTTP status is" << httpStatusCode;

        QString failureMessage = "Server returned an unreadable error response.";
        if (httpStatusCode == 401) {
            failureMessage = "Invalid username or password.";
        }
        emit networkError(endpointContext, "Unreadable error response from server (HTTP " + QString::number(httpStatusCode) + ").");
        emit loginFailed(failureMessage);
        reply->deleteLater();
        return;
    }


    // --- Specific Login Logic based on HTTP Status Code ---
    if (httpStatusCode == 200) { // Successful Login
        if (jsonObj.contains(QStringLiteral("token")) && jsonObj.value(QStringLiteral("token")).isString()) {
            QString token = jsonObj.value(QStringLiteral("token")).toString();
            // m_authToken = token; // Store token for future authorized requests
            qInfo() << "ApiService: Login successful.";
            emit loginSuccess(token, messageFromApi.isEmpty() ? "Login successful" : messageFromApi);
        } else {
            qWarning() << "ApiService: Login HTTP 200 but 'token' missing/invalid in response for" << endpointContext;
            emit loginFailed("Login successful, but server response was incomplete (missing token).");
        }
    } else if (httpStatusCode == 401 || (reply->error() == QNetworkReply::AuthenticationRequiredError && httpStatusCode == 0) ) {
        qWarning() << "ApiService: Login failed (Invalid Credentials or Auth Required) for" << endpointContext << "HTTP Status:" << httpStatusCode << "Message from API:" << messageFromApi;
        emit loginFailed(messageFromApi.isEmpty() ? "Invalid username or password." : messageFromApi);
    } else {
        // Other HTTP errors
        qWarning() << "ApiService: Login failed for" << endpointContext << "HTTP Status:" << httpStatusCode << "Message from API:" << messageFromApi;
        QString generalErrorMessage = "Login failed: " + (messageFromApi.isEmpty() ? ("Server error (HTTP " + QString::number(httpStatusCode) + ")") : messageFromApi);
        emit loginFailed(generalErrorMessage);
        if (reply->error() == QNetworkReply::NoError) {
            emit networkError(endpointContext, "API Error (HTTP " + QString::number(httpStatusCode) + "): " + (messageFromApi.isEmpty() ? "Failed to login." : messageFromApi) );
        }
    }

    reply->deleteLater();
}


void ApiService::onProductsReplyFinished(QNetworkReply *reply) {
    qDebug() << "ApiService: onProductsReplyFinished";
    QJsonObject jsonObj = parseReply(reply, QStringLiteral("/products"));
    if (jsonObj.isEmpty() && reply->error() != QNetworkReply::NoError) {
        reply->deleteLater(); return;
    }
    if (jsonObj.isEmpty() || !jsonObj.contains(QStringLiteral("result"))) {
        qWarning() << "ApiService: onProductsReplyFinished - jsonObj is empty or missing 'result' field.";
        emit productsFetched(QVariantList());
        reply->deleteLater();
        return;
    }

    m_productListCache = jsonObj.value(QStringLiteral("result")).toArray().toVariantList();
    QVariantList enrichedProducts;
    for (const QVariant& productVar : std::as_const(m_productListCache)) {
        QVariantMap productMap = productVar.toMap();
        QString imageId = productMap.value(QStringLiteral("imageId")).toString();
        if (imageId.isEmpty()) {
            imageId = productMap.value(QStringLiteral("id")).toString();
        }
        productMap["iconChar"] = getProductIcon(imageId);
        enrichedProducts.append(productMap);
    }
    emit productsFetched(enrichedProducts);
    reply->deleteLater();
}

void ApiService::onSystemInfoReplyFinished(QNetworkReply *reply) {
    qDebug() << "ApiService: onSystemInfoReplyFinished";
    QJsonObject jsonObj = parseReply(reply, QStringLiteral("/system/info"));
    if (jsonObj.isEmpty() && reply->error() != QNetworkReply::NoError) {
        reply->deleteLater(); return;
    }
    if (jsonObj.isEmpty() || !jsonObj.contains(QStringLiteral("result"))) {
        qWarning() << "ApiService: onSystemInfoReplyFinished - jsonObj is empty or missing 'result' field.";
        emit systemInfoFetched(QVariantMap());
        reply->deleteLater();
        return;
    }
    emit systemInfoFetched(jsonObj.value(QStringLiteral("result")).toObject().toVariantMap());
    reply->deleteLater();
}

void ApiService::onStartSystemReplyFinished(QNetworkReply *reply) {
    qDebug() << "ApiService: onStartSystemReplyFinished (POST)";
    QJsonObject jsonObj = parseReply(reply, QStringLiteral("/system/start (POST)"));
    if (reply->error() == QNetworkReply::NoError && jsonObj.value(QStringLiteral("status")).toInt() == 200) {
        QString successMsg = jsonObj.value(QStringLiteral("result")).toString();
        if (successMsg.isEmpty()) successMsg = QStringLiteral("System started.");
        emit systemCommandSuccess(QStringLiteral("start"), successMsg);
        emit generalMessage("success", "System started successfully.");
        if (m_isRestarting) {
            m_isRestarting = false;
            emit generalMessage("success", "System restarted successfully.");
        }
        fetchSystemInfo();
    } else {
        QString errorMsg;
        if (jsonObj.contains(QStringLiteral("result")) && jsonObj.value(QStringLiteral("result")).isString()) errorMsg = jsonObj.value(QStringLiteral("result")).toString();
        else if (jsonObj.contains(QStringLiteral("error")) && jsonObj.value(QStringLiteral("error")).isString()) errorMsg = jsonObj.value(QStringLiteral("error")).toString();
        else if (jsonObj.contains(QStringLiteral("message")) && jsonObj.value(QStringLiteral("message")).isString()) errorMsg = jsonObj.value(QStringLiteral("message")).toString();
        else if (reply->error() != QNetworkReply::NoError) errorMsg = reply->errorString();
        else errorMsg = QStringLiteral("Unknown error starting system. API Status: ") + QString::number(jsonObj.value(QStringLiteral("status")).toInt(-1));

        emit systemCommandError(QStringLiteral("start"), errorMsg);
        emit generalMessage("error", "Failed to start system: " + errorMsg);
        m_isRestarting = false;
    }
    reply->deleteLater();
}

void ApiService::onStopSystemReplyFinished(QNetworkReply *reply) {
    qDebug() << "ApiService: onStopSystemReplyFinished (POST)";
    QJsonObject jsonObj = parseReply(reply, QStringLiteral("/system/stop (POST)"));
    if (reply->error() == QNetworkReply::NoError && jsonObj.value(QStringLiteral("status")).toInt() == 200) {
        QString successMsg = jsonObj.value(QStringLiteral("result")).toString();
        if (successMsg.isEmpty()) successMsg = QStringLiteral("System stopped.");
        emit systemCommandSuccess(QStringLiteral("stop"), successMsg);
        if (m_isRestarting) {
            emit generalMessage("info", "Restarting system: Starting...");
            startSystem();
        } else {
            emit generalMessage("success", "System stopped successfully.");
        }
        fetchSystemInfo();
    } else {
        QString errorMsg;
        if (jsonObj.contains(QStringLiteral("result")) && jsonObj.value(QStringLiteral("result")).isString()) errorMsg = jsonObj.value(QStringLiteral("result")).toString();
        else if (jsonObj.contains(QStringLiteral("error")) && jsonObj.value(QStringLiteral("error")).isString()) errorMsg = jsonObj.value(QStringLiteral("error")).toString();
        else if (jsonObj.contains(QStringLiteral("message")) && jsonObj.value(QStringLiteral("message")).isString()) errorMsg = jsonObj.value(QStringLiteral("message")).toString();
        else if (reply->error() != QNetworkReply::NoError) errorMsg = reply->errorString();
        else errorMsg = QStringLiteral("Unknown error stopping system. API Status: ") + QString::number(jsonObj.value(QStringLiteral("status")).toInt(-1));

        emit systemCommandError(QStringLiteral("stop"), errorMsg);
        emit generalMessage("error", "Failed to stop system: " + errorMsg);
        if(m_isRestarting) {
            emit generalMessage("error", "Restart failed: Could not stop system properly.");
            m_isRestarting = false;
        }
    }
    reply->deleteLater();
}

void ApiService::onReverseSystemReplyFinished(QNetworkReply *reply) {
    qDebug() << "ApiService: onReverseSystemReplyFinished (POST)";
    QJsonObject jsonObj = parseReply(reply, QStringLiteral("/system/reverse (POST)"));
    if (reply->error() == QNetworkReply::NoError && jsonObj.value(QStringLiteral("status")).toInt() == 200) {
        QString successMsg = jsonObj.value(QStringLiteral("result")).toString();
        if (successMsg.isEmpty()) successMsg = QStringLiteral("System reversed.");
        emit systemCommandSuccess(QStringLiteral("reverse"), successMsg);
        emit generalMessage("success", "System reversed successfully.");
        fetchSystemInfo();
    } else {
        QString errorMsg;
        if (jsonObj.contains(QStringLiteral("result")) && jsonObj.value(QStringLiteral("result")).isString()) errorMsg = jsonObj.value(QStringLiteral("result")).toString();
        else if (jsonObj.contains(QStringLiteral("error")) && jsonObj.value(QStringLiteral("error")).isString()) errorMsg = jsonObj.value(QStringLiteral("error")).toString();
        else if (jsonObj.contains(QStringLiteral("message")) && jsonObj.value(QStringLiteral("message")).isString()) errorMsg = jsonObj.value(QStringLiteral("message")).toString();
        else if (reply->error() != QNetworkReply::NoError) errorMsg = reply->errorString();
        else errorMsg = QStringLiteral("Unknown error reversing system. API Status: ") + QString::number(jsonObj.value(QStringLiteral("status")).toInt(-1));

        emit systemCommandError(QStringLiteral("reverse"), errorMsg);
        emit generalMessage("error", "Failed to reverse system: " + errorMsg);
    }
    reply->deleteLater();
}

void ApiService::onScansReplyFinished(QNetworkReply *reply) {
    qDebug() << "ApiService: onScansReplyFinished";
    QJsonObject jsonObj = parseReply(reply, QStringLiteral("/scans"));
    if (jsonObj.isEmpty() && reply->error() != QNetworkReply::NoError) {
        reply->deleteLater(); return;
    }
    if (jsonObj.isEmpty() || !jsonObj.contains(QStringLiteral("result"))) {
        qWarning() << "ApiService: onScansReplyFinished - jsonObj is empty or missing 'result' field.";
        emit scansFetched(QVariantList());
        reply->deleteLater();
        return;
    }

    QVariantList scans = jsonObj.value(QStringLiteral("result")).toArray().toVariantList();
    QVariantList enrichedScans;
    for (const QVariant& scanVariant : std::as_const(scans)) {
        QVariantMap scanMap = scanVariant.toMap();
        QVariantMap product = getProductById(scanMap.value(QStringLiteral("productId")).toString());
        scanMap["productName"] = product.value(QStringLiteral("name"), scanMap.value(QStringLiteral("productId")).toString());
        QString imageIdForIcon = product.contains(QStringLiteral("imageId")) ? product.value(QStringLiteral("imageId")).toString() : scanMap.value(QStringLiteral("productId")).toString();
        scanMap["productIcon"] = getProductIcon(imageIdForIcon);
        enrichedScans.append(scanMap);
    }
    emit scansFetched(enrichedScans);
    reply->deleteLater();
}

void ApiService::onScanStatisticsReplyFinished(QNetworkReply *reply) {
    QString requestContext = reply->property("requestContext").toString();
    qDebug() << "ApiService: onScanStatisticsReplyFinished for context:" << requestContext;

    QJsonObject jsonObj = parseReply(reply, QStringLiteral("/scans/statistics (context: ") + requestContext + QStringLiteral(")"));

    if (jsonObj.isEmpty() && reply->error() != QNetworkReply::NoError) {
        emit scanStatisticsFetched(QVariantMap(), requestContext);
        reply->deleteLater();
        return;
    }
    if (jsonObj.isEmpty() || !jsonObj.contains(QStringLiteral("result"))) {
        qWarning() << "ApiService: onScanStatisticsReplyFinished - jsonObj from parseReply is empty or missing 'result' for context:" << requestContext;
        if(reply->error() == QNetworkReply::NoError && jsonObj.value("status").toInt() == 200 && !jsonObj.contains("result")) {
            emit scanStatisticsFetched(QVariantMap(), requestContext);
        } else if (reply->error() != QNetworkReply::NoError && jsonObj.isEmpty()){
            emit scanStatisticsFetched(QVariantMap(), requestContext);
        }
        reply->deleteLater();
        return;
    }

    QVariantMap stats = jsonObj.value(QStringLiteral("result")).toObject().toVariantMap();

    if (stats.contains(QStringLiteral("productStatistics"))) {
        QVariantList prodStatsList = stats.value(QStringLiteral("productStatistics")).toList();
        QVariantList enrichedProdStatsList;
        for (const QVariant& prodStatVar : std::as_const(prodStatsList)) {
            QVariantMap prodStatMap = prodStatVar.toMap();
            QVariantMap product = getProductById(prodStatMap.value(QStringLiteral("productId")).toString());
            prodStatMap["productName"] = product.value(QStringLiteral("name"), prodStatMap.value(QStringLiteral("productId")).toString());
            QString imageIdForIcon = product.contains(QStringLiteral("imageId")) ? product.value(QStringLiteral("imageId")).toString() : prodStatMap.value(QStringLiteral("productId")).toString();
            prodStatMap["productIcon"] = getProductIcon(imageIdForIcon);
            enrichedProdStatsList.append(prodStatMap);
        }
        stats["productStatistics"] = enrichedProdStatsList;
    }
    emit scanStatisticsFetched(stats, requestContext);
    reply->deleteLater();
}

void ApiService::onProductDetailsReplyFinished(QNetworkReply *reply) {
    QString productIdFromUrl = reply->url().fileName();
    qDebug() << "ApiService: onProductDetailsReplyFinished for product (from URL):" << productIdFromUrl;
    QJsonObject jsonObj = parseReply(reply, QStringLiteral("/product/") + productIdFromUrl);
    if (jsonObj.isEmpty() && reply->error() != QNetworkReply::NoError) {
        reply->deleteLater(); return;
    }
    if (jsonObj.isEmpty() || !jsonObj.contains(QStringLiteral("result"))) {
        qWarning() << "ApiService: onProductDetailsReplyFinished - jsonObj is empty or missing 'result' field for product:" << productIdFromUrl;
        emit productDetailsFetched(QVariantMap());
        reply->deleteLater();
        return;
    }
    QVariantMap productMap = jsonObj.value(QStringLiteral("result")).toObject().toVariantMap();
    QString imageId = productMap.value(QStringLiteral("imageId")).toString();
    if (imageId.isEmpty()) {
        imageId = productMap.value(QStringLiteral("id")).toString();
    }
    productMap["iconChar"] = getProductIcon(imageId);
    emit productDetailsFetched(productMap);
    reply->deleteLater();
}

void ApiService::onSetSystemAccuracyReplyFinished(QNetworkReply *reply) {
    qDebug() << "ApiService: onSetSystemAccuracyReplyFinished";
    QString endpointContext = "/system/threshold (POST)";
    QJsonObject jsonObj = parseReply(reply, endpointContext);

    if (reply->error() == QNetworkReply::NoError && jsonObj.value(QStringLiteral("status")).toInt() == 200) {
        QString successMsg = jsonObj.value(QStringLiteral("result")).toString();
        if (successMsg.isEmpty()) successMsg = QStringLiteral("System accuracy updated.");
        emit systemCommandSuccess(QStringLiteral("setAccuracy"), successMsg);
        emit generalMessage("success", "System accuracy updated successfully.");
        fetchSystemInfo(); // Refresh system info to reflect the change
    } else {
        QString errorMsg;
        if (jsonObj.contains(QStringLiteral("result")) && jsonObj.value(QStringLiteral("result")).isString()) errorMsg = jsonObj.value(QStringLiteral("result")).toString();
        else if (jsonObj.contains(QStringLiteral("error")) && jsonObj.value(QStringLiteral("error")).isString()) errorMsg = jsonObj.value(QStringLiteral("error")).toString();
        else if (jsonObj.contains(QStringLiteral("message")) && jsonObj.value(QStringLiteral("message")).isString()) errorMsg = jsonObj.value(QStringLiteral("message")).toString();
        else if (reply->error() != QNetworkReply::NoError) errorMsg = reply->errorString();
        else errorMsg = QStringLiteral("Unknown error setting accuracy. API Status: ") + QString::number(jsonObj.value(QStringLiteral("status")).toInt(-1));

        emit systemCommandError(QStringLiteral("setAccuracy"), errorMsg);
        emit generalMessage("error", "Failed to set system accuracy: " + errorMsg);
        fetchSystemInfo();
    }
    reply->deleteLater();
}

// --- Private Helper Methods ---
QVariantMap ApiService::getProductById(const QString &productId) {
    for (const QVariant &item : std::as_const(m_productListCache)) {
        QVariantMap product = item.toMap();
        if (product.value(QStringLiteral("id")).toString() == productId) {
            return product;
        }
    }
    qWarning() << "ApiService: Product with ID" << productId << "not found in cache.";
    return QVariantMap();
}

void ApiService::handleNetworkError(QNetworkReply *reply, const QString &endpointContext) {
    QString errorString = reply ? reply->errorString() : "Null reply object encountered.";
    int statusCode = reply ? reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() : 0;
    qWarning() << "ApiService: Network error on" << endpointContext << ":" << errorString
               << "HTTP Status:" << statusCode;
}

QJsonObject ApiService::parseReply(QNetworkReply *reply, const QString &endpointContext) {
    qDebug() << "ApiService: Parsing reply for" << endpointContext;
    if (!reply) {
        qWarning() << "ApiService: parseReply - Null QNetworkReply for" << endpointContext;
        emit networkError(endpointContext, "Internal error: Null network reply object.");
        emit generalMessage("error", "Application Error (" + endpointContext + "): Network reply was unexpectedly null.");
        return QJsonObject();
    }

    QByteArray responseData = reply->readAll();
    QString responseString = QString::fromUtf8(responseData);
    if (responseString.length() < 1000) {
        qDebug().noquote() << "ApiService: Raw response for" << endpointContext << "HTTP Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << "\n" << responseString;
    } else {
        qDebug().noquote() << "ApiService: Raw response for" << endpointContext << "HTTP Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << "\n" << responseString.left(1000) << "... (truncated)";
    }


    if (reply->error() != QNetworkReply::NoError) {
        QString httpStatus = QString::number(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
        QString errorMessage = reply->errorString() + " (HTTP " + httpStatus + ")";
        qWarning() << "ApiService: Network error for" << endpointContext << "-"
                   << "Qt Error:" << reply->error() << "(" << reply->errorString() << ")"
                   << "HTTP Status:" << httpStatus;

        QJsonDocument errorJsonDoc = QJsonDocument::fromJson(responseData);
        QString apiErrorMsg;
        if (!errorJsonDoc.isNull() && errorJsonDoc.isObject()) {
            QJsonObject errObj = errorJsonDoc.object();
            if (errObj.contains("message") && errObj["message"].isString()) apiErrorMsg = errObj["message"].toString();
            else if (errObj.contains("error") && errObj["error"].isString()) apiErrorMsg = errObj["error"].toString();
            else if (errObj.contains("detail") && errObj["detail"].isString()) apiErrorMsg = errObj["detail"].toString();
            if (!apiErrorMsg.isEmpty()) errorMessage += " - Server: " + apiErrorMsg;
        }

        emit networkError(endpointContext, errorMessage);
        emit generalMessage("error", "Network Error (" + endpointContext + "): " + errorMessage);
        return errorJsonDoc.object();
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qWarning() << "ApiService: Failed to parse JSON response for" << endpointContext << ". Response was:" << responseData.constData();
        emit networkError(endpointContext, "Invalid JSON response from server.");
        emit generalMessage("error", "Application Error (" + endpointContext + "): Could not understand server response (not valid JSON object).");
        return QJsonObject();
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (endpointContext != "/login") {
        if (!jsonObj.contains(QStringLiteral("status"))) {
            qWarning() << "ApiService: JSON response for" << endpointContext << "is missing 'status' field.";
            emit networkError(endpointContext, "Malformed API response: 'status' field missing.");
            emit generalMessage("error", "Application Error (" + endpointContext + "): Malformed server response (missing 'status').");
            return jsonObj;
        }

        int apiStatus = jsonObj.value(QStringLiteral("status")).toInt(-1);

        if (apiStatus != 200) {
            QString errorMsgDetail;
            if (jsonObj.contains(QStringLiteral("result")) && jsonObj.value(QStringLiteral("result")).isString()) {
                errorMsgDetail = jsonObj.value(QStringLiteral("result")).toString();
            } else if (jsonObj.contains(QStringLiteral("error")) && jsonObj.value(QStringLiteral("error")).isString()) {
                errorMsgDetail = jsonObj.value(QStringLiteral("error")).toString();
            } else if (jsonObj.contains(QStringLiteral("message")) && jsonObj.value(QStringLiteral("message")).isString()) {
                errorMsgDetail = jsonObj.value(QStringLiteral("message")).toString();
            } else if (jsonObj.contains(QStringLiteral("detail")) && jsonObj.value(QStringLiteral("detail")).isString()) {
                errorMsgDetail = jsonObj.value(QStringLiteral("detail")).toString();
            } else {
                errorMsgDetail = "Unspecified API error. Full response: " + QString(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact)).left(150);
            }
            QString fullErrorMsg = "API Error on " + endpointContext + " (API Status: " + QString::number(apiStatus) + "): " + errorMsgDetail;
            qWarning() << "ApiService:" << fullErrorMsg;
            emit networkError(endpointContext, fullErrorMsg);
            emit generalMessage("error", "Server Error (" + endpointContext + ", Status " + QString::number(apiStatus) + "): " + errorMsgDetail);
            return jsonObj; // Return the JSON object, it contains the error details
        }

        if (!jsonObj.contains(QStringLiteral("result"))) {
            qWarning() << "ApiService: API response OK (status 200) but 'result' field is missing for" << endpointContext;
            emit networkError(endpointContext, "API success status but 'result' field missing.");
            emit generalMessage("error", "Application Error (" + endpointContext + "): Malformed success response from server (missing 'result').");
            return jsonObj;
        }
    }
    return jsonObj;
}
