/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QLayout>
#include <QtGui/qboxlayout.h>
#include <QtGui/qlayoutitem.h>
#include <QtGui/qsizepolicy.h>
#include "SearchBar.h"
#include "qtermwidget.h"
#include "ColorTables.h"

#include "Session.h"
#include "Screen.h"
#include "ScreenWindow.h"
#include "Emulation.h"
#include "TerminalDisplay.h"
#include "KeyboardTranslator.h"
#include "ColorScheme.h"
#include "SearchBar.h"

#define STEP_ZOOM 3

using namespace Konsole;

void *createTermWidget(int startnow, void *parent)
{
    return (void*) new QTermWidget(startnow, (QWidget*)parent);
}

struct TermWidgetImpl {
    TermWidgetImpl(QWidget* parent = 0);

    TerminalDisplay *m_terminalDisplay;
    Session *m_session;

    Session* createSession();
    TerminalDisplay* createTerminalDisplay(Session *session, QWidget* parent);
};

TermWidgetImpl::TermWidgetImpl(QWidget* parent)
{
    this->m_session = createSession();
    this->m_terminalDisplay = createTerminalDisplay(this->m_session, parent);
}


Session *TermWidgetImpl::createSession()
{
    Session *session = new Session();

    session->setTitle(Session::NameRole, "QTermWidget");

    /* Thats a freaking bad idea!!!!
     * /bin/bash is not there on every system
     * better set it to the current $SHELL
     * Maybe you can also make a list available and then let the widget-owner decide what to use.
     * By setting it to $SHELL right away we actually make the first filecheck obsolete.
     * But as iam not sure if you want to do anything else ill just let both checks in and set this to $SHELL anyway.
     */
    //session->setProgram("/bin/bash");

    session->setProgram(getenv("SHELL"));



    QStringList args("");
    session->setArguments(args);
    session->setAutoClose(true);

    session->setCodec(QTextCodec::codecForName("UTF-8"));

    session->setFlowControlEnabled(true);
    session->setHistoryType(HistoryTypeBuffer(1000));

    session->setDarkBackground(true);

    session->setKeyBindings("");
    return session;
}

TerminalDisplay *TermWidgetImpl::createTerminalDisplay(Session *session, QWidget* parent)
{
//    TerminalDisplay* display = new TerminalDisplay(this);
    TerminalDisplay* display = new TerminalDisplay(parent);

    display->setBellMode(TerminalDisplay::NotifyBell);
    display->setTerminalSizeHint(true);
    display->setTripleClickMode(TerminalDisplay::SelectWholeLine);
    display->setTerminalSizeStartup(true);

    display->setRandomSeed(session->sessionId() * 31);

    return display;
}


QTermWidget::QTermWidget(int startnow, QWidget *parent)
    : QWidget(parent)
{
    init(startnow);
}

QTermWidget::QTermWidget(QWidget *parent)
    : QWidget(parent)
{
    init(1);
}

void QTermWidget::selectionChanged(bool textSelected)
{
    emit copyAvailable(textSelected);
}

void QTermWidget::search(QRegExp regexp, bool forwards, bool next)
{
    int startColumn, startLine;
    
    if (next) // search from just after current selection
    {
        m_impl->m_terminalDisplay->screenWindow()->screen()->getSelectionEnd(startColumn, startLine);
        startColumn++;
    }
    else // search from start of current selection
    {
        m_impl->m_terminalDisplay->screenWindow()->screen()->getSelectionStart(startColumn, startLine);
    }
   
    qDebug() << "current selection starts at: " << startColumn << startLine;
    qDebug() << "current cursor position: " << m_impl->m_terminalDisplay->screenWindow()->cursorPosition(); 


    HistorySearch *historySearch = 
            new HistorySearch(m_impl->m_session->emulation(), regexp, forwards, startColumn, startLine, this);
    connect(historySearch, SIGNAL(matchFound(int, int, int, int)), this, SLOT(matchFound(int, int, int, int)));
    connect(historySearch, SIGNAL(noMatchFound()), this, SLOT(noMatchFound()));
    historySearch->search();
}


void QTermWidget::matchFound(int startColumn, int startLine, int endColumn, int endLine)
{
    ScreenWindow* sw = m_impl->m_terminalDisplay->screenWindow();
    qDebug() << "Scroll to" << startLine;
    sw->scrollTo(startLine);
    sw->setTrackOutput(false);
    sw->notifyOutputChanged();
    sw->setSelectionStart(startColumn, startLine - sw->currentLine(), false);
    sw->setSelectionEnd(endColumn, endLine - sw->currentLine());
}

void QTermWidget::noMatchFound() 
{
        m_impl->m_terminalDisplay->screenWindow()->clearSelection();
}

int QTermWidget::getShellPID()
{
    return m_impl->m_session->processId();
}

void QTermWidget::changeDir(const QString & dir)
{
    /*
       this is a very hackish way of trying to determine if the shell is in
       the foreground before attempting to change the directory.  It may not
       be portable to anything other than Linux.
    */
    QString strCmd;
    strCmd.setNum(getShellPID());
    strCmd.prepend("ps -j ");
    strCmd.append(" | tail -1 | awk '{ print $5 }' | grep -q \\+");
    int retval = system(strCmd.toStdString().c_str());

    if (!retval) {
        QString cmd = "cd " + dir + "\n";
        sendText(cmd);
    }
}

QSize QTermWidget::sizeHint() const
{
    QSize size = m_impl->m_terminalDisplay->sizeHint();
    size.rheight() = 150;
    return size;
}

void QTermWidget::startShellProgram()
{
    if ( m_impl->m_session->isRunning() ) {
        return;
    }

    m_impl->m_session->run();
}

void QTermWidget::init(int startnow)
{
    m_layout = new QVBoxLayout();
    m_layout->setMargin(0);
    setLayout(m_layout);
    
    m_impl = new TermWidgetImpl(this);
    m_impl->m_terminalDisplay->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_layout->addWidget(m_impl->m_terminalDisplay);

    m_searchBar = new SearchBar(this);
    m_searchBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    connect(m_searchBar, SIGNAL(search(QRegExp, bool, bool)), this, SLOT(search(QRegExp, bool, bool)));
    m_layout->addWidget(m_searchBar);
    m_searchBar->hide();

    if (startnow && m_impl->m_session) {
        m_impl->m_session->run();
    }

    this->setFocus( Qt::OtherFocusReason );
    m_impl->m_terminalDisplay->resize(this->size());

    this->setFocusProxy(m_impl->m_terminalDisplay);
    connect(m_impl->m_terminalDisplay, SIGNAL(copyAvailable(bool)),
            this, SLOT(selectionChanged(bool)));
    connect(m_impl->m_terminalDisplay, SIGNAL(termGetFocus()),
            this, SIGNAL(termGetFocus()));
    connect(m_impl->m_terminalDisplay, SIGNAL(termLostFocus()),
            this, SIGNAL(termLostFocus()));
    connect(m_impl->m_terminalDisplay, SIGNAL(keyPressedSignal(QKeyEvent *)),
            this, SIGNAL(termKeyPressed(QKeyEvent *)));
//    m_impl->m_terminalDisplay->setSize(80, 40);

    QFont font = QApplication::font();
    font.setFamily("Monospace");
    font.setPointSize(10);
    font.setStyleHint(QFont::TypeWriter);
    setTerminalFont(font);
    m_searchBar->setFont(font);

    setScrollBarPosition(NoScrollBar);

    m_impl->m_session->addView(m_impl->m_terminalDisplay);

    connect(m_impl->m_session, SIGNAL(finished()), this, SLOT(sessionFinished()));
}


QTermWidget::~QTermWidget()
{
    emit destroyed();
}


void QTermWidget::setTerminalFont(QFont &font)
{
    if (!m_impl->m_terminalDisplay)
        return;
    m_impl->m_terminalDisplay->setVTFont(font);
}

QFont QTermWidget::getTerminalFont()
{
    if (!m_impl->m_terminalDisplay)
        return QFont();
    return m_impl->m_terminalDisplay->getVTFont();
}

void QTermWidget::setTerminalOpacity(qreal level)
{
    if (!m_impl->m_terminalDisplay)
        return;

    m_impl->m_terminalDisplay->setOpacity(level);
}

void QTermWidget::setShellProgram(const QString &progname)
{
    if (!m_impl->m_session)
        return;
    m_impl->m_session->setProgram(progname);
}

void QTermWidget::setWorkingDirectory(const QString& dir)
{
    if (!m_impl->m_session)
        return;
    m_impl->m_session->setInitialWorkingDirectory(dir);
}

void QTermWidget::setArgs(QStringList &args)
{
    if (!m_impl->m_session)
        return;
    m_impl->m_session->setArguments(args);
}

void QTermWidget::setTextCodec(QTextCodec *codec)
{
    if (!m_impl->m_session)
        return;
    m_impl->m_session->setCodec(codec);
}

void QTermWidget::setColorScheme(const QString & name)
{
    const ColorScheme *cs;
    // avoid legacy (int) solution
    if (!availableColorSchemes().contains(name))
        cs = ColorSchemeManager::instance()->defaultColorScheme();
    else
        cs = ColorSchemeManager::instance()->findColorScheme(name);

    if (! cs)
    {
        QMessageBox::information(this,
                                 tr("Color Scheme Error"),
                                 tr("Cannot load color scheme: %1").arg(name));
        return;
    }
    ColorEntry table[TABLE_COLORS];
    cs->getColorTable(table);
    m_impl->m_terminalDisplay->setColorTable(table);
}

QStringList QTermWidget::availableColorSchemes()
{
    QStringList ret;
    foreach (const ColorScheme* cs, ColorSchemeManager::instance()->allColorSchemes())
        ret.append(cs->name());
    return ret;
}

void QTermWidget::setSize(int h, int v)
{
    if (!m_impl->m_terminalDisplay)
        return;
    m_impl->m_terminalDisplay->setSize(h, v);
}

void QTermWidget::setHistorySize(int lines)
{
    if (lines < 0)
        m_impl->m_session->setHistoryType(HistoryTypeFile());
    else
        m_impl->m_session->setHistoryType(HistoryTypeBuffer(lines));
}

void QTermWidget::setScrollBarPosition(ScrollBarPosition pos)
{
    if (!m_impl->m_terminalDisplay)
        return;
    m_impl->m_terminalDisplay->setScrollBarPosition((TerminalDisplay::ScrollBarPosition)pos);
}

void QTermWidget::scrollToEnd()
{
    if (!m_impl->m_terminalDisplay)
        return;
    m_impl->m_terminalDisplay->scrollToEnd();
}

void QTermWidget::sendText(QString &text)
{
    m_impl->m_session->sendText(text);
}

void QTermWidget::resizeEvent(QResizeEvent*)
{
//qDebug("global window resizing...with %d %d", this->size().width(), this->size().height());
    m_impl->m_terminalDisplay->resize(this->size());
}


void QTermWidget::sessionFinished()
{
    emit finished();
}


void QTermWidget::copyClipboard()
{
    m_impl->m_terminalDisplay->copyClipboard();
}

void QTermWidget::pasteClipboard()
{
    m_impl->m_terminalDisplay->pasteClipboard();
}

void QTermWidget::setZoom(int step)
{
    if (!m_impl->m_terminalDisplay)
        return;
    
    QFont font = m_impl->m_terminalDisplay->getVTFont();
    
    font.setPointSize(font.pointSize() + step);
    setTerminalFont(font);
}

void QTermWidget::zoomIn()
{
    setZoom(STEP_ZOOM);
}

void QTermWidget::zoomOut()
{
    setZoom(-STEP_ZOOM);
}

void QTermWidget::setKeyBindings(const QString & kb)
{
    m_impl->m_session->setKeyBindings(kb);
}

void QTermWidget::clear()
{
    m_impl->m_session->emulation()->reset();
    m_impl->m_session->refresh();
    m_impl->m_session->clearHistory();
}

void QTermWidget::setFlowControlEnabled(bool enabled)
{
    m_impl->m_session->setFlowControlEnabled(enabled);
}

QStringList QTermWidget::availableKeyBindings()
{
    return KeyboardTranslatorManager::instance()->allTranslators();
}

QString QTermWidget::keyBindings()
{
    return m_impl->m_session->keyBindings();
}

void QTermWidget::toggleShowSearchBar()
{
    m_searchBar->toggleShown();
}

bool QTermWidget::flowControlEnabled(void)
{
    return m_impl->m_session->flowControlEnabled();
}

void QTermWidget::setFlowControlWarningEnabled(bool enabled)
{
    if (flowControlEnabled()) {
        // Do not show warning label if flow control is disabled
        m_impl->m_terminalDisplay->setFlowControlWarningEnabled(enabled);
    }
}

void QTermWidget::setEnvironment(const QStringList& environment)
{
    m_impl->m_session->setEnvironment(environment);
}

void QTermWidget::setMotionAfterPasting(int action)
{
    m_impl->m_terminalDisplay->setMotionAfterPasting((Konsole::MotionAfterPasting) action);
}
