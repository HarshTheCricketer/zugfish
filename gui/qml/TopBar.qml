import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes

// Brand, game title and the primary actions.
Rectangle {
    id: root
    signal openImport()
    signal openFile()
    signal openSettings()

    color: Theme.bg1
    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.line }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 16
        spacing: 12

        // wordmark
        Item {
            Layout.preferredWidth: brand.implicitWidth
            Layout.fillHeight: true
            Row {
                id: brand
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10
                Item {
                    width: 30; height: 30
                    Rectangle { anchors.fill: parent; radius: 9; color: Theme.accent }
                    Shape {   // a small fish
                        anchors.centerIn: parent
                        width: 24; height: 24
                        ShapePath {
                            strokeWidth: -1
                            fillColor: Theme.accentInk
                            PathSvg { path: "M3 12C6 7 12 6 16 9.5L21 6.5V17.5L16 14.5C12 18 6 17 3 12Z" }
                        }
                        ShapePath {
                            strokeWidth: -1
                            fillColor: Theme.accent
                            PathSvg { path: "M8.2 11a1.1 1.1 0 1 0 2.2 0a1.1 1.1 0 1 0 -2.2 0Z" }
                        }
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "zugfish"
                    color: Theme.text0
                    font.family: Theme.sans; font.weight: Font.Bold; font.pixelSize: Theme.fsXl
                    font.letterSpacing: -0.3
                }
            }
        }

        Rectangle { visible: App.gameLoaded; width: 1; height: 22; color: Theme.line }

        // game title
        ColumnLayout {
            visible: App.gameLoaded
            spacing: 0
            Layout.fillWidth: true
            Text {
                text: (App.gameInfo.white || "") + "  vs  " + (App.gameInfo.black || "")
                color: Theme.text0
                elide: Text.ElideRight
                Layout.fillWidth: true
                font.family: Theme.sans; font.weight: Font.Medium; font.pixelSize: Theme.fsMd + 1
            }
            Text {
                text: [App.gameInfo.event, App.gameInfo.date, App.gameInfo.opening].filter(function (s) { return !!s }).join("  ·  ")
                color: Theme.text2
                elide: Text.ElideRight
                Layout.fillWidth: true
                font.family: Theme.sans; font.pixelSize: Theme.fsSm
            }
        }
        Item { visible: !App.gameLoaded; Layout.fillWidth: true }

        Chip {
            visible: App.gameLoaded && App.gameInfo.result
            text: App.gameInfo.result || ""
            mono: true
        }

        AppButton { variant: "secondary"; icon: "download"; text: "Import"; onClicked: root.openImport() }
        IconButton { icon: "folder-open"; tip: "Open PGN file  (Ctrl+O)"; tipBelow: true; onClicked: root.openFile() }

        // review action
        Item {
            visible: App.gameLoaded
            Layout.preferredWidth: 168
            Layout.preferredHeight: 36
            AppButton {
                visible: App.reviewState !== 1
                anchors.fill: parent
                variant: App.reviewState === 2 ? "secondary" : "primary"
                icon: App.reviewState === 2 ? "refresh-cw" : "zap"
                text: App.reviewState === 2 ? "Re-run review" : App.reviewState === 3 ? "Retry review" : "Review game"
                onClicked: App.startReview()
            }
            Rectangle {   // live progress
                visible: App.reviewState === 1
                anchors.fill: parent
                radius: Theme.radius - 2
                color: Theme.bg2
                border.width: 1
                border.color: Theme.line
                Rectangle {
                    width: parent.width * App.reviewProgress
                    height: parent.height
                    radius: parent.radius
                    color: Qt.rgba(0.55, 0.49, 1, 0.28)
                    Behavior on width { NumberAnimation { duration: 140 } }
                }
                Text {
                    anchors.centerIn: parent
                    text: "Analysing  " + Math.round(App.reviewProgress * 100) + "%"
                    color: Theme.text0
                    font.family: Theme.mono; font.pixelSize: Theme.fsMd
                }
            }
        }

        IconButton { icon: "settings"; tip: "Settings"; tipBelow: true; onClicked: root.openSettings() }
    }
}
