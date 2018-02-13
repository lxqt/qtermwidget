/* $XFree86: xc/programs/xterm/wcwidth.character,v 1.3 2001/07/29 22:08:16 tsi Exp $ */
/*
 * This is an implementation of wcwidth() and wcswidth() as defined in
 * "The Single UNIX Specification, Version 2, The Open Group, 1997"
 * <http://www.UNIX-systems.org/online.html>
 *
 * Markus Kuhn -- 2001-01-12 -- public domain
 */

#include <QString>

#include <cwchar>

#include <unicode/uchar.h>

#include "konsole_wcwidth.h"

static_assert(sizeof(wchar_t) == sizeof(UChar32),
              "wchar_t and UChar32 have different size");

int konsole_wcwidth(wchar_t ucs)
{
    switch (u_getIntPropertyValue(ucs, UCHAR_EAST_ASIAN_WIDTH)) {
        case U_EA_AMBIGUOUS: /*[A]*/
            return 2;
        case U_EA_NEUTRAL:   /*[N]*/
        case U_EA_HALFWIDTH: /*[H]*/
        case U_EA_FULLWIDTH: /*[F]*/
        case U_EA_NARROW:    /*[Na]*/
        case U_EA_WIDE:      /*[W]*/
            break;
    }

    return wcwidth( ucs );
}

// single byte char: +1, multi byte char: +2
int string_width( const std::wstring & wstr )
{
    int w = 0;
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        w += konsole_wcwidth( wstr[ i ] );
    }
    return w;
}
