import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

ApplicationWindow {
    id: window
    width: 392
    height: 780
    visible: true
    title: "GTU Belt"

    readonly property color accent: "#6A5CFF"
    readonly property color borderColor: "#D0D0D0"
    readonly property color heading: "#3A3A3A"
    readonly property color subtext: "#6B6B6B"
    readonly property color success: "#3AA547"
    readonly property color error: "#D23C3C"

    Material.theme: Material.Light
    Material.accent: accent

    property int selectedProductsTab: 0
    property bool isLoggedIn: false
    property string activePage: isLoggedIn ? "home" : "login"

    function navigateToProductsTab(tabIndex) {
        activePage = "products"
        if (stackView.currentItem && stackView.currentItem.objectName !== "productsPage") {
            stackView.replace(productsPage)
        } else if (!stackView.currentItem) {
             stackView.replace(productsPage)
        }
        selectedProductsTab = tabIndex
    }

    function logout() {
        console.log("Main.qml: Logging out");
        isLoggedIn = false;
        activePage = "login";
        stackView.clear();
        stackView.push(loginPage);
    }

    Rectangle {
        anchors.fill: parent
        color: "#FFFBFF"
        radius: 0
    }

    StackView {
        id: stackView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.bottom: bottomNav.visible ? bottomNav.top : parent.bottom
        clip: true
        initialItem: isLoggedIn ? homePage : loginPage
    }

    Component {
        id: loginPage
        Loader {
            source: "LoginPage.qml"
            objectName: "loginPage"
            onLoaded: {
                if (item && item.loginSuccessful) {
                    item.loginSuccessful.connect(function() {
                        isLoggedIn = true
                        activePage = "home"
                        stackView.replace(homePage)
                    })
                }
            }
        }
    }

    Component {
        id: homePage
        Loader {
            source: "HomePage.qml"
            objectName: "homePage"
        }
    }

    Component {
        id: operationsPage
        Loader {
            source: "OperationsPage.qml"
            objectName: "operationsPage"
        }
    }

    Component {
        id: productsPage
        Loader {
            source: "ProductsPage.qml"
            objectName: "productsPage"
        }
    }

    Component {
        id: productStatsPageComponent
        Loader {
            id: statsLoader
            source: "ProductStatsPage.qml"
            objectName: "productStatsPageInstance"

            property var p_productId
            property var p_productName
            property var p_productIcon
            property var p_successScans
            property var p_errorScans

            onLoaded: {
                if (item) {
                    if (p_productId !== undefined && item.productId !== p_productId) item.productId = p_productId;
                    if (p_productName !== undefined && item.productName !== p_productName) item.productName = p_productName;
                    if (p_productIcon !== undefined && item.productIcon !== p_productIcon) item.productIcon = p_productIcon;
                    if (p_successScans !== undefined && item.successScans !== p_successScans) item.successScans = p_successScans;
                    if (p_errorScans !== undefined && item.errorScans !== p_errorScans) item.errorScans = p_errorScans;

                    if (item.backToProductsRequested) {
                        item.backToProductsRequested.connect(function() {
                            stackView.pop();
                        });
                    }
                } else {
                    console.error("ProductStatsPage (Loader.onLoaded): item is NULL!");
                }
            }

            onP_productIdChanged:   if (item && p_productId !== undefined) { item.productId = p_productId; }
            onP_productNameChanged: if (item && p_productName !== undefined) { item.productName = p_productName; }
            onP_productIconChanged: if (item && p_productIcon !== undefined) { item.productIcon = p_productIcon; }
            onP_successScansChanged:if (item && p_successScans !== undefined) { item.successScans = p_successScans; }
            onP_errorScansChanged:  if (item && p_errorScans !== undefined) { item.errorScans = p_errorScans; }
        }
    }

    Rectangle { // Bottom Navigation Bar
        id: bottomNav
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 92
        radius: 24 ;Behavior on radius {NumberAnimation{}}
        color: "#FFFFFF"
        border.color: borderColor
        border.width: 1
        visible: isLoggedIn

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            Repeater {
                model: [
                    { icoSrc: "qrc:/icons/home.svg", lab: "Home", pageId: "home", component: homePage },
                    { icoSrc: "qrc:/icons/operations.svg", lab: "Operations", pageId: "operations", component: operationsPage },
                    { icoSrc: "qrc:/icons/products.svg", lab: "Products", pageId: "products", component: productsPage }
                ]
                delegate: Item {
                    property bool isSelected: activePage === modelData.pageId
                    Layout.fillWidth: true
                    height: parent.height

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (activePage !== modelData.pageId) {
                                console.log("Navigating to: " + modelData.lab)
                                activePage = modelData.pageId
                                stackView.replace(modelData.component)
                            }
                        }
                    }
                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 4
                        Image {
                            source: modelData.icoSrc
                            width: 24
                            height: 24
                            Layout.alignment: Qt.AlignHCenter
                            fillMode: Image.PreserveAspectFit
                            layer.enabled: true
                        }
                        Text {
                            text: modelData.lab; font.pixelSize: 13
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
