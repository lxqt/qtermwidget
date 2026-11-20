/*
    Windows port of Konsole::Pty for QTermWidget.

    Copyright (C) 2026 Fabrizio Pistonesi <fpistonesi@webeasytech.com>

    The POSIX implementation (Pty.cpp + kpty*) drives the shell through a
    Unix pseudo terminal. Windows has no such thing, but since Windows 10
    1809 it provides the "pseudo console" (ConPTY) API, which gives the same
    model: the child process sees a real console, while we exchange a
    VT/UTF-8 byte stream with it through two pipes.

    This class keeps the public interface used by Session (start(),
    sendData(), receivedData(), finished(), setWindowSize(), ...) so that
    the rest of the library is unchanged.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef PTY_WIN_H
#define PTY_WIN_H

#include <QObject>
#include <QProcess>
#include <QSize>
#include <QString>
#include <QStringList>

#include <atomic>

class QThread;
class QWinEventNotifier;

namespace Konsole {

class Pty : public QObject
{
    Q_OBJECT

public:
    explicit Pty(QObject *parent = nullptr);
    ~Pty() override;

    /**
     * Starts @p program inside a new pseudo console.
     * @p arguments contains the program name as first element (like argv).
     * Returns 0 on success, -1 on failure (errorOccurred() is emitted).
     */
    int start(const QString &program,
              const QStringList &arguments,
              const QStringList &environment,
              ulong winid,
              bool addToUtmp);

    // Terminal attributes: termios has no ConPTY equivalent, the values are
    // only stored so that the getters behave consistently.
    void setEmptyPTYProperties();
    void setWriteable(bool writeable);
    void setFlowControlEnabled(bool on);
    bool flowControlEnabled() const;
    void setErase(char erase);
    char erase() const;

    void setWindowSize(int lines, int cols);
    QSize windowSize() const;

    void setInitialWorkingDirectory(const QString &dir);

    /** No process groups on Windows: returns the child process id. */
    int foregroundProcessGroup() const;

    /**
     * Closes the pseudo console. Attached processes receive
     * CTRL_CLOSE_EVENT, the Windows counterpart of SIGHUP.
     */
    void closePty();

    // Subset of the QProcess API used by Session.
    QProcess::ProcessState state() const;
    qint64 processId() const;
    bool waitForFinished(int msecs = 30000);
    QProcess::ExitStatus exitStatus() const;
    int exitCode() const;
    /** Forcefully terminates the child (TerminateProcess). */
    void kill();

public slots:
    void setUtf8Mode(bool on);
    void lockPty(bool lock);
    void sendData(const char *buffer, int length);

signals:
    void receivedData(const char *buffer, int length);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void errorOccurred(QProcess::ProcessError error);

private:
    void handleProcessExit();
    void finishIfDone();
    void closeConsoleAsync();
    bool stopReader(int msecs);
    void releaseHandles();

    // Win32 handles are kept as void* so that <windows.h> stays out of
    // this header (it is included by most of the library).
    void *_process = nullptr;      // HANDLE
    void *_inputWrite = nullptr;   // HANDLE, our end of the input pipe
    void *_outputRead = nullptr;   // HANDLE, our end of the output pipe
    void *_console = nullptr;      // HPCON
    std::atomic<void *> _readerThreadHandle{nullptr}; // to cancel a blocked ReadFile
    std::atomic<bool> _readerDone{false};

    QThread *_reader = nullptr;
    QWinEventNotifier *_exitNotifier = nullptr;

    qint64 _pid = 0;
    int _exitCode = 0;
    QProcess::ExitStatus _exitStatus = QProcess::NormalExit;
    bool _running = false;
    bool _exitSeen = false;
    bool _finishedEmitted = false;
    bool _killed = false;

    QString _workingDirectory;
    int _windowColumns = 0;
    int _windowLines = 0;
    char _eraseChar = 0;
    bool _xonXoff = true;
    bool _utf8 = true;
};

} // namespace Konsole

#endif // PTY_WIN_H
