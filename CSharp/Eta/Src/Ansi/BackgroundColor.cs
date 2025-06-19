/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Select Graphic Rendition code sequences definging background colors.
/// </summary>
/// <seealso cref="ForegroundColor"/>
/// <seealso cref="AnsiAttribute"/>
public sealed class BackgroundColor : ICloneable
{
    private readonly byte[] m_Value;

    private BackgroundColor(byte[] val)
    {
        m_Value = new byte[val.Length];
        for (int i = 0; i < val.Length; i++)
            m_Value[i] = val[i];
    }

    static readonly byte[] color0 = { (byte)';', (byte)'4', (byte)'0' };
    static readonly byte[] color1 = { (byte)';', (byte)'4', (byte)'1' };
    static readonly byte[] color2 = { (byte)';', (byte)'4', (byte)'2' };
    static readonly byte[] color3 = { (byte)';', (byte)'4', (byte)'3' };
    static readonly byte[] color4 = { (byte)';', (byte)'4', (byte)'4' };
    static readonly byte[] color5 = { (byte)';', (byte)'4', (byte)'5' };
    static readonly byte[] color6 = { (byte)';', (byte)'4', (byte)'6' };
    static readonly byte[] color7 = { (byte)';', (byte)'4', (byte)'7' };

    /// <summary>
    /// Black
    /// </summary>
    public readonly static BackgroundColor BColor0 = new BackgroundColor(color0);

    /// <summary>
    /// Red
    /// </summary>
    public readonly static BackgroundColor BColor1 = new BackgroundColor(color1);

    /// <summary>
    /// Green
    /// </summary>
    public readonly static BackgroundColor BColor2 = new BackgroundColor(color2);

    /// <summary>
    /// Yellow
    /// </summary>
    public readonly static BackgroundColor BColor3 = new BackgroundColor(color3);

    /// <summary>
    /// Blue
    /// </summary>
    public readonly static BackgroundColor BColor4 = new BackgroundColor(color4);

    /// <summary>
    /// Magenta
    /// </summary>
    public readonly static BackgroundColor BColor5 = new BackgroundColor(color5);

    /// <summary>
    /// Cyan
    /// </summary>
    public readonly static BackgroundColor BColor6 = new BackgroundColor(color6);

    /// <summary>
    /// White
    /// </summary>
    public readonly static BackgroundColor BColor7 = new BackgroundColor(color7);

    /// <summary>
    /// Predefined background color sequences.
    /// </summary>
    public readonly static BackgroundColor[] BackgroundColors = {
        BColor0, BColor1, BColor2, BColor3, BColor4, BColor5, BColor6, BColor7
    };

    /// <summary>
    /// Returns a deep copy of this BackgroundColor object.
    /// </summary>
    /// <returns>deep copy of this BackgroundColor object.</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a deep copy of this BackgroundColor object.
    /// </summary>
    /// <returns>deep copy of this BackgroundColor object.</returns>
    public BackgroundColor Clone()
    {
        return new BackgroundColor(m_Value);
    }

    /// <summary>
    /// Returns byte representation of this background color sequence.
    /// </summary>
    ///
    /// <returns>byte representation of this background color sequence.</returns>
    public byte[] ToBytes()
    {
        return m_Value;
    }

    /// <summary>
    /// Instantiates a new background color.
    /// </summary>
    private BackgroundColor()
    {
        m_Value = new byte[0];
    }
}
