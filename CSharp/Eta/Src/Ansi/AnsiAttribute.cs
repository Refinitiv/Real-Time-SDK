/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Supported character attribute sequences.
/// </summary>
/// <remarks>
/// <para>
/// These attributes are used to set character display attributes (Select Graphic
/// Rendition, SGR) with Control Sequence Introducer (CSI) sequences.</para>
/// <para>
/// Several attributes can be set in the same sequence.</para>
/// </remarks>
/// <seealso cref="BackgroundColor"/>
public sealed class AnsiAttribute : ICloneable
{
    private readonly byte[] m_Value;

    private AnsiAttribute(byte[] val)
    {
        m_Value = new byte[val.Length];
        for (int i = 0; i < val.Length; i++)
            m_Value[i] = val[i];
    }

    static readonly byte[] attr0 = { (byte)'0' };  /*plain */
    static readonly byte[] attr1 = { (byte)'0', (byte)';', (byte)'5' };    /* blink */
    static readonly byte[] attr2 = { (byte)'0', (byte)';', (byte)'7' };    /* rev vid */
    static readonly byte[] attr3 = { (byte)'0', (byte)';', (byte)'5', (byte)';', (byte)'7' };  /* blink rev */
    static readonly byte[] attr4 = { (byte)'0', (byte)';', (byte)'2' };            /* dim */
    static readonly byte[] attr5 = { (byte)'0', (byte)';', (byte)'2', (byte)';', (byte)'5' };  /* dim blink */
    static readonly byte[] attr6 = { (byte)'0', (byte)';', (byte)'2', (byte)';', (byte)'7' };  /* dim rev */
    static readonly byte[] attr7 = { (byte)'0', (byte)';', (byte)'2', (byte)';', (byte)'5', (byte)';', (byte)'7' };    /* dim blink rev */
    static readonly byte[] attr8 = { (byte)'0', (byte)';', (byte)'4' }; /* underscore */
    static readonly byte[] attr9 = { (byte)'0', (byte)';', (byte)'4', (byte)';', (byte)'5' };  /* under blink */
    static readonly byte[] attr10 = { (byte)'0', (byte)';', (byte)'4', (byte)';', (byte)'7' };  /* under rev */
    static readonly byte[] attr11 = { (byte)'0', (byte)';', (byte)'4', (byte)';', (byte)'5', (byte)';', (byte)'7' };    /* under blink rev */
    static readonly byte[] attr12 = { (byte)'0', (byte)';', (byte)'2', (byte)';', (byte)'4' };  /* under dim */
    static readonly byte[] attr13 = { (byte)'0', (byte)';', (byte)'2', (byte)';', (byte)'4', (byte)';', (byte)'5' };  /* under dim blink */
    static readonly byte[] attr14 = { (byte)'0', (byte)';', (byte)'2', (byte)';', (byte)'4', (byte)';', (byte)'7' };  /* under dim rev */
    static readonly byte[] attr15 = { (byte)'0', (byte)';', (byte)'2', (byte)';', (byte)'4', (byte)';', (byte)'5', (byte)';', (byte)'7' };    /* under dim blink rev */
    static readonly byte[] attr16 = { (byte)'0', (byte)';', (byte)'1' };    /* brt */
    static readonly byte[] attr17 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'5' };  /* brt blink */
    static readonly byte[] attr18 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'7' };  /* brt rev vid */
    static readonly byte[] attr19 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'5', (byte)';', (byte)'7' }; /* brt blink rev */
    static readonly byte[] attr20 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'2' };  /* brt dim */
    static readonly byte[] attr21 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'2', (byte)';', (byte)'5' };    /* brt dim blink */
    static readonly byte[] attr22 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'2', (byte)';', (byte)'7' };    /* brt dim rev */
    static readonly byte[] attr23 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'2', (byte)';', (byte)'5', (byte)';', (byte)'7' };    /* brt dim blink rev */
    static readonly byte[] attr24 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'4' };  /* brt underscore */
    static readonly byte[] attr25 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'4', (byte)';', (byte)'5' };    /* brt under blink */
    static readonly byte[] attr26 = { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'4', (byte)';', (byte)'7' };    /* brt under rev */
    static readonly byte[] attr27 =
        { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'4', (byte)';', (byte)'5', (byte)';', (byte)'7' };    /* brt under blink rev */
    static readonly byte[] attr28 =
        { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'2', (byte)';', (byte)'4' };    /* brt under dim */
    static readonly byte[] attr29 =
        { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'2', (byte)';', (byte)'4', (byte)';', (byte)'5' };    /* brt under dim blink */
    static readonly byte[] attr30 =
        { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'2', (byte)';', (byte)'4', (byte)';', (byte)'7' };    /* brt under dim rev */
    static readonly byte[] attr31 =
        { (byte)'0', (byte)';', (byte)'1', (byte)';', (byte)'2', (byte)';', (byte)'4', (byte)';', (byte)'5', (byte)';', (byte)'7' };    /* brt under dim blink rev */

    /// <summary>
    /// Plain, all attributes are off (CSI 0). This is the default.
    /// </summary>
    public static readonly AnsiAttribute Attribute0 = new AnsiAttribute(attr0);

    /// <summary>
    /// Blink on (CSI 5).
    /// </summary>
    public static readonly AnsiAttribute Attribute1 = new AnsiAttribute(attr1);

    /// <summary>
    /// Reverse video on (CSI 7).
    /// </summary>
    public static readonly AnsiAttribute Attribute2 = new AnsiAttribute(attr2);

    /// <summary>
    /// A combination of <see cref="Attribute1">blink</see> and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute3 = new AnsiAttribute(attr3);

    /// <summary>
    /// Dim, half-intensity (CSI 2).
    /// </summary>
    public static readonly AnsiAttribute Attribute4 = new AnsiAttribute(attr4);

    /// <summary>
    /// A combination of <see cref="Attribute1">dim</see> and <see cref="Attribute4">blink on</see>.
    /// </summary>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute4"/>
    public static readonly AnsiAttribute Attribute5 = new AnsiAttribute(attr5);

    /// <summary>
    /// A combination of <see cref="Attribute2">dim</see> and <see cref="Attribute4">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute2"/>
    /// <seealso cref="Attribute4"/>
    public static readonly AnsiAttribute Attribute6 = new AnsiAttribute(attr6);

    /// <summary>
    /// A combination of <see cref="Attribute4">dim</see>, <see cref="Attribute1">blink</see>
    /// and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute2"/>
    /// <seealso cref="Attribute4"/>
    public static readonly AnsiAttribute Attribute7 = new AnsiAttribute(attr7);

    /// <summary>
    /// Underscore, underline on (CSI 4).
    /// </summary>
    public static readonly AnsiAttribute Attribute8 = new AnsiAttribute(attr8);

    /// <summary>
    /// A combination of <see cref="Attribute8">underscore</see> and <see cref="Attribute1">blink</see>.
    /// </summary>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute1"/>
    public static readonly AnsiAttribute Attribute9 = new AnsiAttribute(attr9);

    /// <summary>
    /// A combination of <see cref="Attribute8">underscore</see> and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute10 = new AnsiAttribute(attr10);

    /// <summary>
    /// A combination of <see cref="Attribute8">underscore</see>, <see cref="Attribute1">blink</see>
    /// and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute11 = new AnsiAttribute(attr11);

    /// <summary>
    /// A combination of <see cref="Attribute8">underscore</see> and <see cref="Attribute4">dim</see>.
    /// </summary>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute4"/>
    public static readonly AnsiAttribute Attribute12 = new AnsiAttribute(attr12);

    /// <summary>
    /// A combination of <see cref="Attribute8">underscore</see>, <see cref="Attribute4">dim</see>
    /// and <see cref="Attribute1">blink</see>.
    /// </summary>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute1"/>
    public static readonly AnsiAttribute Attribute13 = new AnsiAttribute(attr13);

    /// <summary>
    /// A combination of <see cref="Attribute8">underscore</see>, <see cref="Attribute4">dim</see>
    /// and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute14 = new AnsiAttribute(attr14);

    /// <summary>
    /// A combination of <see cref="Attribute8">underscore</see>, <see cref="Attribute4">dim</see>,
    /// <see cref="Attribute1">blink</see> and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute15 = new AnsiAttribute(attr15);

    /// <summary>
    /// High-intensity, bold (CSI 1).
    /// </summary>
    public static readonly AnsiAttribute Attribute16 = new AnsiAttribute(attr16);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see> and <see cref="Attribute1">blink</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute1"/>
    public static readonly AnsiAttribute Attribute17 = new AnsiAttribute(attr17);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see> and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute18 = new AnsiAttribute(attr18);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute1">blink</see>
    /// and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute19 = new AnsiAttribute(attr19);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see> and <see cref="Attribute1">blink</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute1"/>
    public static readonly AnsiAttribute Attribute20 = new AnsiAttribute(attr20);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute4">dim</see>
    /// and <see cref="Attribute1">blink</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute1"/>
    public static readonly AnsiAttribute Attribute21 = new AnsiAttribute(attr21);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute4">dim</see>
    /// and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute22 = new AnsiAttribute(attr22);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute4">dim</see>,
    /// <see cref="Attribute1">blink</see> and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute23 = new AnsiAttribute(attr23);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see> and <see cref="Attribute8">underscore</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute8"/>
    public static readonly AnsiAttribute Attribute24 = new AnsiAttribute(attr24);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute8">underscore</see>
    /// and <see cref="Attribute1">blink</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute1"/>
    public static readonly AnsiAttribute Attribute25 = new AnsiAttribute(attr25);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute8">underscore</see>
    /// and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute26 = new AnsiAttribute(attr26);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute8">underscore</see>,
    /// <see cref="Attribute1">blink</see> and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute27 = new AnsiAttribute(attr27);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute8">underscore</see>
    /// and <see cref="Attribute4">dim</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute4"/>
    public static readonly AnsiAttribute Attribute28 = new AnsiAttribute(attr28);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute8">underscore</see>,
    /// <see cref="Attribute4">dim</see> and <see cref="Attribute1">blink</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute1"/>
    public static readonly AnsiAttribute Attribute29 = new AnsiAttribute(attr29);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute8">underscore</see>,
    /// <see cref="Attribute4">dim</see> and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute30 = new AnsiAttribute(attr30);

    /// <summary>
    /// A combination of <see cref="Attribute16">bold</see>, <see cref="Attribute8">underscore</see>, <see cref="Attribute4">dim</see>,
    /// <see cref="Attribute1">blink</see> and <see cref="Attribute2">reverse video on</see>.
    /// </summary>
    /// <seealso cref="Attribute16"/>
    /// <seealso cref="Attribute8"/>
    /// <seealso cref="Attribute4"/>
    /// <seealso cref="Attribute1"/>
    /// <seealso cref="Attribute2"/>
    public static readonly AnsiAttribute Attribute31 = new AnsiAttribute(attr31);

    /// <summary>
    /// Enumerates character attributes defined in <see cref="AnsiAttribute"/>.
    /// </summary>
    public static readonly AnsiAttribute[] attribute = {
        Attribute0, Attribute1, Attribute2, Attribute3, Attribute4, Attribute5,
        Attribute6, Attribute7, Attribute8, Attribute9, Attribute10, Attribute11,
        Attribute12, Attribute13, Attribute14, Attribute15, Attribute16, Attribute17,
        Attribute18, Attribute19, Attribute20, Attribute21, Attribute22, Attribute23,
        Attribute24, Attribute25, Attribute26, Attribute27, Attribute28, Attribute29,
        Attribute30, Attribute31
    };

    /// <summary>
    /// Returns byte representation of this ANSI character attribute sequence.
    /// </summary>
    ///
    /// <returns>byte representation of this ANSI character attribute sequence.</returns>
    public byte[] ToBytes()
    {
        return m_Value;
    }

    /// <summary>
    /// Returns a copy of this character attribute sequence.
    /// </summary>
    /// <returns>a deep copy of this character attribute sequence</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a copy of this character attribute sequence.
    /// </summary>
    /// <returns>a deep copy of this character attribute sequence</returns>
    public AnsiAttribute Clone()
    {
        return new AnsiAttribute(m_Value);
    }

    /// <summary>
    /// Instantiates a new attribute.
    /// </summary>
    private AnsiAttribute()
    {
        m_Value = new byte[0];
    }
}
