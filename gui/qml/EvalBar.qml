import QtQuick

// Vertical advantage bar: the light part is White's share of the winning chances.
Item {
    id: root
    property real winWhite: 50          // 0..100
    property string label: "0.0"
    property bool hasEval: false
    property bool flipped: false

    implicitWidth: 22

    readonly property real target: hasEval ? Math.max(0.025, Math.min(0.975, winWhite / 100)) : 0.5
    property real fraction: target
    Behavior on fraction { NumberAnimation { duration: 460; easing.type: Easing.OutCubic } }
    readonly property bool whiteAhead: fraction >= 0.5

    Rectangle { anchors.fill: parent; color: "#14181F"; border.width: 1; border.color: Qt.rgba(0, 0, 0, 0.5) }
    Rectangle {
        width: parent.width
        height: parent.height * root.fraction
        y: root.flipped ? 0 : parent.height - height
        color: "#ECEFF5"
    }
    // centre tick
    Rectangle { width: parent.width; height: 1; y: parent.height / 2; color: Qt.rgba(0.5, 0.55, 0.65, 0.55) }

    Text {
        // sits inside the winning side's colour so it always has contrast
        readonly property bool atBottom: root.flipped ? !root.whiteAhead : root.whiteAhead
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        fontSizeMode: Text.HorizontalFit
        minimumPixelSize: 7
        y: atBottom ? parent.height - height - 6 : 6
        text: root.label
        color: root.whiteAhead ? "#10141A" : "#ECEFF5"
        font.family: Theme.mono
        font.weight: Font.Medium
        font.pixelSize: 10
        opacity: root.hasEval ? 1 : 0
    }
}
