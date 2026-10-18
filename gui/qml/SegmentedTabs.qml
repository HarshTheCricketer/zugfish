import QtQuick

// Pill-shaped tab switcher with a sliding indicator.
Item {
    id: root
    property var model: []
    property int currentIndex: 0
    signal activated(int index)

    implicitHeight: 36

    Rectangle {
        anchors.fill: parent
        radius: Theme.radius - 1
        color: Theme.bg0
        border.width: 1
        border.color: Theme.line
    }
    Rectangle {
        id: indicator
        y: 3
        height: parent.height - 6
        width: (root.width - 6) / Math.max(1, root.model.length)
        x: 3 + root.currentIndex * width
        radius: Theme.radius - 3
        color: Theme.bg3
        border.width: 1
        border.color: Theme.lineStrong
        Behavior on x { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
    }
    Row {
        x: 3
        y: 3
        height: parent.height - 6
        Repeater {
            model: root.model
            delegate: Item {
                required property int index
                required property var modelData
                width: (root.width - 6) / Math.max(1, root.model.length)
                height: parent.height
                Text {
                    anchors.centerIn: parent
                    text: parent.modelData
                    color: root.currentIndex === parent.index ? Theme.text0 : hover.hovered ? Theme.text1 : Theme.text2
                    font.family: Theme.sans
                    font.weight: Font.Medium
                    font.pixelSize: Theme.fsMd
                }
                HoverHandler { id: hover; cursorShape: Qt.PointingHandCursor }
                TapHandler { onTapped: { root.currentIndex = parent.index; root.activated(parent.index) } }
            }
        }
    }
}
