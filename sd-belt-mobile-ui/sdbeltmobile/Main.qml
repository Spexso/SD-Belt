import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

ApplicationWindow {
    id: window
    width: 392                // logical dp width tested on Pixel‑5‑like devices
    height: 780               // logical dp height
    visible: true
    title: "GTU Belt"

    readonly property color accent      : "#6A5CFF"
    readonly property color borderColor : "#D0D0D0"
    readonly property color heading     : "#3A3A3A"
    readonly property color subtext     : "#6B6B6B"
    readonly property color success     : "#3AA547"
    readonly property color error       : "#D23C3C"

    Material.theme  : Material.Light
    Material.accent : accent

    // Properties to control tab selection in Products page
    property int selectedProductsTab: 0

    // Function to navigate to Products page with specific tab
    function navigateToProductsTab(tabIndex) {
        activePage = "products"
        stackView.replace(productsPage)
        selectedProductsTab = tabIndex
    }

    // Store the currently active page
    property string activePage: "home"

    Rectangle { anchors.fill: parent; color: "#FFFBFF"; radius: 24 }

    // Main stack for page navigation
    StackView {
        id: stackView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: bottomNav.top
        initialItem: homePage  // Start with home page (settings layout)
    }

    // Load pages as components
    Component {
        id: homePage
        Loader {
            source: "HomePage.qml"  // HomePage.qml will now use the settings page layout
        }
    }

    Component {
        id: operationsPage
        Loader {
            source: "OperationsPage.qml"
        }
    }

    Component {
        id: productsPage
        Loader {
            source: "ProductsPage.qml"
        }
    }

    // ────────────────────────────────────────────────────────────────────────
    //  Bottom Navigation Bar
    // ────────────────────────────────────────────────────────────────────────
    Rectangle {
        id: bottomNav
        anchors.left   : parent.left
        anchors.right  : parent.right
        anchors.bottom : parent.bottom
        height         : 92
        radius         : 24, 24, 0, 0
        color          : "#FFFFFF"
        border.color   : borderColor
        border.width   : 1

        RowLayout {
            anchors.fill    : parent
            anchors.margins: 8
            Repeater {
                model: [
                    { ico: "\u2302", lab: "Home", page: "home" },
                    { ico: "\u25A3", lab: "Operations", page: "operations" },
                    { ico: "\u25A4", lab: "Products", page: "products" }
                ]
                delegate: Item {
                    property bool isSelected: activePage === modelData.page
                    Layout.fillWidth: true
                    height: parent.height

                    // This MouseArea covers the entire item
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log("Clicked on: " + modelData.lab)
                            // Set active page
                            activePage = modelData.page

                            // Navigate to selected page
                            if (activePage === "home") {
                                stackView.replace(homePage)
                            } else if (activePage === "operations") {
                                stackView.replace(operationsPage)
                            } else if (activePage === "products") {
                                stackView.replace(productsPage)
                            }
                        }
                    }

                    // ColumnLayout for visual elements
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 4

                        Text {
                            text: modelData.ico
                            font.pixelSize: 24
                            horizontalAlignment: Text.AlignHCenter
                            Layout.alignment: Qt.AlignHCenter
                            color: isSelected ? accent : heading
                        }
                        Text {
                            text: modelData.lab
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            Layout.alignment: Qt.AlignHCenter
                            color: isSelected ? accent : heading
                        }
                    }
                }
            }
        }
    }
}
