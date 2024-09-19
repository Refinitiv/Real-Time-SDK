/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.AnsiPage;

class NullCell : PageCell
{
    internal static readonly NullCell Singleton = new NullCell();

    private NullCell() : base((char)0, GraphicSet.Unknown,
            new CellStyle((byte)0), new CellStyle((byte)0),
            CellColor.Black, CellColor.Black,
            CellColor.Black, CellColor.Black)
    {
    }

    public override void SetChar(char cChar)
    {
        throw new NotImplementedException();
    }

    public override void SetColor(CellColor foregroundColor, CellColor backgroundColor)
    {
        throw new NotImplementedException();
    }

    public override void SetFadeColor(CellColor foregroundFadeColor, CellColor backgroundFadeColor)
    {
        throw new NotImplementedException();
    }

    public override void SetFadeStyle(CellStyle fadeStyle)
    {
        throw new NotImplementedException();
    }

    public override void SetGraphicSet(GraphicSet graphicSet)
    {
        throw new NotImplementedException();
    }

    public override void SetStyle(CellStyle style)
    {
        throw new NotImplementedException();
    }
}
