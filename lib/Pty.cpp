/*
 * This file is a part of QTerminal - http://gitorious.org/qterminal
 *
 * This file was un-linked from KDE and modified
 * by Maxim Bourmistrov <maxim@unixconn.com>
 *
 */

/*
    This file is part of Konsole, an X terminal.
    Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "Pty.h"

// Qt
#include <QStringList>
#include <qplatformdefs.h>

#ifndef Q_OS_WIN // Unix backend

// System
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <termios.h>
#include <csignal>

// Qt
#include <QStringList>
#include <QtDebug>

#include "kpty.h"
#include "kptydevice.h"

using namespace Konsole;

void Pty::setWindowSize(int lines, int cols)
{
  _windowColumns = cols;
  _windowLines = lines;

  if (pty()->masterFd() >= 0)
    pty()->setWinSize(lines, cols);
}
QSize Pty::windowSize() const
{
    return {_windowColumns,_windowLines};
}

void Pty::setFlowControlEnabled(bool enable)
{
  _xonXoff = enable;

  if (pty()->masterFd() >= 0)
  {
    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    if (!enable)
      ttmode.c_iflag &= ~(IXOFF | IXON);
    else
      ttmode.c_iflag |= (IXOFF | IXON);
    if (!pty()->tcSetAttr(&ttmode))
      qWarning() << "Unable to set terminal attributes.";
  }
}
bool Pty::flowControlEnabled() const
{
    if (pty()->masterFd() >= 0)
    {
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        return ttmode.c_iflag & IXOFF &&
               ttmode.c_iflag & IXON;
    }
    qWarning() << "Unable to get flow control status, terminal not connected.";
    return false;
}

void Pty::setUtf8Mode(bool enable)
{
#ifdef IUTF8 // XXX not a reasonable place to check it.
  _utf8 = enable;

  if (pty()->masterFd() >= 0)
  {
    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    if (!enable)
      ttmode.c_iflag &= ~IUTF8;
    else
      ttmode.c_iflag |= IUTF8;
    if (!pty()->tcSetAttr(&ttmode))
      qWarning() << "Unable to set terminal attributes.";
  }
#endif
}

void Pty::setErase(char erase)
{
  _eraseChar = erase;

  if (pty()->masterFd() >= 0)
  {
    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    ttmode.c_cc[VERASE] = erase;
    if (!pty()->tcSetAttr(&ttmode))
      qWarning() << "Unable to set terminal attributes.";
  }
}

char Pty::erase() const
{
    if (pty()->masterFd() >= 0)
    {
        struct ::termios ttyAttributes;
        pty()->tcGetAttr(&ttyAttributes);
        return ttyAttributes.c_cc[VERASE];
    }

    return _eraseChar;
}

void Pty::addEnvironmentVariables(const QStringList& environment)
{

    bool termEnvVarAdded = false;
    for (const QString &pair : environment)
    {
        // split on the first '=' character
        int pos = pair.indexOf(QLatin1Char('='));

        if ( pos >= 0 )
        {
            QString variable = pair.left(pos);
            QString value = pair.mid(pos+1);

            setEnv(variable,value);

            if (variable == QLatin1String("TERM")) {
                termEnvVarAdded = true;
        }
    }

    // fallback to ensure that $TERM is always set
    if (!termEnvVarAdded) {
        setEnv(QStringLiteral("TERM"), QStringLiteral("xterm-256color"));
    }
}
}

int Pty::start(const QString& program,
               const QStringList& programArguments,
               const QStringList& environment
            //    ulong winid,
            //    bool addToUtmp
               //const QString& dbusService,
               //const QString& dbusSession
               )
{
  clearProgram();

  // For historical reasons, the first argument in programArguments is the
  // name of the program to execute, so create a list consisting of all
  // but the first argument to pass to setProgram()
  Q_ASSERT(programArguments.count() >= 1);
  setProgram(program, programArguments.mid(1));

  addEnvironmentVariables(environment);

//   setEnv(QLatin1String("WINDOWID"), QString::number(winid));
  setEnv(QLatin1String("COLORTERM"), QLatin1String("truecolor"));

  // unless the LANGUAGE environment variable has been set explicitly
  // set it to a null string
  // this fixes the problem where KCatalog sets the LANGUAGE environment
  // variable during the application's startup to something which
  // differs from LANG,LC_* etc. and causes programs run from
  // the terminal to display messages in the wrong language
  //
  // this can happen if LANG contains a language which KDE
  // does not have a translation for
  //
  // BR:149300
  setEnv(QLatin1String("LANGUAGE"),QString(),false /* do not overwrite existing value if any */);

  struct ::termios ttmode;
  pty()->tcGetAttr(&ttmode);
  if (!_xonXoff)
    ttmode.c_iflag &= ~(IXOFF | IXON);
  else
    ttmode.c_iflag |= (IXOFF | IXON);
#ifdef IUTF8 // XXX not a reasonable place to check it.
  if (!_utf8)
    ttmode.c_iflag &= ~IUTF8;
  else
    ttmode.c_iflag |= IUTF8;
#endif

  if (_eraseChar != 0)
      ttmode.c_cc[VERASE] = _eraseChar;

  if (!pty()->tcSetAttr(&ttmode))
    qWarning() << "Unable to set terminal attributes.";

  pty()->setWinSize(_windowLines, _windowColumns);

  KProcess::start();

  if (!waitForStarted())
      return -1;

  return 0;
}

void Pty::setEmptyPTYProperties()
{
    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    if (!_xonXoff)
      ttmode.c_iflag &= ~(IXOFF | IXON);
    else
      ttmode.c_iflag |= (IXOFF | IXON);
  #ifdef IUTF8 // XXX not a reasonable place to check it.
    if (!_utf8)
      ttmode.c_iflag &= ~IUTF8;
    else
      ttmode.c_iflag |= IUTF8;
  #endif

    if (_eraseChar != 0)
        ttmode.c_cc[VERASE] = _eraseChar;

    if (!pty()->tcSetAttr(&ttmode))
      qWarning() << "Unable to set terminal attributes.";
}

void Pty::setWriteable(bool writeable)
{
  struct stat sbuf;
  stat(pty()->ttyName(), &sbuf);
  if (writeable)
    chmod(pty()->ttyName(), sbuf.st_mode | S_IWGRP);
  else
    chmod(pty()->ttyName(), sbuf.st_mode & ~(S_IWGRP|S_IWOTH));
}

Pty::Pty(int masterFd, QObject* parent)
    : KPtyProcess(masterFd,parent)
{
    init();
}
Pty::Pty(QObject* parent)
    : KPtyProcess(parent)
{
    init();
}
void Pty::init()
{
    // Must call parent class child process modifier, as it sets file descriptors ...etc
    auto parentChildProcModifier = KPtyProcess::childProcessModifier();
    setChildProcessModifier([parentChildProcModifier = std::move(parentChildProcModifier)]() {
        if (parentChildProcModifier) {
            parentChildProcModifier();
        }

        // reset all signal handlers
        // this ensures that terminal applications respond to
        // signals generated via key sequences such as Ctrl+C
        // (which sends SIGINT)
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_handler = SIG_DFL;
        action.sa_flags = 0;
        for (int signal = 1; signal < NSIG; signal++) {
            sigaction(signal, &action, nullptr);
        }
    });

  _windowColumns = 0;
  _windowLines = 0;
  _eraseChar = 0;
  _xonXoff = true;
  _utf8 =true;

  connect(pty(), SIGNAL(readyRead()) , this , SLOT(dataReceived()));
  setPtyChannels(KPtyProcess::AllChannels);
}

Pty::~Pty()
{
}

void Pty::sendData(const char* data, int length)
{
  if (!length)
      return;

  if (!pty()->write(data,length))
  {
    qWarning() << "Pty::doSendJobs - Could not send input data to terminal process.";
    return;
  }
}

void Pty::dataReceived()
{
    QByteArray data = pty()->readAll();
    if (data.isEmpty())
    {
        return;
    }
    emit receivedData(data.constData(),data.size());
}

int Pty::foregroundProcessGroup() const
{
    const int master_fd = pty()->masterFd();
    if (master_fd >= 0)
    {
        int pid = tcgetpgrp(master_fd);

        if (pid != -1)
        {
            return pid;
        }
    }

    return 0;
}

void Pty::closePty()
{
    pty()->close();
}

#else // Windows backend

#include "ptyqt/conptyprocess.h"

using Konsole::Pty;

Pty::Pty(QObject *aParent)
    : Pty(-1, aParent)
{
}

Pty::Pty(int masterFd, QObject *aParent)
    : QObject(aParent)
{
    Q_UNUSED(masterFd)

    m_proc = std::make_unique<ConPtyProcess>();
    if (!m_proc->isAvailable()) {
        m_proc.reset();
    }

    _windowColumns = 0;
    _windowLines = 0;
    _eraseChar = 0;
    _xonXoff = true;
    _utf8 = true;

    setErase(_eraseChar);
}

Pty::~Pty() = default;

void Pty::sendData(const char* buffer, int length)
{
    if (m_proc) {
        m_proc->write(buffer, length);
    }
}

void Pty::dataReceived()
{
    if (m_proc) {
        auto data = m_proc->readAll();
        Q_EMIT receivedData(data.constData(), data.length());
    }
}

void Pty::setWindowSize(int lines, int columns)
{
    if (m_proc && isRunning())
        m_proc->resize(columns, lines);
}

QSize Pty::windowSize() const
{
    if (!m_proc) {
        return {};
    }
    auto s = m_proc->size();
    return QSize(s.first, s.second);
}

void Pty::setFlowControlEnabled(bool enable)
{
    _xonXoff = enable;
}

bool Pty::flowControlEnabled() const
{
    return false;
}

void Pty::setUtf8Mode(bool)
{
}

void Pty::setErase(char eChar)
{
    _eraseChar = eChar;
}

char Pty::erase() const
{
    return _eraseChar;
}

void Pty::setWorkingDirectory(const QString & /*dir*/)
{
}

void Pty::addEnvironmentVariables(const QStringList & /*environmentVariables*/)
{
}

int Pty::start(const QString &program, const QStringList &arguments, const QString &workingDir, const QStringList &environment, int cols, int lines)
{
    if (!m_proc || !m_proc->isAvailable()) {
        return -1;
    }
    bool res = m_proc->startProcess(program, arguments, workingDir, environment, cols, lines);
    if (!res) {
        return -1;
    } else {
        auto n = m_proc->notifier();
        connect(n, &QIODevice::readyRead, this, &Pty::dataReceived);
        connect(m_proc.get(), &IPtyProcess::exited, this, [this] {
            Q_EMIT finished(exitCode(), QProcess::NormalExit);
        });
        connect(n, &QIODevice::aboutToClose, this, [this] {
            Q_EMIT finished(exitCode(), QProcess::NormalExit);
        });
    }
    return 0;
}

void Pty::setWriteable(bool)
{
}

void Pty::closePty()
{
    if (m_proc) {
        m_proc->kill();
    }
}

int Pty::foregroundProcessGroup() const
{
    return 0;
}


void Pty::setEmptyPTYProperties() {
  
}


#endif

void Pty::lockPty(bool lock)
{
    Q_UNUSED(lock)

// TODO: Support for locking the Pty
//   if (lock)
    // suspend();
  //else
    //resume();
}