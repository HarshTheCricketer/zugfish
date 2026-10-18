import QtQuick

// iOS-style switch.
Item {
    id: root
    property bool checked: false
    signal toggled(bool value)

    implicitWidth: 40
    implicitHeight: 22

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: root.checked ? Theme.accent : Theme.bg4
        Behavior on color { ColorAnimation { duration: 120 } }
    }
    Rectangle {
        width: 16; height: 16; radius: 8
        y: 3
        x: root.checked ? parent.width - width - 3 : 3
        color: root.checked ? Theme.accentInk : Theme.text1
        Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
    }
    HoverHandler { cursorShape: Qt.PointingHandCursor }
    TapHandler { onTapped: root.toggled(!root.checked) }
}
