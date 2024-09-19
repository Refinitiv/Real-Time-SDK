/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Drawing;

namespace LSEG.Eta.AnsiPage;

/// <summary>
/// The <see cref="CellColor"/> enumeration is used to define the foreground and background
/// colors for each <see cref="PageCell"/> object.
/// </summary>
/// <remarks>
///
/// The first color of the CellColor is <c>Black</c> which
/// is 0, the last is <c>Mono</c> which is 15.</remarks>
public sealed class CellColor
{
    private readonly string m_Name;
    private readonly byte m_Value;
    private readonly Color m_Color;

    /// <summary>
    /// Returns a String object representing this CellColor's color.
    /// </summary>
    ///
    /// <returns>a string representation of the color of this CellColor.</returns>
    public override string ToString()
    {
        return m_Name;
    }

    /// <summary>Returns a byte representing this CellColor.
    /// </summary>
    /// <returns>a byte representation of the color of this CellColor</returns>
    public byte ToByte()
    {
        return m_Value;
    }

    /// <summary>
    /// Constructs a CellColor using a byte value
    /// </summary>
    ///
    /// <param name="val">the value used to set the color of the CellColor</param>
    private CellColor(int val)
    {
        switch (val)
        {
            case 0x0f:
                m_Name = "mono";
                m_Color = Color.Black;
                break;
            case 0x00:
                m_Name = "black";
                m_Color = Color.Black;
                break;
            case 0x01:
                m_Name = "red";
                m_Color = Color.Red;
                break;
            case 0x02:
                m_Name = "green";
                m_Color = Color.Green;
                break;
            case 0x03:
                m_Name = "yellow";
                m_Color = Color.Yellow;
                break;
            case 0x04:
                m_Name = "blue";
                m_Color = Color.Blue;
                break;
            case 0x05:
                m_Name = "magenta";
                m_Color = Color.Magenta;
                break;
            case 0x06:
                m_Name = "cyan";
                m_Color = Color.Cyan;
                break;
            case 0x07:
                m_Name = "white";
                m_Color = Color.White;
                break;
            default:
                m_Name = string.Empty;
                break;
        }
        m_Value = (byte)val;
    }

    /// <summary>
    /// Value of the CellColor indicating Mono.
    /// </summary>
    public readonly static CellColor Mono = new CellColor(0x0f);
    /// <summary>
    /// Value of the CellColor indicating Black.
    /// </summary>
    public readonly static CellColor Black = new CellColor(0x00);
    /// <summary>
    /// Value of the CellColor indicating Red.
    /// </summary>
    public readonly static CellColor Red = new CellColor(0x01);
    /// <summary>
    /// Value of the CellColor indicating Green.
    /// </summary>
    public readonly static CellColor Green = new CellColor(0x02);
    /// <summary>
    /// Value of the CellColor indicating Yellow.
    /// </summary>
    public readonly static CellColor Yellow = new CellColor(0x03);
    /// <summary>
    /// Value of the CellColor indicating Blue.
    /// </summary>
    public readonly static CellColor Blue = new CellColor(0x04);
    /// <summary>
    /// Value of the CellColor indicating Magenta.
    /// </summary>
    public readonly static CellColor Magenta = new CellColor(0x05);
    /// <summary>
    /// Value of the CellColor indicating Cyan.
    /// </summary>
    public readonly static CellColor Cyan = new CellColor(0x06);
    /// <summary>
    /// Value of the CellColor indicating White.
    /// </summary>
    public readonly static CellColor White = new CellColor(0x07);


    /// <summary>
    /// Array of CellColor
    /// </summary>
    public readonly static CellColor[] CellColors =
    {
        Mono,    // Mono (default)
        Black,   // Black
        Red,     // Red
        Green,   // Green
        Yellow,  // Yellow
        Blue,    // Blue
        Magenta, // Magenta
        Cyan,    // Cyan
        White    // White
    };

    /// <summary>
    /// Gets the color.
    /// </summary>
    ///
    /// <returns>the color</returns>
    public Color GetColor()
    {
        return m_Color;
    }

    internal static CellColor GetCellColor(int v)
    {
        if (v <= 7)
            return CellColors[v + 1];
        return CellColors[0];
    }
}
