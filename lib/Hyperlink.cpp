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

#include "Hyperlink.h"

namespace Konsole
{

HyperlinkTable HyperlinkTable::instance;

quint16 HyperlinkTable::createHyperlink(const QString& idParam, const QString& uri)
{
    if (uri.isEmpty())
        return 0;

    if (!idParam.isEmpty()) {
        const QString key = idParam + QLatin1Char('\0') + uri;
        auto it = _keyToId.constFind(key);
        if (it != _keyToId.constEnd())
            return it.value();

        // Wrap around if we somehow exhaust the id space; keep 0 as "none".
        if (_nextId == 0)
            ++_nextId;

        const quint16 id = _nextId++;
        _keyToId.insert(key, id);
        _idToUri.insert(id, uri);
        return id;
    }

    if (_nextId == 0)
        ++_nextId;

    const quint16 id = _nextId++;
    _idToUri.insert(id, uri);
    return id;
}

QString HyperlinkTable::href(quint16 id) const
{
    if (id == 0)
        return QString();
    return _idToUri.value(id);
}

}
