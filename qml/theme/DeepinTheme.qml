pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    property bool isDark: true
    property int themeMode: 0  // 0=dark, 1=light, 2=system

    onThemeModeChanged: {
        if (themeMode === 0) isDark = true
        else if (themeMode === 1) isDark = false
        else isDark = true  // system: default dark for now
    }

    // ---- Dark Theme ----
    readonly property color darkBg: "#0d1117"
    readonly property color darkCard: "#161b22"
    readonly property color darkCardHover: "#1c2129"
    readonly property color darkBorder: "#30363d"
    readonly property color darkText: "#ffffff"
    readonly property color darkSubText: "#ffffff"
    readonly property color darkInputBg: "#283240"
    readonly property color darkInputBorder: "#30363d"
    readonly property color darkPanelBg: "#0d1117"

    // ---- Light Theme ----
    readonly property color lightBg: "#f6f8fa"
    readonly property color lightCard: "#ffffff"
    readonly property color lightCardHover: "#f3f4f6"
    readonly property color lightBorder: "#d0d7de"
    readonly property color lightText: "#1a3a6e"
    readonly property color lightSubText: "#4a6a8a"
    readonly property color lightInputBg: "#ffffff"
    readonly property color lightInputBorder: "#d0d7de"
    readonly property color lightPanelBg: "#ffffff"

    // ---- Accent Colors ----
    readonly property color blue: "#2f81f7"
    readonly property color blueHover: "#388bfd"
    readonly property color green: "#3fb950"
    readonly property color red: "#f85149"
    readonly property color orange: "#d29922"
    readonly property color purple: "#a371f7"
    readonly property color cyan: "#39d2c0"

    // ---- Dynamic Colors ----
    readonly property color bg: isDark ? darkBg : lightBg
    readonly property color card: isDark ? darkCard : lightCard
    readonly property color cardHover: isDark ? darkCardHover : lightCardHover
    readonly property color border: isDark ? darkBorder : lightBorder
    readonly property color text: isDark ? darkText : lightText
    readonly property color subText: isDark ? darkSubText : lightSubText
    readonly property color inputBg: isDark ? darkInputBg : lightInputBg
    readonly property color inputBorder: isDark ? darkInputBorder : lightInputBorder
    readonly property color panelBg: isDark ? darkPanelBg : lightPanelBg

    readonly property color receiveText: isDark ? "#7ee787" : "#1a7f37"
    readonly property color sendText: isDark ? "#79c0ff" : "#0969da"
    readonly property color sendBg: isDark ? "#161b22" : "#f6f8fa"

    // ---- Radius ----
    readonly property int radiusTiny: 4
    readonly property int radiusSmall: 6
    readonly property int radiusMedium: 8
    readonly property int radiusLarge: 12
    readonly property int radiusXLarge: 16

    // ---- Font ----
    readonly property string fontFamily: "Noto Sans, Noto Sans CJK SC, 国标黑体, sans-serif"
    readonly property string monoFamily: "DejaVu Sans Mono, Source Han Mono SC, Liberation Mono, monospace"

    readonly property int fontSizeSmall: 11
    readonly property int fontSizeNormal: 13
    readonly property int fontSizeLarge: 15
    readonly property int fontSizeTitle: 18

    // ---- Spacing ----
    readonly property int spacingTiny: 4
    readonly property int spacingSmall: 8
    readonly property int spacingNormal: 12
    readonly property int spacingLarge: 16
    readonly property int spacingXLarge: 24

    // ---- Layout ----
    readonly property int leftPanelWidth: 300
    readonly property int headerHeight: 44
    readonly property int statusBarHeight: 30
    readonly property int buttonHeight: 32
    readonly property int inputHeight: 36
}
