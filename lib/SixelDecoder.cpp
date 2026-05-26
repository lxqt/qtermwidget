/*
    This file is part of qtermwidget.

    Copyright 2026 The qtermwidget contributors.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "SixelDecoder.h"

#ifdef QTERMWIDGET_SIXEL
#include <sixel.h>
#endif

using namespace Konsole;

QImage SixelDecoder::decode(const QByteArray& payload, bool transparentBackground)
{
#ifdef QTERMWIDGET_SIXEL
    if (payload.isEmpty())
        return QImage();

    unsigned char* pixels  = nullptr;
    unsigned char* palette = nullptr;
    int width   = 0;
    int height  = 0;
    int ncolors = 0;

    // libsixel's sixel_decode_raw scans for the DCS 'q' final byte before it
    // starts parsing, pass the canonical DCS envelope (ESC P q <data> ESC \)
    // around our captured image data, matching what sixel2png feeds it.
    QByteArray wrapped;
    wrapped.reserve(payload.size() + 5);
    wrapped.append("\033Pq", 3);
    wrapped.append(payload);
    wrapped.append("\033\\", 2);

    SIXELSTATUS status = sixel_decode_raw(
        reinterpret_cast<unsigned char*>(wrapped.data()),
        wrapped.size(),
        &pixels,
        &width,
        &height,
        &palette,
        &ncolors,
        /*allocator=*/nullptr);

    if (SIXEL_FAILED(status) || !pixels || !palette || width <= 0 || height <= 0) {
        if (pixels)  free(pixels);
        if (palette) free(palette);
        return QImage();
    }

    QImage img(width, height, QImage::Format_ARGB32);
    const int total = width * height;
    for (int i = 0; i < total; ++i) {
        int idx = pixels[i];
        if (idx >= ncolors) idx = 0;
        // libsixel returns the palette as packed RGB (3 bytes per entry),
        // despite the header's "ARGB palette" comment. Untouched pixels
        // stay at index 0 (from libsixel's calloc); when the producer
        // requested P2 == 1, treat them as transparent so the terminal
        // background shows through.
        const unsigned char a = (transparentBackground && idx == 0) ? 0 : 0xff;
        const unsigned char r = palette[idx * 3 + 0];
        const unsigned char g = palette[idx * 3 + 1];
        const unsigned char b = palette[idx * 3 + 2];
        reinterpret_cast<QRgb*>(img.bits())[i] = qRgba(r, g, b, a);
    }

    free(pixels);
    free(palette);
    return img;
#else
    Q_UNUSED(payload);
    Q_UNUSED(transparentBackground);
    return QImage();
#endif
}
