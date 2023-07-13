/*
 * This file is a part of QTerminal - http://gitorious.org/qterminal
 *
 * This file was un-linked from KDE and modified
 * by Maxim Bourmistrov <maxim@unixconn.com>
 *
 */

/* This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

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

#ifndef kptydev_h
#define kptydev_h

#include "kpty_p.h"

#include <QIODevice>

#define KMAXINT ((int)(~0U >> 1))

class KPtyDevicePrivate;
class QSocketNotifier;

/**
 * Encapsulates KPty into a QIODevice, so it can be used with Q*Stream, etc.
 */
class KPtyDevice : public QIODevice, public KPty {
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(KPty::d_ptr, KPtyDevice)

public:

    /**
     * Constructor
     */
    KPtyDevice(QObject *parent = nullptr);

    /**
     * Destructor:
     *
     *  If the pty is still open, it will be closed. Note, however, that
     *  an utmp registration is @em not undone.
     */
    ~KPtyDevice() override;

    /**
     * Create a pty master/slave pair.
     *
     * @return true if a pty pair was successfully opened
     */
    bool open(OpenMode mode = ReadWrite | Unbuffered) override;

    /**
     * Open using an existing pty master. The ownership of the fd
     * remains with the caller, i.e., close() will not close the fd.
     *
     * This is useful if you wish to attach a secondary "controller" to an
     * existing pty device such as a terminal widget.
     * Note that you will need to use setSuspended() on both devices to
     * control which one gets the incoming data from the pty.
     *
     * @param fd an open pty master file descriptor.
     * @param mode the device mode to open the pty with.
     * @return true if a pty pair was successfully opened
     */
    bool open(int fd, OpenMode mode = ReadWrite | Unbuffered);

    /**
     * Close the pty master/slave pair.
     */
    void close() override;

    /**
     * Sets whether the KPtyDevice monitors the pty for incoming data.
     *
     * When the KPtyDevice is suspended, it will no longer attempt to buffer
     * data that becomes available from the pty and it will not emit any
     * signals.
     *
     * Do not use on closed ptys.
     * After a call to open(), the pty is not suspended. If you need to
     * ensure that no data is read, call this function before the main loop
     * is entered again (i.e., immediately after opening the pty).
     */
    void setSuspended(bool suspended);

    /**
     * Returns true if the KPtyDevice is not monitoring the pty for incoming
     * data.
     *
     * Do not use on closed ptys.
     *
     * See setSuspended()
     */
    bool isSuspended() const;

    /**
     * @return always true
     */
    bool isSequential() const override;

    /**
     * @reimp
     */
    bool canReadLine() const override;

    /**
     * @reimp
     */
    bool atEnd() const override;

    /**
     * @reimp
     */
    qint64 bytesAvailable() const override;

    /**
     * @reimp
     */
    qint64 bytesToWrite() const override;

    bool waitForBytesWritten(int msecs = -1) override;
    bool waitForReadyRead(int msecs = -1) override;


Q_SIGNALS:
    /**
     * Emitted when EOF is read from the PTY.
     *
     * Data may still remain in the buffers.
     */
    void readEof();

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 readLineData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private:
    Q_PRIVATE_SLOT(d_func(), bool _k_canRead())
    Q_PRIVATE_SLOT(d_func(), bool _k_canWrite())
};

/////////////////////////////////////////////////////
// Helper. Remove when QRingBuffer becomes public. //
/////////////////////////////////////////////////////

#include <QByteArray>
#include <list>

#define CHUNKSIZE 4096

class KRingBuffer
{
public:
    KRingBuffer()
    {
        clear();
    }

    void clear()
    {
        buffers.clear();
        QByteArray tmp;
        tmp.resize(CHUNKSIZE);
        buffers.push_back(tmp);
        head = tail = 0;
        totalSize = 0;
    }

    inline bool isEmpty() const
    {
        return buffers.size() == 1 && !tail;
    }

    inline int size() const
    {
        return totalSize;
    }

    inline int readSize() const
    {
        return (buffers.size() == 1 ? tail : buffers.front().size()) - head;
    }

    inline const char *readPointer() const
    {
        Q_ASSERT(totalSize > 0);
        return buffers.front().constData() + head;
    }

    void free(int bytes)
    {
        totalSize -= bytes;
        Q_ASSERT(totalSize >= 0);

        forever {
            int nbs = readSize();

            if (bytes < nbs) {
                head += bytes;
                if (head == tail && buffers.size() == 1) {
                    buffers.front().resize(CHUNKSIZE);
                    head = tail = 0;
                }
                break;
            }

            bytes -= nbs;
            if (buffers.size() == 1) {
                buffers.front().resize(CHUNKSIZE);
                head = tail = 0;
                break;
            }

            buffers.pop_front();
            head = 0;
        }
    }

    char *reserve(int bytes)
    {
        totalSize += bytes;

        char *ptr;
        if (tail + bytes <= buffers.back().size()) {
            ptr = buffers.back().data() + tail;
            tail += bytes;
        } else {
            buffers.back().resize(tail);
            QByteArray tmp;
            tmp.resize(qMax(CHUNKSIZE, bytes));
            ptr = tmp.data();
            buffers.push_back(tmp);
            tail = bytes;
        }
        return ptr;
    }

    // release a trailing part of the last reservation
    inline void unreserve(int bytes)
    {
        totalSize -= bytes;
        tail -= bytes;
    }

    inline void write(const char *data, int len)
    {
        memcpy(reserve(len), data, len);
    }

    // Find the first occurrence of c and return the index after it.
    // If c is not found until maxLength, maxLength is returned, provided
    // it is smaller than the buffer size. Otherwise -1 is returned.
    int indexAfter(char c, int maxLength = KMAXINT) const
    {
        int index = 0;
        int start = head;
        std::list<QByteArray>::const_iterator it = buffers.cbegin();
        forever {
            if (!maxLength)
                return index;
            if (index == size())
                return -1;
            const QByteArray &buf = *it;
            ++it;
            int len = qMin((it == buffers.cend() ? tail : buf.size()) - start,
                           maxLength);
            const char *ptr = buf.data() + start;
            if (const char *rptr = (const char *)memchr(ptr, c, len))
                return index + (rptr - ptr) + 1;
            index += len;
            maxLength -= len;
            start = 0;
        }
    }

    inline int lineSize(int maxLength = KMAXINT) const
    {
        return indexAfter('\n', maxLength);
    }

    inline bool canReadLine() const
    {
        return lineSize() != -1;
    }

    int read(char *data, int maxLength)
    {
        int bytesToRead = qMin(size(), maxLength);
        int readSoFar = 0;
        while (readSoFar < bytesToRead) {
            const char *ptr = readPointer();
            int bs = qMin(bytesToRead - readSoFar, readSize());
            memcpy(data + readSoFar, ptr, bs);
            readSoFar += bs;
            free(bs);
        }
        return readSoFar;
    }

    int readLine(char *data, int maxLength)
    {
        return read(data, lineSize(qMin(maxLength, size())));
    }

private:
    std::list<QByteArray> buffers;
    int head, tail;
    int totalSize;
};

class KPtyDevicePrivate : public KPtyPrivate {

    Q_DECLARE_PUBLIC(KPtyDevice)

public:
    KPtyDevicePrivate(KPty* parent) :
        KPtyPrivate(parent),
        emittedReadyRead(false), emittedBytesWritten(false),
        readNotifier(nullptr), writeNotifier(nullptr)
    {
    }

    bool _k_canRead();
    bool _k_canWrite();

    bool doWait(int msecs, bool reading);
    void finishOpen(QIODevice::OpenMode mode);

    bool emittedReadyRead;
    bool emittedBytesWritten;
    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;
    KRingBuffer readBuffer;
    KRingBuffer writeBuffer;
};

#endif

