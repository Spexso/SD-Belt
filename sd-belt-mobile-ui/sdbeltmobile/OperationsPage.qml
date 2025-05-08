import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Item {
    id: operationsRoot
    anchors.fill: parent

    // Access parent window properties
    property color accent: window.accent
    property color borderColor: window.borderColor
    property color heading: window.heading
    property color subtext: window.subtext

        // Add timers for simulating system actions
        Timer {
            id: restartTimer
            interval: 3000 // 3 seconds for restart simulation
            onTriggered: {
                statusText.text = "Connected - Running"
                statusText.color = success
            }
        }

        Timer {
            id: shutdownTimer
            interval: 3000 // 3 seconds for shutdown simulation
            onTriggered: {
                statusText.text = "Disconnected"
                statusText.color = error
            }
        }

        Timer {
            id: sliderTimer
            interval: 2000 // 2 seconds to simulate adjustment completion
            onTriggered: {
                if (statusText.text.indexOf("Adjusting") >= 0) {
                    statusText.text = "Connected - Running"
                    statusText.color = success
                }
            }
        }

    // ────────────────────────────────────────────────────────────────────────
    //  Main Content (fits exactly between status bar and nav bar)
    // ────────────────────────────────────────────────────────────────────────
    ColumnLayout {
        id: body
        anchors.left   : parent.left
        anchors.right  : parent.right
        anchors.top    : parent.top
        anchors.bottom : parent.bottom
        anchors.margins: 16
        spacing        : 16

        // ---- Header with back button ----------------------------------------
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                width: 44
                height: 44
                radius: 8
                color: "#F0F0F0"

                Text {
                    anchors.centerIn: parent
                    text: "←"
                    font.pixelSize: 24
                    color: heading
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // Navigate back to Home page
                        window.activePage = "home"
                        stackView.replace(homePage)
                    }
                }
            }

            // ---- Title with centered text ----------------------------------
            Item { Layout.fillWidth: true }

            Text {
                text: "GTU Belt Operations"
                font.pixelSize: 18
                font.bold: true
                color: heading
            }

            Item { Layout.fillWidth: true }
        }

        // ---- Stats (4 rows) ----------------------------------------------
        ColumnLayout {
            spacing: 6
            Layout.fillWidth: true
            Layout.topMargin: 16

            RowLayout {
                Layout.fillWidth: true
                Text { text: "ID"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                Text { text: "af351113fa-aaa31-31f1-712341224242"; font.pixelSize: 14; font.bold: true; color: heading; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
            }

            RowLayout {
                Layout.fillWidth: true
                Text { text: "Runtime"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                Text { text: "1d 23h 45 minutes"; font.pixelSize: 14; font.bold: true; color: heading; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
            }

            RowLayout {
                Layout.fillWidth: true
                Text { text: "Status"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                Text {
                    id: statusText
                    text: "Connected"
                    font.pixelSize: 14
                    font.bold: true
                    color: success
                    wrapMode: Text.WrapAnywhere
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Text { text: "Total Scanned"; font.pixelSize: 14; color: subtext; Layout.preferredWidth: 118 }
                Text { text: "2321424"; font.pixelSize: 14; font.bold: true; color: heading; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
            }
        }

        // ---- Metric Cards (2×) ------------------------------------------
        Repeater {
            model: [
                { t: "Belt Speed", lbl: "Current Speed",   suf: "m/h", start: 20, hint: "Change belt speed" },
                { t: "Accuracy",   lbl: "Current Accuracy", suf: "%",   start: 80, hint: "Change belt accuracy" }
            ]
            delegate: Rectangle {
                width          : body.width
                radius         : 12
                border.color   : borderColor
                border.width   : 1
                color          : "#FFFFFF"
                Layout.preferredHeight: 136   // reduced to ensure overall fit
                property real val: modelData.start

                ColumnLayout {
                    anchors.fill   : parent
                    anchors.margins: 12
                    spacing        : 6

                    Text { text: modelData.t; font.pixelSize: 16; font.bold: true; color: heading }
                    Rectangle { height: 1; color: borderColor; Layout.fillWidth: true }

                    RowLayout { Layout.fillWidth: true
                        Text { text: modelData.lbl; font.pixelSize: 14; color: subtext }
                        Item { Layout.fillWidth: true }
                        Text { text: Math.round(val) + modelData.suf; font.pixelSize: 14; font.bold: true; color: heading }
                    }

                    // --- Custom slider (progress & knob) -----------------
                    Rectangle {
                        id: track; height: 6; radius: 3; color: "#E0E0E0"; Layout.fillWidth: true
                        Rectangle { id: prog; anchors.verticalCenter: parent.verticalCenter; height: parent.height; radius: parent.radius; color: accent; width: parent.width * val / 100 }
                        Rectangle {
                            id          : knob
                            width       : 18; height: 18; radius: 9
                            color       : accent
                            border.color: "#FFFFFF"; border.width: 1
                            y           : -6; x: prog.width - width/2
                            MouseArea {
                                anchors.fill: parent
                                drag.target : parent; drag.axis: Drag.XAxis
                                drag.minimumX: -parent.width/2
                                drag.maximumX: track.width - parent.width/2
                                onPositionChanged: {
                                    val = Math.round(Math.max(0, Math.min(100, (knob.x + knob.width/2) / track.width * 100)))

                                    // Update the operation status when slider is adjusted
                                    if (statusText.text.indexOf("Running") >= 0) {
                                        statusText.text = "Connected - Adjusting"
                                        statusText.color = accent

                                        // Reset after adjustment
                                        if (sliderTimer.running)
                                            sliderTimer.restart()
                                        else
                                            sliderTimer.start()
                                    }
                                }
                            }
                        }
                    }

                    Text { text: modelData.hint; font.pixelSize: 12; color: subtext }
                }
            }
        }

        // System Card with Big Buttons
        Rectangle {
            width: body.width; radius: 12; border.color: borderColor; border.width: 1; color: "#FFFFFF"; Layout.fillHeight: true
            ColumnLayout {
                anchors.fill: parent;
                anchors.margins: 12;
                spacing: 4

                Text {
                    text: "System";
                    font.pixelSize: 16;
                    font.bold: true;
                    color: heading
                }

                Rectangle {
                    height: 1;
                    color: borderColor;
                    Layout.fillWidth: true
                }

                Item {
                    // This spacer helps center the button column
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: 10
                }

                ColumnLayout {
                    spacing: 0; // Added spacing between buttons
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter // Center the column vertically

                    Repeater {
                        model: [
                            { i: "\u23FB", txt: "Shutdown",       col: "#D23C3C" },
                            { i: "\u21BB", txt: "Restart system", col: "#3578E5" },
                            { i: "\u25B6", txt: "Start system",   col: "#3AA547" },
                            { i: "\u23F8", txt: "Pause system",   col: "#DB8836" }
                        ]
                        delegate: Button {
                            id: sysBtn
                            Layout.fillWidth: false;
                            height: 56  // bigger button
                            background: Rectangle { color: "transparent" }
                            contentItem: RowLayout {
                                anchors.fill: parent;
                                anchors.margins: 1;
                                spacing: 12  // Increased spacing between icon and text

                                Rectangle {
                                    width: 40;
                                    height: 40;
                                    radius: 20;
                                    border.color: modelData.col;
                                    border.width: 1;
                                    color: "transparent"

                                    Text {
                                        anchors.centerIn: parent;
                                        text: modelData.i;
                                        font.pixelSize: 18;
                                        color: modelData.col
                                    }
                                }

                                Text {
                                    text: modelData.txt;
                                    font.pixelSize: 16; // Increased text size
                                    color: heading;
                                    Layout.fillWidth: true  // Make text take available space
                                    verticalAlignment: Text.AlignVCenter  // Center text vertically
                                }
                            }
                            onClicked: {
                                console.log(modelData.txt + " pressed")

                                // Update status in real-time
                                if (modelData.txt === "Start system") {
                                    statusText.text = "Connected - Running"
                                    statusText.color = success
                                } else if (modelData.txt === "Pause system") {
                                    statusText.text = "Connected - Paused"
                                    statusText.color = "#DB8836" // Orange
                                } else if (modelData.txt === "Restart system") {
                                    statusText.text = "Restarting..."
                                    statusText.color = "#3578E5" // Blue
                                    // Simulate restart
                                    restartTimer.start()
                                } else if (modelData.txt === "Shutdown") {
                                    statusText.text = "Shutting down..."
                                    statusText.color = error
                                    // Simulate shutdown
                                    shutdownTimer.start()
                                }
                            }
                        }
                    }
                }

                Item {
                    // This spacer helps center the button column
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: 10
                }
            }
        }
    }
}
