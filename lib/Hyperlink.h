/*
    Copyright (C) 2026 The LXQt Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see
    <https://www.gnu.org/licenses/>.
*/

#ifndef HYPERLINK_H
#define HYPERLINK_H

// Qt
#include <QHash>
#include <QString>

namespace Konsole
{

/**
 * Maps per-cell hyperlink identifiers to URI strings for OSC-8 support.
 *
 * Character cells store a small id; the URI lives in this table so screen and
 * history buffers stay compact. Id 0 means "no hyperlink".
 */
class HyperlinkTable
{
public:
    HyperlinkTable() = default;

    /**
     * Allocates or reuses an id for the given OSC-8 params and URI.
     *
     * @param idParam Optional id= value from OSC-8 params (may be empty)
     * @param uri The hyperlink target URI
     * @return Non-zero id, or 0 if @p uri is empty
     */
    quint16 createHyperlink(const QString& idParam, const QString& uri);

    /** Returns the URI for @p id, or an empty string if unknown / id is 0. */
    QString href(quint16 id) const;

    /** Global table shared by all screens (same pattern as ExtendedCharTable). */
    static HyperlinkTable instance;

private:
    QHash<quint16, QString> _idToUri;
    // Key is "idParam\\0uri" when idParam is non-empty, allowing reuse of the
    // same OSC-8 id across screen updates. Empty idParam always allocates a
    // fresh id (VTE-style).
    QHash<QString, quint16> _keyToId;
    quint16 _nextId = 1;
};

}

#endif // HYPERLINK_H
