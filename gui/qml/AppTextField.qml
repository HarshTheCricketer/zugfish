import QtQuick
import QtQuick.Controls

TextField {
    id: control
    property bool monospace: false
    implicitHeight: 36
    leftPadding: 12
    rightPadding: 12
    color: Theme.text0
    placeholderTextColor: Theme.text3
    selectionColor: Theme.accent
    selectedTextColor: Theme.accentInk
    font.family: monospace ? Theme.mono : Theme.sans
    font.pixelSize: Theme.fsMd
    verticalAlignment: TextInput.AlignVCenter
    background: Rectangle {
        radius: Theme.radius - 2
        color: Theme.bg0
        border.width: 1
        border.color: control.activeFocus ? Theme.accent : Theme.line
        Behavior on border.color { ColorAnimation { duration: 120 } }
    }
}
