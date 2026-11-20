/* $XFree86: xc/programs/xterm/wcwidth.character,v 1.3 2001/07/29 22:08:16 tsi Exp $ */
/*
 * This is an implementation of wcwidth() and wcswidth() as defined in
 * "The Single UNIX Specification, Version 2, The Open Group, 1997"
 * <http://www.UNIX-systems.org/online.html>
 *
 * Markus Kuhn -- 2001-01-12 -- public domain
 */

#include <QString>

#ifdef HAVE_UTF8PROC
#include <utf8proc.h>
#else
#include <cwchar>
#endif

#include "konsole_wcwidth.h"

#if defined(Q_OS_WIN) && !defined(HAVE_UTF8PROC)
// The Windows C runtime has no wcwidth(). Same rules as Markus Kuhn's
// mk_wcwidth(), using Qt's Unicode tables for the zero width classes.
static bool isWide(char32_t ucs)
{
    return (ucs >= 0x1100 && ucs <= 0x115f)            // Hangul Jamo init. consonants
        || ucs == 0x2329 || ucs == 0x232a
        || (ucs >= 0x231a && ucs <= 0x231b)            // watch, hourglass
        || (ucs >= 0x23e9 && ucs <= 0x23ec) || ucs == 0x23f0 || ucs == 0x23f3
        || (ucs >= 0x25fd && ucs <= 0x25fe)
        || (ucs >= 0x2614 && ucs <= 0x2615)
        || (ucs >= 0x2648 && ucs <= 0x2653)
        || ucs == 0x267f || ucs == 0x2693 || ucs == 0x26a1
        || (ucs >= 0x26aa && ucs <= 0x26ab)
        || (ucs >= 0x26bd && ucs <= 0x26be)
        || (ucs >= 0x26c4 && ucs <= 0x26c5)
        || ucs == 0x26ce || ucs == 0x26d4 || ucs == 0x26ea
        || (ucs >= 0x26f2 && ucs <= 0x26f3) || ucs == 0x26f5 || ucs == 0x26fa || ucs == 0x26fd
        || ucs == 0x2705 || (ucs >= 0x270a && ucs <= 0x270b) || ucs == 0x2728 || ucs == 0x274c
        || ucs == 0x274e || (ucs >= 0x2753 && ucs <= 0x2755) || ucs == 0x2757
        || (ucs >= 0x2795 && ucs <= 0x2797) || ucs == 0x27b0 || ucs == 0x27bf
        || (ucs >= 0x2b1b && ucs <= 0x2b1c) || ucs == 0x2b50 || ucs == 0x2b55
        || (ucs >= 0x2e80 && ucs <= 0xa4cf && ucs != 0x303f) // CJK ... Yi
        || (ucs >= 0xac00 && ucs <= 0xd7a3)            // Hangul Syllables
        || (ucs >= 0xf900 && ucs <= 0xfaff)            // CJK Compatibility Ideographs
        || (ucs >= 0xfe10 && ucs <= 0xfe19)            // Vertical forms
        || (ucs >= 0xfe30 && ucs <= 0xfe6f)            // CJK Compatibility Forms
        || (ucs >= 0xff00 && ucs <= 0xff60)            // Fullwidth Forms
        || (ucs >= 0xffe0 && ucs <= 0xffe6)
        || (ucs >= 0x16fe0 && ucs <= 0x18cff)          // Tangut, Khitan
        || (ucs >= 0x1b000 && ucs <= 0x1b2ff)          // Kana supplement
        || ucs == 0x1f004 || ucs == 0x1f0cf || ucs == 0x1f18e
        || (ucs >= 0x1f191 && ucs <= 0x1f19a)
        || (ucs >= 0x1f200 && ucs <= 0x1f251)
        || (ucs >= 0x1f300 && ucs <= 0x1f64f)          // pictographs, emoticons
        || (ucs >= 0x1f680 && ucs <= 0x1f6ff)          // transport and map
        || (ucs >= 0x1f7e0 && ucs <= 0x1f7eb)
        || (ucs >= 0x1f900 && ucs <= 0x1f9ff)          // supplemental symbols
        || (ucs >= 0x1fa70 && ucs <= 0x1faff)
        || (ucs >= 0x20000 && ucs <= 0x2fffd)          // CJK Extension B..F
        || (ucs >= 0x30000 && ucs <= 0x3fffd);
}

static int windows_wcwidth(char32_t ucs)
{
    if (ucs == 0)
        return 0;
    if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
        return -1;
    if (ucs > 0x10ffff)
        return 1;

    const QChar::Category cat = QChar::category(ucs);
    if (cat == QChar::Mark_NonSpacing || cat == QChar::Mark_Enclosing
        || (cat == QChar::Other_Format && ucs != 0x00ad)
        || (ucs >= 0x1160 && ucs <= 0x11ff)            // Hangul medial vowels / final consonants
        || ucs == 0x200b)
        return 0;

    return isWide(ucs) ? 2 : 1;
}
#endif

#ifdef Q_OS_WIN
int konsole_wcwidth(char32_t ucs)
#else
int konsole_wcwidth(wchar_t ucs)
#endif
{
#ifdef HAVE_UTF8PROC
    utf8proc_category_t cat = utf8proc_category( ucs );
    if (cat == UTF8PROC_CATEGORY_CO) {
        // Co: Private use area. libutf8proc makes them zero width, while tmux
        // assumes them to be width 1, and glibc's default width is also 1
        return 1;
    }
    return utf8proc_charwidth( ucs );
#elif defined(Q_OS_WIN)
    return windows_wcwidth( ucs );
#else
    return wcwidth( ucs );
#endif
}

// single byte char: +1, multi byte char: +2
int string_width( const std::wstring & wstr )
{
    int w = 0;
#ifdef Q_OS_WIN
    // std::wstring is UTF-16 on Windows: combine surrogate pairs.
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        char32_t c = static_cast<char16_t>( wstr[ i ] );
        if ( QChar::isHighSurrogate( c ) && i + 1 < wstr.length()
             && QChar::isLowSurrogate( static_cast<char16_t>( wstr[ i + 1 ] ) ) ) {
            c = QChar::surrogateToUcs4( static_cast<char16_t>( c ),
                                        static_cast<char16_t>( wstr[ i + 1 ] ) );
            ++i;
        }
        w += konsole_wcwidth( c );
    }
#else
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        w += konsole_wcwidth( wstr[ i ] );
    }
#endif
    return w;
}
