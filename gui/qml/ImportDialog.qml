import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Import a game: Chess.com, Lichess, or a pasted PGN.
Popup {
    id: root
    modal: true
    focus: true
    padding: 0
    anchors.centerIn: Overlay.overlay
    width: Math.min(760, Overlay.overlay ? Overlay.overlay.width - 48 : 760)
    height: Math.min(620, Overlay.overlay ? Overlay.overlay.height - 48 : 620)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property int tab: 0
    function openAt(t) { tab = t; open() }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 140 }
        NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: 180; easing.type: Easing.OutCubic }
    }
    exit: Transition { NumberAnimation { property: "opacity"; to: 0; duration: 100 } }
    Overlay.modal: Rectangle { color: Qt.rgba(0.02, 0.03, 0.05, 0.66) }

    background: Rectangle {
        radius: Theme.radiusLg + 2
        color: Theme.bg1
        border.width: 1
        border.color: Theme.lineStrong
    }

    property string pasteError: ""

    contentItem: ColumnLayout {
        spacing: 0

        // header
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 20
            Layout.bottomMargin: 14
            Text {
                text: "Import a game"
                color: Theme.text0
                font.family: Theme.sans; font.weight: Font.Bold; font.pixelSize: Theme.fsXl + 1
                Layout.fillWidth: true
            }
            IconButton { icon: "x"; tip: "Close  (Esc)"; onClicked: root.close() }
        }

        SegmentedTabs {
            Layout.fillWidth: true
            Layout.leftMargin: 20; Layout.rightMargin: 20
            model: ["Chess.com", "Lichess", "Paste PGN"]
            currentIndex: root.tab
            onActivated: (i) => root.tab = i
        }

        // ---------- online ----------
        ColumnLayout {
            visible: root.tab < 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 20
            spacing: 12

            RowLayout {
                spacing: 10
                AppTextField {
                    id: user
                    Layout.fillWidth: true
                    placeholderText: root.tab === 0 ? "Chess.com username" : "Lichess username"
                    onAccepted: App.fetchOnline(root.tab, text, count.value)
                }
                // how many games
                Rectangle {
                    id: count
                    property int value: 10
                    Layout.preferredWidth: 150; Layout.preferredHeight: 36
                    radius: Theme.radius - 2; color: Theme.bg0; border.width: 1; border.color: Theme.line
                    Row {
                        anchors.fill: parent; anchors.margins: 3; spacing: 2
                        Repeater {
                            model: [10, 25, 50]
                            delegate: Rectangle {
                                required property int modelData
                                width: (count.width - 6 - 4) / 3; height: parent.height
                                radius: Theme.radius - 4
                                color: count.value === modelData ? Theme.bg3 : "transparent"
                                Text { anchors.centerIn: parent; text: modelData
                                    color: count.value === modelData ? Theme.text0 : Theme.text2
                                    font.family: Theme.mono; font.pixelSize: Theme.fsMd }
                                TapHandler { onTapped: count.value = parent.modelData }
                                HoverHandler { cursorShape: Qt.PointingHandCursor }
                            }
                        }
                    }
                }
                AppButton {
                    variant: "primary"
                    icon: "search"
                    text: App.onlineBusy ? "Fetching…" : "Fetch games"
                    busy: false
                    enabled: !App.onlineBusy
                    onClicked: App.fetchOnline(root.tab, user.text, count.value)
                }
            }

            Text {
                visible: App.onlineStatus !== ""
                text: App.onlineStatus
                color: App.onlineError ? Theme.danger : Theme.text2
                font.family: Theme.sans; font.pixelSize: Theme.fsMd
            }

            // results
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: Theme.radius
                color: Theme.bg0
                border.width: 1
                border.color: Theme.line
                clip: true

                Column {
                    anchors.centerIn: parent
                    visible: list.count === 0
                    spacing: 8
                    Icon { anchors.horizontalCenter: parent.horizontalCenter; name: "swords"; size: 28; color: Theme.text3 }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Your recent games will appear here"
                        color: Theme.text3; font.family: Theme.sans; font.pixelSize: Theme.fsMd
                    }
                }

                ListView {
                    id: list
                    anchors.fill: parent
                    anchors.margins: 6
                    model: App.onlineGames
                    spacing: 2
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ThemedScrollBar {}
                    delegate: Rectangle {
                        required property int index
                        required property string date
                        required property string white
                        required property string black
                        required property int whiteRating
                        required property int blackRating
                        required property string speed
                        required property string result
                        width: list.width - 10
                        height: 52
                        radius: 8
                        color: rowTap.pressed ? Theme.bg3 : rowHover.hovered ? Theme.bg2 : "transparent"
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12; anchors.rightMargin: 12
                            spacing: 12
                            Text { text: date; color: Theme.text2; Layout.preferredWidth: 78
                                font.family: Theme.mono; font.pixelSize: Theme.fsSm + 1 }
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 1
                                Text { text: white + (whiteRating > 0 ? "  " + whiteRating : ""); color: Theme.text0
                                    elide: Text.ElideRight; Layout.fillWidth: true
                                    font.family: Theme.sans; font.pixelSize: Theme.fsMd }
                                Text { text: black + (blackRating > 0 ? "  " + blackRating : ""); color: Theme.text2
                                    elide: Text.ElideRight; Layout.fillWidth: true
                                    font.family: Theme.sans; font.pixelSize: Theme.fsMd }
                            }
                            Chip { text: speed }
                            Chip { text: result; mono: true }
                            AppButton {
                                variant: "secondary"; compact: true; text: "Review"
                                onClicked: { if (App.loadOnlineGame(index) === "") root.close() }
                            }
                        }
                        HoverHandler { id: rowHover }
                        TapHandler { id: rowTap; onDoubleTapped: { if (App.loadOnlineGame(index) === "") root.close() } }
                    }
                }
            }
        }

        // ---------- paste ----------
        ColumnLayout {
            visible: root.tab === 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 20
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: Theme.radius
                color: Theme.bg0
                border.width: 1
                border.color: pgn.activeFocus ? Theme.accent : Theme.line
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 4
                    TextArea {
                        id: pgn
                        placeholderText: "Paste a PGN here…\n\n[Event \"Live Chess\"]\n[White \"you\"]\n…\n1. e4 e5 2. Nf3 Nc6 …"
                        placeholderTextColor: Theme.text3
                        color: Theme.text0
                        selectionColor: Theme.accent
                        selectedTextColor: Theme.accentInk
                        font.family: Theme.mono
                        font.pixelSize: Theme.fsMd
                        wrapMode: TextEdit.Wrap
                        background: null
                        onTextChanged: root.pasteError = ""
                    }
                }
            }
            RowLayout {
                spacing: 10
                Text {
                    Layout.fillWidth: true
                    text: root.pasteError
                    color: Theme.danger
                    wrapMode: Text.WordWrap
                    font.family: Theme.sans; font.pixelSize: Theme.fsMd
                }
                AppButton { variant: "ghost"; icon: "clipboard-paste"; text: "Paste"; onClicked: { pgn.forceActiveFocus(); pgn.paste() } }
                AppButton { variant: "ghost"; text: "Clear"; onClicked: pgn.clear() }
                AppButton {
                    variant: "primary"; icon: "arrow-right"; iconAfter: true; text: "Load game"
                    onClicked: {
                        var err = App.loadPgn(pgn.text)
                        if (err === "") { root.pasteError = ""; pgn.clear(); root.close() }
                        else root.pasteError = err
                    }
                }
            }
        }
    }
}
