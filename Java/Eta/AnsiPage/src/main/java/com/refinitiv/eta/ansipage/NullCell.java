/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.ansipage;

class NullCell extends PageCell
{
    static final NullCell Singleton = new NullCell();

    private NullCell()
    {
        super((char)0, GraphicSet.Unknown,
            new CellStyle((byte)0), new CellStyle((byte)0),
            CellColor.black, CellColor.black,
            CellColor.black, CellColor.black);
    }

    public void setChar(char cChar)
    {
      throw new UnsupportedOperationException();
    }

    public void setColor(CellColor foregroundColor, CellColor backgroundColor)
    {
        throw new UnsupportedOperationException();
    }

    public void setFadeColor(CellColor foregroundFadeColor, CellColor backgroundFadeColor)
    {
        throw new UnsupportedOperationException();
    }

    public void setFadeStyle(CellStyle fadeStyle)
    {
        throw new UnsupportedOperationException();
    }

    public void setGraphicSet (GraphicSet graphicSet)
    {
        throw new UnsupportedOperationException();
    }

    public void setStyle (CellStyle style)
    {
        throw new UnsupportedOperationException();
    }

}

