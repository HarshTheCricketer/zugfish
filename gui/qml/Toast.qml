import QtQuick

// Transient message at the bottom of the window. kind: 0 info, 1 success, 2 error
Rectangle {
    id: root
    property int kind: 0
    property alias text: label.text

    function show(message, k) {
        label.text = message
        kind = k
        hideTimer.restart()
        opacity = 1
    }

    opacity: 0
    visible: opacity > 0.01
    width: Math.min(560, label.implicitWidth + 64)
    height: 44
    radius: 22
    color: Theme.bg3
    border.width: 1
    border.color: kind === 2 ? Theme.danger : kind === 1 ? Theme.positive : Theme.lineStrong
    Behavior on opacity { NumberAnimation { duration: 180 } }

    Row {
        anchors.centerIn: parent
        spacing: 10
        Icon {
            anchors.verticalCenter: parent.verticalCenter
            name: root.kind === 2 ? "triangle-alert" : root.kind === 1 ? "check" : "circle-alert"
            color: root.kind === 2 ? Theme.danger : root.kind === 1 ? Theme.positive : Theme.text1
            size: 16
        }
        Text {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(implicitWidth, 480)
            elide: Text.ElideRight
            color: Theme.text0
            font.family: Theme.sans
            font.pixelSize: Theme.fsMd
        }
    }
    Timer { id: hideTimer; interval: 3600; onTriggered: root.opacity = 0 }
}
