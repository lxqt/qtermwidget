/*
    This file is part of qtermwidget.

    Copyright 2026 The qtermwidget contributors.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef SIXEL_DECODER_H
#define SIXEL_DECODER_H

#include <QByteArray>
#include <QImage>

namespace Konsole
{

namespace SixelDecoder
{
    // Decode a sixel payload (the bytes between "ESC P" and "ESC \",
    // i.e. parameters and intermediate bytes, the 'q' final, and the
    // sixel data, *not* including ESC, P, or the terminating ST).
    //
    // Returns a null QImage on parse failure or when built without libsixel.
    //
    // If transparentBackground is true (DCS P2 == 1), pixels not written by
    // the sixel data are kept fully transparent so the terminal background
    // shows through. Otherwise palette index 0 is rendered opaque.
    QImage decode(const QByteArray& payload, bool transparentBackground);
}

}

#endif // SIXEL_DECODER_H
