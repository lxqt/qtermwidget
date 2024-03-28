from pyqtbuild import PyQtBindings, PyQtProject

class QTermWidget(PyQtProject):
    def __init__(self):
        super().__init__()
        self.bindings_factories = [QTermWidgetBindings]

class QTermWidgetBindings(PyQtBindings):
    def __init__(self, project):
        super().__init__(project, name='QTermWidget', sip_file='qtermwidget.sip', qmake_QT=['widgets'])
        self._project = project

    def apply_user_defaults(self, tool):
        self.libraries.append('qtermwidget6')
        super().apply_user_defaults(tool)
