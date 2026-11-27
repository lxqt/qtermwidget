/*
    Windows port of Konsole::Pty for QTermWidget, based on ConPTY.

    Copyright (C) 2026 Fabrizio Pistonesi <fpistonesi@webeasytech.com>

    Data flow:

        TerminalDisplay/Emulation --sendData()--> input pipe  --> conhost (pseudo console) --> child
        TerminalDisplay/Emulation <--receivedData()-- reader thread <-- output pipe <-- conhost

    conhost translates between the console API used by the child and a
    VT/UTF-8 byte stream, which is exactly what the emulation expects from a
    Unix pty.

    Shutdown order matters with ConPTY: the output pipe is only broken when
    conhost exits, i.e. after ClosePseudoConsole(). On older Windows builds
    ClosePseudoConsole() blocks until the output pipe is drained, so it is
    called from a helper thread while the reader thread keeps draining.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "Pty_win.h"

#include <windows.h>

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QWinEventNotifier>
#include <QtDebug>

#include <string>
#include <vector>

using namespace Konsole;

namespace {

#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE 0x00020016
#endif

// The ConPTY entry points are resolved at runtime: this keeps the library
// loadable (with a clear error) on Windows versions older than 10 1809 and
// does not depend on how recent the toolchain's SDK headers are.
using HPCONHandle = void *;
using CreatePseudoConsoleFn = HRESULT(WINAPI *)(COORD, HANDLE, HANDLE, DWORD, HPCONHandle *);
using ResizePseudoConsoleFn = HRESULT(WINAPI *)(HPCONHandle, COORD);
using ClosePseudoConsoleFn = void(WINAPI *)(HPCONHandle);

struct ConPtyApi
{
    CreatePseudoConsoleFn create = nullptr;
    ResizePseudoConsoleFn resize = nullptr;
    ClosePseudoConsoleFn close = nullptr;

    bool available() const { return create && resize && close; }
};

template <typename Fn>
Fn resolve(HMODULE module, const char *name)
{
    return reinterpret_cast<Fn>(reinterpret_cast<void *>(GetProcAddress(module, name)));
}

const ConPtyApi &conPty()
{
    static const ConPtyApi api = [] {
        ConPtyApi a;
        if (HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll")) {
            a.create = resolve<CreatePseudoConsoleFn>(kernel32, "CreatePseudoConsole");
            a.resize = resolve<ResizePseudoConsoleFn>(kernel32, "ResizePseudoConsole");
            a.close = resolve<ClosePseudoConsoleFn>(kernel32, "ClosePseudoConsole");
        }
        return a;
    }();
    return api;
}

COORD consoleSize(int columns, int lines)
{
    // CreatePseudoConsole rejects a 0x0 size; the real size arrives with the
    // first resize of the view.
    COORD size;
    size.X = static_cast<SHORT>(qBound(1, columns > 0 ? columns : 80, 0x7fff));
    size.Y = static_cast<SHORT>(qBound(1, lines > 0 ? lines : 24, 0x7fff));
    return size;
}

// Quotes one argument following the rules of CommandLineToArgvW / the MSVC
// runtime, so that the child receives exactly the original argv.
QString quoteArgument(const QString &arg)
{
    if (!arg.isEmpty()
        && !arg.contains(QLatin1Char(' ')) && !arg.contains(QLatin1Char('\t'))
        && !arg.contains(QLatin1Char('\n')) && !arg.contains(QLatin1Char('\v'))
        && !arg.contains(QLatin1Char('"'))) {
        return arg;
    }

    QString quoted = QStringLiteral("\"");
    int backslashes = 0;
    for (const QChar c : arg) {
        if (c == QLatin1Char('\\')) {
            ++backslashes;
            continue;
        }
        if (c == QLatin1Char('"')) {
            // escape the pending backslashes and the quote itself
            quoted += QString(backslashes * 2 + 1, QLatin1Char('\\'));
        } else {
            quoted += QString(backslashes, QLatin1Char('\\'));
        }
        quoted += c;
        backslashes = 0;
    }
    // backslashes before the closing quote must be doubled
    quoted += QString(backslashes * 2, QLatin1Char('\\'));
    quoted += QLatin1Char('"');
    return quoted;
}

// Resolves the program to an absolute path. Searching PATH ourselves (instead
// of letting CreateProcess do it) avoids picking up executables from the
// current directory.
QString resolveProgram(const QString &program)
{
    if (program.isEmpty())
        return QString();

    const QFileInfo info(program);
    if (info.isAbsolute() || program.contains(QLatin1Char('/')) || program.contains(QLatin1Char('\\'))) {
        if (info.isFile())
            return QDir::toNativeSeparators(info.absoluteFilePath());
        const QFileInfo withExe(program + QLatin1String(".exe"));
        if (withExe.isFile())
            return QDir::toNativeSeparators(withExe.absoluteFilePath());
        return QString();
    }

    // honours PATHEXT (.exe, .cmd, .bat, ...)
    return QDir::toNativeSeparators(QStandardPaths::findExecutable(program));
}

std::wstring buildEnvironmentBlock(const QStringList &extra)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    bool termSet = false;
    for (const QString &pair : extra) {
        const int pos = pair.indexOf(QLatin1Char('='));
        if (pos <= 0)
            continue;
        const QString name = pair.left(pos);
        env.insert(name, pair.mid(pos + 1));
        if (name.compare(QLatin1String("TERM"), Qt::CaseInsensitive) == 0)
            termSet = true;
    }
    if (!termSet)
        env.insert(QStringLiteral("TERM"), QStringLiteral("xterm-256color"));
    env.insert(QStringLiteral("COLORTERM"), QStringLiteral("truecolor"));

    // "NAME=value\0NAME=value\0\0", sorted by name (QProcessEnvironment
    // keeps the keys ordered case-insensitively on Windows).
    std::wstring block;
    const QStringList entries = env.toStringList();
    for (const QString &entry : entries) {
        block += entry.toStdWString();
        block.push_back(L'\0');
    }
    block.push_back(L'\0');
    return block;
}

void closeHandle(void *&handle)
{
    if (handle && handle != INVALID_HANDLE_VALUE)
        CloseHandle(static_cast<HANDLE>(handle));
    handle = nullptr;
}

} // namespace

Pty::Pty(QObject *parent)
    : QObject(parent)
{
}

Pty::~Pty()
{
    // No signals from a half-destroyed object.
    blockSignals(true);

    if (_process && !_exitSeen) {
        _killed = true;
        TerminateProcess(static_cast<HANDLE>(_process), 1);
        WaitForSingleObject(static_cast<HANDLE>(_process), 1000);
        _exitSeen = true;
    }
    releaseHandles();
}

int Pty::start(const QString &program,
               const QStringList &arguments,
               const QStringList &environment,
               ulong winid,
               bool addToUtmp)
{
    Q_UNUSED(winid)
    Q_UNUSED(addToUtmp) // no utmp on Windows

    if (_running) {
        qWarning() << "Pty::start: a process is already running";
        return -1;
    }

    _exitSeen = false;
    _finishedEmitted = false;
    _killed = false;
    _readerDone = false;
    _exitCode = 0;
    _exitStatus = QProcess::NormalExit;

    auto fail = [this](const char *what) {
        qWarning() << "Pty::start:" << what << "- error" << GetLastError();
        emit errorOccurred(QProcess::FailedToStart);
        return -1;
    };

    if (!conPty().available())
        return fail("ConPTY is not available (Windows 10 1809 or later is required)");

    const QString executable = resolveProgram(program);
    if (executable.isEmpty()) {
        qWarning() << "Pty::start: program not found:" << program;
        emit errorOccurred(QProcess::FailedToStart);
        return -1;
    }

    // Pipes: input (we write -> conhost reads) and output (conhost writes -> we read).
    HANDLE inputRead = nullptr, inputWrite = nullptr;
    HANDLE outputRead = nullptr, outputWrite = nullptr;
    if (!CreatePipe(&inputRead, &inputWrite, nullptr, 64 * 1024))
        return fail("CreatePipe (input) failed");
    if (!CreatePipe(&outputRead, &outputWrite, nullptr, 64 * 1024)) {
        CloseHandle(inputRead);
        CloseHandle(inputWrite);
        return fail("CreatePipe (output) failed");
    }

    HPCONHandle console = nullptr;
    const HRESULT hr = conPty().create(consoleSize(_windowColumns, _windowLines),
                                       inputRead, outputWrite, 0, &console);
    // conhost owns duplicates of these ends now.
    CloseHandle(inputRead);
    CloseHandle(outputWrite);
    if (FAILED(hr)) {
        CloseHandle(inputWrite);
        CloseHandle(outputRead);
        qWarning() << "Pty::start: CreatePseudoConsole failed, HRESULT" << Qt::hex << hr;
        emit errorOccurred(QProcess::FailedToStart);
        return -1;
    }

    // Attribute list that attaches the child to the pseudo console.
    SIZE_T attrSize = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &attrSize);
    std::vector<char> attrBuffer(attrSize);
    auto attrs = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrBuffer.data());
    const bool attrsOk =
        InitializeProcThreadAttributeList(attrs, 1, 0, &attrSize)
        && UpdateProcThreadAttribute(attrs, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                     console, sizeof(console), nullptr, nullptr);

    STARTUPINFOEXW si;
    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(STARTUPINFOEXW);
    // Do not let the child inherit our own (possibly redirected) std handles:
    // its console I/O must go through the pseudo console.
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    si.StartupInfo.hStdInput = INVALID_HANDLE_VALUE;
    si.StartupInfo.hStdOutput = INVALID_HANDLE_VALUE;
    si.StartupInfo.hStdError = INVALID_HANDLE_VALUE;
    si.lpAttributeList = attrs;

    // arguments[0] is the program name (argv[0] convention of Session).
    QString commandLine = quoteArgument(executable);
    for (int i = 1; i < arguments.size(); ++i)
        commandLine += QLatin1Char(' ') + quoteArgument(arguments.at(i));
    std::wstring cmd = commandLine.toStdWString();

    std::wstring environmentBlock = buildEnvironmentBlock(environment);

    std::wstring workDir;
    if (!_workingDirectory.isEmpty() && QFileInfo(_workingDirectory).isDir())
        workDir = QDir::toNativeSeparators(_workingDirectory).toStdWString();

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    const BOOL created = attrsOk && CreateProcessW(
        nullptr,                    // resolved path is the first token of cmd
        cmd.data(),
        nullptr, nullptr,
        FALSE,                      // no handle inheritance
        EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT,
        environmentBlock.data(),
        workDir.empty() ? nullptr : workDir.c_str(),
        &si.StartupInfo,
        &pi);
    const DWORD createError = GetLastError();

    if (attrsOk)
        DeleteProcThreadAttributeList(attrs);

    if (!created) {
        // Close our pipe ends first: ClosePseudoConsole may otherwise wait
        // for the output to be drained.
        CloseHandle(outputRead);
        CloseHandle(inputWrite);
        conPty().close(console);
        qWarning() << "Pty::start: CreateProcess failed for" << commandLine << "- error" << createError;
        emit errorOccurred(QProcess::FailedToStart);
        return -1;
    }

    CloseHandle(pi.hThread);
    _process = pi.hProcess;
    _pid = static_cast<qint64>(pi.dwProcessId);
    _console = console;
    _inputWrite = inputWrite;
    _outputRead = outputRead;
    _running = true;

    // Reader thread: blocking ReadFile on the output pipe, data is handed to
    // the GUI thread with queued calls (so ordering is preserved).
    _reader = QThread::create([this, out = outputRead]() {
        HANDLE self = nullptr;
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
                        &self, 0, FALSE, DUPLICATE_SAME_ACCESS);
        _readerThreadHandle.store(self);

        char buffer[16 * 1024];
        for (;;) {
            DWORD read = 0;
            if (!ReadFile(out, buffer, sizeof(buffer), &read, nullptr))
                break; // ERROR_BROKEN_PIPE once conhost is gone, or cancelled
            if (read == 0)
                continue;
            QByteArray chunk(buffer, static_cast<int>(read));
            QMetaObject::invokeMethod(this, [this, chunk]() {
                emit receivedData(chunk.constData(), static_cast<int>(chunk.size()));
            }, Qt::QueuedConnection);
        }

        _readerDone = true;
        QMetaObject::invokeMethod(this, [this]() { finishIfDone(); }, Qt::QueuedConnection);
    });
    _reader->setObjectName(QStringLiteral("QTermWidget ConPTY reader"));

    _exitNotifier = new QWinEventNotifier(static_cast<HANDLE>(_process), this);
    connect(_exitNotifier, &QWinEventNotifier::activated, this, [this]() { handleProcessExit(); });

    _reader->start();
    return 0;
}

void Pty::handleProcessExit()
{
    if (_exitSeen || !_process)
        return;
    _exitSeen = true;
    if (_exitNotifier)
        _exitNotifier->setEnabled(false);

    DWORD code = 0;
    GetExitCodeProcess(static_cast<HANDLE>(_process), &code);
    _exitCode = static_cast<int>(code);
    // NTSTATUS error codes (access violation, ...) mean the process crashed.
    _exitStatus = (_killed || code >= 0xC0000000u) ? QProcess::CrashExit : QProcess::NormalExit;

    // conhost stays alive until the pseudo console is closed; closing it
    // breaks the output pipe and lets the reader finish after draining.
    closeConsoleAsync();

    // Safety net in case the reader does not terminate by itself.
    QTimer::singleShot(3000, this, [this]() {
        if (!_finishedEmitted && stopReader(1000)) {
            _readerDone = true;
            finishIfDone();
        }
    });

    finishIfDone();
}

void Pty::finishIfDone()
{
    if (_finishedEmitted || !_exitSeen || !_readerDone)
        return;

    _finishedEmitted = true;
    _running = false;
    releaseHandles();
    emit finished(_exitCode, _exitStatus);
}

void Pty::closeConsoleAsync()
{
    // Input is meaningless once the console is closing.
    closeHandle(_inputWrite);

    if (!_console)
        return;
    HPCONHandle console = _console;
    _console = nullptr;

    // ClosePseudoConsole can block (pre Windows 11 24H2) until the output
    // pipe is drained or the attached processes have handled
    // CTRL_CLOSE_EVENT: keep it off the GUI thread.
    QThread *closer = QThread::create([console]() { conPty().close(console); });
    closer->setObjectName(QStringLiteral("QTermWidget ConPTY close"));
    QObject::connect(closer, &QThread::finished, closer, &QObject::deleteLater);
    closer->start();
}

bool Pty::stopReader(int msecs)
{
    if (!_reader)
        return true;
    if (_reader->wait(msecs))
        return true;

    // Still blocked in ReadFile: cancel it.
    for (int attempt = 0; attempt < 40; ++attempt) {
        if (HANDLE thread = static_cast<HANDLE>(_readerThreadHandle.load()))
            CancelSynchronousIo(thread);
        if (_reader->wait(50))
            return true;
    }
    return false;
}

void Pty::releaseHandles()
{
    if (_exitNotifier) {
        _exitNotifier->setEnabled(false);
        _exitNotifier->deleteLater(); // we may be inside its activated() signal
        _exitNotifier = nullptr;
    }

    closeConsoleAsync();

    if (_reader) {
        if (!stopReader(2000)) {
            // The reader references this object: it must not outlive it.
            qWarning() << "Pty: waiting for the ConPTY reader thread to terminate";
            _reader->wait();
        }
        delete _reader;
        _reader = nullptr;
    }

    void *readerThread = _readerThreadHandle.exchange(nullptr);
    closeHandle(readerThread);
    closeHandle(_outputRead);
    closeHandle(_inputWrite);
    closeHandle(_process);
}

void Pty::closePty()
{
    closeConsoleAsync();
}

void Pty::kill()
{
    if (_process && !_exitSeen) {
        _killed = true;
        TerminateProcess(static_cast<HANDLE>(_process), 1);
    }
}

QProcess::ProcessState Pty::state() const
{
    return (_running && !_exitSeen) ? QProcess::Running : QProcess::NotRunning;
}

qint64 Pty::processId() const
{
    return _running ? _pid : 0;
}

bool Pty::waitForFinished(int msecs)
{
    if (!_running || !_process)
        return false;

    if (!_exitSeen) {
        const DWORD timeout = msecs < 0 ? INFINITE : static_cast<DWORD>(msecs);
        if (WaitForSingleObject(static_cast<HANDLE>(_process), timeout) != WAIT_OBJECT_0)
            return false;
        handleProcessExit();
    }

    if (!_finishedEmitted) {
        stopReader(2000);
        _readerDone = true;
        finishIfDone();
    }
    return true;
}

QProcess::ExitStatus Pty::exitStatus() const
{
    return _exitStatus;
}

int Pty::exitCode() const
{
    return _exitCode;
}

void Pty::sendData(const char *data, int length)
{
    if (!_inputWrite || length <= 0)
        return;

    const char *p = data;
    DWORD left = static_cast<DWORD>(length);
    while (left > 0) {
        DWORD written = 0;
        if (!WriteFile(static_cast<HANDLE>(_inputWrite), p, left, &written, nullptr)) {
            qWarning() << "Pty::sendData - could not send input data to terminal process, error" << GetLastError();
            return;
        }
        p += written;
        left -= written;
    }
}

void Pty::setWindowSize(int lines, int cols)
{
    _windowColumns = cols;
    _windowLines = lines;

    if (_console && lines > 0 && cols > 0)
        conPty().resize(_console, consoleSize(cols, lines));
}

QSize Pty::windowSize() const
{
    return {_windowColumns, _windowLines};
}

void Pty::setInitialWorkingDirectory(const QString &dir)
{
    _workingDirectory = dir;
}

int Pty::foregroundProcessGroup() const
{
    return static_cast<int>(processId());
}

void Pty::setFlowControlEnabled(bool on)
{
    _xonXoff = on;
}

bool Pty::flowControlEnabled() const
{
    return _xonXoff;
}

void Pty::setErase(char erase)
{
    _eraseChar = erase;
}

char Pty::erase() const
{
    return _eraseChar;
}

void Pty::setUtf8Mode(bool on)
{
    // ConPTY always speaks UTF-8.
    _utf8 = on;
}

void Pty::lockPty(bool lock)
{
    Q_UNUSED(lock)
}

void Pty::setEmptyPTYProperties()
{
}

void Pty::setWriteable(bool writeable)
{
    Q_UNUSED(writeable)
}
