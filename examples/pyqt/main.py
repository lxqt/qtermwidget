#!/usr/bin/python3
from PyQt5 import QtWidgets
import QTermWidget


class Terminal(QTermWidget.QTermWidget):
    def __init__(self, process: str, args: list):
        super().__init__(0)
        self.setTerminalSizeHint(False)
        self.setColorScheme("DarkPastels")
        self.setShellProgram(process)
        self.setArgs(args)
        self.startShellProgram()
        self.finished.connect(self.close)
        self.show()
        for x in dir(self):
            print(x)


if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    args = ["--clean", "--noplugin"]
    term = Terminal("vim", args)
    app.exec()
