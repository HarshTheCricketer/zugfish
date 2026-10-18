import QtQuick
import QtQuick.Controls

// "Moves" tab: full move list, two columns, with quality badges.
Item {
    id: root

    Connections {
        target: App
        function onCurrentChanged() {
            if (App.currentPly > 0)
                list.positionViewAtIndex(Math.floor((App.currentPly - 1) / 2), ListView.Contain)
        }
    }

    ListView {
        id: list
        anchors.fill: parent
        model: App.plies
        clip: true
        spacing: 2
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ThemedScrollBar {}

        delegate: Item {
            id: rowItem
            required property int moveNo
            required property int wPly
            required property string wSan
            required property int wCls
            required property string wEval
            required property int bPly
            required property string bSan
            required property int bCls
            required property string bEval

            width: list.width - 8
            height: 36

            Text {
                x: 6; width: 30
                anchors.verticalCenter: parent.verticalCenter
                text: rowItem.moveNo + "."
                color: Theme.text3
                font.family: Theme.mono; font.pixelSize: Theme.fsMd
            }
            MoveCell {
                x: 40; width: (parent.width - 44) / 2 - 2
                ply: rowItem.wPly; san: rowItem.wSan; cls: rowItem.wCls; evalText: rowItem.wEval
            }
            MoveCell {
                x: 40 + (parent.width - 44) / 2 + 2; width: (parent.width - 44) / 2 - 2
                visible: rowItem.bPly > 0
                ply: rowItem.bPly; san: rowItem.bSan; cls: rowItem.bCls; evalText: rowItem.bEval
            }
        }
    }

    // one clickable half-move
    component MoveCell: Rectangle {
        id: cell
        property int ply
        property string san
        property int cls: -1
        property string evalText
        readonly property bool active: App.currentPly === ply
        height: 34
        radius: 7
        color: active ? Theme.bg4 : cellTap.pressed ? Theme.bg3 : cellHover.hovered ? Theme.bg2 : "transparent"
        border.width: active ? 1 : 0
        border.color: Theme.lineStrong
        Behavior on color { ColorAnimation { duration: 80 } }
        Row {
            anchors.left: parent.left; anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            ClassBadge { cls: cell.cls; size: 16; anchors.verticalCenter: parent.verticalCenter }
            Text {
                text: cell.san
                color: cell.active ? Theme.text0 : Theme.text1
                font.family: Theme.mono; font.weight: cell.active ? Font.Medium : Font.Normal; font.pixelSize: Theme.fsMd + 1
            }
        }
        Text {
            anchors.right: parent.right; anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: cell.evalText
            color: Theme.text3
            font.family: Theme.mono; font.pixelSize: Theme.fsXs + 1
        }
        HoverHandler { id: cellHover; cursorShape: Qt.PointingHandCursor }
        TapHandler { id: cellTap; onTapped: App.goTo(cell.ply) }
    }
}
