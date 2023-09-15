/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Current status in page representation.
/// </summary>
public sealed class StatusType : ICloneable
{
    internal short Row;
    internal short Column;

    /// <summary>
    /// monochrome attribute
    /// </summary>
    internal char CurAttribute;

    /// <summary>
    /// color attribute
    /// </summary>
    internal char ColorAttribute;

    /// <summary>
    /// color fading attr
    /// </summary>
    internal char ColorFadingAttribute;

    /// <summary>
    /// mono fading attr
    /// </summary>
    internal char Fading;

    /// <summary>
    /// flag to select G0 or G1
    /// </summary>
    internal char GR_Set;

    /// <summary>
    /// G0 graphics set
    /// </summary>
    internal char G0_Set;

    /// <summary>
    /// G1 graphics set
    /// </summary>
    internal char G1_Set;

    /// <summary>
    /// flag for line wrap
    /// </summary>
    internal bool IsWrapOn;

    /// <summary>
    /// flag for vertical editing mode
    /// </summary>
    private bool m_VertEditMode;

    /// <summary>
    /// flag for horizontal edit mode
    /// </summary>
    private bool m_HorizEditMode;

    private bool m_Fade;

    internal const int DB_BITS = 0xE0;
    internal const int DB_SHT_BITES = 0x07;
    internal const int DB_SHIFT = 5;
    internal const int DB_ATT_MASK = 0x1f;

    internal const int BACK_COL_MASK = 0xf0;
    internal const int FORG_COL_MASK = 0xf0;

    /// <summary>
    /// Returns a deep copy of this StatusType object.
    /// </summary>
    /// <returns>deep copy of this StatusType object.</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a deep copy of this StatusType object.
    /// </summary>
    /// <returns>deep copy of this StatusType object.</returns>
    public StatusType Clone()
    {
        StatusType status = new StatusType();
        status.Row = Row;
        status.Column = Column;
        status.CurAttribute = CurAttribute; /* monochrome attribute */
        status.ColorAttribute = ColorAttribute;     /* color attribute */
        status.ColorFadingAttribute = ColorFadingAttribute; /* color fading attr */
        status.Fading = Fading;     /* mono fading attr */
        status.GR_Set = GR_Set;     /* flag to select G0 or G1 */
        status.G0_Set = G0_Set;     /* G0 graphics set */
        status.G1_Set = G1_Set;     /* G1 graphics set */
        status.IsWrapOn = IsWrapOn;   /* flag for line wrap */
        status.m_VertEditMode = m_VertEditMode;   /* flag for vert editing mode */
        status.m_HorizEditMode = m_HorizEditMode;   /* flag for horiz edit mode */
        status.m_Fade = m_Fade;
        return status;
    }

    /// <summary>
    /// Inits the status.
    /// </summary>
    public void InitStatus()
    {
        Row = 1;
        Column = 1;
        CurAttribute = CharType.PLAIN;      /* monochrome attribute */
        ColorAttribute = CharType.MONO;         /* color attribute */
        ColorFadingAttribute = CharType.MONO;    /* color fading attr */
        Fading = CharType.PLAIN;        /* mono fading attr */
        GR_Set = (char)0;               /* flag to select G0 or G1 */
        G0_Set = CharType.USAscii;      /* G0 graphics set */
        G1_Set = CharType.USAscii;      /* G1 graphics set */
        IsWrapOn = false;                /* flag for line wrap */
        Fade = false;                   /* flag for fading */
    }

    internal bool IsAttr(char b)
    {
        return ((m_Fade) ? Fading & b : CurAttribute & b) == 0;
    }

    internal void OffAttr(char c)
    {
        if (m_Fade)
            Fading &= c;
        else
            CurAttribute &= c;
    }

    internal void OnAttr(char arg)
    {
        if (m_Fade)
            Fading |= arg;
        else
            CurAttribute |= arg;
    }

    internal void SetAttr(char b)
    {
        if (m_Fade)
            Fading &= b;
        else
            CurAttribute &= b;
    }

    internal void SetColor(char b)
    {
        if (m_Fade)
            ColorFadingAttribute = b;
        else
            ColorAttribute = b;
    }

    internal bool Fade { get => m_Fade; set { m_Fade = value; } }
    internal bool VerticalEditMode { get => m_VertEditMode; set { m_VertEditMode = value; } }
    internal bool HorizontalEditMode { get => m_HorizEditMode; set { m_HorizEditMode = value; } }

    internal void SetPlain()
    {
        if (m_Fade)
            Fading = (char)(Fading & (CharType.PLAIN | DB_BITS));
        else
            CurAttribute = (char)(CurAttribute & (CharType.PLAIN | DB_BITS));
    }
}
