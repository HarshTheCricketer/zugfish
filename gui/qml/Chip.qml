import QtQuick

// Small read-only label pill.
Rectangle {
    id: root
    property string text
    property color dot: "transparent"
    property color tone: Theme.bg3
    property color textColor: Theme.text1
    property bool mono: false

    implicitHeight: 24
    implicitWidth: row.implicitWidth + 18
    radius: height / 2
    color: tone

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6
        Rectangle {
            visible: root.dot.a > 0
            width: 7; height: 7; radius: 3.5
            color: root.dot
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: root.textColor
            font.family: root.mono ? Theme.mono : Theme.sans
            font.weight: Font.Medium
            font.pixelSize: Theme.fsXs + 1
        }
    }
}
