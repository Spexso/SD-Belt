// ProductsPage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Item {
    id: productsRoot
    anchors.fill: parent

    property color accent: window.accent
    property color borderColor: window.borderColor
    property color heading: window.heading
    property color subtext: window.subtext
    property color success: window.success
    property color error: window.error

    // --- Data Models & Properties ---
    property ListModel allProductsListModel: ListModel {}
    property ListModel todayScansModel: ListModel {}
    property ListModel yesterdayScansModel: ListModel {}
    property var rawProductList: []

    // --- Properties for Statistics Tab ---
    property var overallStats: ({
        successRate: 0,
        totalScanned: 0,
        totalSuccess: 0,
        totalFailed: 0,
        productStatistics: []
    })
    property ListModel productScanAmountsModel: ListModel {}
    property ListModel overallWeeklyPerformanceModel: ListModel {}
    property bool isLoadingOverallStats: true
    property bool isLoadingOverallWeeklyChart: true

    property int overallWeeklyChartDaysLoaded: 0
    property var overallDailyStatsCollector: [null, null, null, null, null, null, null]
    property int productColorsIndex: 0
    readonly property var predefinedBarColors: ["#4CAF50", "#2196F3", "#FF9800", "#9C27B0", "#FF5722", "#607D8B", "#E91E63", "#795548", "#00BCD4", "#8BC34A"]


    // --- Loading States for other tabs ---
    property bool isLoadingAllProducts: true
    property bool isLoadingScanStatsForAllProducts: true
    property bool isLoadingTodayScans: true
    property bool isLoadingYesterdayScans: true


    Component.onCompleted: {
        console.log("ProductsPage: Component.onCompleted");
        if (rawProductList.length === 0 && isLoadingAllProducts) {
            apiService.fetchProducts();
        }
        handleTabChange(productTabs.currentIndex);
    }

    Connections {
        target: apiService

        function onProductsFetched(products) {
            console.log("ProductsPage: Products fetched, count:", products.length);
            rawProductList = products;
            isLoadingAllProducts = false;

            if (productTabs.currentIndex === 0) {
                if (isLoadingScanStatsForAllProducts || overallStats.productStatistics.length === 0) {
                    var veryStartDate = "2000-01-01T00:00:00Z";
                    var farFutureDate = "2099-12-31T23:59:59Z";
                    apiService.fetchScanStatistics(veryStartDate, farFutureDate, "allProductsTabStats");
                    isLoadingScanStatsForAllProducts = true;
                } else {
                    populateAllProductsListModel();
                }
            } else if (productTabs.currentIndex === 2 && !isLoadingOverallStats) {
                populateProductScanAmountsModel();
            }
        }

        function onScanStatisticsFetched(stats, context) {
            console.log("ProductsPage: Scan statistics fetched for context:", context, "Data keys:", Object.keys(stats));

            if (Object.keys(stats).length === 0 && !context.startsWith("overallStatWeekDay_")) {
                console.warn("ProductsPage: Received empty/invalid statistics object for context:", context);
                if (context === "allProductsTabStats") {
                    isLoadingScanStatsForAllProducts = false;
                    populateAllProductsListModel();
                } else if (context === "statisticsTabOverall") {
                    isLoadingOverallStats = false;
                    overallStats = { successRate: 0, productStatistics: [], totalScanned: 0, totalSuccess:0, totalFailed:0 };
                    productScanAmountsModel.clear();
                    overallWeeklyPerformanceModel.clear();
                    isLoadingOverallWeeklyChart = false;
                }
                return;
            }

            if (context === "allProductsTabStats") {
                console.log("ProductsPage: Stats for 'All Products' tab list counts context:", context);
                if (stats.productStatistics) {
                    overallStats.productStatistics = stats.productStatistics;
                }
                isLoadingScanStatsForAllProducts = false;
                populateAllProductsListModel();

            } else if (context === "statisticsTabOverall") {
                console.log("ProductsPage: Stats for 'Statistics Tab Overall' context:", context);
                overallStats = stats;
                isLoadingOverallStats = false;
                populateProductScanAmountsModel();

                if (overallWeeklyPerformanceModel.count === 0 && isLoadingOverallWeeklyChart) {
                    loadOverallWeeklyPerformanceChartData();
                } else if (!isLoadingOverallWeeklyChart && overallWeeklyPerformanceModel.count === 0 && overallStats.totalScanned > 0) {
                     loadOverallWeeklyPerformanceChartData();
                } else if (overallStats.totalScanned === 0){
                    isLoadingOverallWeeklyChart = false;
                    overallWeeklyPerformanceModel.clear();
                }

            } else if (context.startsWith("overallStatWeekDay_")) {
                if (!isLoadingOverallWeeklyChart) return;

                var expectedPrefix = "overallStatWeekDay_";
                var dayIndexString = context.substring(expectedPrefix.length);
                var dayIndex = parseInt(dayIndexString);

                if (isNaN(dayIndex) || dayIndex < 0 || dayIndex >= 7 || overallDailyStatsCollector[dayIndex] !== null) {
                     var firstNullSlotError = overallDailyStatsCollector.indexOf(null);
                     if (firstNullSlotError !== -1 && (isNaN(dayIndex) || dayIndex < 0 || dayIndex >= 7)) {
                        overallDailyStatsCollector[firstNullSlotError] = { success: 0, failed: 0, total: 0, error: true };
                        overallWeeklyChartDaysLoaded++;
                     }
                    if (overallWeeklyChartDaysLoaded === 7) finishOverallWeeklyChartLoad();
                    return;
                }

                if (stats && stats.totalScanned !== undefined) {
                    overallDailyStatsCollector[dayIndex] = {
                        success: stats.totalSuccess || 0,
                        failed: stats.totalFailed || 0,
                        total: stats.totalScanned || 0,
                        error: false
                    };
                } else {
                    overallDailyStatsCollector[dayIndex] = { success: 0, failed: 0, total: 0, error: true };
                }
                overallWeeklyChartDaysLoaded++;
                if (overallWeeklyChartDaysLoaded === 7) {
                    finishOverallWeeklyChartLoad();
                }
            }
        }
        function finishOverallWeeklyChartLoad() {
            processAndDisplayOverallWeeklyChart();
            isLoadingOverallWeeklyChart = false;
        }

        function onScansForRangeFetched(scans, context) {
            if (context === "todayScans") {
                todayScansModel.clear();
                scans.forEach(function(scan) {
                    todayScansModel.append({ product: scan.productName || "Unknown", time: formatTimeOnly(scan.timestamp), success: scan.isSuccess, iconChar: scan.productIcon || "📷" });
                });
                isLoadingTodayScans = false;
            } else if (context === "yesterdayScans") {
                yesterdayScansModel.clear();
                scans.forEach(function(scan) {
                    yesterdayScansModel.append({ product: scan.productName || "Unknown", time: formatTimeOnly(scan.timestamp), success: scan.isSuccess, iconChar: scan.productIcon || "📷" });
                });
                isLoadingYesterdayScans = false;
            }
        }

        function onNetworkError(endpoint, errorString) {
            var contextFromErrorMatch = endpoint.match(/\(context: ([^)]+)\)/);
            var contextFromError = contextFromErrorMatch ? contextFromErrorMatch[1] : "";

            if (endpoint.includes("/products")) {
                isLoadingAllProducts = false;
                isLoadingScanStatsForAllProducts = false;
                rawProductList = [];
                populateAllProductsListModel();
                 if(productTabs.currentIndex === 2) {
                    isLoadingOverallStats = false;
                    productScanAmountsModel.clear();
                 }
            }
            if (contextFromError === "allProductsTabStats") {
                isLoadingScanStatsForAllProducts = false;
                populateAllProductsListModel();
            }
            if (contextFromError === "statisticsTabOverall") {
                isLoadingOverallStats = false;
                overallStats = { successRate: 0, productStatistics: [], totalScanned: 0, totalSuccess:0, totalFailed:0 };
                productScanAmountsModel.clear();
                isLoadingOverallWeeklyChart = false;
                overallWeeklyPerformanceModel.clear();
            }
            if (contextFromError.startsWith("overallStatWeekDay_") && isLoadingOverallWeeklyChart) {
                var expectedPrefix = "overallStatWeekDay_";
                var dayIndexString = contextFromError.substring(expectedPrefix.length);
                var dayIndex = parseInt(dayIndexString);

                if (isNaN(dayIndex) || dayIndex < 0 || dayIndex >= 7) {
                     var firstNullSlotNetErr = overallDailyStatsCollector.indexOf(null);
                     if (firstNullSlotNetErr !== -1) {
                        overallDailyStatsCollector[firstNullSlotNetErr] = { success: 0, failed: 0, total: 0, error: true };
                        overallWeeklyChartDaysLoaded++;
                     }
                } else if (overallDailyStatsCollector[dayIndex] === null) {
                    overallDailyStatsCollector[dayIndex] = { success: 0, failed: 0, total: 0, error: true };
                    overallWeeklyChartDaysLoaded++;
                }
                if (overallWeeklyChartDaysLoaded === 7) finishOverallWeeklyChartLoad();
            }
            if (endpoint.includes("/scans") && !endpoint.includes("statistics")) {
                if (contextFromError === "todayScans") isLoadingTodayScans = false;
                if (contextFromError === "yesterdayScans") isLoadingYesterdayScans = false;
            }
        }
    }

    function populateAllProductsListModel() {
        if (isLoadingAllProducts && rawProductList.length === 0) {
            return;
        }
        allProductsListModel.clear();

        (rawProductList || []).forEach(function(product) {
            var prodStat = null;
            if (!isLoadingScanStatsForAllProducts && overallStats.productStatistics && overallStats.productStatistics.length > 0) {
                prodStat = overallStats.productStatistics.find(function(stat) { return stat.productId === product.id; });
            }
            allProductsListModel.append({
                id: product.id,
                name: product.name || product.id,
                success: isLoadingScanStatsForAllProducts ? "..." : (prodStat ? (prodStat.totalSuccess || 0) : 0),
                error: isLoadingScanStatsForAllProducts ? "..." : (prodStat ? (prodStat.totalFailed || 0) : 0),
                iconChar: product.iconChar || "📦"
            });
        });
    }

    function populateProductScanAmountsModel() {
        productScanAmountsModel.clear();
        productColorsIndex = 0;

        if (isLoadingAllProducts || isLoadingOverallStats || !overallStats || !overallStats.productStatistics || overallStats.totalScanned === 0) {
            return;
        }

        var tempArray = [];
        overallStats.productStatistics.forEach(function(prodStat) {
            var productInfo = rawProductList.find(p => p.id === prodStat.productId);
            var percentage = (overallStats.totalScanned > 0) ? (prodStat.totalScanned / overallStats.totalScanned) : 0;
            tempArray.push({
                name: productInfo ? productInfo.name : prodStat.productId,
                scanCount: prodStat.totalScanned || 0,
                percentage: percentage,
            });
        });

        tempArray.sort(function(a, b) {
            return b.scanCount - a.scanCount;
        });

        tempArray.forEach(function(item) {
            productScanAmountsModel.append({
                name: item.name,
                scanCount: item.scanCount,
                percentage: item.percentage,
                color: predefinedBarColors[productColorsIndex % predefinedBarColors.length]
            });
            productColorsIndex++;
        });
    }


    function handleTabChange(index) {
        if (index === 0) {
            if (rawProductList.length === 0 && !isLoadingAllProducts) {
                isLoadingAllProducts = true;
                apiService.fetchProducts();
            } else if (rawProductList.length > 0 && (isLoadingScanStatsForAllProducts || allProductsListModel.count === 0)) {
                var veryStartDate = "2000-01-01T00:00:00Z";
                var farFutureDate = "2099-12-31T23:59:59Z";
                apiService.fetchScanStatistics(veryStartDate, farFutureDate, "allProductsTabStats");
                isLoadingScanStatsForAllProducts = true;
            }
        } else if (index === 1) {
            if (isLoadingTodayScans || todayScansModel.count === 0) fetchLatestScansForTab();
        } else if (index === 2) {
            if (isLoadingOverallStats || (!overallStats.totalScanned && overallStats.totalScanned !== 0)) {
                var veryStartDateOverall = "2000-01-01T00:00:00Z";
                var farFutureDateOverall = "2099-12-31T23:59:59Z";
                apiService.fetchScanStatistics(veryStartDateOverall, farFutureDateOverall, "statisticsTabOverall");
                isLoadingOverallStats = true;
            } else if (!isLoadingOverallStats && overallStats.totalScanned >= 0) {
                if (productScanAmountsModel.count === 0 && overallStats.productStatistics && overallStats.productStatistics.length > 0) {
                    populateProductScanAmountsModel();
                }
                if (isLoadingOverallWeeklyChart && overallWeeklyPerformanceModel.count === 0) {
                    loadOverallWeeklyPerformanceChartData();
                }
            }
        }
    }

    function fetchLatestScansForTab() {
        isLoadingTodayScans = true;
        isLoadingYesterdayScans = true;
        var today = new Date();
        var todayStart = new Date(today.getFullYear(), today.getMonth(), today.getDate(), 0, 0, 0);
        var todayEnd = new Date(today.getFullYear(), today.getMonth(), today.getDate(), 23, 59, 59, 999);
        var yesterday = new Date(today);
        yesterday.setDate(today.getDate() - 1);
        var yesterdayStart = new Date(yesterday.getFullYear(), yesterday.getMonth(), yesterday.getDate(), 0, 0, 0);
        var yesterdayEnd = new Date(yesterday.getFullYear(), yesterday.getMonth(), yesterday.getDate(), 23, 59, 59, 999);

        apiService.fetchScansForRange(todayStart.toISOString(), todayEnd.toISOString(), "", "todayScans");
        apiService.fetchScansForRange(yesterdayStart.toISOString(), yesterdayEnd.toISOString(), "", "yesterdayScans");
    }

    function formatTimeOnly(isoTimestamp) {
        if (!isoTimestamp) return "N/A";
        var date = new Date(isoTimestamp);
        return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', hour12: false });
    }

    function loadOverallWeeklyPerformanceChartData() {
        if (overallStats.totalScanned === 0 && !isLoadingOverallStats) {
            isLoadingOverallWeeklyChart = false;
            overallWeeklyPerformanceModel.clear();
            return;
        }
        if (!isLoadingOverallWeeklyChart && overallWeeklyPerformanceModel.count === 7) {
             return;
        }

        isLoadingOverallWeeklyChart = true;
        overallWeeklyChartDaysLoaded = 0;
        overallDailyStatsCollector = [null, null, null, null, null, null, null];
        overallWeeklyPerformanceModel.clear();

        var today = new Date();
        var currentDayOfWeek = today.getDay();
        var diffToMonday = (currentDayOfWeek === 0) ? -6 : (1 - currentDayOfWeek);
        var monday = new Date(today);
        monday.setDate(today.getDate() + diffToMonday);

        for (var i = 0; i < 7; i++) {
            var dayStart = new Date(monday);
            dayStart.setDate(monday.getDate() + i);
            dayStart.setHours(0, 0, 0, 0);
            var dayEnd = new Date(dayStart);
            dayEnd.setHours(23, 59, 59, 999);
            var context = "overallStatWeekDay_" + i;
            apiService.fetchScanStatistics(dayStart.toISOString(), dayEnd.toISOString(), context);
        }
    }
    function processAndDisplayOverallWeeklyChart() {
        overallWeeklyPerformanceModel.clear();
        var daysOfWeek = ["M", "T", "W", "Th", "F", "Sa", "Su"];

        for (var i = 0; i < 7; i++) {
            var dayStat = overallDailyStatsCollector[i];
            var successP = 0, errorP = 0, successC = 0, errorC = 0, totalC = 0;
            var dayHadError = (dayStat && dayStat.error) || false;
            var dayIsDataPresent = false;

            if (dayStat && !dayHadError && dayStat.total !== undefined) {
                totalC = dayStat.total;
                successC = dayStat.success;
                errorC = dayStat.failed;
                dayIsDataPresent = true;

                var sumSuccessError = successC + errorC;
                if (sumSuccessError > 0) {
                    successP = successC / sumSuccessError;
                    errorP = errorC / sumSuccessError;
                    var tempSum = successP + errorP;
                    if (tempSum > 1.0001) {
                        successP = successP / tempSum;
                        errorP = errorP / tempSum;
                    }
                }
            }
            overallWeeklyPerformanceModel.append({
                day: daysOfWeek[i], successPercent: Math.max(0, successP), errorPercent: Math.max(0, errorP),
                successCount: successC, errorCount: errorC, totalCount: totalC,
                hadError: dayHadError, isDataPresent: dayIsDataPresent
            });
        }
    }


    Rectangle { anchors.fill: parent; color: "#FFFBFF"; radius: 0 }

    ColumnLayout {
        id: body
        anchors.fill: parent; anchors.margins: 16; spacing: 16

        RowLayout { /* Header */
            Layout.fillWidth: true; spacing: 8
            Rectangle { width: 44; height: 44; radius: 8; color: "#F0F0F0"; Text { anchors.centerIn: parent; text: "←"; font.pixelSize: 24; color: heading } MouseArea { anchors.fill: parent; onClicked: { window.activePage = "home"; stackView.replace(homePage) } } }
            Text { text: "Products"; font.pixelSize: 20; font.bold: true; color: heading }
            Item { Layout.fillWidth: true }
        }

        TabBar {
            id: productTabs
            Layout.fillWidth: true
            currentIndex: window.selectedProductsTab
            onCurrentIndexChanged: {
                window.selectedProductsTab = currentIndex;
                handleTabChange(currentIndex);
            }
            TabButton { text: "All Products"; width: implicitWidth + 20; font.pixelSize: 14 }
            TabButton { text: "Latest Scans"; width: implicitWidth + 20; font.pixelSize: 14 }
            TabButton { text: "Statistics"; width: implicitWidth + 20; font.pixelSize: 14 }
        }

        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: productTabs.currentIndex

            // ---- ALL PRODUCTS TAB ----
            Item {
                id: allProductsTabContent
                Text {
                    anchors.centerIn: parent
                    text: (isLoadingAllProducts && allProductsListModel.count === 0) ? "Loading products..."
                                                                                      : (allProductsListModel.count === 0 && !isLoadingAllProducts ? "No products found." : "")
                    visible: (isLoadingAllProducts && allProductsListModel.count === 0) || (allProductsListModel.count === 0 && !isLoadingAllProducts)
                    color: subtext; font.italic: true
                }
                ListView {
                    id: allProductsListView
                    anchors.fill: parent; clip: true; spacing: 12
                    visible: allProductsListModel.count > 0
                    model: allProductsListModel
                    delegate: Rectangle {
                        width: parent.width; height: 80; radius: 12
                        border.width: 1; border.color: productsRoot.borderColor; color: "#FFFFFF"
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 12; spacing: 12
                            Rectangle { width: 56; height: 56; radius: 8; color: "#F5F5F5"; Text { anchors.centerIn: parent; text: model.iconChar; font.pixelSize: 24 } }
                            ColumnLayout { Layout.fillWidth: true; spacing: 2
                                Text { text: model.name; font.pixelSize: 16; font.bold: true; color: productsRoot.heading }
                                RowLayout { spacing: 12; Layout.topMargin: 4
                                    Row { spacing: 4; Rectangle { width: 10; height: 10; radius: 5; color: productsRoot.success } Text { text: model.success.toString(); font.pixelSize: 12; color: productsRoot.success } }
                                    Row { spacing: 4; Rectangle { width: 10; height: 10; radius: 5; color: productsRoot.error } Text { text: model.error.toString(); font.pixelSize: 12; color: productsRoot.error } }
                                }
                            }
                            Rectangle { width: 32; height: 32; radius: 16; color: "#F5F5F5"; Text { anchors.centerIn: parent; text: "→"; font.pixelSize: 16; color: productsRoot.accent }
                                MouseArea {
                                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        stackView.push(productStatsPageComponent, {
                                                           "p_productId": model.id,
                                                           "p_productName": model.name,
                                                           "p_productIcon": model.iconChar,
                                                           "p_successScans": (typeof model.success === "string" || model.success === undefined) ? 0 : model.success,
                                                           "p_errorScans": (typeof model.error === "string" || model.error === undefined) ? 0 : model.error
                                                       });
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ---- LATEST SCANS TAB ----
            Item {
                id: latestScansTabContent
                ColumnLayout {
                    anchors.fill: parent
                    Text {
                        text: isLoadingTodayScans && todayScansModel.count === 0 ? "Loading today's scans..." : (todayScansModel.count === 0 && !isLoadingTodayScans ? "No scans for today." : "")
                        visible: (isLoadingTodayScans && todayScansModel.count === 0) || (todayScansModel.count === 0 && !isLoadingTodayScans)
                        Layout.alignment: Qt.AlignHCenter
                        color: subtext
                        font.italic: true
                    }
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 8
                        visible: todayScansModel.count > 0
                        model: todayScansModel
                        delegate: Rectangle {
                            width: parent.width
                            height: 70
                            radius: 12
                            border.width: 1
                            border.color: productsRoot.borderColor
                            color: "#FFFFFF"
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12
                                Rectangle {
                                    width: 46
                                    height: 46
                                    radius: 23
                                    color: "#F5F5F5"
                                    Text {
                                        anchors.centerIn: parent
                                        text: model.iconChar
                                        font.pixelSize: 20
                                    }
                                }
                                Column {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: model.product
                                        font.pixelSize: 16
                                        color: productsRoot.heading
                                    }
                                    Text {
                                        text: "Scanned at " + model.time
                                        font.pixelSize: 12
                                        color: productsRoot.subtext
                                    }
                                }
                                Rectangle {
                                    width: 80
                                    height: 30
                                    radius: 15
                                    color: model.success ? Qt.lighter(productsRoot.success, 1.8) : Qt.lighter(productsRoot.error, 1.8)
                                    Text {
                                        anchors.centerIn: parent
                                        text: model.success ? "Success" : "Failed"
                                        font.pixelSize: 12
                                        color: model.success ? productsRoot.success : productsRoot.error
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ---- STATISTICS TAB ----
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth
                clip: true

                ColumnLayout {
                    id: statisticsTabContent
                    width: parent.width
                    spacing: 16

                    Text {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: isLoadingOverallStats && (overallStats.totalScanned === undefined || overallStats.totalScanned === 0) ? "Loading overall statistics..."
                                                    : ((overallStats.totalScanned === undefined || overallStats.totalScanned === 0) && !isLoadingOverallStats ? "No statistics available to display." : "")
                        visible: text !== ""
                        color: subtext; font.italic: true
                        topPadding: 20; bottomPadding: 20
                    }

                    // --- Success Rate Card ---
                    Rectangle {
                        Layout.fillWidth: true; height: 100; radius: 12
                        color: "#F5F7FF"; border.width: 1; border.color: borderColor
                        visible: !isLoadingOverallStats && overallStats.totalScanned !== undefined && overallStats.totalScanned > 0

                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 16; spacing: 4
                            Text { text: "Overall Success Rate"; font.pixelSize: 14; color: subtext }
                            Text {
                                text: Math.round(overallStats.successRate || 0) + "%"
                                font.pixelSize: 28; font.bold: true
                                color: (overallStats.successRate || 0) >= 70 ? success : ((overallStats.successRate || 0) >= 40 ? accent : error)
                            }
                            Rectangle {
                                Layout.fillWidth: true; height: 8; radius: 4; color: "#E0E0E0"
                                Rectangle {
                                    width: parent.width * ((overallStats.successRate || 0) / 100)
                                    height: parent.height; radius: parent.radius
                                    color: (overallStats.successRate || 0) >= 70 ? success : ((overallStats.successRate || 0) >= 40 ? accent : error)
                                }
                            }
                        }
                    }

                    // --- Product Scan Amounts Card ---
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: productScanAmountsModel.count > 0 ? (productScanAmountsModel.count * 65 + 60) : 0
                        radius: 12; color: "#FFFFFF"; border.width: 1; border.color: borderColor
                        visible: !isLoadingOverallStats && productScanAmountsModel.count > 0 && overallStats.totalScanned !== undefined && overallStats.totalScanned > 0
                        clip: true

                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 16; spacing: 12
                            Text { text: "Scanned Product Amounts"; font.pixelSize: 16; font.bold: true; color: heading }
                            Repeater {
                                model: productScanAmountsModel
                                delegate: ColumnLayout {
                                    Layout.fillWidth: true; spacing: 4; Layout.bottomMargin: 8
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Text { text: model.name; font.pixelSize: 14; color: heading; elide: Text.ElideRight; Layout.preferredWidth: statisticsTabContent.width * 0.5 }
                                        Item { Layout.fillWidth: true }
                                        Text {
                                            text: model.scanCount + " (" + Math.round(model.percentage * 100) + "%)"
                                            font.pixelSize: 14; font.bold: true; color: heading
                                        }
                                    }
                                    Rectangle {
                                        Layout.fillWidth: true; height: 8; radius: 4; color: "#E0E0E0"
                                        Rectangle {
                                            width: parent.width * model.percentage
                                            height: parent.height; radius: parent.radius; color: model.color
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // --- Overall Weekly Performance Chart ---
                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 250
                        radius: 12; color: "#FFFFFF"; border.width: 1; border.color: borderColor
                        visible: !isLoadingOverallStats && overallStats.totalScanned !== undefined && overallStats.totalScanned >= 0

                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 16; spacing: 12
                            Text { text: "Overall Weekly Performance"; font.pixelSize: 16; font.bold: true; color: heading }
                            Text {
                                id: overallWeeklyStatusText
                                Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter
                                text: {
                                    if (isLoadingOverallWeeklyChart && overallWeeklyChartDaysLoaded < 7) return "Loading weekly performance data...";
                                    var hasValidData = false;
                                    if (overallWeeklyPerformanceModel.count === 7) {
                                        for(var i=0; i<7; ++i) {
                                            if(overallWeeklyPerformanceModel.get(i).isDataPresent) {
                                                hasValidData = true;
                                                break;
                                            }
                                        }
                                    }
                                    if (!hasValidData && !isLoadingOverallWeeklyChart) return "No weekly performance data available.";
                                    return "";
                                }
                                visible: text !== ""
                                color: subtext; font.italic: true
                            }
                            Row {
                                id: overallWeeklyChartRow
                                Layout.fillWidth: true; Layout.fillHeight: true
                                visible: overallWeeklyStatusText.text === ""
                                spacing: overallWeeklyChartRow.width > 0 ? Math.max(4, (overallWeeklyChartRow.width - (7 * 30) - (2 * 16)) / 6) : 4

                                Repeater {
                                    model: overallWeeklyPerformanceModel
                                    delegate: Column {
                                        spacing: 4; height: overallWeeklyChartRow.height; width: 30
                                        Item {
                                            width: parent.width; height: Math.max(0, parent.height - 40)
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            Rectangle {
                                                anchors.fill: parent
                                                color: model.isDataPresent ? "transparent" : (model.hadError ? Qt.rgba(error.r,error.g,error.b,0.15) : "#F0F0F0")
                                            }
                                            Rectangle {
                                                anchors.bottom: parent.bottom; width: parent.width
                                                height: parent.height * model.errorPercent
                                                color: Qt.rgba(error.r, error.g, error.b, 0.7)
                                                visible: model.isDataPresent && model.errorPercent > 0
                                            }
                                            Rectangle {
                                                anchors.bottom: parent.bottom; anchors.bottomMargin: (model.isDataPresent && model.errorPercent > 0) ? (parent.height * model.errorPercent) : 0
                                                width: parent.width; height: parent.height * model.successPercent
                                                color: success
                                                visible: model.isDataPresent && model.successPercent > 0
                                            }
                                        }
                                        Text { text: model.day; font.pixelSize: 12; anchors.horizontalCenter: parent.horizontalCenter; color: subtext }
                                        Text {
                                            text: model.isDataPresent ? (model.totalCount > 0 ? (Math.round(model.successPercent * 100) + "%") : "0") : (model.hadError ? "Err" : "N/A")
                                            font.pixelSize: 11; anchors.horizontalCenter: parent.horizontalCenter; color: model.isDataPresent ? heading : subtext
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
