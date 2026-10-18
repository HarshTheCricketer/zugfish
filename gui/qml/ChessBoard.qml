import QtQuick
import QtQuick.Shapes

// The board: squares, coordinates, last-move highlight, pieces (SVG), animated moves,
// engine arrows and the move-quality badge. All state comes from `App`.
Item {
    id: root
    property real size: 560
    width: size
    height: size

    readonly property real sq: size / 8
    readonly property bool flipped: App.flipped
    readonly property var boardPalette: Theme.boardThemes[App.boardTheme] || Theme.boardThemes[0]
    readonly property var cur: App.current

    // board index (a1 = 0 ... h8 = 63) -> pixel centre
    function col(idx) { var f = idx % 8; return flipped ? 7 - f : f }
    function row(idx) { var r = Math.floor(idx / 8); return flipped ? r : 7 - r }
    function cx(idx) { return (col(idx) + 0.5) * sq }
    function cy(idx) { return (row(idx) + 0.5) * sq }

    // arrow outline as a polygon (tail, shaft, head)
    function arrow(from, to, width) {
        if (from < 0 || to < 0 || from === to)
            return []
        var x1 = cx(from), y1 = cy(from), x2 = cx(to), y2 = cy(to)
        var dx = x2 - x1, dy = y2 - y1
        var len = Math.sqrt(dx * dx + dy * dy)
        var ux = dx / len, uy = dy / len, nx = -uy, ny = ux
        var headLen = sq * 0.34, headHalf = sq * 0.27, half = width / 2
        var sx = x1 + ux * sq * 0.22, sy = y1 + uy * sq * 0.22      // start slightly off-centre
        var tx = x2 - ux * sq * 0.10, ty = y2 - uy * sq * 0.10      // tip
        var bx = tx - ux * headLen, by = ty - uy * headLen          // head base
        return [Qt.point(sx + nx * half, sy + ny * half), Qt.point(bx + nx * half, by + ny * half),
                Qt.point(bx + nx * headHalf, by + ny * headHalf), Qt.point(tx, ty),
                Qt.point(bx - nx * headHalf, by - ny * headHalf), Qt.point(bx - nx * half, by - ny * half),
                Qt.point(sx - nx * half, sy - ny * half), Qt.point(sx + nx * half, sy + ny * half)]
    }

    // ---- squares -------------------------------------------------------
    Repeater {
        model: 64
        delegate: Rectangle {
            required property int index
            readonly property int c: index % 8
            readonly property int r: Math.floor(index / 8)
            x: c * root.sq
            y: r * root.sq
            width: root.sq
            height: root.sq
            color: (c + r) % 2 === 0 ? root.boardPalette.light : root.boardPalette.dark

            Text {   // rank number on the left edge
                visible: parent.c === 0
                x: 4; y: 3
                text: root.flipped ? parent.r + 1 : 8 - parent.r
                color: (parent.c + parent.r) % 2 === 0 ? root.boardPalette.dark : root.boardPalette.light
                font.family: Theme.sans; font.weight: Font.Bold
                font.pixelSize: Math.max(10, root.sq * 0.17)
            }
            Text {   // file letter on the bottom edge
                visible: parent.r === 7
                anchors.right: parent.right; anchors.bottom: parent.bottom
                anchors.rightMargin: 4; anchors.bottomMargin: 2
                text: "abcdefgh".charAt(root.flipped ? 7 - parent.c : parent.c)
                color: (parent.c + parent.r) % 2 === 0 ? root.boardPalette.dark : root.boardPalette.light
                font.family: Theme.sans; font.weight: Font.Bold
                font.pixelSize: Math.max(10, root.sq * 0.17)
            }
        }
    }

    // ---- last move highlight --------------------------------------------
    Repeater {
        model: [App.lastFrom, App.lastTo]
        delegate: Rectangle {
            required property int modelData
            visible: modelData >= 0
            x: modelData >= 0 ? root.col(modelData) * root.sq : 0
            y: modelData >= 0 ? root.row(modelData) * root.sq : 0
            width: root.sq; height: root.sq
            color: Theme.lastMove
            opacity: 0.42
        }
    }

    // ---- pieces ------------------------------------------------------
    property int hiddenSquare: -1          // square whose piece is being animated in
    Repeater {
        model: 64
        delegate: Image {
            required property int index
            readonly property string code: App.squares[index] || ""
            visible: code !== "" && root.hiddenSquare !== index
            x: root.col(index) * root.sq
            y: root.row(index) * root.sq
            width: root.sq; height: root.sq
            source: code !== "" ? "qrc:/assets/pieces/" + code + ".svg" : ""
            sourceSize: Qt.size(Math.ceil(root.sq * Screen.devicePixelRatio), Math.ceil(root.sq * Screen.devicePixelRatio))
            smooth: true
        }
    }

    // sliding piece used when stepping one move forwards/backwards
    Image {
        id: mover
        z: 6
        visible: false
        width: root.sq; height: root.sq
        sourceSize: Qt.size(Math.ceil(root.sq * Screen.devicePixelRatio), Math.ceil(root.sq * Screen.devicePixelRatio))
        smooth: true
    }
    property real fromX: 0
    property real fromY: 0
    property real toX: 0
    property real toY: 0
    ParallelAnimation {
        id: slide
        NumberAnimation { target: mover; property: "x"; from: root.fromX; to: root.toX; duration: 170; easing.type: Easing.OutCubic }
        NumberAnimation { target: mover; property: "y"; from: root.fromY; to: root.toY; duration: 170; easing.type: Easing.OutCubic }
        onFinished: { mover.visible = false; root.hiddenSquare = -1 }
    }

    property int prevPly: 0
    property int prevFrom: -1
    property int prevTo: -1
    Connections {
        target: App
        function onCurrentChanged() {
            var step = App.currentPly - root.prevPly
            var from = step === 1 ? App.lastFrom : root.prevTo
            var to = step === 1 ? App.lastTo : root.prevFrom
            root.prevPly = App.currentPly
            root.prevFrom = App.lastFrom
            root.prevTo = App.lastTo
            slide.stop()
            mover.visible = false
            root.hiddenSquare = -1
            if (!App.animateMoves || Math.abs(step) !== 1 || from < 0 || to < 0)
                return
            var code = App.squares[to]
            if (!code)
                return
            mover.source = "qrc:/assets/pieces/" + code + ".svg"
            root.fromX = root.col(from) * root.sq
            root.fromY = root.row(from) * root.sq
            root.toX = root.col(to) * root.sq
            root.toY = root.row(to) * root.sq
            root.hiddenSquare = to
            mover.x = root.fromX
            mover.y = root.fromY
            mover.visible = true
            slide.restart()
        }
    }

    // ---- arrows --------------------------------------------------------
    Shape {
        anchors.fill: parent
        visible: App.showArrows
        z: 4
        ShapePath {   // engine's preferred move
            strokeWidth: -1
            fillColor: Qt.rgba(Theme.bestArrow.r, Theme.bestArrow.g, Theme.bestArrow.b, 0.78)
            PathPolyline { path: root.arrow(App.bestFrom, App.bestTo, root.sq * 0.17) }
        }
        ShapePath {   // the move that was played, tinted by its quality
            strokeWidth: -1
            fillColor: {
                var c = Qt.color(Theme.classColor(root.cur.cls))
                return Qt.rgba(c.r, c.g, c.b, root.cur.has ? 0.82 : 0)
            }
            PathPolyline { path: root.cur.has ? root.arrow(App.lastFrom, App.lastTo, root.sq * 0.2) : [] }
        }
    }

    // ---- quality badge on the destination square ----------------------------
    ClassBadge {
        z: 5
        cls: root.cur.has ? root.cur.cls : -1
        visible: root.cur.has && root.cur.cls >= 0 && App.lastTo >= 0
        size: Math.max(22, root.sq * 0.36)
        ring: true
        x: App.lastTo >= 0 ? (root.col(App.lastTo) + 1) * root.sq - width * 0.82 : 0
        y: App.lastTo >= 0 ? root.row(App.lastTo) * root.sq - height * 0.18 : 0
        scale: visible ? 1 : 0.4
        Behavior on scale { NumberAnimation { duration: 160; easing.type: Easing.OutBack } }
    }

    // frame
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: Qt.rgba(0, 0, 0, 0.45)
        z: 7
    }

    // scroll to step through the game
    WheelHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onWheel: (event) => { if (event.angleDelta.y > 0) App.prev(); else if (event.angleDelta.y < 0) App.next() }
    }
}
