import QtQuick
import QtQuick.Layouts

// Empty state: how to get a game in, plus bundled samples.
Item {
    id: root
    signal openImport(int tab)
    signal openFile()

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: Math.max(height, hero.implicitHeight + 80)
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: hero
            width: Math.min(760, parent.width - 48)
            anchors.horizontalCenter: parent.horizontalCenter
            y: Math.max(36, (parent.height - implicitHeight) / 2 - 20)
            spacing: 0

            Text {
                text: "Review your games\nlike a coach would."
                color: Theme.text0
                font.family: Theme.sans; font.weight: Font.Bold; font.pixelSize: Theme.fs3xl
                font.letterSpacing: -0.8
                lineHeight: 1.1
            }
            Text {
                Layout.topMargin: 14
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "Bring a game from Chess.com or Lichess, paste a PGN, or open a file. Stockfish runs on your own machine, rates every move and shows you where the game turned."
                color: Theme.text1
                font.family: Theme.sans; font.pixelSize: Theme.fsLg
                lineHeight: 1.3
            }

            RowLayout {
                Layout.topMargin: 28
                spacing: 10
                AppButton { variant: "primary"; icon: "download"; text: "Import games"; onClicked: root.openImport(0) }
                AppButton { icon: "clipboard-paste"; text: "Paste PGN"; onClicked: root.openImport(2) }
                AppButton { icon: "folder-open"; text: "Open file"; onClicked: root.openFile() }
            }
            Text {
                Layout.topMargin: 12
                text: "…or drop a .pgn file anywhere in this window."
                color: Theme.text3
                font.family: Theme.sans; font.pixelSize: Theme.fsSm + 1
            }

            Text {
                Layout.topMargin: 44
                text: "TRY A FAMOUS GAME"
                color: Theme.text3; font.family: Theme.sans; font.weight: Font.DemiBold
                font.pixelSize: Theme.fsXs; font.letterSpacing: 1.1
            }
            Flow {
                Layout.topMargin: 12
                Layout.fillWidth: true
                spacing: 12
                Repeater {
                    model: App.samples
                    delegate: Rectangle {
                        required property var modelData
                        width: Math.min(240, (hero.width - 24) / 3)
                        height: 112
                        radius: Theme.radiusLg - 2
                        color: tap.pressed ? Theme.bg3 : hover.hovered ? Theme.bg3 : Theme.bg2
                        border.width: 1
                        border.color: hover.hovered ? Theme.lineStrong : Theme.line
                        Behavior on color { ColorAnimation { duration: 100 } }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 3
                            Text {
                                text: modelData.year
                                color: Theme.accentHi
                                font.family: Theme.mono; font.pixelSize: Theme.fsSm
                            }
                            Text {
                                text: modelData.title
                                color: Theme.text0
                                font.family: Theme.sans; font.weight: Font.DemiBold; font.pixelSize: Theme.fsLg
                                elide: Text.ElideRight; Layout.fillWidth: true
                            }
                            Text {
                                text: modelData.subtitle
                                color: Theme.text2
                                font.family: Theme.sans; font.pixelSize: Theme.fsSm + 1
                                elide: Text.ElideRight; Layout.fillWidth: true
                            }
                            Item { Layout.fillHeight: true }
                            Text {
                                text: modelData.plies + " plies"
                                color: Theme.text3
                                font.family: Theme.mono; font.pixelSize: Theme.fsSm
                            }
                        }
                        HoverHandler { id: hover; cursorShape: Qt.PointingHandCursor }
                        TapHandler { id: tap; onTapped: App.loadSample(modelData.id) }
                    }
                }
            }

            Chip {
                visible: App.backendName === "Demo data"
                Layout.topMargin: 36
                text: "Demo mode: bundled games only. Build with -DZUGFISH_CORE_DIR to use your engine."
                dot: Theme.warning
            }
        }
    }
}
