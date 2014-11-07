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
        colorScheme: "DarkPastels"
        session: QMLTermSession{
	    id: mainsession
            initialWorkingDirectory: "$HOME"
        }
        onUsesMouseChanged: console.log(usesMouse);
        onTerminalSizeChanged: console.log(terminalSize);
        Component.onCompleted: mainsession.startShellProgram();
    }
    Component.onCompleted: terminal.forceActiveFocus();
}
