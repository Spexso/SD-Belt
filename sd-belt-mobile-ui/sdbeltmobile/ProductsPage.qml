import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Item {
    id: productsRoot
    anchors.fill: parent

    // Property to access the tab bar from outside
    property alias tabBar: productTabs

    // Access parent window properties
    property color accent: window.accent
    property color borderColor: window.borderColor
    property color heading: window.heading
    property color subtext: window.subtext
    property color success: window.success
    property color error: window.error

    Rectangle { anchors.fill: parent; color: "#FFFBFF"; radius: 24 }

    // ────────────────────────────────────────────────────────────────────────
    //  Main Content
    // ────────────────────────────────────────────────────────────────────────
    ColumnLayout {
        id: body
        anchors.left   : parent.left
        anchors.right  : parent.right
        anchors.top    : parent.top
        anchors.bottom : parent.bottom
        anchors.margins: 16
        spacing        : 16

        // ---- Header Section -------------------------------------------
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

            Text {
                text: "Products"
                font.pixelSize: 20
                font.bold: true
                color: heading
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 40
                height: 40
                radius: 8
                color: "#F0F0F0"

                Text {
                    anchors.centerIn: parent
                    text: "🔍"
                    font.pixelSize: 18
                    color: heading
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Search products clicked")
                }
            }

            Rectangle {
                width: 40
                height: 40
                radius: 8
                color: "#F0F0F0"

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    font.pixelSize: 24
                    color: heading
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Add product clicked")
                }
            }
        }

        // ---- Tab Navigation ------------------------------------------
        TabBar {
            id: productTabs
            Layout.fillWidth: true
            currentIndex: window.selectedProductsTab

            TabButton {
                text: "All Products"
                width: implicitWidth
                font.pixelSize: 14
            }

            TabButton {
                text: "Latest Scans"
                width: implicitWidth
                font.pixelSize: 14
            }

            TabButton {
                text: "Statistics"
                width: implicitWidth
                font.pixelSize: 14
            }

            // Connect changes to update the window property
            onCurrentIndexChanged: window.selectedProductsTab = currentIndex
        }

        // Tab content stack
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: productTabs.currentIndex

            // ---- ALL PRODUCTS TAB ----------------------------------------
            Item {
                // All Products tab content
                ListView {
                    anchors.fill: parent
                    clip: true
                    spacing: 12

                    model: ListModel {
                        ListElement { name: "Plastic Bottle"; category: "Recyclable"; success: 8523; error: 1532 }
                        ListElement { name: "Glass Bottle"; category: "Recyclable"; success: 6234; error: 935 }
                        ListElement { name: "Metal Can"; category: "Recyclable"; success: 4325; error: 632 }
                        ListElement { name: "Paper"; category: "Recyclable"; success: 3256; error: 845 }
                        ListElement { name: "Food Waste"; category: "Organic"; success: 2753; error: 1263 }
                        ListElement { name: "Plastic Bag"; category: "Non-recyclable"; success: 1856; error: 842 }
                    }

                    delegate: Rectangle {
                        width: parent.width
                        height: 80
                        radius: 12
                        border.width: 1
                        border.color: borderColor
                        color: "#FFFFFF"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Rectangle {
                                width: 56
                                height: 56
                                radius: 8
                                color: "#F5F5F5"

                                Text {
                                    anchors.centerIn: parent
                                    text: "📷"
                                    font.pixelSize: 24
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: model.name
                                    font.pixelSize: 16
                                    font.bold: true
                                    color: heading
                                }

                                Text {
                                    text: model.category
                                    font.pixelSize: 12
                                    color: subtext
                                }

                                RowLayout {
                                    spacing: 12

                                    Row {
                                        spacing: 4

                                        Rectangle {
                                            width: 10
                                            height: 10
                                            radius: 5
                                            color: success
                                        }

                                        Text {
                                            text: model.success
                                            font.pixelSize: 12
                                            color: success
                                        }
                                    }

                                    Row {
                                        spacing: 4

                                        Rectangle {
                                            width: 10
                                            height: 10
                                            radius: 5
                                            color: error
                                        }

                                        Text {
                                            text: model.error
                                            font.pixelSize: 12
                                            color: error
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                width: 32
                                height: 32
                                radius: 16
                                color: "#F5F5F5"

                                Text {
                                    anchors.centerIn: parent
                                    text: "→"
                                    font.pixelSize: 16
                                    color: accent
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: console.log("View details for " + model.name)
                                }
                            }
                        }
                    }
                }
            }

            // ---- LATEST SCANS TAB ----------------------------------------
            Item {
                // Latest Scans tab content
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Text {
                        text: "Today"
                        font.pixelSize: 16
                        font.bold: true
                        color: heading
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        clip: true
                        spacing: 8

                        model: ListModel {
                            ListElement { product: "Plastic Bottle"; time: "14:32"; success: true }
                            ListElement { product: "Metal Can"; time: "14:20"; success: true }
                            ListElement { product: "Glass Bottle"; time: "13:55"; success: false }
                            ListElement { product: "Paper"; time: "13:22"; success: true }
                            ListElement { product: "Plastic Bag"; time: "12:18"; success: false }
                        }

                        delegate: Rectangle {
                            width: parent.width
                            height: 70
                            radius: 12
                            border.width: 1
                            border.color: borderColor
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
                                        text: "📷"
                                        font.pixelSize: 20
                                    }
                                }

                                Column {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        text: model.product
                                        font.pixelSize: 16
                                        color: heading
                                    }

                                    Text {
                                        text: "Scanned at " + model.time
                                        font.pixelSize: 12
                                        color: subtext
                                    }
                                }

                                Rectangle {
                                    width: 80
                                    height: 30
                                    radius: 15
                                    color: model.success ? "#E8F5E9" : "#FFEBEE"

                                    Text {
                                        anchors.centerIn: parent
                                        text: model.success ? "Başarılı" : "Başarısız"
                                        font.pixelSize: 12
                                        color: model.success ? success : error
                                    }
                                }
                            }
                        }
                    }

                    Text {
                        text: "Yesterday"
                        font.pixelSize: 16
                        font.bold: true
                        color: heading
                        Layout.topMargin: 16
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 8

                        model: ListModel {
                            ListElement { product: "Plastic Bottle"; time: "17:45"; success: true }
                            ListElement { product: "Food Waste"; time: "16:32"; success: false }
                            ListElement { product: "Glass Bottle"; time: "15:18"; success: true }
                        }

                        delegate: Rectangle {
                            width: parent.width
                            height: 70
                            radius: 12
                            border.width: 1
                            border.color: borderColor
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
                                        text: "📷"
                                        font.pixelSize: 20
                                    }
                                }

                                Column {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        text: model.product
                                        font.pixelSize: 16
                                        color: heading
                                    }

                                    Text {
                                        text: "Scanned at " + model.time
                                        font.pixelSize: 12
                                        color: subtext
                                    }
                                }

                                Rectangle {
                                    width: 80
                                    height: 30
                                    radius: 15
                                    color: model.success ? "#E8F5E9" : "#FFEBEE"

                                    Text {
                                        anchors.centerIn: parent
                                        text: model.success ? "Başarılı" : "Başarısız"
                                        font.pixelSize: 12
                                        color: model.success ? success : error
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ---- STATISTICS TAB ----------------------------------------
            Item {
                // Statistics tab content
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    // Success Rate Card
                    Rectangle {
                        Layout.fillWidth: true
                        height: 100
                        radius: 12
                        color: "#F5F7FF"
                        border.width: 1
                        border.color: borderColor

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 4

                            Text {
                                text: "Success Rate"
                                font.pixelSize: 14
                                color: subtext
                            }

                            Text {
                                text: "84%"
                                font.pixelSize: 28
                                font.bold: true
                                color: success
                            }

                            // Simple progress bar for visualization
                            Rectangle {
                                Layout.fillWidth: true
                                height: 8
                                radius: 4
                                color: "#E0E0E0"

                                Rectangle {
                                    width: parent.width * 0.84
                                    height: parent.height
                                    radius: parent.radius
                                    color: success
                                }
                            }
                        }
                    }

                    // Product Categories Stats
                    Rectangle {
                        Layout.fillWidth: true
                        height: 200
                        radius: 12
                        color: "#FFFFFF"
                        border.width: 1
                        border.color: borderColor

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Text {
                                text: "Product Categories"
                                font.pixelSize: 16
                                font.bold: true
                                color: heading
                            }

                            // Category breakdown
                            Repeater {
                                model: [
                                    { name: "Recyclable", count: 68, color: "#4CAF50" },
                                    { name: "Organic", count: 22, color: "#2196F3" },
                                    { name: "Non-recyclable", count: 10, color: "#FF5722" }
                                ]

                                delegate: ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: modelData.name
                                            font.pixelSize: 14
                                            color: heading
                                        }

                                        Item { Layout.fillWidth: true }

                                        Text {
                                            text: modelData.count + "%"
                                            font.pixelSize: 14
                                            font.bold: true
                                            color: heading
                                        }
                                    }

                                    // Progress bar showing percentage
                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 8
                                        radius: 4
                                        color: "#E0E0E0"

                                        Rectangle {
                                            width: parent.width * (modelData.count / 100)
                                            height: parent.height
                                            radius: parent.radius
                                            color: modelData.color
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Weekly Performance
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 12
                        color: "#FFFFFF"
                        border.width: 1
                        border.color: borderColor

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Text {
                                text: "Weekly Performance"
                                font.pixelSize: 16
                                font.bold: true
                                color: heading
                            }

                            // Simple bar chart
                            Row {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: (parent.width - 7*30) / 6  // Evenly space 7 bars

                                Repeater {
                                    model: [
                                        { day: "M", success: 65, error: 35 },
                                        { day: "T", success: 72, error: 28 },
                                        { day: "W", success: 81, error: 19 },
                                        { day: "T", success: 75, error: 25 },
                                        { day: "F", success: 85, error: 15 },
                                        { day: "S", success: 60, error: 40 },
                                        { day: "S", success: 78, error: 22 }
                                    ]

                                    delegate: Column {
                                        spacing: 4
                                        height: parent.height

                                        Item {
                                            width: 30
                                            height: parent.height - 40

                                            // Error bar (bottom)
                                            Rectangle {
                                                anchors.bottom: parent.bottom
                                                width: parent.width
                                                height: parent.height * (modelData.error / 100)
                                                color: error + "60"  // Semi-transparent
                                            }

                                            // Success bar (top)
                                            Rectangle {
                                                anchors.bottom: parent.bottom
                                                anchors.bottomMargin: parent.height * (modelData.error / 100)
                                                width: parent.width
                                                height: parent.height * (modelData.success / 100)
                                                color: success
                                            }
                                        }

                                        Text {
                                            text: modelData.day
                                            font.pixelSize: 12
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            color: subtext
                                        }

                                        Text {
                                            text: modelData.success + "%"
                                            font.pixelSize: 12
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            color: heading
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
