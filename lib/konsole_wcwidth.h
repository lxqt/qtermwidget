/* $XFree86: xc/programs/xterm/wcwidth.h,v 1.2 2001/06/18 19:09:27 dickey Exp $ */

/* Markus Kuhn -- 2001-01-12 -- public domain */
/* Adaptations for KDE by Waldo Bastian <bastian@kde.org> */
/*
    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>
*/


#ifndef _KONSOLE_WCWIDTH_H_
#define _KONSOLE_WCWIDTH_H_

// Standard
#include <string>

#include <QtGlobal>

#ifdef Q_OS_WIN
// wchar_t is 16 bit on Windows: take a full code point instead.
int konsole_wcwidth(char32_t ucs);
#else
int konsole_wcwidth(wchar_t ucs);
#endif

int string_width( const std::wstring & wstr );

#endif
