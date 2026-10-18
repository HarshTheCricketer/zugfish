import QtQuick

// The round icon that marks a move's quality (!!, ?, book, ...).
Item {
    id: root
    property int cls: -1
    property real size: 20
    property bool ring: false          // thin outline so it stays readable on any square

    implicitWidth: size
    implicitHeight: size
    visible: cls >= 0

    readonly property var glyphs: ["!!", "!", "", "", "", "", "?!", "?", "??", "", ""]
    readonly property var iconNames: ["", "", "star", "check", "check", "book-open", "", "", "", "x", "arrow-right"]
    readonly property color fg: (cls === 6 || cls === 3 || cls === 4) ? "#10141A" : "#FFFFFF"

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: Theme.classColor(root.cls)
        border.width: root.ring ? Math.max(1.5, root.size * 0.08) : 0
        border.color: "#0B0E14"
    }
    Text {
        anchors.centerIn: parent
        visible: root.glyphs[root.cls] !== undefined && root.glyphs[root.cls] !== ""
        text: root.glyphs[root.cls] || ""
        color: root.fg
        font.family: Theme.sans
        font.weight: Font.Bold
        font.pixelSize: root.size * (text.length > 1 ? 0.46 : 0.58)
        renderType: Text.QtRendering
    }
    Icon {
        anchors.centerIn: parent
        visible: !!root.iconNames[root.cls]
        name: root.iconNames[root.cls] || ""
        color: root.fg
        size: root.size * 0.58
        strokeWidth: 2.6
        filled: root.cls === 2
    }
}
