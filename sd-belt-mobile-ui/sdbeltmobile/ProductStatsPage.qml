// ProductStatsPage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Item {
    id: productStatsRoot
    anchors.fill: parent

    property string productId: "UNKNOWN"
    property string productName: "Unknown Product"
    property string productIcon: "📷"
    property int successScans: 0
    property int errorScans: 0

    readonly property real successRate: (successScans + errorScans > 0) ? (successScans / (successScans + errorScans)) : 0
    readonly property string successRateText: Math.round(successRate * 100) + "%"

    property color accent: window.accent
    property color borderColor: window.borderColor
    property color heading: window.heading
    property color subtext: window.subtext
    property color success: window.success
    property color error: window.error

    signal backToProductsRequested()

    // --- Weekly Chart Properties ---
    property ListModel productWeeklyPerformanceModel: ListModel {}
    property bool isLoadingWeeklyChart: true
    property int productWeeklyChartDaysLoaded: 0
    property var productDailyStatsCollector: [null, null, null, null, null, null, null]

    Component.onCompleted: {
        console.log("ProductStatsPage: Component.onCompleted. Initial productId:", productId, "Name:", productName);
        if (productId !== "UNKNOWN" && productId !== "") {
            if (isLoadingWeeklyChart && productWeeklyPerformanceModel.count === 0 && productWeeklyChartDaysLoaded === 0) {
                 loadProductWeeklyPerformanceChartData();
            }
        } else {
            isLoadingWeeklyChart = false;
        }
    }

    onProductIdChanged: {
        console.log("ProductStatsPage: productId CHANGED to:", productId, "Current Name prop value:", productName);
        if (productId !== "UNKNOWN" && productId !== "") {
            isLoadingWeeklyChart = true;
            productWeeklyChartDaysLoaded = 0;
            productDailyStatsCollector = [null, null, null, null, null, null, null];
            productWeeklyPerformanceModel.clear();
            loadProductWeeklyPerformanceChartData();
        } else {
            isLoadingWeeklyChart = false;
            productWeeklyPerformanceModel.clear();
            productDailyStatsCollector = [null, null, null, null, null, null, null];
            productWeeklyChartDaysLoaded = 0;
            console.warn("ProductStatsPage: productId changed to invalid. Cleared weekly stats.");
        }
    }

    Connections {
        target: apiService
        function onScanStatisticsFetched(stats, context) {
            if (!isLoadingWeeklyChart || !context || !productId || productId === "UNKNOWN") {
                return;
            }

            var expectedPrefix = "prodStatWeekDay_" + productId + "_";
            if (!context.startsWith(expectedPrefix)) {
                return;
            }

            var dayIndexString = context.substring(expectedPrefix.length);
            var dayIndex = parseInt(dayIndexString);

            if (isNaN(dayIndex) || dayIndex < 0 || dayIndex >= 7) {
                console.error("ProductStatsPage: ("+productId+") Failed to parse valid dayIndex from context suffix '"+dayIndexString+"'. Full context:", context);
                var firstNullSlotError = productDailyStatsCollector.indexOf(null);
                if (firstNullSlotError !== -1) {
                    productDailyStatsCollector[firstNullSlotError] = { success: 0, failed: 0, total: 0, error: true, note: "Bad dayIndex in context: " + context };
                    productWeeklyChartDaysLoaded++;
                }
                if (productWeeklyChartDaysLoaded === 7) finishWeeklyChartLoad();
                return;
            }

            if (productDailyStatsCollector[dayIndex] !== null) {
                console.warn("ProductStatsPage: ("+productId+") Data for dayIndex", dayIndex, "already processed. Ignoring duplicate/late signal. Context:", context);
                var allSlotsNowFilled = productDailyStatsCollector.every(function(item){ return item !== null; });
                if (allSlotsNowFilled) finishWeeklyChartLoad();
                return;
            }

            console.log("ProductStatsPage: ("+productId+") Received stats for dayIndex", dayIndex, "Context:", context, "Data:", JSON.stringify(stats).substring(0,200));

            if (stats && stats.totalScanned !== undefined) {
                 productDailyStatsCollector[dayIndex] = {
                    success: stats.totalSuccess || 0,
                    failed: stats.totalFailed || 0,
                    total: stats.totalScanned || 0,
                    error: false
                };
            } else {
                console.warn("ProductStatsPage: ("+productId+") No valid scan data (missing totalScanned) in response for dayIndex", dayIndex, "Context:", context);
                productDailyStatsCollector[dayIndex] = { success: 0, failed: 0, total: 0, error: true, note: "Missing or invalid stats data from API" };
            }

            productWeeklyChartDaysLoaded++;

            if (productWeeklyChartDaysLoaded === 7) {
                finishWeeklyChartLoad();
            } else if (productWeeklyChartDaysLoaded > 7) {
                 console.error("ProductStatsPage: ("+productId+") productWeeklyChartDaysLoaded exceeded 7! Value:", productWeeklyChartDaysLoaded);
                 finishWeeklyChartLoad();
            }
        }

        function onNetworkError(endpoint, errorString) {
            if (!isLoadingWeeklyChart || !productId || productId === "UNKNOWN") {
                return;
            }

            var contextMatch = endpoint.match(/\(context: (prodStatWeekDay_[^_]+_\d+)\)/);
            var contextFromError = contextMatch && contextMatch[1] ? contextMatch[1] : "";

            if (!contextFromError) return;

            var expectedPrefix = "prodStatWeekDay_" + productId + "_";
            if (!contextFromError.startsWith(expectedPrefix)) {
                return;
            }

            var dayIndexString = contextFromError.substring(expectedPrefix.length);
            var dayIndex = parseInt(dayIndexString);

            console.error("ProductStatsPage: ("+productId+") Network Error for weekly chart. Context:", contextFromError, "Parsed dayIndex:", dayIndex, "Error:", errorString.substring(0,100));

            if (isNaN(dayIndex) || dayIndex < 0 || dayIndex >= 7) {
                console.error("ProductStatsPage: ("+productId+") Could not parse valid dayIndex from network error context:", contextFromError);
                var firstNullSlotNetErr = productDailyStatsCollector.indexOf(null);
                if (firstNullSlotNetErr !== -1) {
                    productDailyStatsCollector[firstNullSlotNetErr] = { success: 0, failed: 0, total: 0, error: true, note: "Network error, bad dayIndex in context: " + contextFromError };
                    productWeeklyChartDaysLoaded++;
                }
            } else {
                if (productDailyStatsCollector[dayIndex] === null) {
                    productDailyStatsCollector[dayIndex] = { success: 0, failed: 0, total: 0, error: true, note: "Network error for day " + dayIndex };
                    productWeeklyChartDaysLoaded++;
                } else {
                    console.warn("ProductStatsPage: ("+productId+") Network error for an already processed/filled dayIndex", dayIndex, ". Context:", contextFromError);
                }
            }

            if (productWeeklyChartDaysLoaded === 7) {
                finishWeeklyChartLoad();
            } else if (productWeeklyChartDaysLoaded > 7) {
                console.error("ProductStatsPage: ("+productId+") productWeeklyChartDaysLoaded (in NWErr) exceeded 7! Value:", productWeeklyChartDaysLoaded);
                finishWeeklyChartLoad();
            }
        }
    }
    function finishWeeklyChartLoad() {
        processAndDisplayProductWeeklyChart();
        isLoadingWeeklyChart = false;
        console.log("ProductStatsPage: ("+productId+") Weekly chart load finished. isLoadingWeeklyChart:", isLoadingWeeklyChart);
    }

    function loadProductWeeklyPerformanceChartData() {
        if (productId === "UNKNOWN" || productId === "") {
            console.error("ProductStatsPage (loadProductWeekly): Invalid productId. Aborting.");
            isLoadingWeeklyChart = false;
            productWeeklyPerformanceModel.clear();
            return;
        }
        console.log("ProductStatsPage: ("+productId+") Initiating weekly performance data load.");

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
            var context = "prodStatWeekDay_" + productId + "_" + i;
            apiService.fetchScanStatistics(dayStart.toISOString(), dayEnd.toISOString(), context, productId);
        }
    }

    function processAndDisplayProductWeeklyChart() {
        console.log("ProductStatsPage: ("+productId+") Processing collected daily stats for chart:", JSON.stringify(productDailyStatsCollector));
        productWeeklyPerformanceModel.clear();
        var daysOfWeek = ["M", "T", "W", "Th", "F", "Sa", "Su"];

        for (var i = 0; i < 7; i++) {
            var dayStat = productDailyStatsCollector[i];
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


            productWeeklyPerformanceModel.append({
                day: daysOfWeek[i], successPercent: successP, errorPercent: errorP,
                successCount: successC, errorCount: errorC, totalCount: totalC,
                hadError: dayHadError,
                isDataPresent: dayIsDataPresent
            });
        }

    }

    Rectangle { anchors.fill: parent; color: "#FFFBFF" }

    ColumnLayout {
        id: body
        anchors.fill: parent; anchors.margins: 16; spacing: 16

        RowLayout { /* Header */
            Layout.fillWidth: true; spacing: 8
            Rectangle { width: 44; height: 44; radius: 8; color: "#F0F0F0"; Text { anchors.centerIn: parent; text: "←"; font.pixelSize: 24; color: heading } MouseArea { anchors.fill: parent; onClicked: productStatsRoot.backToProductsRequested() } }
            Text { text: "Stats for " + productName; font.pixelSize: 18; font.bold: true; color: heading; Layout.fillWidth: true; elide: Text.ElideRight; wrapMode: Text.WrapAnywhere }
        }

        Rectangle { /* Product Summary Card */
            Layout.fillWidth: true; height: 80; radius: 12; color: "#FFFFFF"; border.width: 1; border.color: borderColor
            RowLayout { anchors.fill: parent; anchors.margins: 16; spacing: 12; Rectangle { width: 48; height: 48; radius: 24; color: "#F5F5F5"; Text { anchors.centerIn: parent; text: productIcon; font.pixelSize: 22 } } ColumnLayout { Layout.fillWidth: true; spacing: 2; Text { text: productName; font.pixelSize: 16; font.bold: true; color: heading } Row { spacing: 4; Text { text: "Total Scans (Overall):"; font.pixelSize: 13; color: subtext } Text { text: (successScans + errorScans).toString(); font.pixelSize: 13; font.bold: true; color: heading } } } }
        }

        Rectangle { /* Success Rate Card */
            Layout.fillWidth: true; height: 100; radius: 12; color: "#F5F7FF"; border.width: 1; border.color: borderColor
            ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 4; Text { text: "Success Rate (Overall for this Product)"; font.pixelSize: 14; color: subtext } Text { text: successRateText; font.pixelSize: 28; font.bold: true; color: successRate >= 0.7 ? success : (successRate >= 0.4 ? accent : error) } Rectangle { Layout.fillWidth: true; height: 8; radius: 4; color: "#E0E0E0"; Rectangle { width: parent.width * successRate; height: parent.height; radius: parent.radius; color: successRate >= 0.7 ? success : (successRate >= 0.4 ? accent : error) } } }
        }

        Rectangle { // Weekly Performance Card
            Layout.fillWidth: true; Layout.fillHeight: true; radius: 12
            color: "#FFFFFF"; border.width: 1; border.color: borderColor

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 16; spacing: 12
                Text { text: "Weekly Performance for " + productName; font.pixelSize: 16; font.bold: true; color: heading }
                Text {
                    id: weeklyStatusText
                    text: {
                        if (isLoadingWeeklyChart && productWeeklyChartDaysLoaded < 7) return "Loading weekly chart data...";
                        if (!isLoadingWeeklyChart && productWeeklyPerformanceModel.count === 0) return "No weekly data available for this product."; // Should not happen if logic correct

                        var hasValidData = false;
                        if (productWeeklyPerformanceModel.count === 7) {
                            for (var i = 0; i < 7; i++) {
                                var item = productWeeklyPerformanceModel.get(i);
                                if (item.isDataPresent) {
                                    hasValidData = true;
                                    break;
                                }
                            }
                        }
                        if (!hasValidData && !isLoadingWeeklyChart) {
                             return "No weekly data available for this product.";
                        }
                        return "";
                    }
                    visible: text !== ""
                    Layout.alignment: Qt.AlignHCenter; color: subtext; font.italic: true
                }

                Row {
                    id: weeklyProductChartRow
                    Layout.fillWidth: true; Layout.fillHeight: true
                    visible: weeklyStatusText.text === ""
                    spacing: weeklyProductChartRow.width > 0 ? Math.max(4, (weeklyProductChartRow.width - (7 * 30) - (2 * 16)) / 6) : 4

                    Repeater {
                        model: productWeeklyPerformanceModel
                        delegate: Column {
                            spacing: 4; height: weeklyProductChartRow.height; width: 30
                            Item { // Bar container
                                width: parent.width; height: Math.max(0, parent.height - 40)
                                anchors.horizontalCenter: parent.horizontalCenter

                                Rectangle { // Base/Empty/Error bar background
                                    anchors.fill: parent
                                    color: model.isDataPresent ? "transparent" : (model.hadError ? Qt.rgba(error.r,error.g,error.b,0.15) : "#F0F0F0")
                                                                                                // Light red for error day, grey for no data received yet
                                }
                                Rectangle { // Error portion
                                    anchors.bottom: parent.bottom; width: parent.width
                                    height: parent.height * model.errorPercent
                                    color: Qt.rgba(error.r, error.g, error.b, 0.7)
                                    visible: model.isDataPresent && model.errorPercent > 0
                                }
                                Rectangle { // Success portion
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
