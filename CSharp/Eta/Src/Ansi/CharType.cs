/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Drawing;

namespace LSEG.Eta.Ansi;

/// <summary>
/// A character in the ANSI <see cref="PageType">Page image representation</see>.
/// </summary>
public sealed class CharType : ICloneable
{
    #region Attribute byte definitions

    /// <summary>Plain.</summary>
    public const char PLAIN = (char)0;

    /// <summary>Blink.</summary>
    public const char BLINK = (char)1;

    /// <summary>Reverse video.</summary>
    public const char REVVID = (char)2;

    /// <summary>Dim.</summary>
    public const char DIM = (char)4;

    /// <summary>Underline (underscore).</summary>
    public const char UNDLN = (char)8;

    /// <summary>Bright.</summary>
    public const char BRIGHT = (char)16;

    #endregion

    #region Definitions for Color

    /// <summary>
    /// Background color mask.
    /// </summary>
    public const int BACK_COL_MASK = 0xf0;

    /// <summary>
    /// Foreground color mask.
    /// </summary>
    public const int FORG_COL_MASK = 0x0f;

    /// <summary>
    /// Black.
    /// </summary>
    public const char BLACK = (char)0;

    /// <summary>
    /// Red.
    /// </summary>
    public const char RED = (char)1;

    /// <summary>
    /// Green.
    /// </summary>
    public const char GREEN = (char)2;

    /// <summary>
    /// Yellow.
    /// </summary>
    public const char YELLOW = (char)3;

    /// <summary>
    /// Blue.
    /// </summary>
    public const char BLUE = (char)4;

    /// <summary>
    /// Magenta.
    /// </summary>
    public const char MAGENTA = (char)5;

    /// <summary>
    /// Cyan.
    /// </summary>
    public const char CYAN = (char)6;

    /// <summary>
    /// White.
    /// </summary>
    public const char WHITE = (char)7;

    /// <summary>
    /// Monocolor.
    /// </summary>
    public const char MONOCOLOR = (char)15;

    /// <summary>
    /// Monochrome
    /// </summary>
    public const char MONO = (char)255;

    #endregion

    #region Character Sets

    /// <summary>
    /// US ASCII.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char USAscii = 'B';

    /// <summary>
    /// UK ASCII.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char UKAscii = 'A';

    /// <summary>
    /// VT100 Graphics.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char VT100Graphics = '0';

    /// <summary>
    /// Chapdelaine special ASCII.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char ChapdelaineAscii = ':';

    /// <summary>
    /// RMJ ASCII.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char RMJAscii = ';';

    /// <summary>
    /// Garban ASCII.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char GarbanAscii = '<';

    /// <summary>
    /// Garban graphics.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char GarbanGraphics = 'b';

    /// <summary>
    /// Mabon ASCII.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char MabonAscii = '=';

    /// <summary>
    /// Mosaic graphics.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char MosaicGraphics = '>';

    /// <summary>
    /// FBI special ASCII.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char FBIAscii = '?';

    /// <summary>
    /// FBI special graphics.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char FBIGraphics = 'f';

    /// <summary>
    /// Telerate ASCII.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char TelerateAscii = 's';

    /// <summary>
    /// Topic character set.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char Topic = 't';

    /// <summary>
    /// General graphics.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char GeneralGraphics = 'g';

    /// <summary>
    /// Viewdata mosaic.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char ViewdataMosaic = 'v';

    /// <summary>
    /// Viewdata separated mosaic.
    /// </summary>
    /// <remarks>
    /// Define character set selection parameter for use with
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// routine.  These parameters are to be used to indicate character set
    /// or sets being used to create the page image.
    /// </remarks>
    /// <seealso cref="GS"/>
    public const char ViewdataSeparatedMosaic = 'w';

    #endregion

    /// <summary>
    /// Character.
    /// </summary>
    public char Ch;

    /// <summary>
    /// Graphic set
    /// </summary>
    public char GS;

    /// <summary>
    /// mono attrib
    /// </summary>
    public char Attr;

    /// <summary>
    /// mono fading
    /// </summary>
    public char FadeAttr;

    /// <summary>
    /// color attrib
    /// </summary>
    public char ColorAttrib;

    /// <summary>
    /// color fading
    /// </summary>
    public char ColorFadeAttr;

    /// <summary>
    /// Copy from.
    /// </summary>
    ///
    /// <param name="from">the from</param>
    public void CopyFrom(CharType from)
    {
        Ch = from.Ch;
        GS = from.GS;
        Attr = from.Attr;
        FadeAttr = from.FadeAttr;
        ColorAttrib = from.ColorAttrib;
        ColorFadeAttr = from.ColorFadeAttr;
    }

    /// <summary>
    /// Returns a copy of this character type instance.
    /// </summary>
    /// <returns>a deep copy of this character type instance.</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a copy of this character type instance.
    /// </summary>
    /// <returns>a deep copy of this character type instance.</returns>
    public CharType Clone()
    {
        CharType charType = new CharType();
        charType.Ch = Ch;
        charType.GS = GS;               /* graphic set */
        charType.Attr = Attr;           /* mono attrib */
        charType.FadeAttr = FadeAttr; /* mono fading */
        charType.ColorAttrib = ColorAttrib;           /* color attrib */
        charType.ColorFadeAttr = ColorFadeAttr; /* color fading */
        return charType;
    }

    /// <summary>
    /// Instantiates a new char type.
    /// </summary>
    public CharType()
    {
        Clear();
    }

    /// <summary>
    /// Clear.
    /// </summary>
    public void Clear()
    {
        Ch = ' ';
        GS = USAscii;
        Attr = PLAIN;
        FadeAttr = PLAIN;
        ColorAttrib = MONO;
        ColorFadeAttr = MONO;
    }

    /// <summary>
    /// Color for the given code.
    /// </summary>
    ///
    /// <param name="i">color code</param>
    /// <returns>the color</returns>
    public static Color ColorFor(int i)
    {
        Color c = i switch
        {
            BLACK => Color.Black,
            RED => Color.Red,
            GREEN => Color.Green,
            YELLOW => Color.Yellow,
            BLUE => Color.Blue,
            MAGENTA => Color.Magenta,
            CYAN => Color.Cyan,
            WHITE => Color.White,
            _ => Color.White
        };
        return c;
    }
}
