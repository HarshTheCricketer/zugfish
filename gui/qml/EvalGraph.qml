import QtQuick

// Advantage over time. The light area is White's share; click or drag to scrub the game.
Item {
    id: root
    implicitHeight: 64

    readonly property var series: App.winSeries
    readonly property var classes: App.classSeries
    readonly property int current: App.currentPly
    readonly property bool ready: series.length > 1
    property real hoverX: -1

    function xAt(i) { return ready ? i * (width / (series.length - 1)) : 0 }
    function yAt(v) { return height * (1 - Math.max(4, Math.min(96, v)) / 100) }
    function indexAt(x) { return ready ? Math.max(0, Math.min(series.length - 1, Math.round(x / (width / (series.length - 1))))) : 0 }

    Rectangle {
        anchors.fill: parent
        radius: Theme.radius
        color: Theme.bg2
        border.width: 1
        border.color: Theme.line
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 1
        antialiasing: true
        visible: root.ready
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var s = root.series, w = width, h = height, n = s.length
            if (n < 2) return
            var dx = w / (n - 1)
            function X(i) { return i * dx }
            function Y(v) { return h * (1 - Math.max(4, Math.min(96, v)) / 100) }

            // White's share (area under the line)
            ctx.beginPath()
            ctx.moveTo(0, h)
            for (var i = 0; i < n; i++) ctx.lineTo(X(i), Y(s[i]))
            ctx.lineTo(w, h)
            ctx.closePath()
            var g = ctx.createLinearGradient(0, 0, 0, h)
            g.addColorStop(0, "rgba(220,226,238,0.55)")
            g.addColorStop(1, "rgba(220,226,238,0.92)")
            ctx.fillStyle = g
            ctx.fill()

            // equal-position line
            ctx.strokeStyle = "rgba(139,150,170,0.45)"
            ctx.lineWidth = 1
            ctx.setLineDash([3, 4])
            ctx.beginPath(); ctx.moveTo(0, h / 2); ctx.lineTo(w, h / 2); ctx.stroke()
            ctx.setLineDash([])

            // the curve
            ctx.beginPath()
            for (var j = 0; j < n; j++) { if (j === 0) ctx.moveTo(X(j), Y(s[j])); else ctx.lineTo(X(j), Y(s[j])) }
            ctx.strokeStyle = "#F4F6FA"
            ctx.lineWidth = 1.6
            ctx.lineJoin = "round"
            ctx.stroke()

            // key moves
            var cls = root.classes
            for (var k = 0; k < cls.length; k++) {
                var c = cls[k]
                if (c === 0 || c === 1 || c === 7 || c === 8 || c === 9) {
                    ctx.beginPath()
                    ctx.arc(X(k + 1), Y(s[k + 1]), 3.6, 0, Math.PI * 2)
                    ctx.fillStyle = Theme.classColor(c)
                    ctx.fill()
                    ctx.lineWidth = 1.4
                    ctx.strokeStyle = "#0B0E14"
                    ctx.stroke()
                }
            }

            // playhead
            var px = X(root.current)
            ctx.beginPath(); ctx.moveTo(px, 0); ctx.lineTo(px, h)
            ctx.strokeStyle = "#8B7CFF"; ctx.lineWidth = 2; ctx.stroke()
            ctx.beginPath(); ctx.arc(px, Y(s[Math.min(root.current, n - 1)]), 4.4, 0, Math.PI * 2)
            ctx.fillStyle = "#8B7CFF"; ctx.fill()
            ctx.lineWidth = 2; ctx.strokeStyle = "#0B0E14"; ctx.stroke()

            // hover guide
            if (root.hoverX >= 0) {
                ctx.beginPath(); ctx.moveTo(root.hoverX, 0); ctx.lineTo(root.hoverX, h)
                ctx.strokeStyle = "rgba(238,241,246,0.35)"; ctx.lineWidth = 1; ctx.stroke()
            }
        }
        Connections {
            target: root
            function onSeriesChanged() { canvas.requestPaint() }
            function onClassesChanged() { canvas.requestPaint() }
            function onCurrentChanged() { canvas.requestPaint() }
            function onHoverXChanged() { canvas.requestPaint() }
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    // empty / running state
    Column {
        anchors.centerIn: parent
        visible: !root.ready
        spacing: 6
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: App.reviewState === 1 ? "Analysing…" : "Run the review to see how the game swung"
            color: Theme.text2
            font.family: Theme.sans
            font.pixelSize: Theme.fsSm + 1
        }
        Rectangle {
            visible: App.reviewState === 1
            anchors.horizontalCenter: parent.horizontalCenter
            width: 180; height: 3; radius: 1.5; color: Theme.bg4
            Rectangle { width: parent.width * App.reviewProgress; height: parent.height; radius: 1.5; color: Theme.accent
                Behavior on width { NumberAnimation { duration: 120 } } }
        }
    }

    // hover read-out
    Rectangle {
        visible: root.ready && root.hoverX >= 0
        readonly property int idx: root.indexAt(root.hoverX)
        x: Math.max(4, Math.min(root.width - width - 4, root.hoverX - width / 2))
        y: -height - 6
        width: tipText.implicitWidth + 16
        height: 26
        radius: 6
        color: "#0B0E14"
        border.color: Theme.lineStrong
        Text {
            id: tipText
            anchors.centerIn: parent
            text: {
                var i = parent.idx
                if (i === 0) return "Start"
                var v = root.series[i]
                return Math.ceil(i / 2) + (i % 2 ? "." : "...") + "  ·  White " + Math.round(v) + "%"
            }
            color: Theme.text0
            font.family: Theme.mono
            font.pixelSize: Theme.fsXs + 1
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.ready
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onPositionChanged: (m) => { root.hoverX = m.x; if (pressed) App.goTo(root.indexAt(m.x)) }
        onPressed: (m) => App.goTo(root.indexAt(m.x))
        onExited: root.hoverX = -1
    }
}
