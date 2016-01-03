%Module QTermWidget




%Import QtGui/QtGuimod.sip
%Import QtCore/QtCoremod.sip
%Import QtWidgets/QtWidgetsmod.sip

class QTermWidget : QWidget {

%TypeHeaderCode
#include <../lib/qtermwidget.h>
%End

public:
    enum ScrollBarPosition
    {
        NoScrollBar=0,
        ScrollBarLeft=1,
        ScrollBarRight=2
    };

    QTermWidget(int startnow = 1, QWidget *parent = 0);
    ~QTermWidget();

    QSize sizeHint() const;
    void startShellProgram();
    int getShellPID();
    void changeDir(const QString & dir);
    void setTerminalFont(QFont &font);
    QFont getTerminalFont();
    void setTerminalOpacity(qreal level);
    void setEnvironment(const QStringList & environment);
    void setShellProgram(const QString & progname);
    void setWorkingDirectory(const QString & dir);
    QString workingDirectory();
    void setArgs(QStringList &args);
    void setTextCodec(QTextCodec *codec);
    void setColorScheme(const QString & name);
    static QStringList availableColorSchemes();
    void setHistorySize(int lines);
    void setScrollBarPosition(ScrollBarPosition);
    void scrollToEnd();
    void sendText(QString &text);
    void setFlowControlEnabled(bool enabled);
    bool flowControlEnabled();
    void setFlowControlWarningEnabled(bool enabled);
    static QStringList availableKeyBindings();
    QString keyBindings();
    void setMotionAfterPasting(int);
    int historyLinesCount();
    int screenColumnsCount();
    void setSelectionStart(int row, int column);
    void setSelectionEnd(int row, int column);
    void getSelectionStart(int& row, int& column);
    void getSelectionEnd(int& row, int& column);
    QString selectedText(bool preserveLineBreaks = true);
    void setMonitorActivity(bool);
    void setMonitorSilence(bool);
    void setSilenceTimeout(int seconds);
signals:
    void finished();
    void copyAvailable(bool);
    void termGetFocus();
    void termLostFocus();
    void termKeyPressed(QKeyEvent *);
    void urlActivated(const QUrl&);
    void bell(const QString& message);
    void activity();
    void silence();
public slots:
    void copyClipboard();
    void pasteClipboard();
    void pasteSelection();
    void zoomIn();
    void zoomOut();
    void setKeyBindings(const QString & kb);
    void clear();
    void toggleShowSearchBar();
    void setSize(const QSize&);
protected:
    void resizeEvent(QResizeEvent *e);
private:
    void *createTermWidget(int startnow, void *parent);
};
