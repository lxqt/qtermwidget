/*
    Copyright 2007-2008 by Robert Knight <robertknight@gmail.com>

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

#ifndef FILTER_P_H
#define FILTER_P_H

#include "Filter.h"

namespace Konsole {

class QTERMWIDGET_NO_EXPORT FilterObject : public QObject
{
    Q_OBJECT
public:
    FilterObject(Filter::HotSpot* filter) : _filter(filter) {}

    void emitActivated(const QUrl& url, bool fromContextMenu);
public slots:
    void activate();
private:
    Filter::HotSpot* _filter;
signals:
    void activated(const QUrl& url, bool fromContextMenu);
};

/** A filter chain which processes character images from terminal displays */
class QTERMWIDGET_NO_EXPORT TerminalImageFilterChain : public FilterChain
{
public:
    TerminalImageFilterChain();
    ~TerminalImageFilterChain() override;

    /**
     * Set the current terminal image to @p image.
     *
     * @param image The terminal image
     * @param lines The number of lines in the terminal image
     * @param columns The number of columns in the terminal image
     * @param lineProperties The line properties to set for image
     */
    void setImage(const Character* const image , int lines , int columns,
                  const QVector<LineProperty>& lineProperties);

private:
    QString* _buffer;
    QList<int>* _linePositions;
};

}

#endif // FILTER_P_H
