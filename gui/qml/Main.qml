import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 1600
    height: 960
    minimumWidth: 1120
    minimumHeight: 720
    visibility: Window.Windowed
    property bool fullscreenMode: false

    function toggleFullscreen() {
        fullscreenMode = !fullscreenMode
        visibility = fullscreenMode ? Window.FullScreen : Window.Windowed
    }
    visible: true
    color: Theme.bg0
    title: App.gameLoaded ? (App.gameInfo.white + " vs " + App.gameInfo.black + "  ·  zugfish") : "zugfish"

    property int tabIndex: 0      // 0 Review, 1 Moves, 2 Details
    readonly property bool typing: !!activeFocusItem && activeFocusItem.hasOwnProperty("cursorPosition")

    function openDialog(name) {
        if (name === "import") importDialog.openAt(0)
        else if (name === "paste") importDialog.openAt(2)
        else if (name === "settings") settingsDialog.open()
    }

    // ------------------------------------------------------------------ layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TopBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 58
            onOpenImport: importDialog.openAt(0)
            onOpenFile: fileDialog.open()
            onOpenSettings: settingsDialog.open()
        }

        Item {
            id: content
            Layout.fillWidth: true
            Layout.fillHeight: true

            WelcomeView {
                anchors.fill: parent
                visible: !App.gameLoaded
                onOpenImport: (tab) => importDialog.openAt(tab)
                onOpenFile: fileDialog.open()
            }

            Item {
                id: workspace
                anchors.fill: parent
                visible: App.gameLoaded

                // sizes (the board is the unit everything else is fitted around)
                readonly property real pad: 20
                readonly property real gap: 10
                readonly property real stripH: 42
                readonly property real graphH: 0
                readonly property real controlsH: 42
                readonly property real evalW: 20
                readonly property real panelMin: 390
                readonly property real colGap: 16
                readonly property real fitH: height - 2 * pad - 2 * stripH - controlsH - 3 * gap - 12
                // Reserve roughly 60% of the workspace for the board and 40% for analysis.
                // The square board still respects available vertical space on smaller windows.
                readonly property real usableW: width - 2 * pad - colGap
                readonly property real boardShareW: usableW * 0.60
                readonly property real panelW: Math.max(340, width - (leftX + boardColW + colGap) - pad)
                readonly property real fitW: boardShareW - evalW - gap
                readonly property real boardSize: Math.floor(Math.max(280, Math.min(fitH, fitW)) / 8) * 8
                readonly property real boardColW: evalW + gap + boardSize
                readonly property real leftX: pad
                readonly property real colH: 2 * stripH + boardSize + controlsH + 3 * gap + 12
                readonly property real topY: Math.max(pad, (height - colH) / 2)

                // top = the side at the top of the board
                readonly property bool topIsWhite: App.flipped
                readonly property var mat: App.material
                readonly property string resultText: App.gameInfo.result || ""
                readonly property bool atEnd: App.currentPly === App.plyCount

                function sideName(white) { return white ? App.gameInfo.white : App.gameInfo.black }
                function sideRating(white) { return white ? (App.gameInfo.whiteElo || "") : (App.gameInfo.blackElo || "") }
                function sideCaptured(white) { return (white ? mat.whiteCaptured : mat.blackCaptured) || [] }
                function sideLead(white) { var d = mat.diff || 0; return Math.max(0, white ? d : -d) }
                function sideResult(white) {
                    if (!atEnd) return ""
                    if (resultText === "1/2-1/2") return "½"
                    if (resultText === "1-0") return white ? "1" : "0"
                    if (resultText === "0-1") return white ? "0" : "1"
                    return ""
                }

                Column {
                    id: leftCol
                    x: workspace.leftX
                    y: workspace.topY
                    spacing: workspace.gap

                    Item {
                        width: workspace.boardColW; height: workspace.stripH
                        PlayerStrip {
                            x: workspace.evalW + workspace.gap; width: workspace.boardSize; height: workspace.stripH
                            isWhite: workspace.topIsWhite
                            name: workspace.sideName(isWhite) || ""
                            rating: workspace.sideRating(isWhite)
                            toMove: App.whiteToMove === isWhite && App.currentPly < App.plyCount
                            captured: workspace.sideCaptured(isWhite)
                            lead: workspace.sideLead(isWhite)
                            resultText: workspace.sideResult(isWhite)
                        }
                    }

                    Row {
                        spacing: workspace.gap
                        EvalBar {
                            width: workspace.evalW
                            height: workspace.boardSize
                            winWhite: App.current.winWhite
                            hasEval: !!App.current.hasEval
                            label: App.current.evalText || ""
                            flipped: App.flipped
                        }
                        ChessBoard { size: workspace.boardSize }
                    }

                    Item {
                        width: workspace.boardColW; height: workspace.stripH
                        PlayerStrip {
                            x: workspace.evalW + workspace.gap; width: workspace.boardSize; height: workspace.stripH
                            isWhite: !workspace.topIsWhite
                            name: workspace.sideName(isWhite) || ""
                            rating: workspace.sideRating(isWhite)
                            toMove: App.whiteToMove === isWhite && App.currentPly < App.plyCount
                            captured: workspace.sideCaptured(isWhite)
                            lead: workspace.sideLead(isWhite)
                            resultText: workspace.sideResult(isWhite)
                        }
                    }

                    Item {
                        width: workspace.boardColW; height: workspace.controlsH
                        BoardControls {
                            x: workspace.evalW + workspace.gap
                            width: workspace.boardSize
                            height: workspace.controlsH
                        }
                    }
                }

                // ---------------- right-hand panel ----------------
                Rectangle {
                    x: workspace.leftX + workspace.boardColW + workspace.colGap
                    y: workspace.pad
                    width: workspace.panelW
                    height: workspace.height - 2 * workspace.pad
                    radius: Theme.radiusLg
                    color: Theme.bg1
                    border.width: 1
                    border.color: Theme.line

                    SegmentedTabs {
                        id: tabs
                        x: 16; y: 16
                        width: parent.width - 32
                        model: ["Review", "Moves", "Details"]
                        currentIndex: win.tabIndex
                        onActivated: (i) => win.tabIndex = i
                    }
                    Item {
                        x: 16; y: 16 + tabs.height + 14
                        width: parent.width - 32
                        height: parent.height - y - 8
                        clip: true

                        ReviewPanel  { anchors.fill: parent; opacity: win.tabIndex === 0 ? 1 : 0; visible: opacity > 0.01
                                       Behavior on opacity { NumberAnimation { duration: 140 } } }
                        MovesPanel   { anchors.fill: parent; opacity: win.tabIndex === 1 ? 1 : 0; visible: opacity > 0.01
                                       Behavior on opacity { NumberAnimation { duration: 140 } } }
                        DetailsPanel { anchors.fill: parent; opacity: win.tabIndex === 2 ? 1 : 0; visible: opacity > 0.01
                                       Behavior on opacity { NumberAnimation { duration: 140 } } }
                    }
                }
            }
        }
    }

    // ------------------------------------------------------------------ dialogs & overlays
    ImportDialog { id: importDialog }
    SettingsDialog { id: settingsDialog }

    FileDialog {
        id: fileDialog
        title: "Open a PGN file"
        nameFilters: ["PGN files (*.pgn *.txt)", "All files (*)"]
        onAccepted: App.loadPgnFile(selectedFile)
    }

    // Floating fullscreen toggle remains available even when the native title bar is hidden.
    Rectangle {
        id: fullscreenButton
        z: 150
        visible: App.gameLoaded
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: win.fullscreenMode ? 18 : 18
        anchors.bottomMargin: win.fullscreenMode ? 18 : 18
        width: 42
        height: 42
        radius: 12
        color: fullscreenMouse.containsMouse ? Theme.bg3 : Theme.bg2
        border.width: 1
        border.color: Theme.line
        Icon {
            anchors.centerIn: parent
            name: win.fullscreenMode ? "minimize" : "maximize"
            color: Theme.text0
            size: 18
        }
        MouseArea {
            id: fullscreenMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: win.toggleFullscreen()
        }
        ToolTip.visible: fullscreenMouse.containsMouse
        ToolTip.text: win.fullscreenMode ? "Exit full screen (F11 / Ctrl+F)" : "Full screen (F11 / Ctrl+F)"
        ToolTip.delay: 500
    }

    Toast {
        id: toast
        z: 100
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
    }
    Connections {
        target: App
        function onToast(message, kind) { toast.show(message, kind) }
    }

    // drag a .pgn file (or selected PGN text) onto the window
    DropArea {
        anchors.fill: parent
        onDropped: (drop) => {
            if (drop.hasUrls && drop.urls.length > 0) App.loadPgnFile(drop.urls[0])
            else if (drop.hasText) {
                var err = App.loadPgn(drop.text)
                if (err !== "") toast.show(err, 2)
            }
        }
        Rectangle {
            anchors.fill: parent
            anchors.margins: 12
            visible: parent.containsDrag
            radius: Theme.radiusLg
            color: Qt.rgba(0.55, 0.49, 1, 0.08)
            border.width: 2
            border.color: Theme.accent
            z: 200
            Text {
                anchors.centerIn: parent
                text: "Drop to open this game"
                color: Theme.accentHi
                font.family: Theme.sans; font.weight: Font.DemiBold; font.pixelSize: Theme.fsXl
            }
        }
    }

    // ------------------------------------------------------------------ keyboard
    readonly property bool keysOn: !typing && !importDialog.visible && !settingsDialog.visible
    Shortcut { sequence: "Left";  enabled: win.keysOn; onActivated: App.prev() }
    Shortcut { sequence: "Right"; enabled: win.keysOn; onActivated: App.next() }
    Shortcut { sequence: "Home";  enabled: win.keysOn; onActivated: App.first() }
    Shortcut { sequence: "End";   enabled: win.keysOn; onActivated: App.last() }
    Shortcut { sequence: "Space"; enabled: win.keysOn && App.gameLoaded; onActivated: App.toggleAutoplay() }
    Shortcut { sequence: "F";     enabled: win.keysOn; onActivated: App.toggleFlip() }
    Shortcut { sequence: "K";     enabled: win.keysOn; onActivated: App.jumpKey(1) }
    Shortcut { sequence: "J";     enabled: win.keysOn; onActivated: App.jumpKey(-1) }
    Shortcut { sequence: "R";     enabled: win.keysOn && App.gameLoaded; onActivated: App.startReview() }
    Shortcut { sequence: "Ctrl+O"; onActivated: fileDialog.open() }
    Shortcut { sequence: "Ctrl+I"; onActivated: importDialog.openAt(0) }
    Shortcut { sequence: "Ctrl+V"; enabled: win.keysOn; onActivated: importDialog.openAt(2) }
    Shortcut { sequence: "Ctrl+,"; onActivated: settingsDialog.open() }
    Shortcut { sequence: "F11"; onActivated: win.toggleFullscreen() }
    Shortcut { sequence: "Ctrl+F"; onActivated: win.toggleFullscreen() }
}
