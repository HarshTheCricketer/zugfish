import QtQuick

// Text button. variant: "primary" | "secondary" | "ghost"
Item {
    id: root
    property string text
    property string icon
    property bool iconAfter: false
    property string variant: "secondary"
    property bool compact: false
    property bool busy: false
    signal clicked()

    implicitHeight: compact ? 32 : 40
    implicitWidth: row.implicitWidth + (compact ? 20 : 28)
    opacity: enabled ? 1 : 0.4
    scale: tap.pressed ? 0.98 : hover.hovered ? 1.015 : 1
    Behavior on scale { NumberAnimation { duration: 110; easing.type: Easing.OutCubic } }

    readonly property bool isPrimary: variant === "primary"
    readonly property color fg: isPrimary ? Theme.accentInk : hover.hovered ? Theme.text0 : Theme.text1

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusSm + 2
        color: root.isPrimary ? (tap.pressed ? Theme.accent : hover.hovered ? Theme.accentHi : Theme.accent)
             : root.variant === "ghost" ? (tap.pressed ? Theme.bg4 : hover.hovered ? Theme.bg3 : "transparent")
             : (tap.pressed ? Theme.bg4 : hover.hovered ? Theme.bg3 : Theme.bg2)
        border.width: root.variant === "secondary" ? 1 : (root.isPrimary ? 1 : 0)
        border.color: root.isPrimary ? (hover.hovered ? Theme.accentHi : Theme.accent) : (hover.hovered ? Theme.lineStrong : Theme.line)
        Behavior on color { ColorAnimation { duration: 120 } }
    }
    Row {
        id: row
        anchors.centerIn: parent
        spacing: 7
        layoutDirection: root.iconAfter ? Qt.RightToLeft : Qt.LeftToRight
        Icon {
            visible: root.icon !== ""
            anchors.verticalCenter: parent.verticalCenter
            name: root.icon
            size: root.compact ? 14 : 16
            color: root.fg
            RotationAnimator on rotation { running: root.busy; from: 0; to: 360; duration: 900; loops: Animation.Infinite }
        }
        Text {
            visible: root.text !== ""
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: root.fg
            font.family: Theme.sans
            font.weight: Font.Medium
            font.pixelSize: root.compact ? Theme.fsSm : Theme.fsMd
        }
    }
    HoverHandler { id: hover; cursorShape: Qt.PointingHandCursor }
    TapHandler { id: tap; onTapped: root.clicked() }
}
