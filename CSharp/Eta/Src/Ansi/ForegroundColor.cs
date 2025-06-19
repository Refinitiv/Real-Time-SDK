/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Select Graphic Rendition code sequences definging foreground colors.
/// </summary>
/// <seealso cref="BackgroundColor"/>
/// <seealso cref="AnsiAttribute"/>
public sealed class ForegroundColor
{
    private readonly byte[] m_Value;

    private ForegroundColor(byte[] val)
    {
        m_Value = new byte[val.Length];
        for (int i = 0; i < val.Length; i++)
            m_Value[i] = val[i];
    }

    readonly static byte[] color0 = { (byte)';', (byte)'3', (byte)'0' };
    readonly static byte[] color1 = { (byte)';', (byte)'3', (byte)'1' };
    readonly static byte[] color2 = { (byte)';', (byte)'3', (byte)'2' };
    readonly static byte[] color3 = { (byte)';', (byte)'3', (byte)'3' };
    readonly static byte[] color4 = { (byte)';', (byte)'3', (byte)'4' };
    readonly static byte[] color5 = { (byte)';', (byte)'3', (byte)'5' };
    readonly static byte[] color6 = { (byte)';', (byte)'3', (byte)'6' };
    readonly static byte[] color7 = { (byte)';', (byte)'3', (byte)'7' };

    /// <summary>
    /// Black
    /// </summary>
    public readonly static ForegroundColor FColor0 = new ForegroundColor(color0);

    /// <summary>
    /// Red
    /// </summary>
    public readonly static ForegroundColor FColor1 = new ForegroundColor(color1);

    /// <summary>
    /// Green
    /// </summary>
    public readonly static ForegroundColor FColor2 = new ForegroundColor(color2);

    /// <summary>
    /// Yellow
    /// </summary>
    public readonly static ForegroundColor FColor3 = new ForegroundColor(color3);

    /// <summary>
    /// Blue
    /// </summary>
    public readonly static ForegroundColor FColor4 = new ForegroundColor(color4);

    /// <summary>
    /// Magenta
    /// </summary>
    public readonly static ForegroundColor FColor5 = new ForegroundColor(color5);

    /// <summary>
    /// Cyan
    /// </summary>
    public readonly static ForegroundColor FColor6 = new ForegroundColor(color6);

    /// <summary>
    /// White
    /// </summary>
    public readonly static ForegroundColor FColor7 = new ForegroundColor(color7);

    /// <summary>
    /// Predefined foreground color sequences.
    /// </summary>
    public readonly static ForegroundColor[] ForegroundColors = {
        FColor0, FColor1, FColor2, FColor3, FColor4, FColor5, FColor6, FColor7
    };

    /// <summary>
    /// Returns byte representation of this foreground color sequence.
    /// </summary>
    ///
    /// <returns>byte representation of this foreground color sequence.</returns>
    public byte[] ToBytes()
    {
        return m_Value;
    }
}
