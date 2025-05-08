import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Item {
    id: homeRoot
    anchors.fill: parent

    // Access parent window properties
    property color accent: window.accent
    property color borderColor: window.borderColor
    property color heading: window.heading
    property color subtext: window.subtext
    property color success: window.success
    property color error: window.error

    Rectangle { anchors.fill: parent; color: "#FFFBFF"; radius: 24 }

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
        spacing        : 12  // Reduced spacing to fit content better

        // ---- Header with Back Button and Title -------------------------
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
                    text: "≡"
                    font.pixelSize: 24
                    color: heading
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Menu clicked")
                }
            }

            Item { Layout.fillWidth: true }

            Column {
                spacing: 2
                Layout.alignment: Qt.AlignCenter

                Text {
                    text: "Jonah Freddy Atson"
                    font.pixelSize: 15
                    color: heading
                    horizontalAlignment: Text.AlignRight
                }

                Text {
                    text: "Belt: GTU Belt"
                    font.pixelSize: 14
                    color: subtext
                    horizontalAlignment: Text.AlignRight
                }
            }

            Rectangle {
                width: 44
                height: 44
                radius: 22
                color: "#3578E5"

                Text {
                    anchors.centerIn: parent
                    text: "👤"
                    font.pixelSize: 20
                    color: "white"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Profile clicked")
                }
            }
        }

        // ---- System Control Buttons ----------------------------------
        Row {
            spacing: 8
            Layout.fillWidth: true

            // Shutdown button
            Rectangle {
                width: (parent.width - 16) / 3  // Distribute evenly with spacing
                height: 56
                radius: 12
                border.width: 1
                border.color: error
                color: "transparent"

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: "⏻"
                        font.pixelSize: 18
                        color: error
                    }

                    Text {
                        text: "Shutdown"
                        font.pixelSize: 14
                        color: error
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Shutdown clicked")
                }
            }

            // Restart button
            Rectangle {
                width: (parent.width - 16) / 3  // Distribute evenly with spacing
                height: 56
                radius: 12
                border.width: 1
                border.color: accent
                color: "transparent"

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: "↻"
                        font.pixelSize: 18
                        color: accent
                    }

                    Text {
                        text: "Restart"
                        font.pixelSize: 14
                        color: accent
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Restart clicked")
                }
            }

            // Start button
            Rectangle {
                width: (parent.width - 16) / 3  // Distribute evenly with spacing
                height: 56
                radius: 12
                border.width: 1
                border.color: success
                color: "transparent"

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: "▶"
                        font.pixelSize: 18
                        color: success
                    }

                    Text {
                        text: "Start"
                        font.pixelSize: 14
                        color: success
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Start clicked")
                }
            }
        }

        // ---- System Section with Border -----------------------------------
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 16

            Text {
                text: "System"
                font.pixelSize: 16
                color: heading
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 24
                height: 20
                color: "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "☰"
                    font.pixelSize: 15
                    color: subtext
                }
            }
        }

        Rectangle {
            height: 1
            color: borderColor
            Layout.fillWidth: true
        }

        // ---- System Stats ------------------------------------------
        GridLayout {
            Layout.fillWidth: true
            Layout.topMargin: 16
            columnSpacing: 10
            rowSpacing: 25
            columns: 2

            // Header with title and view more button
            RowLayout {
                Layout.columnSpan: 2
                Layout.fillWidth: true

                Text {
                    text: "System Stats"
                    font.pixelSize: 16
                    font.bold: true
                    color: heading
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    width: 28
                    height: 28
                    radius: 14
                    color: "#F0F0F0"

                    Text {
                        anchors.centerIn: parent
                        text: "→"
                        font.pixelSize: 14
                        color: accent
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // Navigate to Products page and select the Statistics tab
                            window.navigateToProductsTab(2)  // Index 2 is Statistics
                        }
                    }
                }
            }

            // Runtime
            Column {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: "1d 3h 5m"
                    font.pixelSize: 22
                    font.bold: true
                    color: heading
                }

                Text {
                    text: "Runtime"
                    font.pixelSize: 13
                    color: subtext
                }
            }

            // Valid scans
            Column {
                width: 80
                spacing: 2

                Text {
                    text: "278"
                    font.pixelSize: 22
                    font.bold: true
                    color: success
                    anchors.right: parent.right
                }

                Text {
                    text: "valid scan"
                    font.pixelSize: 13
                    color: subtext
                    anchors.right: parent.right
                }
            }

            // Scanned today
            Column {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: "351"
                    font.pixelSize: 22
                    font.bold: true
                    color: heading
                }

                Text {
                    text: "Scanned today"
                    font.pixelSize: 13
                    color: subtext
                }
            }

            // Failed scans
            Column {
                width: 80
                spacing: 2

                Text {
                    text: "56"
                    font.pixelSize: 22
                    font.bold: true
                    color: error
                    anchors.right: parent.right
                }

                Text {
                    text: "failed scan"
                    font.pixelSize: 13
                    color: subtext
                    anchors.right: parent.right
                }
            }
        }

        // ---- Latest Scan Section -----------------------------------
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: 16
            spacing: 8

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Latest Scan"
                    font.pixelSize: 16
                    font.bold: true
                    color: heading
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    width: 32
                    height: 32
                    radius: 8
                    color: "transparent"
                    border.width: 1
                    border.color: accent

                    Text {
                        anchors.centerIn: parent
                        text: "→"
                        font.pixelSize: 16
                        color: accent
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // Navigate to Products page and select the Latest Scans tab
                            window.navigateToProductsTab(1)  // Index 1 is Latest Scans
                        }
                    }
                }
            }

            // Scan Item 1
            Rectangle {
                Layout.fillWidth: true
                height: 56
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: "#F5F5F5"

                        Text {
                            anchors.centerIn: parent
                            text: "📷"
                            font.pixelSize: 18
                        }
                    }

                    Text {
                        text: "Plastic Bottle"
                        font.pixelSize: 14
                        color: heading
                        Layout.fillWidth: true
                    }

                    Column {
                        spacing: 4
                        width: 80

                        Text {
                            text: "Başarılı"
                            font.pixelSize: 13
                            color: success
                            anchors.right: parent.right
                        }

                        Text {
                            text: "Dün 22:50"
                            font.pixelSize: 11
                            color: subtext
                            anchors.right: parent.right
                        }
                    }
                }
            }

            // Scan Item 2
            Rectangle {
                Layout.fillWidth: true
                height: 56
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: "#F5F5F5"

                        Text {
                            anchors.centerIn: parent
                            text: "📷"
                            font.pixelSize: 18
                        }
                    }

                    Text {
                        text: "Plastic Bottle"
                        font.pixelSize: 14
                        color: heading
                        Layout.fillWidth: true
                    }

                    Column {
                        spacing: 4
                        width: 80

                        Text {
                            text: "Başarısız"
                            font.pixelSize: 13
                            color: error
                            anchors.right: parent.right
                        }

                        Text {
                            text: "Dün 21:04"
                            font.pixelSize: 11
                            color: subtext
                            anchors.right: parent.right
                        }
                    }
                }
            }
        }

        // ---- Products Section -------------------------------------
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: 8
            spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Products"
                font.pixelSize: 16
                font.bold: true
                color: heading
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 32
                height: 32
                radius: 8
                color: "transparent"
                border.width: 1
                border.color: accent

                Text {
                    anchors.centerIn: parent
                    text: "→"
                    font.pixelSize: 16
                    color: accent
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // Navigate to Products page
                        window.activePage = "products"
                        stackView.replace(productsPage)
                    }
                }
            }
        }

            // Product Item 1
            Rectangle {
                Layout.fillWidth: true
                height: 56
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: "#F5F5F5"

                        Text {
                            anchors.centerIn: parent
                            text: "📷"
                            font.pixelSize: 18
                        }
                    }

                    Text {
                        text: "Plastic Bottle"
                        font.pixelSize: 14
                        color: heading
                        Layout.fillWidth: true
                    }

                    Column {
                        spacing: 4
                        width: 80

                        Text {
                            text: "324"
                            font.pixelSize: 13
                            color: success
                            anchors.right: parent.right
                        }

                        Text {
                            text: "321"
                            font.pixelSize: 11
                            color: error
                            anchors.right: parent.right
                        }
                    }
                }
            }

            // Product Item 2
            Rectangle {
                Layout.fillWidth: true
                height: 56
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: "#F5F5F5"

                        Text {
                            anchors.centerIn: parent
                            text: "📷"
                            font.pixelSize: 18
                        }
                    }

                    Text {
                        text: "Golden Fish"
                        font.pixelSize: 14
                        color: heading
                        Layout.fillWidth: true
                    }

                    Column {
                        spacing: 4
                        width: 80

                        Text {
                            text: "324"
                            font.pixelSize: 13
                            color: success
                            anchors.right: parent.right
                        }

                        Text {
                            text: "321"
                            font.pixelSize: 11
                            color: error
                            anchors.right: parent.right
                        }
                    }
                }
            }
        }
    }
}
