import QtQuick
import QtQuick.Shapes
import "icons.js" as Icons

// Stroke icon from the Lucide set (24x24 grid), drawn as vector paths:
// resolution independent, recolourable, no image files.
Item {
    id: root
    property string name
    property color color: Theme.text1
    property real size: 18
    property real strokeWidth: 2
    property bool filled: false

    implicitWidth: size
    implicitHeight: size

    Shape {
        width: 24
        height: 24
        scale: root.size / 24
        transformOrigin: Item.TopLeft
        ShapePath {
            strokeColor: root.color
            strokeWidth: root.strokeWidth
            fillColor: root.filled ? root.color : "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathSvg { path: Icons.path(root.name) }
        }
    }
}
