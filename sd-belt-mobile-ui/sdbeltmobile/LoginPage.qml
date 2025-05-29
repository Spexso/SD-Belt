import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Item {
    id: loginScreen
    anchors.fill: parent

    property color accent: window.accent
    property color borderColor: window.borderColor
    property color heading: window.heading
    property color subtext: window.subtext
    property color errorColor: window.error

    signal loginSuccessful()

    property bool isLoading: false
    property string errorMessage: ""

    Connections {
        target: apiService
        function onLoginSuccess(token, message) {
            isLoading = false;
            errorMessage = "";
            loginScreen.loginSuccessful();
        }
        function onLoginFailed(message) {
            isLoading = false;
            errorMessage = message;
        }
        function onNetworkError(endpoint, errorString) {
            if (endpoint.includes("/login")) {
                isLoading = false;
                errorMessage = "Network connection issue. Please try again.";
            }
        }
    }

    Rectangle { // Page Background
        anchors.fill: parent
        color: "#FFFBFF"
    }

    ColumnLayout { // Main body layout
        id: body
        anchors.fill: parent
        anchors.topMargin: 30
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        anchors.bottomMargin: 16
        spacing: 20

        // --- HEADER SECTION ---
        ColumnLayout {
            id: headerContentLayout
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            Image {
                id: appLogo
                source: "qrc:/icons/app_logo.png"
                Layout.preferredHeight: 100
                Layout.maximumWidth: parent.width * 0.85
                Layout.preferredWidth: implicitWidth

                Layout.alignment: Qt.AlignHCenter
                fillMode: Image.PreserveAspectFit
                smooth: true

                onStatusChanged: {
                    if (status === Image.Error) {
                        console.warn("LoginPage: Failed to load app_logo.png. Source: " + source);
                    }
                }
            }

            Text {
                text: "Sign in to continue"
                font.pixelSize: 17
                font.weight: Font.Normal
                color: loginScreen.subtext
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 4
            }
        }
        // --- END OF HEADER SECTION ---


        Rectangle { // Login Form Card
            id: loginFormCard
            Layout.fillWidth: true
            Layout.preferredHeight: 360
            radius: 16
            color: "#FFFFFF"
            border.width: 1
            border.color: loginScreen.borderColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 18

                ColumnLayout {
                    Layout.fillWidth: true; spacing: 6
                    Text { text: "Username"; font.pixelSize: 14; color: loginScreen.heading }
                    Rectangle {
                        id: usernameFieldContainer
                        Layout.fillWidth: true; height: 56; radius: 12
                        color: "#F7F9FC"
                        antialiasing: true
                        border.width: usernameField.activeFocus ? 2.5 : 2
                        border.color: usernameField.activeFocus ? loginScreen.accent : "#B0B0B0"
                        Behavior on border.color { ColorAnimation { duration: 150 } }
                        Behavior on border.width { NumberAnimation { duration: 100 } }

                        TextField {
                            id: usernameField
                            anchors.fill: parent
                            anchors.leftMargin: 16; anchors.rightMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
                            placeholderText: "Enter your username"
                            placeholderTextColor: Qt.lighter(loginScreen.subtext, 1.2)
                            color: loginScreen.heading; font.pixelSize: 16
                            selectByMouse: true
                            background: Rectangle { color: usernameFieldContainer.color }
                            Keys.onEnterPressed: passwordField.focus = true
                            Keys.onReturnPressed: passwordField.focus = true
                            onTextChanged: errorMessage = ""
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true; spacing: 6
                    Text { text: "Password"; font.pixelSize: 14; color: loginScreen.heading }
                    Rectangle {
                        id: passwordFieldContainer
                        Layout.fillWidth: true; height: 56; radius: 12
                        color: "#F7F9FC"
                        antialiasing: true
                        border.width: passwordField.activeFocus ? 2.5 : 2
                        border.color: passwordField.activeFocus ? loginScreen.accent : "#B0B0B0"
                        Behavior on border.color { ColorAnimation { duration: 150 } }
                        Behavior on border.width { NumberAnimation { duration: 100 } }

                        TextField {
                            id: passwordField
                            anchors.fill: parent
                            anchors.leftMargin: 16; anchors.rightMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
                            placeholderText: "Enter your password"
                            placeholderTextColor: Qt.lighter(loginScreen.subtext, 1.2)
                            color: loginScreen.heading; font.pixelSize: 16
                            echoMode: TextInput.Password
                            selectByMouse: true
                            background: Rectangle { color: passwordFieldContainer.color }
                            Keys.onEnterPressed: loginButtonMouseArea.clicked()
                            Keys.onReturnPressed: loginButtonMouseArea.clicked()
                            onTextChanged: errorMessage = ""
                        }
                    }
                }

                Text {
                    id: errorText
                    Layout.fillWidth: true
                    text: loginScreen.errorMessage
                    color: loginScreen.errorColor
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    visible: loginScreen.errorMessage !== ""
                    Layout.topMargin: 4
                    Layout.bottomMargin: 4
                }


                Rectangle {
                    id: loginButton
                    Layout.fillWidth: true
                    height: 56; radius: 12
                    color: loginButtonMouseArea.enabled ? loginScreen.accent : Qt.rgba(loginScreen.accent.r, loginScreen.accent.g, loginScreen.accent.b, 0.20)

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8
                        visible: !loginScreen.isLoading

                        Text {
                            text: "Sign In"
                            font.pixelSize: 16; font.weight: Font.Medium
                            color: loginButtonMouseArea.enabled ? "white" : Qt.rgba(loginScreen.accent.r, loginScreen.accent.g, loginScreen.accent.b, 0.65)
                        }
                    }
                     BusyIndicator {
                        anchors.centerIn: parent
                        running: loginScreen.isLoading
                        visible: loginScreen.isLoading
                        Material.accent: "white"
                    }

                    MouseArea {
                        id: loginButtonMouseArea
                        anchors.fill: parent
                        enabled: usernameField.text.trim().length > 0 && passwordField.text.trim().length > 0 && !loginScreen.isLoading
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: {
                            if (enabled) {
                                loginScreen.isLoading = true;
                                loginScreen.errorMessage = "";
                                apiService.login(usernameField.text.trim(), passwordField.text.trim());
                            }
                        }
                    }
                    Rectangle {
                        anchors.fill: parent; radius: parent.radius; color: "black"
                        opacity: loginButtonMouseArea.pressed && loginButtonMouseArea.enabled ? 0.12 : (loginButtonMouseArea.hovered && loginButtonMouseArea.enabled ? 0.06 : 0)
                        visible: loginButtonMouseArea.enabled
                        Behavior on opacity { OpacityAnimator { duration: 100 } }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true; Layout.minimumHeight: 20 }

        Text { // Footer
            text: "SD Belt by Belt Innovators"; font.pixelSize: 13
            color: loginScreen.subtext; Layout.alignment: Qt.AlignHCenter
        }
    }
}
