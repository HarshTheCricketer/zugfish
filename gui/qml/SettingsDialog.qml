import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Popup {
    id: root
    modal: true
    focus: true
    padding: 0
    anchors.centerIn: Overlay.overlay
    width: 540
    height: Math.min(640, Overlay.overlay ? Overlay.overlay.height - 48 : 640)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 140 }
        NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: 180; easing.type: Easing.OutCubic }
    }
    exit: Transition { NumberAnimation { property: "opacity"; to: 0; duration: 100 } }
    Overlay.modal: Rectangle { color: Qt.rgba(0.02, 0.03, 0.05, 0.66) }
    background: Rectangle { radius: Theme.radiusLg + 2; color: Theme.bg1; border.width: 1; border.color: Theme.lineStrong }

    FileDialog {
        id: enginePicker
        title: "Select the Stockfish executable"
        onAccepted: App.enginePath = App.toLocalFile(selectedFile)
    }

    component SectionTitle: Text {
        color: Theme.text3
        font.family: Theme.sans; font.weight: Font.DemiBold; font.pixelSize: Theme.fsXs
        font.letterSpacing: 1.1
    }
    component SettingRow: RowLayout {
        id: sr
        property string title
        property string hint
        default property alias control: slot.data
        Layout.fillWidth: true
        Layout.minimumWidth: 300
        spacing: 12
        ColumnLayout {
            spacing: 1
            Layout.fillWidth: true
            Layout.preferredWidth: 1000
            Text { text: sr.title; color: Theme.text0; font.family: Theme.sans; font.pixelSize: Theme.fsMd + 1 }
            Text { visible: sr.hint !== ""; text: sr.hint; color: Theme.text2; wrapMode: Text.WordWrap
                Layout.fillWidth: true; font.family: Theme.sans; font.pixelSize: Theme.fsSm + 1 }
        }
        Item { id: slot; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
               Layout.preferredWidth: childrenRect.width; Layout.preferredHeight: childrenRect.height }
    }

    contentItem: ColumnLayout {
        spacing: 0
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 20
            Layout.bottomMargin: 8
            Text {
                text: "Settings"
                color: Theme.text0
                font.family: Theme.sans; font.weight: Font.Bold; font.pixelSize: Theme.fsXl + 1
                Layout.fillWidth: true
            }
            IconButton { icon: "x"; tip: "Close  (Esc)"; onClicked: root.close() }
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: inner.implicitHeight + 24
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ThemedScrollBar {}

            ColumnLayout {
                id: inner
                x: 20
                width: parent.width - 40 - 6
                spacing: 16

                SectionTitle { text: "ENGINE" }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Text { text: "Stockfish executable"; color: Theme.text0; font.family: Theme.sans; font.pixelSize: Theme.fsMd + 1 }
                    RowLayout {
                        spacing: 8
                        AppTextField {
                            Layout.fillWidth: true
                            monospace: true
                            text: App.enginePath
                            placeholderText: "/usr/bin/stockfish"
                            onEditingFinished: App.enginePath = text
                        }
                        AppButton { text: "Browse…"; onClicked: enginePicker.open() }
                    }
                    Text {
                        text: App.enginePath === "" ? "Not found on your PATH. Install Stockfish or pick the binary above."
                                                    : "Zugfish runs the engine as a separate process."
                        color: App.enginePath === "" ? Theme.warning : Theme.text2
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                        font.family: Theme.sans; font.pixelSize: Theme.fsSm + 1
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    RowLayout {
                        Text { text: "Analysis depth"; color: Theme.text0; font.family: Theme.sans; font.pixelSize: Theme.fsMd + 1; Layout.fillWidth: true }
                        Text { text: App.depth; color: Theme.accentHi; font.family: Theme.mono; font.pixelSize: Theme.fsLg }
                    }
                    Slider {
                        id: depthSlider
                        Layout.fillWidth: true
                        from: 8; to: 24; stepSize: 1
                        value: App.depth
                        onMoved: App.depth = value
                        implicitHeight: 24
                        background: Rectangle {
                            x: depthSlider.leftPadding; y: depthSlider.topPadding + depthSlider.availableHeight / 2 - height / 2
                            width: depthSlider.availableWidth; height: 4; radius: 2; color: Theme.bg4
                            Rectangle { width: depthSlider.visualPosition * parent.width; height: parent.height; radius: 2; color: Theme.accent }
                        }
                        handle: Rectangle {
                            x: depthSlider.leftPadding + depthSlider.visualPosition * (depthSlider.availableWidth - width)
                            y: depthSlider.topPadding + depthSlider.availableHeight / 2 - height / 2
                            width: 18; height: 18; radius: 9
                            color: Theme.text0
                            border.width: depthSlider.pressed ? 4 : 0; border.color: Qt.rgba(0.55, 0.49, 1, 0.35)
                        }
                    }
                    Text {
                        text: "Higher is stricter but slower. 12 to 16 is a good balance for reviewing games."
                        color: Theme.text2; wrapMode: Text.WordWrap; Layout.fillWidth: true
                        font.family: Theme.sans; font.pixelSize: Theme.fsSm + 1
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.line }
                SectionTitle { text: "BOARD" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Repeater {
                        model: Theme.boardThemes
                        delegate: Rectangle {
                            required property int index
                            required property var modelData
                            Layout.fillWidth: true
                            implicitHeight: 74
                            radius: Theme.radius
                            color: Theme.bg2
                            border.width: App.boardTheme === index ? 2 : 1
                            border.color: App.boardTheme === index ? Theme.accent : Theme.line
                            Column {
                                anchors.centerIn: parent
                                spacing: 6
                                Grid {   // 4x2 swatch
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    columns: 4
                                    Repeater {
                                        model: 8
                                        delegate: Rectangle {
                                            required property int index
                                            width: 14; height: 14
                                            color: ((index % 4) + Math.floor(index / 4)) % 2 === 0 ? modelData.light : modelData.dark
                                        }
                                    }
                                }
                                Text { anchors.horizontalCenter: parent.horizontalCenter; text: modelData.name
                                    color: Theme.text1; font.family: Theme.sans; font.pixelSize: Theme.fsSm + 1 }
                            }
                            TapHandler { onTapped: App.boardTheme = index }
                            HoverHandler { cursorShape: Qt.PointingHandCursor }
                        }
                    }
                }
                SettingRow {
                    title: "Show move arrows"
                    hint: "The played move, tinted by its quality, and the engine's preferred move."
                    Toggle { checked: App.showArrows; onToggled: (v) => App.showArrows = v }
                }
                SettingRow {
                    title: "Animate moves"
                    Toggle { checked: App.animateMoves; onToggled: (v) => App.animateMoves = v }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.line }
                SectionTitle { text: "BEHAVIOUR" }
                SettingRow {
                    title: "Review games automatically"
                    hint: "Start the analysis as soon as a game is opened."
                    Toggle { checked: App.autoReview; onToggled: (v) => App.autoReview = v }
                }
                Item { Layout.preferredHeight: 4 }
            }
        }
    }
}
