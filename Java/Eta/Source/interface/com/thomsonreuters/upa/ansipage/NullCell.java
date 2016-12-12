package com.thomsonreuters.upa.ansipage;

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

