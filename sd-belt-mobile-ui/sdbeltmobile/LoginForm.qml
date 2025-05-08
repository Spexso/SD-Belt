import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15


ApplicationWindow {
    id: window
    width: 360
    height: 640
    visible: true
    title: "Sign in"

    // overall light‑lavender background like the screenshot
    color: "#FFFBFF"            // very subtle pink‑lavender tint

    // centralised design variables
    readonly property color accent: "#6A5CFF"

    // We stick to Material Light but override accent explicitly
    Material.theme: Material.Light
    Material.accent: accent

    ColumnLayout {
        id: stack
        anchors.fill: parent
        anchors.margins: 32
        spacing: 28

        // ── Header ───────────────────────────────────────────────────────
        ColumnLayout {
            spacing: 6
            Text {
                text: "Sign in to"
                font.pixelSize: 32
                font.bold: true
                color: "#000000"
            }
            Text {
                text: "Belt System"
                font.pixelSize: 32
                font.bold: true
                color: "#000000"
            }
            Text {
                text: "Enter your email and password to log in"
                font.pixelSize: 15
                color: "#666666"
            }
        }

        // ── Email input ─────────────────────────────────────────────────
        ColumnLayout {
            spacing: 8
            Text {
                text: "Email"
                font.pixelSize: 15
                color: "#000000"
            }
            TextField {
                id: emailField
                placeholderText: "you@example.com"
                placeholderTextColor: accent   // shown purple in reference
                Layout.fillWidth: true
                height: 52
                inputMethodHints: Qt.ImhEmailCharactersOnly

                background: Rectangle {
                    radius: 12
                    color: "#FFFFFF"
                    border.color: "#DADADA"
                }
            }
        }

        // ── Password input ──────────────────────────────────────────────
        ColumnLayout {
            spacing: 8
            Text {
                text: "Password"
                font.pixelSize: 15
                color: "#000000"
            }
            TextField {
                id: passwordField
                placeholderText: "Password"
                placeholderTextColor: accent
                echoMode: TextInput.Password
                Layout.fillWidth: true
                height: 52

                background: Rectangle {
                    radius: 12
                    color: "#FFFFFF"
                    border.color: "#DADADA"
                }
            }
        }

        // ── Forgot password link ────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            Text {
                Layout.alignment: Qt.AlignLeft
                text: "Forgot Password ?"
                font.pixelSize: 14
                color: accent
                MouseArea {
                    anchors.fill: parent
                    onClicked: forgotClicked()
                }
            }
        }

        // ── Log in button (lavender -> deep purple gradient) ─────────---
        Button {
            id: loginButton
            text: "Log In"
            Layout.fillWidth: true
            height: 52
            enabled: emailField.text.length > 0 && passwordField.text.length > 0

            contentItem: Text {
                text: loginButton.text
                font.pixelSize: 17
                font.bold: true
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.fill: parent
            }

            background: Rectangle {
                radius: 12
                gradient: Gradient {
                    // lighter top, darker bottom as per screenshot
                    GradientStop { position: 0.0; color: "#D0CEFF" }
                    GradientStop { position: 1.0; color: "#9B97FF" }
                }
                opacity: loginButton.enabled ? 1.0 : 0.45
            }

            onClicked: console.log("Logging in", emailField.text)
        }

        // ── Support link row ────────────────────────────────────────────
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 4
            Text {
                text: "Having problems?"
                font.pixelSize: 14
                color: "#666666"
            }
            Text {
                text: "Reach us."
                font.pixelSize: 14
                color: accent
                MouseArea { anchors.fill: parent; onClicked: reachClicked() }
            }
        }

        // Push remaining space to bottom so that all content stays at top
        Item { Layout.fillHeight: true }

        // helper callbacks ------------------------------------------------
        function forgotClicked() {
            console.log("Forgot password clicked")
        }
        function reachClicked() {
            console.log("Reach us clicked")
        }
    }
}
