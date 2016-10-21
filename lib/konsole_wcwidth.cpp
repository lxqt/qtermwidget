/* $XFree86: xc/programs/xterm/wcwidth.character,v 1.3 2001/07/29 22:08:16 tsi Exp $ */
/*
 * This is an implementation of wcwidth() and wcswidth() as defined in
 * "The Single UNIX Specification, Version 2, The Open Group, 1997"
 * <http://www.UNIX-systems.org/online.html>
 *
 * Markus Kuhn -- 2001-01-12 -- public domain
 */

#include <QString>

#include "konsole_wcwidth.h"
#include "wcwidth.h"

int konsole_wcwidth(quint16 ucs)
{
    return termux_wcwidth( ucs );
}

// single byte char: +1, multi byte char: +2
int string_width( const QString & txt )
{
    int w = 0;
    std::wstring wstr = txt.toStdWString();
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        w += wcwidth( wstr[ i ] );
    }
    return w;
}
