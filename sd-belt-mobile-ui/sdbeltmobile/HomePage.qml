import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Item {
    id: homeRoot
    anchors.fill: parent

    property color accent: window.accent
    property color borderColor: window.borderColor
    property color heading: window.heading
    property color subtext: window.subtext
    property color success: window.success
    property color error: window.error

    // Data properties
    property var systemInfo: ({ status: "Loading...", runAt: "" })
    property var scanStatsToday: ({ totalSuccess: 0, totalFailed: 0, totalScanned: 0 })
    property var overallScanStatistics: ({ productStatistics: [] })
    property ListModel latestScansModel: ListModel {}
    property ListModel homeProductsModel: ListModel {}

    // Loading states
    property bool isLoadingSystemInfo: true
    property bool isLoadingScanStatsToday: true
    property bool isLoadingLatestScans: true
    property bool isLoadingOverallScanStats: true

    Dialog { // Logout confirmation dialog
        id: logoutConfirmDialog
        title: "Confirm Sign Out"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        width: Math.min(homeRoot.width * 0.85, 360)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 3
        contentWidth: contentItem.implicitWidth + 40
        background: Rectangle {
            color: "#FFFFFF"; radius: 12
            border.color: borderColor; border.width: 1
        }
        contentItem: Text {
            text: "Are you sure you want to sign out?"
            wrapMode: Text.WordWrap
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 40; font.pixelSize: 16
            color: subtext; horizontalAlignment: Text.AlignHCenter
        }
        onAccepted: {
            console.log("Logout confirmed from HomePage");
            window.logout();
        }
    }

    Component.onCompleted: {
        console.log("HomePage: Component.onCompleted");
        if (window.isLoggedIn) {
            isLoadingSystemInfo = true;
            isLoadingScanStatsToday = true;
            isLoadingLatestScans = true;
            isLoadingOverallScanStats = true;
            apiService.fetchProducts();
        }
    }

    Timer { // Automatic system info refresh timer
        id: systemInfoRefreshTimer
        interval: 30000; running: window.isLoggedIn
        repeat: true
        onTriggered: {
            if (window.activePage === "home" && window.isLoggedIn) {
                 apiService.fetchSystemInfo();
            }
        }
    }

    Connections {
        target: apiService
        function onProductsFetched(products) {
            if (!window.isLoggedIn || window.activePage !== "home") return;
            apiService.fetchSystemInfo();
            fetchTodaysScanStatistics();
            apiService.fetchScans(2);

            var veryStartDate = "2000-01-01T00:00:00Z";
            var farFutureDate = "2099-12-31T23:59:59Z";
            apiService.fetchScanStatistics(veryStartDate, farFutureDate, "homePageOverallStats");
            isLoadingOverallScanStats = true;
        }
        function onSystemInfoFetched(info) {
            if (!window.isLoggedIn || window.activePage !== "home") return;
            systemInfo = info;
            isLoadingSystemInfo = false;
            updateRuntimeText();
        }
        function onScanStatisticsFetched(stats, context) {
            if (!window.isLoggedIn || window.activePage !== "home") return;
            if (context === "homePageOverallStats") {
                overallScanStatistics = stats;
                isLoadingOverallScanStats = false;
                populateHomeProductsModel();
            } else if (context === "homePageTodayStats") {
                scanStatsToday = stats;
                isLoadingScanStatsToday = false;
            }
        }
        function onScansFetched(scans) {
            if (!window.isLoggedIn || window.activePage !== "home") return;
            latestScansModel.clear();
            var count = 0;
            for (var i = 0; i < scans.length && count < 2; ++i) {
                var scanItem = scans[i];
                latestScansModel.append({
                    productName: scanItem.productName || "Unknown Product",
                    scanTime: formatScanTime(scanItem.timestamp),
                    isSuccess: scanItem.isSuccess,
                    productIcon: scanItem.productIcon || "📷"
                });
                count++;
            }
            isLoadingLatestScans = false;
        }
        function onSystemCommandSuccess(command, message) {
            if (!window.isLoggedIn || window.activePage !== "home") return;
            apiService.fetchSystemInfo();
        }
        function onSystemCommandError(command, errorMsg) {
            if (!window.isLoggedIn || window.activePage !== "home") return;
        }
        function onNetworkError(endpoint, errorString) {
            if (!window.isLoggedIn || window.activePage !== "home") return;
            if (endpoint.includes("/system/info")) isLoadingSystemInfo = false;
            if (endpoint.includes("/scans/statistics")) {
                isLoadingScanStatsToday = false;
                isLoadingOverallScanStats = false;
            }
            if (endpoint.includes("/scans") && !endpoint.includes("statistics")) isLoadingLatestScans = false;
            if (endpoint.includes("/products")) isLoadingOverallScanStats = false;
        }
    }

    function populateHomeProductsModel() {
        if (isLoadingOverallScanStats || !overallScanStatistics || !overallScanStatistics.productStatistics) {
            if (!isLoadingOverallScanStats) homeProductsModel.clear();
            return;
        }
        homeProductsModel.clear();
        var displayedCount = 0;
        for (var i = 0; i < overallScanStatistics.productStatistics.length && displayedCount < 2; ++i) {
            var prodStat = overallScanStatistics.productStatistics[i];
            homeProductsModel.append({
                name: prodStat.productName || prodStat.productId,
                iconChar: prodStat.productIcon || "📦",
                successCount: prodStat.totalSuccess || 0,
                errorCount: prodStat.totalFailed || 0
            });
            displayedCount++;
        }
    }
    function updateRuntimeText() {
        if (isLoadingSystemInfo || !systemInfo || !systemInfo.runAt) {
            runtimeText.text = isLoadingSystemInfo ? "Loading..." : "N/A";
            return;
        }
        if (systemInfo.status === "ACTIVE" || systemInfo.status === "RUNNING" || systemInfo.status === "STARTED") {
            var runAtDate = new Date(systemInfo.runAt);
            var now = new Date();
            var diffMs = now - runAtDate;
            if (diffMs < 0) { runtimeText.text = "Syncing..."; return; }
            var days = Math.floor(diffMs / (1000 * 60 * 60 * 24));
            diffMs -= days * (1000 * 60 * 60 * 24);
            var hours = Math.floor(diffMs / (1000 * 60 * 60));
            diffMs -= hours * (1000 * 60 * 60);
            var minutes = Math.floor(diffMs / (1000 * 60));
            runtimeText.text = days + "d " + hours + "h " + minutes + "m";
        } else {
            runtimeText.text = "N/A";
        }
    }
    function fetchTodaysScanStatistics() {
        if (!window.isLoggedIn) return;
        isLoadingScanStatsToday = true;
        var today = new Date();
        var startDate = new Date(today.getFullYear(), today.getMonth(), today.getDate(), 0, 0, 0).toISOString();
        var endDate = new Date(today.getFullYear(), today.getMonth(), today.getDate(), 23, 59, 59, 999).toISOString();
        apiService.fetchScanStatistics(startDate, endDate, "homePageTodayStats");
    }
    function formatScanTime(isoTimestamp) {
        if (!isoTimestamp) return "N/A";
        var date = new Date(isoTimestamp);
        var today = new Date();
        var yesterday = new Date(today);
        yesterday.setDate(today.getDate() - 1);

        if (date.toDateString() === today.toDateString()) {
            return "Today " + date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', hour12: false });
        } else if (date.toDateString() === yesterday.toDateString()) {
            return "Yesterday " + date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', hour12: false });
        } else {
            return date.toLocaleDateString([], {year: 'numeric', month: 'short', day: 'numeric'}) + " " + date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', hour12: false });
        }
    }

    Rectangle { anchors.fill: parent; color: "#FFFBFF"; radius: 24 }

    ColumnLayout {
        id: body
        anchors.fill: parent; anchors.margins: 16; spacing: 12

        // Fixed Header - Always visible at top
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Item { Layout.fillWidth: true }
            Column {
                spacing: 2; Layout.alignment: Qt.AlignCenter
                Text { text: "Erkan Zergeroglu"; font.pixelSize: 15; color: heading; horizontalAlignment: Text.AlignRight }
                Text { text: "Belt: SD Belt"; font.pixelSize: 14; color: subtext; horizontalAlignment: Text.AlignRight }
            }
            Rectangle { // Sign Out Button
                id: signOutButton
                width: 44; height: 44; radius: 8
                color: Qt.lighter(Material.color(Material.Red, Material.Shade50), 0.9)
                border.color: Material.color(Material.Red, Material.Shade400); border.width: 1
                Accessible.name: "Sign Out"; Accessible.role: Accessible.Button

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/icons/signout.svg"
                    width: 22
                    height: 22
                    fillMode: Image.PreserveAspectFit
                    layer.enabled: true
                }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: logoutConfirmDialog.open()
                }
            }
        }

        // Scrollable Content
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            clip: true

            ColumnLayout {
                id: scrollableContent
                width: parent.width
                spacing: 12

                // System Control Buttons
                Row {
                    spacing: 8
                    Layout.fillWidth: true
                    Rectangle { // Stop
                        width: (scrollableContent.width - 16) / 3; height: 56; radius: 12
                        border.width: 1; border.color: error; color: "transparent"
                        RowLayout {
                            anchors.centerIn: parent; spacing: 8
                            Image {
                                source: "qrc:/icons/shutdown.svg"; width: 18; height: 18; fillMode: Image.PreserveAspectFit
                                layer.enabled: true;
                            }
                            Text { text: "Stop"; font.pixelSize: 14; color: error }
                        }
                        MouseArea { anchors.fill: parent; onClicked: apiService.stopSystem(); cursorShape: Qt.PointingHandCursor }
                    }
                    Rectangle { // Reverse
                        width: (scrollableContent.width - 16) / 3; height: 56; radius: 12
                        border.width: 1; border.color: accent; color: "transparent"
                        RowLayout {
                            anchors.centerIn: parent; spacing: 8
                            Image {
                                source: "qrc:/icons/revert.svg"; width: 18; height: 18; fillMode: Image.PreserveAspectFit
                                layer.enabled: true;
                            }
                            Text { text: "Reverse"; font.pixelSize: 14; color: accent }
                        }
                        MouseArea { anchors.fill: parent; onClicked: apiService.reverseSystem(); cursorShape: Qt.PointingHandCursor }
                    }
                    Rectangle { // Start
                        width: (scrollableContent.width - 16) / 3; height: 56; radius: 12
                        border.width: 1; border.color: success; color: "transparent"
                        RowLayout {
                            anchors.centerIn: parent; spacing: 8
                            Image {
                                source: "qrc:/icons/start.svg"; width: 18; height: 18; fillMode: Image.PreserveAspectFit
                                layer.enabled: true;
                            }
                            Text { text: "Start"; font.pixelSize: 14; color: success }
                        }
                        MouseArea { anchors.fill: parent; onClicked: apiService.startSystem(); cursorShape: Qt.PointingHandCursor }
                    }
                }

                // ---- System Section Title & Stats ----
                RowLayout {
                    Layout.fillWidth: true; Layout.topMargin: 16
                    Text { text: "System"; font.pixelSize: 16; color: heading }
                    Item { Layout.fillWidth: true }
                }
                Rectangle { height: 1; color: borderColor; Layout.fillWidth: true }
                GridLayout {
                    Layout.fillWidth: true; Layout.topMargin: 16; columnSpacing: 10; rowSpacing: 25; columns: 2
                    Column {
                        Layout.fillWidth: true; spacing: 2
                        Text {
                            id: runtimeText; text: isLoadingSystemInfo ? "Loading..." : "N/A"
                            font.pixelSize: 22; font.bold: true; color: heading
                        }
                        Text { text: "Runtime"; font.pixelSize: 13; color: subtext }
                    }
                    Column {
                        width: 80; spacing: 2
                        Text {
                            text: isLoadingScanStatsToday ? "..." : scanStatsToday.totalSuccess.toString()
                            font.pixelSize: 22; font.bold: true; color: success; anchors.right: parent.right
                        }
                        Text { text: "valid today"; font.pixelSize: 13; color: subtext; anchors.right: parent.right }
                    }
                    Column {
                        Layout.fillWidth: true; spacing: 2
                        Text {
                            text: isLoadingScanStatsToday ? "..." : scanStatsToday.totalScanned.toString()
                            font.pixelSize: 22; font.bold: true; color: heading
                        }
                        Text { text: "Scanned today"; font.pixelSize: 13; color: subtext }
                    }
                    Column {
                        width: 80; spacing: 2
                        Text {
                            text: isLoadingScanStatsToday ? "..." : scanStatsToday.totalFailed.toString()
                            font.pixelSize: 22; font.bold: true; color: error; anchors.right: parent.right
                        }
                        Text { text: "failed today"; font.pixelSize: 13; color: subtext; anchors.right: parent.right }
                    }
                }

                // ---- Latest Scans Section ----
                ColumnLayout {
                    Layout.fillWidth: true; Layout.topMargin: 16; spacing: 8
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Latest Scans"; font.pixelSize: 16; font.bold: true; color: heading }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 32; height: 32; radius: 8; color: "transparent"; border.width: 1; border.color: accent
                            Text { anchors.centerIn: parent; text: "→"; font.pixelSize: 16; color: accent }
                            MouseArea { anchors.fill: parent; onClicked: window.navigateToProductsTab(1) }
                        }
                    }
                    Text {
                        text: isLoadingLatestScans ? "Loading latest scans..." : (latestScansModel.count === 0 ? "No recent scans." : "")
                        visible: isLoadingLatestScans || latestScansModel.count === 0
                        Layout.alignment: Qt.AlignHCenter; color: subtext
                    }
                    Repeater {
                        model: latestScansModel
                        delegate: Rectangle {
                            Layout.fillWidth: true; height: 56; color: "transparent"
                            RowLayout {
                                anchors.fill: parent; spacing: 12
                                Rectangle {
                                    width: 40; height: 40; radius: 20; color: "#F5F5F5"
                                    Text { anchors.centerIn: parent; text: model.productIcon; font.pixelSize: 18 }
                                }
                                Text {
                                    text: model.productName; font.pixelSize: 14; color: heading; Layout.fillWidth: true
                                }
                                Column {
                                    spacing: 4; width: 100
                                    Text {
                                        text: model.isSuccess ? "Success" : "Failed"
                                        font.pixelSize: 13; color: model.isSuccess ? success : error; anchors.right: parent.right
                                    }
                                    Text {
                                        text: model.scanTime; font.pixelSize: 11; color: subtext; anchors.right: parent.right
                                    }
                                }
                            }
                        }
                    }
                }

                // ---- Products Section ----
                ColumnLayout {
                    Layout.fillWidth: true; Layout.topMargin: 8; spacing: 8
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Products"; font.pixelSize: 16; font.bold: true; color: heading }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 32; height: 32; radius: 8; color: "transparent"; border.width: 1; border.color: accent
                            Text { anchors.centerIn: parent; text: "→"; font.pixelSize: 16; color: accent }
                            MouseArea { anchors.fill: parent; onClicked: window.navigateToProductsTab(0) }
                        }
                    }
                    Text {
                        text: (isLoadingOverallScanStats && homeProductsModel.count === 0) ? "Loading products..."
                              : (homeProductsModel.count === 0 && !isLoadingOverallScanStats ? "No products found." : "")
                        visible: (isLoadingOverallScanStats && homeProductsModel.count === 0) || (homeProductsModel.count === 0 && !isLoadingOverallScanStats)
                        Layout.alignment: Qt.AlignHCenter; color: subtext
                    }
                    Repeater {
                        model: homeProductsModel
                        delegate: Rectangle {
                            Layout.fillWidth: true; height: 56; color: "transparent"
                            RowLayout {
                                anchors.fill: parent; spacing: 12
                                Rectangle {
                                    width: 40; height: 40; radius: 20; color: "#F5F5F5"
                                    Text { anchors.centerIn: parent; text: model.iconChar; font.pixelSize: 18 }
                                }
                                Text {
                                    text: model.name; font.pixelSize: 14; color: heading; Layout.fillWidth: true
                                }
                                Column {
                                    spacing: 4; width: 80
                                    Text {
                                        text: model.successCount.toString(); font.pixelSize: 13; color: success; anchors.right: parent.right
                                    }
                                    Text {
                                        text: model.errorCount.toString(); font.pixelSize: 11; color: error; anchors.right: parent.right
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
