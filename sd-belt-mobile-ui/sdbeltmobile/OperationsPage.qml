import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Item {
    id: operationsRoot
    anchors.fill: parent

    property color accent: window.accent
    property color borderColor: window.borderColor
    property color heading: window.heading
    property color subtext: window.subtext
    property color success: window.success
    property color error: window.error

    property var systemInfo: ({
        id: "Loading...",
        runAt: "",
        status: "Loading...",
        name: "Loading...",
        threshold: 90.0,
        speed: 0,
        beltDirection: "N/A",
        cpuUsage: 0.0,
        cpuTemperature: 0.0,
        memoryUsage: 0.0
    })
    property string totalScanned: "Loading..."

    property bool isLoadingSystemInfo: true
    property bool isLoadingTotalScanned: true

    Timer {
        id: sliderTimer
        interval: 2000
        onTriggered: {
            if (systemInfo.status === "ACTIVE" || systemInfo.status === "RUNNING" || systemInfo.status === "STARTED") {
                 statusText.text = formatStatus(systemInfo.status);
                 statusText.color = getStatusColor(systemInfo.status);
            } else if (statusText.text.indexOf("Adjusting") >= 0) {
                 statusText.text = formatStatus(systemInfo.status);
                 statusText.color = getStatusColor(systemInfo.status);
            }
        }
    }

    Component.onCompleted: {
        apiService.fetchSystemInfo();
        var veryStartDate = "2000-01-01T00:00:00Z";
        var farFutureDate = "2099-12-31T23:59:59Z";
        apiService.fetchScanStatistics(veryStartDate, farFutureDate, "operationsPageTotalScanned");
        isLoadingTotalScanned = true;
    }

    Timer {
        id: sysInfoRefreshTimerOperations
        interval: 15000
        running: true
        repeat: true
        onTriggered: {
             if (window.activePage === "operations" && window.isLoggedIn) {
                apiService.fetchSystemInfo();
             }
        }
    }

    Connections {
        target: apiService
        function onSystemInfoFetched(info) {
            systemInfo = info;
            accuracySlider.val = systemInfo.threshold !== undefined ? systemInfo.threshold : 80;
            isLoadingSystemInfo = false;
            updateRuntimeText();
            if (statusText.text.indexOf("Adjusting") === -1 ||
                !(systemInfo.status === "ACTIVE" || systemInfo.status === "RUNNING" || systemInfo.status === "STARTED") ) {
                statusText.text = formatStatus(systemInfo.status);
                statusText.color = getStatusColor(systemInfo.status);
            }
        }
        function onScanStatisticsFetched(stats, context) {
            if (context === "operationsPageTotalScanned") {
                totalScanned = stats.totalScanned !== undefined ? stats.totalScanned.toString() : "N/A";
                isLoadingTotalScanned = false;
            }
        }
        function onSystemCommandSuccess(command, message) {
            if (command !== "setAccuracy") {
                 apiService.fetchSystemInfo();
            } else {
                apiService.fetchSystemInfo();
            }
        }
        function onSystemCommandError(command, errorMsg) {
            if (command !== "setAccuracy") {
                apiService.fetchSystemInfo();
            } else {
                accuracySlider.val = systemInfo.threshold !== undefined ? systemInfo.threshold : 80;
                apiService.fetchSystemInfo();
            }
        }
        function onNetworkError(endpoint, errorString) {
            if (endpoint.includes("/system/info")) isLoadingSystemInfo = false;
            if (endpoint.includes("/scans/statistics")) isLoadingTotalScanned = false;
            if (endpoint.includes("/system/threshold")) {
                accuracySlider.val = systemInfo.threshold !== undefined ? systemInfo.threshold : 80;
                apiService.fetchSystemInfo();
            }
        }
    }

    function updateRuntimeText() {
        if (isLoadingSystemInfo || !systemInfo.runAt) {
            runtimeDisplay.text = systemInfo.status === "Loading..." ? "Loading..." : "N/A";
            return;
        }
        if (systemInfo.status === "ACTIVE" || systemInfo.status === "RUNNING" || systemInfo.status === "STARTED") {
            var runAtDate = new Date(systemInfo.runAt);
            var now = new Date();
            var diffMs = now - runAtDate;
            if (diffMs < 0) {
                runtimeDisplay.text = "Syncing...";
                return;
            }
            var days = Math.floor(diffMs / (1000 * 60 * 60 * 24));
            diffMs -= days * (1000 * 60 * 60 * 24);
            var hours = Math.floor(diffMs / (1000 * 60 * 60));
            diffMs -= hours * (1000 * 60 * 60);
            var minutes = Math.floor(diffMs / (1000 * 60));
            runtimeDisplay.text = days + "d " + hours + "h " + minutes + "m";
        } else {
            runtimeDisplay.text = "N/A";
        }
    }

    function formatStatus(apiStatus) {
        if (!apiStatus) return "Unknown";
        switch(apiStatus.toUpperCase()) {
            case "ACTIVE": case "RUNNING": case "STARTED": return "Connected - Running";
            case "INACTIVE": case "STOPPED": return "Disconnected";
            case "PAUSED": return "Connected - Paused";
            default: return apiStatus;
        }
    }

    function getStatusColor(apiStatus) {
         if (!apiStatus) return subtext;
         switch(apiStatus.toUpperCase()) {
            case "ACTIVE": case "RUNNING": case "STARTED": return success;
            case "INACTIVE": case "STOPPED": return error;
            case "PAUSED": return accent;
            default: return subtext;
        }
    }

    Rectangle { anchors.fill: parent; color: "#FFFBFF" }

    ColumnLayout {
        id: body
        anchors.fill: parent; anchors.margins: 16; spacing: 16

        // Fixed Header
        RowLayout {
            Layout.fillWidth: true; spacing: 8
            Rectangle {
                width: 44; height: 44; radius: 8; color: "#F0F0F0"
                Text { anchors.centerIn: parent; text: "←"; font.pixelSize: 24; color: heading }
                MouseArea { anchors.fill: parent; onClicked: { window.activePage = "home"; stackView.replace(homePage); } cursorShape: Qt.PointingHandCursor }
            }
            Item { Layout.fillWidth: true }
            Text { text: "SD Belt Operations"; font.pixelSize: 18; font.bold: true; color: heading }
            Item { Layout.fillWidth: true }
            Item { width: 44; height: 44; visible: false}
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
                spacing: 16

                // Stats section
                ColumnLayout {
                    spacing: 6; Layout.fillWidth: true; Layout.topMargin: 16
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "ID"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            text: isLoadingSystemInfo ? "Loading..." : (systemInfo.id || "N/A")
                            font.pixelSize: 14; font.bold: true; color: heading; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Runtime"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            id: runtimeDisplay; text: "Loading..."
                            font.pixelSize: 14; font.bold: true; color: heading; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Status"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            id: statusText; text: isLoadingSystemInfo ? "Loading..." : formatStatus(systemInfo.status)
                            font.pixelSize: 14; font.bold: true; color: isLoadingSystemInfo ? subtext : getStatusColor(systemInfo.status)
                            wrapMode: Text.WrapAnywhere; Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Total Scanned"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            text: isLoadingTotalScanned ? "Loading..." : totalScanned
                            font.pixelSize: 14; font.bold: true; color: heading; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "CPU Usage"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            text: isLoadingSystemInfo ? "..." : (typeof systemInfo.cpuUsage === 'number' ? systemInfo.cpuUsage.toFixed(1) + "%" : "N/A")
                            font.pixelSize: 14; font.bold: true; color: heading; Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "CPU Temp"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            text: isLoadingSystemInfo ? "..." : (typeof systemInfo.cpuTemperature === 'number' ? systemInfo.cpuTemperature.toFixed(1) + "°C" : "N/A")
                            font.pixelSize: 14; font.bold: true; color: heading; Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Memory Usage"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            text: isLoadingSystemInfo ? "..." : (typeof systemInfo.memoryUsage === 'number' ? systemInfo.memoryUsage.toFixed(1) + "%" : "N/A")
                            font.pixelSize: 14; font.bold: true; color: heading; Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Belt Direction"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            text: isLoadingSystemInfo ? "..." : (systemInfo.beltDirection || "N/A")
                            font.pixelSize: 14; font.bold: true; color: heading; Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Speed"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                        Text {
                            text: isLoadingSystemInfo ? "..." : (typeof systemInfo.speed === 'number' ? systemInfo.speed.toString() : "N/A")
                            font.pixelSize: 14; font.bold: true; color: heading; Layout.fillWidth: true
                        }
                    }
                }

                // Accuracy Slider
                Rectangle {
                    id: accuracySlider
                    Layout.fillWidth: true
                    radius: 12
                    border.color: borderColor; border.width: 1
                    color: "#FFFFFF"
                    Layout.preferredHeight: 136
                    property real val: systemInfo.threshold
                    Component.onCompleted: {
                        // Initialize with current threshold value
                        if (systemInfo.threshold !== undefined) {
                            accuracySlider.val = systemInfo.threshold;
                        } else {
                            accuracySlider.val = 80;
                        }
                    }

                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 6
                        Text { text: "Accuracy"; font.pixelSize: 16; font.bold: true; color: heading }
                        Rectangle { height: 1; color: borderColor; Layout.fillWidth: true }
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "Current Accuracy"; font.pixelSize: 14; color: subtext }
                            Item { Layout.fillWidth: true }
                            Text { text: Math.round(accuracySlider.val) + "%"; font.pixelSize: 14; font.bold: true; color: heading }
                        }

                        // --- Custom slider (progress & knob) ---
                        Rectangle {
                            id: track
                            height: 6
                            radius: 3
                            color: "#E0E0E0"
                            Layout.fillWidth: true

                            Rectangle {
                                id: prog
                                anchors.verticalCenter: parent.verticalCenter
                                height: parent.height
                                radius: parent.radius
                                color: accent
                                width: parent.width * (accuracySlider.val / 100)
                            }

                            Rectangle {
                                id: knob
                                width: 18; height: 18; radius: 9
                                color: accent
                                border.color: "#FFFFFF"; border.width: 1

                                y: -6
                                x: prog.width - (width/2)

                                MouseArea {
                                    anchors.fill: parent
                                    drag.target: parent
                                    drag.axis: Drag.XAxis

                                    drag.minimumX: -parent.width/2
                                    drag.maximumX: track.width - parent.width/2

                                    onPositionChanged: {
                                        var knobCenterX = parent.x + parent.width/2;
                                        var newPercentage = Math.round(Math.max(0, Math.min(100, (knobCenterX / track.width) * 100)));

                                        if (accuracySlider.val !== newPercentage) {
                                            accuracySlider.val = newPercentage;

                                            if (systemInfo.status === "ACTIVE" || systemInfo.status === "RUNNING" || systemInfo.status === "STARTED") {
                                                if (statusText.text.indexOf("Adjusting") === -1) {
                                                    statusText.text = "Connected - Adjusting";
                                                    statusText.color = accent;
                                                }
                                                sliderTimer.restart();
                                            }
                                        }
                                    }
                                    onPressedChanged: {
                                        if (!pressed) {
                                            console.log("Setting accuracy to:", Math.round(accuracySlider.val));
                                            apiService.setSystemAccuracy(Math.round(accuracySlider.val));
                                        }
                                    }
                                }
                            }
                        }
                        Text { text: "Change belt accuracy"; font.pixelSize: 12; color: subtext }
                    }
                }

                // System Card with Big Buttons
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 280
                    radius: 12; border.color: borderColor; border.width: 1; color: "#FFFFFF"

                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 4
                        Text { text: "System"; font.pixelSize: 16; font.bold: true; color: heading }
                        Rectangle { height: 1; color: borderColor; Layout.fillWidth: true }
                        Item { Layout.fillHeight: true; Layout.fillWidth: true; Layout.preferredHeight: 10 }
                        ColumnLayout {
                            id: systemButtonsLayout; spacing: 8; Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter
                            property var buttonModel: [
                                { iconSrc: "qrc:/icons/shutdown.svg", txt: "Stop system",       btnColor: operationsRoot.error, action: function() { apiService.stopSystem(); } },
                                { iconSrc: "qrc:/icons/start.svg",    txt: "Start system",   btnColor: operationsRoot.success, action: function() { apiService.startSystem(); } },
                                { iconSrc: "qrc:/icons/revert.svg",   txt: "Reverse system",   btnColor: accent, action: function() { apiService.reverseSystem(); } }
                            ]
                            Repeater {
                                model: systemButtonsLayout.buttonModel
                                delegate: Button {
                                    Layout.fillWidth: true; height: 56; background: Rectangle { color: "transparent" }
                                    contentItem: RowLayout {
                                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 12
                                        Rectangle {
                                            width: 40; height: 40; radius: 20; border.color: modelData.btnColor; border.width: 1; color: "transparent"
                                            Image {
                                                anchors.centerIn: parent; source: modelData.iconSrc; width: 20; height: 20
                                                fillMode: Image.PreserveAspectFit; layer.enabled: true
                                            }
                                        }
                                        Text {
                                            text: modelData.txt; font.pixelSize: 16; color: heading
                                            Layout.fillWidth: true; verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                    onClicked: modelData.action(); Accessible.name: modelData.txt
                                }
                            }
                        }
                        Item { Layout.fillHeight: true; Layout.fillWidth: true; Layout.preferredHeight: 10 }
                    }
                }
            }
        }
    }
}
