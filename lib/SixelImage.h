/*
    This file is part of qtermwidget.

    Copyright 2026 The qtermwidget contributors.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef SIXEL_IMAGE_H
#define SIXEL_IMAGE_H

#include <QImage>
#include <QtGlobal>

namespace Konsole
{

// A decoded sixel image anchored to an absolute terminal line.
// The anchor line is "history line index" (0..history->getLines()-1) at
// emission time; as the history drops lines, the anchor effectively becomes
// negative and the image is discarded.
struct SixelImage
{
    QImage image;        // ARGB32
    qint64 anchorLine;   // absolute line index = histLines + cuY at emission
    int    anchorColumn; // cell column at emission
    int    cellRows;     // ceil(image.height() / fontHeight) at emission
    int    cellCols;     // ceil(image.width()  / fontWidth)  at emission
};

}

#endif // SIXEL_IMAGE_H
