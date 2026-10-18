import QtQuick

// Square icon-only button with a hover/press state and a small tooltip.
Item {
    id: root
    property string icon
    property string tip
    property bool active: false
    property bool primary: false
    property bool tipBelow: false
    property real iconSize: 18
    signal clicked()

    implicitWidth: 36
    implicitHeight: 36
    opacity: enabled ? 1 : 0.35

    Rectangle {
        anchors.fill: parent
        radius: Theme.radius - 2
        color: root.primary ? (tap.pressed ? Theme.accent : hover.hovered ? Theme.accentHi : Theme.accent)
                            : tap.pressed ? Theme.bg4 : (hover.hovered || root.active) ? Theme.bg3 : "transparent"
        Behavior on color { ColorAnimation { duration: 90 } }
    }
    Icon {
        anchors.centerIn: parent
        name: root.icon
        size: root.iconSize
        color: root.primary ? Theme.accentInk : (hover.hovered || root.active) ? Theme.text0 : Theme.text1
    }

    HoverHandler { id: hover; cursorShape: Qt.PointingHandCursor }
    TapHandler { id: tap; onTapped: root.clicked() }

    Rectangle {
        visible: opacity > 0.01
        opacity: hover.hovered && root.tip !== "" ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 120 } }
        z: 1000
        x: (root.width - width) / 2
        y: root.tipBelow ? root.height + 6 : -height - 6
        width: tipText.implicitWidth + 16
        height: tipText.implicitHeight + 10
        radius: 6
        color: "#0B0E14"
        border.color: Theme.lineStrong
        Text {
            id: tipText
            anchors.centerIn: parent
            text: root.tip
            color: Theme.text0
            font.family: Theme.sans
            font.pixelSize: Theme.fsXs
        }
    }
}
