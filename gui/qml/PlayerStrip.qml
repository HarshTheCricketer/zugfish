import QtQuick

// Name, rating, captured material and "to move" marker for one side.
Item {
    id: root
    property string name
    property string rating
    property bool isWhite: true
    property bool toMove: false
    property var captured: []
    property int lead: 0              // material points this side is ahead by
    property string resultText: ""    // "1", "0" or "½" once the game is over

    implicitHeight: 46

    Rectangle {      // avatar
        id: avatar
        width: 34; height: 34; radius: 17
        anchors.verticalCenter: parent.verticalCenter
        color: root.isWhite ? "#ECEFF5" : "#1A2029"
        border.width: 2
        border.color: root.toMove ? Theme.accent : (root.isWhite ? "#ECEFF5" : Theme.lineStrong)
        Behavior on border.color { ColorAnimation { duration: 160 } }
        Text {
            anchors.centerIn: parent
            text: root.name.length > 0 ? root.name.charAt(0).toUpperCase() : "?"
            color: root.isWhite ? "#10141A" : Theme.text0
            font.family: Theme.sans
            font.weight: Font.Bold
            font.pixelSize: 15
        }
    }

    Column {
        anchors.left: avatar.right
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1
        Row {
            spacing: 8
            Text {
                text: root.name
                color: Theme.text0
                font.family: Theme.sans
                font.weight: Font.DemiBold
                font.pixelSize: Theme.fsLg
                elide: Text.ElideRight
                width: Math.min(implicitWidth, root.width * 0.42)
            }
            Text {
                visible: root.rating !== ""
                anchors.baseline: parent.children[0].baseline
                text: root.rating
                color: Theme.text2
                font.family: Theme.mono
                font.pixelSize: Theme.fsSm + 1
            }
            Rectangle {
                visible: root.toMove
                anchors.verticalCenter: parent.verticalCenter
                width: 7; height: 7; radius: 3.5
                color: Theme.accent
                SequentialAnimation on opacity {
                    running: root.toMove
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.35; duration: 700 }
                    NumberAnimation { to: 1.0; duration: 700 }
                }
            }
        }
        // captured pieces on a light tray, so black pieces stay readable on the dark UI
        Row {
            spacing: 6
            Rectangle {
                visible: root.captured.length > 0
                height: 20
                radius: 10
                width: tray.implicitWidth + 12
                color: "#B9C1D1"
                Row {
                    id: tray
                    anchors.centerIn: parent
                    spacing: -4
                    Repeater {
                        model: root.captured
                        delegate: Image {
                            required property string modelData
                            width: 16; height: 16
                            source: "qrc:/assets/pieces/" + modelData + ".svg"
                            sourceSize: Qt.size(32, 32)
                            smooth: true
                        }
                    }
                }
            }
            Text {
                visible: root.lead > 0
                anchors.verticalCenter: parent.verticalCenter
                text: "+" + root.lead
                color: Theme.text1
                font.family: Theme.mono
                font.pixelSize: Theme.fsXs + 1
            }
            Item { visible: root.captured.length === 0 && root.lead === 0; width: 1; height: 20 }
        }
    }

    Chip {
        visible: root.resultText !== ""
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: root.resultText
        mono: true
        tone: root.resultText === "1" ? Qt.rgba(0.29, 0.87, 0.5, 0.16) : Theme.bg3
        textColor: root.resultText === "1" ? Theme.positive : Theme.text1
    }
}
