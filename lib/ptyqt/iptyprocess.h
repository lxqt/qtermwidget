/*
    SPDX-FileCopyrightText: 2019 Vitaly Petrov <v31337@gmail.com>
    SPDX-License-Identifier: MIT
*/
#ifndef IPTYPROCESS_H
#define IPTYPROCESS_H

#include <QDebug>
#include <QIODevice>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#define CONPTY_MINIMAL_WINDOWS_VERSION 18309

class IPtyProcess : public QObject
{
    Q_OBJECT
public:
    enum PtyType { UnixPty = 0, WinPty = 1, ConPty = 2, AutoPty = 3 };

    IPtyProcess()
        : m_pid(0)
        , m_trace(false)
    {
    }
    virtual ~IPtyProcess();

    virtual bool startProcess(const QString &shellPath,
                              const QStringList &arguments,
                              const QString &workingDirectory,
                              QStringList environment,
                              qint16 cols,
                              qint16 rows) = 0;
    virtual bool resize(qint16 cols, qint16 rows) = 0;
    virtual bool kill() = 0;
    virtual PtyType type() = 0;
    virtual QString dumpDebugInfo() = 0;
    virtual QIODevice *notifier() = 0;
    virtual QByteArray readAll() = 0;
    virtual qint64 write(const char *data, int size) = 0;
    virtual bool isAvailable() = 0;
    virtual void moveToThread(QThread *targetThread) = 0;
    virtual int processList() const = 0; // 0 - unsupported, 1 - no process , 2 - run process
    qint64 pid()
    {
        return m_pid;
    }
    QPair<qint16, qint16> size()
    {
        return m_size;
    }
    const QString lastError()
    {
        return m_lastError;
    }
    bool toggleTrace()
    {
        m_trace = !m_trace;
        return m_trace;
    }
    int exitCode() const
    {
        return m_exitCode;
    }
Q_SIGNALS:
    void started();
    void exited();

protected:
    QString m_shellPath;
    QString m_lastError;
    qint64 m_pid;
    QPair<qint16, qint16> m_size; // cols / rows
    bool m_trace;
    int m_exitCode = -1;
};

#endif // IPTYPROCESS_H
