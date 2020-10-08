package com.refinitiv.eta.ansipage;
import com.refinitiv.ansi.CharType;

/**
 * PageCell is used to create the building blocks that make up an ANSI Page.
 * These positions, or "cells," are used to construct the layout of each Page object.
 */
public class PageCell implements Cloneable
{
  private CharType _pageCell;
  private CellStyle _style;
  private CellStyle _fadeStyle;


  /**
   * Constructs a PageCell with the specified character and attributes.
   * 
   * @param cChar the char to be set in the PageCell.
   * @param graphicSet the GraphicSet to be set for the PageCell.
   * @param style the CellStyle to be set for the style of the PageCell.
   * @param fadeStyle the CellStype to be set for the fade style of the PageCell.
   * @param foregroundColor the CellColor to be set for the foreground color of the PageCell.
   * @param backgroundColor the CellColor to be set for the background color of the PageCell.
   * @param foregroundFadeColor the CellColor to be set for the foreground fade color of the PageCell.
   * @param backgroundFadeColor the CellColor to be set for the background fadeColor of the PageCell.
   */
  public PageCell(char cChar, GraphicSet graphicSet, CellStyle style,
                  CellStyle fadeStyle, CellColor foregroundColor, CellColor backgroundColor,
                  CellColor foregroundFadeColor, CellColor backgroundFadeColor)
  {
    _pageCell = new CharType();
    _pageCell.ch = cChar;
    _pageCell.gs = (char)graphicSet.toByte();
    _pageCell.attr = (char)style.toByte();
    _pageCell.fade_attr = (char)fadeStyle.toByte();
    
    //Shifting on an enum is not possible, so using a temp as a workaround is needed.
    long nTempVal = backgroundColor.toByte();
    nTempVal <<= 4;
    _pageCell.c_attr = (char)(foregroundColor.toByte()|nTempVal);
    nTempVal = backgroundFadeColor.toByte();
    nTempVal <<= 4;
    _pageCell.c_fade_attr = (char)(foregroundFadeColor.toByte()|nTempVal);
  }

  PageCell(CharType charType)
  {
    _pageCell = (CharType)charType.clone();
  }

  /**
   * Default PageCell() constructor, with character set to ' ',
   * GraphicSet is set to USAscii, style set to plain, fade style set to plain, 
   * foreground color set mono, foreground fade color set to mono.
   */
  public PageCell()
  {
    _pageCell = new CharType();
    _pageCell.ch = (char)0x20;
    _pageCell.gs = (char)GraphicSet.USAscii.toByte();
    _pageCell.attr = (char)CellStyle.plain.toByte();
    _pageCell.fade_attr = (char)CellStyle.plain.toByte();
    
    //Shifting on an enum is not possible, so using a temp as a workaround is needed.
    long nTempVal = CellColor.mono.toByte();
    nTempVal <<= 4;
    _pageCell.c_attr = (char)(CellColor.mono.toByte()|nTempVal);
    nTempVal = CellColor.mono.toByte();
    nTempVal<<= 4;
    _pageCell.c_fade_attr = (char)(CellColor.mono.toByte()|nTempVal);
  }

  /**
   * Used to determine the background color of the PageCell object.
   * 
   * @return the background color of the PageCell object.
   */
  public CellColor getBackgroundColor()
  {
    return CellColor.getCellColor(_pageCell.c_attr >> 4);
  }
  
  /**
   * Used to determine the background fade color of the PageCell object.
   * 
   * @return the background fade color of the PageCell object
   */
  public CellColor getBackgroundFadeColor()
  {
    return CellColor.getCellColor(_pageCell.c_fade_attr >> 4);
  }
  
  /**
   * Used to determine the actual character within the PageCell object.
   * 
   * @return the character within the PageCell object.
   */
  public char getChar()
  {
    return _pageCell.ch;
  }
  
  /**
   * Used to determine the fade style attribute.
   * 
   * @return the fade style attribute for the PageCell object.
   */
  public CellStyle getFadeStyle()
  {
      if (_fadeStyle == null)
          _fadeStyle = new CellStyle((byte)_pageCell.fade_attr);
      return _fadeStyle;
  }
  
  /**
   * Used to determine the foreground color of the PageCell object.
   * 
   * @return the foreground color of the PageCell object.
   */
  public CellColor getForegroundColor()
  {
    return CellColor.getCellColor(_pageCell.c_attr& 0x0f );
  }

  /**
   * Used to determine the foreground fade color of the PageCell object.
   * 
   * @return the foreground fade color of the PageCell object.
   */
  public CellColor getForegroundFadeColor()
  {
    return CellColor.getCellColor(_pageCell.c_fade_attr & 0x0f);
  }
  
  /**
   * Used to determine the graphic set being used.
   * 
   * @return the graphic set being used by the PageCell object.
   */
  public GraphicSet getGraphicSet()
  {
    return GraphicSet.getGraphicSet(_pageCell.gs);
  }

  /**
   * Used to determine the style attribute of the PageCell object.
   * 
   * @return the style attribute of the PageCell object.
   */
  public CellStyle getStyle()
  {
      if (_style == null)
          _style = new CellStyle((byte)_pageCell.attr);
      return _style;
  }

  /**
   * Used to modify the actual character within the PageCell object.
   * 
   * @param cChar Character that the PageCell object will contain.  Type: char
   */
  public void setChar(char cChar)
  {
    _pageCell.ch = cChar;
  }

  /**
   * Used to modify the foreground and background colors of the PageCell object.
   * 
   * @param foregroundColor Color to change the foreground to.  Type: CellColor
   * @param backgroundColor Color to change the background to.  Type: CellColor
   */
  public void setColor(CellColor foregroundColor, CellColor backgroundColor)
  {
    long nTempBackgroundColor = backgroundColor.toByte();
    nTempBackgroundColor <<= 4;
    _pageCell.c_attr = (char)(foregroundColor.toByte()|nTempBackgroundColor);
  }

  /**
   * Used to modify the foreground and background colors of the PageCell object.
   * 
   * @param foregroundFadeColor Color to change the foreground to.  Type: CellColor
   * @param backgroundFadeColor Color to change the background to.  Type: CellColor
   */
  public void setFadeColor(CellColor foregroundFadeColor, CellColor backgroundFadeColor)
  {
    long nTempBackgroundFadeColor = backgroundFadeColor.toByte();
    nTempBackgroundFadeColor <<= 4;
    _pageCell.c_fade_attr = (char)(foregroundFadeColor.toByte()|nTempBackgroundFadeColor);
  }

  /**
   * Used to modify the fade style attribute of the PageCell object.
   * 
   * @param fadeStyle Fade style that the PageCell object will be changed to.  Type: CellStyle
   */
  public void setFadeStyle(CellStyle fadeStyle)
  {
    _pageCell.fade_attr = (char)fadeStyle.toByte();
  }

  /**
   * Used to modify the graphic set being used by the PageCell object.
   * 
   * @param graphicSet Graphic set that the PageCell object will be changed to.  Type: GraphicSet
   */
  public void setGraphicSet (GraphicSet graphicSet)
  {
    _pageCell.gs = (char)graphicSet.toByte();
  }

  /**
   * Used to modify the style attribute of the PageCell object.
   * 
   * @param style Style that the PageCell object will be changed to.  Type: CellStyle
   */
  public void setStyle (CellStyle style)
  {
    _pageCell.attr = (char)style.toByte();
  }

  /**
   * Makes a copy of this PageCell. Changes to the copy will not affect the original and vice versa.
   * 
   * @return copy of this PageCell.
   */
  public Object clone()
  {
    PageCell newPageCell = new PageCell(_pageCell);
    newPageCell._style = (CellStyle) _style.clone();
    newPageCell._fadeStyle = (CellStyle)_fadeStyle.clone();
    return newPageCell;
  }

  CharType toCharType()
  {
    return _pageCell;
  }
}


