/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Ansi;

namespace LSEG.Eta.AnsiPage;

/// <summary>
/// <c>PageCell</c> is used to create the building blocks that make up an ANSI Page.
/// These positions, or "cells," are used to construct the layout of each <c>Page</c>
/// object.
/// </summary>
///
/// <seealso cref="Page"/>
/// <seealso cref="PageUpdate"/>
public class PageCell : ICloneable
{
    private CharType m_PageCell;
    private CellStyle? m_Style;
    private CellStyle? m_FadeStyle;

    /// <summary>
    /// Constructs a <c>PageCell</c> with the specified character and attributes.
    /// </summary>
    ///
    /// <param name="cChar">the char to be set in the PageCell.</param>
    /// <param name="graphicSet">the GraphicSet to be set for the PageCell.</param>
    /// <param name="style">the CellStyle to be set for the style of the PageCell.</param>
    /// <param name="fadeStyle">the CellStype to be set for the fade style of the PageCell.</param>
    /// <param name="foregroundColor">the CellColor to be set for the foreground color of the PageCell.</param>
    /// <param name="backgroundColor">the CellColor to be set for the background color of the PageCell.</param>
    /// <param name="foregroundFadeColor">the CellColor to be set for the foreground fade color of the PageCell.</param>
    /// <param name="backgroundFadeColor">the CellColor to be set for the background fadeColor of the PageCell.</param>
    public PageCell(char cChar, GraphicSet graphicSet, CellStyle style,
                    CellStyle fadeStyle, CellColor foregroundColor, CellColor backgroundColor,
                    CellColor foregroundFadeColor, CellColor backgroundFadeColor)
    {
        m_PageCell = new CharType();
        m_PageCell.Ch = cChar;
        m_PageCell.GS = (char)graphicSet.ToByte();
        m_PageCell.Attr = (char)style.ToByte();
        m_PageCell.FadeAttr = (char)fadeStyle.ToByte();

        // Shifting on an enum is not possible, so using a temp as a workaround is needed.
        long nTempVal = backgroundColor.ToByte();
        nTempVal <<= 4;
        m_PageCell.ColorAttrib = (char)(foregroundColor.ToByte() | nTempVal);
        nTempVal = backgroundFadeColor.ToByte();
        nTempVal <<= 4;
        m_PageCell.ColorFadeAttr = (char)(foregroundFadeColor.ToByte() | nTempVal);
    }

    internal PageCell(CharType charType)
    {
        m_PageCell = charType.Clone();
    }

    /// <summary>
    /// Default constructor, with character set to ' ', <c>GraphicSet</c> is set to
    /// <c>USAscii</c>, style set to plain, fade style set to plain, foreground color set
    /// mono, foreground fade color set to mono.
    /// </summary>
    public PageCell()
    {
        m_PageCell = new CharType();
        m_PageCell.Ch = (char)0x20;
        m_PageCell.GS = (char)GraphicSet.USAscii.ToByte();
        m_PageCell.Attr = (char)CellStyle.Plain.ToByte();
        m_PageCell.FadeAttr = (char)CellStyle.Plain.ToByte();

        //Shifting on an enum is not possible, so using a temp as a workaround is needed.
        long nTempVal = CellColor.Mono.ToByte();
        nTempVal <<= 4;
        m_PageCell.ColorAttrib = (char)(CellColor.Mono.ToByte() | nTempVal);
        nTempVal = CellColor.Mono.ToByte();
        nTempVal <<= 4;
        m_PageCell.ColorFadeAttr = (char)(CellColor.Mono.ToByte() | nTempVal);
    }

    /// <summary>
    /// Used to determine the background color of the <c>PageCell</c> object.
    /// </summary>
    ///
    /// <returns>the background color of the <c>PageCell</c> object.</returns>
    public CellColor BackgroundColor { get => CellColor.GetCellColor(m_PageCell.ColorAttrib >> 4); }

    /// <summary>
    /// Used to determine the background fade color of the <c>PageCell</c> object.
    /// </summary>
    ///
    /// <returns>the background fade color of the <c>PageCell</c> object</returns>
    public CellColor GetBackgroundFadeColor()
    {
        return CellColor.GetCellColor(m_PageCell.ColorFadeAttr >> 4);
    }

    /// <summary>
    /// Used to determine the actual character within the PageCell object.
    /// </summary>
    ///
    /// <returns>the character within the PageCell object.</returns>
    public char GetChar()
    {
        return m_PageCell.Ch;
    }

    /// <summary>
    /// Used to determine the fade style attribute.
    /// </summary>
    ///
    /// <returns>the fade style attribute for the PageCell object.</returns>
    public CellStyle GetFadeStyle()
    {
        if (m_FadeStyle == null)
            m_FadeStyle = new CellStyle((byte)m_PageCell.FadeAttr);
        return m_FadeStyle;
    }

    /// <summary>
    /// Used to determine the foreground color of the PageCell object.
    /// </summary>
    ///
    /// <returns>the foreground color of the PageCell object.</returns>
    public CellColor GetForegroundColor()
    {
        return CellColor.GetCellColor(m_PageCell.ColorAttrib & 0x0f);
    }

    /// <summary>
    /// Used to determine the foreground fade color of the PageCell object.
    /// </summary>
    ///
    /// <returns>the foreground fade color of the PageCell object.</returns>
    public CellColor GetForegroundFadeColor()
    {
        return CellColor.GetCellColor(m_PageCell.ColorFadeAttr & 0x0f);
    }

    /// <summary>
    /// Used to determine the graphic set being used.
    /// </summary>
    ///
    /// <returns>the graphic set being used by the PageCell object.</returns>
    public GraphicSet GetGraphicSet()
    {
        return GraphicSet.GetGraphicSet(m_PageCell.GS);
    }

    /// <summary>
    /// Used to determine the style attribute of the PageCell object.
    /// </summary>
    ///
    /// <returns>the style attribute of the PageCell object.</returns>
    public CellStyle GetStyle()
    {
        if (m_Style == null)
            m_Style = new CellStyle((byte)m_PageCell.Attr);
        return m_Style;
    }

    /// <summary>
    /// Used to modify the actual character within the PageCell object.
    /// </summary>
    ///
    /// <param name="cChar">Character that the PageCell object will contain.</param>
    public virtual void SetChar(char cChar)
    {
        m_PageCell.Ch = cChar;
    }

    /// <summary>
    /// Used to modify the foreground and background colors of the PageCell object.
    /// </summary>
    ///
    /// <param name="foregroundColor">Color to change the foreground to.</param>
    /// <param name="backgroundColor">Color to change the background to.</param>
    public virtual void SetColor(CellColor foregroundColor, CellColor backgroundColor)
    {
        long nTempBackgroundColor = backgroundColor.ToByte();
        nTempBackgroundColor <<= 4;
        m_PageCell.ColorAttrib = (char)(foregroundColor.ToByte() | nTempBackgroundColor);
    }

    /// <summary>
    /// Used to modify the foreground and background colors of the PageCell object.
    /// </summary>
    ///
    /// <param name="foregroundFadeColor">Color to change the foreground to.</param>
    /// <param name="backgroundFadeColor">Color to change the background to.</param>
    public virtual void SetFadeColor(CellColor foregroundFadeColor, CellColor backgroundFadeColor)
    {
        long nTempBackgroundFadeColor = backgroundFadeColor.ToByte();
        nTempBackgroundFadeColor <<= 4;
        m_PageCell.ColorFadeAttr = (char)(foregroundFadeColor.ToByte() | nTempBackgroundFadeColor);
    }

    /// <summary>
    /// Used to modify the fade style attribute of the PageCell object.
    /// </summary>
    ///
    /// <param name="fadeStyle">Fade style that the PageCell object will be changed to.</param>
    public virtual void SetFadeStyle(CellStyle fadeStyle)
    {
        m_PageCell.FadeAttr = (char)fadeStyle.ToByte();
    }

    /// <summary>
    /// Used to modify the graphic set being used by the PageCell object.
    /// </summary>
    ///
    /// <param name="graphicSet">Graphic set that the PageCell object will be changed to.</param>
    public virtual void SetGraphicSet(GraphicSet graphicSet)
    {
        m_PageCell.GS = (char)graphicSet.ToByte();
    }

    /// <summary>
    /// Used to modify the style attribute of the PageCell object.
    /// </summary>
    ///
    /// <param name="style">Style that the PageCell object will be changed to.</param>
    public virtual void SetStyle(CellStyle style)
    {
        m_PageCell.Attr = (char)style.ToByte();
    }

    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Makes a copy of this PageCell. Changes to the copy will not affect the original
    /// and vice versa.
    /// </summary>
    ///
    /// <returns>copy of this PageCell.</returns>
    public PageCell Clone()
    {
        PageCell newPageCell = new PageCell(m_PageCell);
        newPageCell.m_Style = m_Style?.Clone();
        newPageCell.m_FadeStyle = m_FadeStyle?.Clone();
        return newPageCell;
    }

    internal CharType ToCharType()
    {
        return m_PageCell;
    }
}
