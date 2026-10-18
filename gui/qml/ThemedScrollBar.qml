import QtQuick
import QtQuick.Controls

ScrollBar {
    id: control
    implicitWidth: 10
    padding: 2
    contentItem: Rectangle {
        implicitWidth: 6
        radius: 3
        color: control.pressed ? Theme.text2 : Theme.lineStrong
        opacity: control.policy === ScrollBar.AlwaysOn || (control.active && control.size < 1.0) ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 180 } }
    }
    background: Item {}
}
