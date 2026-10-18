pragma Singleton
import QtQuick

// Design tokens. Every colour, size and radius in the UI comes from here.
QtObject {
    // ---- surfaces (dark, blue-tinted neutrals) ----
    readonly property color bg0: "#090C11"        // window
    readonly property color bg1: "#0F131A"        // bars, panels
    readonly property color bg2: "#151A23"        // cards
    readonly property color bg3: "#1C2330"        // raised / hover
    readonly property color bg4: "#262F40"        // pressed / tracks
    readonly property color line: "#232B38"
    readonly property color lineStrong: "#334055"

    // ---- text ----
    readonly property color text0: "#EEF1F6"
    readonly property color text1: "#B4BCCB"
    readonly property color text2: "#7B8599"
    readonly property color text3: "#525B6E"

    // ---- brand ----
    readonly property color accent: "#8B7CFF"
    readonly property color accentHi: "#A79CFF"
    readonly property color accentInk: "#0C0A1F"
    readonly property color positive: "#4ADE80"
    readonly property color warning: "#F2C94C"
    readonly property color danger: "#F87171"

    // ---- type ----
    readonly property string sans: "IBM Plex Sans"
    readonly property string mono: "IBM Plex Mono"
    readonly property int fsXs: 11
    readonly property int fsSm: 12
    readonly property int fsMd: 13
    readonly property int fsLg: 15
    readonly property int fsXl: 18
    readonly property int fs2xl: 24
    readonly property int fs3xl: 34

    // ---- shape & rhythm (4px grid) ----
    readonly property int s1: 4
    readonly property int s2: 8
    readonly property int s3: 12
    readonly property int s4: 16
    readonly property int s5: 20
    readonly property int s6: 24
    readonly property int radiusSm: 6
    readonly property int radius: 10
    readonly property int radiusLg: 14

    // ---- move classes: index = zq::MoveClass ----
    readonly property var classColors: [
        "#22D3EE",  // 0 Brilliant
        "#5B9DF5",  // 1 Great
        "#6FCB6B",  // 2 Best
        "#9BD66E",  // 3 Excellent
        "#8DA395",  // 4 Good
        "#C9A27E",  // 5 Book
        "#F2C94C",  // 6 Inaccuracy
        "#F2994A",  // 7 Mistake
        "#EB5757",  // 8 Blunder
        "#C8509A",  // 9 Missed win
        "#8892A6"   // 10 Forced
    ]
    readonly property var classNames: ["Brilliant", "Great", "Best", "Excellent", "Good", "Book",
                                       "Inaccuracy", "Mistake", "Blunder", "Missed win", "Forced"]
    function classColor(c) { return c >= 0 && c < classColors.length ? classColors[c] : text2 }
    function className(c) { return c >= 0 && c < classNames.length ? classNames[c] : "" }

    // ---- board palettes ----
    readonly property var boardThemes: [
        { name: "Slate",  light: "#DCE2EE", dark: "#7B91B3" },
        { name: "Walnut", light: "#EBD7B5", dark: "#B07A52" },
        { name: "Forest", light: "#EDEDD1", dark: "#6E9953" },
        { name: "Mono",   light: "#D8D8DC", dark: "#8A8A93" }
    ]
    readonly property color lastMove: "#FFD54A"
    readonly property color bestArrow: "#4FB0FF"

    function winColor(v) { return v >= 85 ? positive : v >= 70 ? warning : "#F2994A" }
}
