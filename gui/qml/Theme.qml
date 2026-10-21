pragma Singleton
import QtQuick

// Design tokens. Every colour, size and radius in the UI comes from here.
QtObject {
    // ---- surfaces (dark, blue-tinted neutrals) ----
    readonly property color bg0: "#080B12"        // window
    readonly property color bg1: "#101620"        // bars, panels
    readonly property color bg2: "#151D2A"        // cards
    readonly property color bg3: "#202B3B"        // raised / hover
    readonly property color bg4: "#2B394D"        // pressed / tracks
    readonly property color line: "#263347"
    readonly property color lineStrong: "#40516B"

    // ---- text ----
    readonly property color text0: "#F3F6FC"
    readonly property color text1: "#C0CADB"
    readonly property color text2: "#93A1B7"
    readonly property color text3: "#687890"

    // ---- brand ----
    readonly property color accent: "#9B8AFF"
    readonly property color accentHi: "#B8AEFF"
    readonly property color accentInk: "#0C0A1F"
    readonly property color positive: "#55DFA0"
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
    readonly property int radiusSm: 8
    readonly property int radius: 12
    readonly property int radiusLg: 18

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
