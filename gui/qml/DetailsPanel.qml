import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// "Details" tab: position, engine numbers for the current move, game information.
Item {
    id: root
    readonly property var cur: App.current
    readonly property var info: App.gameInfo

    component Section: ColumnLayout {
        property string title
        Layout.fillWidth: true
        spacing: 8
        Text {
            text: parent.title.toUpperCase()
            color: Theme.text3; font.family: Theme.sans; font.weight: Font.DemiBold
            font.pixelSize: Theme.fsXs; font.letterSpacing: 1.1
        }
    }
    component Row2: RowLayout {
        property string label
        property string value
        property bool monoValue: false
        Layout.fillWidth: true
        visible: value !== ""
        Text { text: parent.label; color: Theme.text2; font.family: Theme.sans; font.pixelSize: Theme.fsMd
            Layout.preferredWidth: 110 }
        Text { text: parent.value; Layout.fillWidth: true; wrapMode: Text.WrapAnywhere; color: Theme.text0
            font.family: parent.monoValue ? Theme.mono : Theme.sans; font.pixelSize: Theme.fsMd }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        contentWidth: width
        contentHeight: col.implicitHeight + 24
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ThemedScrollBar {}

        ColumnLayout {
            id: col
            width: flick.width - 6
            spacing: 18

            Section {
                title: "Position"
                Card {
                    Layout.fillWidth: true
                    implicitHeight: fenRow.implicitHeight + 24
                    RowLayout {
                        id: fenRow
                        anchors.fill: parent; anchors.margins: 12; spacing: 8
                        Text {
                            Layout.fillWidth: true
                            text: App.fen
                            wrapMode: Text.WrapAnywhere
                            color: Theme.text1
                            font.family: Theme.mono; font.pixelSize: Theme.fsSm + 1
                        }
                        IconButton { icon: "clipboard-paste"; tip: "Copy FEN"; onClicked: App.copyToClipboard(App.fen) }
                    }
                }
            }

            Section {
                title: "This move"
                visible: root.cur.has
                Card {
                    Layout.fillWidth: true
                    implicitHeight: moveCol.implicitHeight + 28
                    ColumnLayout {
                        id: moveCol
                        anchors.fill: parent; anchors.margins: 14; spacing: 8
                        Row2 { label: "Played"; value: root.cur.moveLabel || ""; monoValue: true }
                        Row2 { label: "Engine's choice"; value: root.cur.bestBeforeSan || ""; monoValue: true }
                        Row2 { label: "Evaluation"; value: root.cur.evalText || ""; monoValue: true }
                        Row2 { label: "Win chance lost"; value: root.cur.has ? Math.round((root.cur.winLoss || 0) * 100) + "%" : ""; monoValue: true }
                        Row2 { label: "Centipawn loss"; value: root.cur.has ? String(root.cur.cpLoss || 0) : ""; monoValue: true }
                        Row2 { label: "Engine's next"; value: root.cur.nextBestSan || ""; monoValue: true }
                    }
                }
            }

            Section {
                title: "Game"
                Card {
                    Layout.fillWidth: true
                    implicitHeight: gameCol.implicitHeight + 28
                    ColumnLayout {
                        id: gameCol
                        anchors.fill: parent; anchors.margins: 14; spacing: 8
                        Row2 { label: "Event"; value: root.info.event || "" }
                        Row2 { label: "Site"; value: root.info.site || "" }
                        Row2 { label: "Date"; value: root.info.date || "" }
                        Row2 { label: "Opening"; value: root.info.opening || "" }
                        Row2 { label: "Time control"; value: root.info.timeControl || ""; monoValue: true }
                        Row2 { label: "Result"; value: root.info.result || ""; monoValue: true }
                        Row2 { label: "Termination"; value: root.info.termination || "" }
                        Row2 { label: "Moves"; value: App.plyCount + " plies"; monoValue: true }
                    }
                }
            }

            Section {
                title: "Engine"
                Card {
                    Layout.fillWidth: true
                    implicitHeight: engCol.implicitHeight + 28
                    ColumnLayout {
                        id: engCol
                        anchors.fill: parent; anchors.margins: 14; spacing: 8
                        Row2 { label: "Backend"; value: App.backendName }
                        Row2 { label: "Stockfish"; value: App.enginePath !== "" ? App.enginePath : "not set"; monoValue: true }
                        Row2 { label: "Depth"; value: String(App.depth); monoValue: true }
                    }
                }
            }
        }
    }
}
