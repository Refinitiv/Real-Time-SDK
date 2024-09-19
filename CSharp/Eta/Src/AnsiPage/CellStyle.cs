/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.AnsiPage;

/// <summary>
/// The CellStyle enumeration is used to define the style of each <see cref="PageCell"/> object.
/// </summary>
public sealed class CellStyle : ICloneable
{
    private string m_Name;
    private byte m_Value;
    private bool m_Immutable = false;

    /// <summary>
    /// Instantiates a new cell style.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    private CellStyle(string name, byte value)
    {
        m_Name = name;
        m_Value = value;
        m_Immutable = true;
    }

    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Makes a copy of this CellStyle. Changes to the copy will not affect the original
    /// and vice versa.
    /// </summary>
    ///
    /// <returns>copy of this CellStyle</returns>
    public CellStyle Clone()
    {
        return new CellStyle(m_Value);
    }

    /// <summary>
    /// Returns a String object representing this CellStyle's color.
    /// </summary>
    ///
    /// <returns>a string representation of the style of this CellStyle.</returns>
    public override String ToString()
    {
        return m_Name;
    }

    /// <summary>
    /// Constructs a CellStyle using a byte value.
    /// </summary>
    ///
    /// <param name="val">the value used to set the style of the CellStyle</param>
    public CellStyle(byte val)
    {
        m_Name = val switch
        {
            0x00 => "plain",
            0x01 => "blink",
            0x02 => "reverse",
            0x04 => "dim",
            0x08 => "underline",
            0x10 => "bright",
            _ => string.Empty
        };
        m_Value = val;
    }

    /// <summary>Plain</summary>
    public readonly static CellStyle Plain = new CellStyle("plain", (byte)0x00);
    /// <summary>Blink</summary>
    public readonly static CellStyle Blink = new CellStyle("blink", (byte)0x01);
    /// <summary>Reverse</summary>
    public readonly static CellStyle Reverse = new CellStyle("reverse", (byte)0x02);
    /// <summary>Dim</summary>
    public readonly static CellStyle Dim = new CellStyle("dim", (byte)0x04);
    /// <summary>Underline</summary>
    public readonly static CellStyle Underline = new CellStyle("underline", (byte)0x08);
    /// <summary>Bright</summary>
    public readonly static CellStyle Bright = new CellStyle("bright", (byte)0x10);

    /// <summary>
    /// Returns a byte representing this CellStyle.
    /// </summary>
    ///
    /// <returns>a byte representation of the style of this CellStyle.</returns>
    internal byte ToByte()
    {
        return m_Value;

    }

    /// <summary>
    /// Default constructor that assigns plain and not underlined as the style of the CellStyle.
    /// </summary>
    public CellStyle()
    {
        m_Name = "plain";
        m_Value = 0;
    }

    /// <summary>
    /// Checks for style.
    /// </summary>
    ///
    /// <param name ="c">the style to check for</param>
    ///
    /// <returns><c>true</c> if this object contains the cell style any of the styles specified.</returns>
    public bool HasStyle(CellStyle c)
    {
        return (m_Value & c.m_Value) != 0;
    }

    /// <summary>
    /// Adds the style or styles to the existing cell styles stored in this object.
    /// </summary>
    ///
    /// <param name="c">the new style</param>
    public void SetStyle(CellStyle c)
    {
        if (m_Immutable)
            throw new InvalidOperationException("Cannot modify a CellStyle constant");
        m_Value |= c.m_Value;
    }

    /// <summary>
    /// Clears all styles, leaving the style equivalent to <see cref="Plain"/>.
    /// </summary>
    public void Reset()
    {
        m_Value = 0;
    }
}
