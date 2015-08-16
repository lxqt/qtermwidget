import QtQuick 2.0
import QMLTermWidget 1.0
import QtQuick.Controls 1.2

Rectangle {
    width: 640
    height: 480

    Action{
        onTriggered: terminal.copyClipboard();
        shortcut: "Ctrl+Shift+C"
    }

    Action{
        onTriggered: terminal.pasteClipboard();
        shortcut: "Ctrl+Shift+V"
    }

    QMLTermWidget {
        id: terminal
        anchors.fill: parent
        font.family: "Monospace"
        font.pointSize: 12
        colorScheme: "cool-retro-term"
        session: QMLTermSession{
            id: mainsession
            initialWorkingDirectory: "$HOME"
            onMatchFound: {
                console.log("found at: %1 %2 %3 %4".arg(startColumn).arg(startLine).arg(endColumn).arg(endLine));
            }
            onNoMatchFound: {
                console.log("not found");
            }
        }
        onTerminalUsesMouseChanged: console.log(terminalUsesMouse);
        onTerminalSizeChanged: console.log(terminalSize);
        Component.onCompleted: mainsession.startShellProgram();

        QMLTermScrollbar {
            terminal: terminal
            width: 20
            Rectangle {
                opacity: 0.4
                anchors.margins: 5
                radius: width * 0.5
                anchors.fill: parent
            }
        }

    }

    Button {
        text: "Find version"
        height: 32
        width: 48
        onClicked: mainsession.search("version");

    }
    Component.onCompleted: terminal.forceActiveFocus();
}
