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


#ifndef _Q_TERM_WIDGET
#define _Q_TERM_WIDGET

#include <QtGui>

struct TermWidgetImpl;


class QTermWidget : public QWidget {
    Q_OBJECT
public:

    enum ScrollBarPosition {
        /** Do not show the scroll bar. */
        NoScrollBar=0,
        /** Show the scroll bar on the left side of the display. */
        ScrollBarLeft=1,
        /** Show the scroll bar on the right side of the display. */
        ScrollBarRight=2
    };


    //Creation of widget
    QTermWidget(int startnow = 1, //start shell programm immediatelly
                QWidget * parent = 0);
    ~QTermWidget();

    //Initial size
    QSize sizeHint() const;

    //start shell program if it was not started in constructor
    void startShellProgram();

    int getShellPID();

    void changeDir(const QString & dir);

    //look-n-feel, if you don`t like defaults

    //  Terminal font
    // Default is application font with family Monospace, size 10
    // USE ONLY FIXED-PITCH FONT!
    // otherwise symbols' position could be incorrect
    void setTerminalFont(QFont & font);

    void setTerminalOpacity(qreal level);

    //environment
    void setEnvironment(const QStringList & environment);

    //  Shell program, default is /bin/bash
    void setShellProgram(const QString & progname);

    //working directory
    void setWorkingDirectory(const QString & dir);

    // Shell program args, default is none
    void setArgs(QStringList & args);

    //Text codec, default is UTF-8
    void setTextCodec(QTextCodec * codec);

    //Color scheme, default is white on black
    void setColorScheme(const QString & name);
    static QStringList availableColorSchemes();

    //set size
    void setSize(int h, int v);

    // History size for scrolling
    void setHistorySize(int lines); //infinite if lines < 0

    // Presence of scrollbar
    void setScrollBarPosition(ScrollBarPosition);

    // Send some text to terminal
    void sendText(QString & text);

    // Sets whether flow control is enabled
    void setFlowControlEnabled(bool enabled);

    // Returns whether flow control is enabled
    bool flowControlEnabled(void);

    /**
     * Sets whether the flow control warning box should be shown
     * when the flow control stop key (Ctrl+S) is pressed.
     */
    void setFlowControlWarningEnabled(bool enabled);

    /*! Get all available keyboard bindings
     */
    static QStringList availableKeyBindings();

    //! Return current key bindings
    QString keyBindings();

signals:
    void finished();
    void copyAvailable(bool);

    void termGetFocus();
    void termLostFocus();

public slots:
    // Paste clipboard content to terminal
    void copyClipboard();

    // Copies selection to clipboard
    void pasteClipboard();

    /*! Set named key binding for given widget
     */
    void setKeyBindings(const QString & kb);

protected:
    virtual void resizeEvent(QResizeEvent *);

protected slots:
    void sessionFinished();
    void selectionChanged(bool textSelected);

private:
    void init();
    TermWidgetImpl * m_impl;
};


//Maybe useful, maybe not

#ifdef __cplusplus
extern "C"
#endif
void * createTermWidget(int startnow, void * parent);

#endif

