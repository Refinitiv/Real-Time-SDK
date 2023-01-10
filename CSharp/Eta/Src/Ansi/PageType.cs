/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// ANSI Page image representation.
/// </summary>
public sealed class PageType : ICloneable
{
    /// <summary>
    /// Characters contained by this page.
    /// </summary>
    /// <remarks>
    /// size of [PAGEROWS * PAGECOLS]
    /// </remarks>
    public CharType[] Page;

    /// <summary>
    /// Current status.
    /// </summary>
    public StatusType Status;

    /// <summary>
    /// <see cref="AnsiEncoder.QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    /// marks last entry in UpdateList it was able to encode
    /// </summary>
    public short LastMod;

    /// <summary>
    /// Gets the encoder.
    /// </summary>
    public AnsiEncoder Encoder { get; private set; }

    /// <summary>
    /// Gets the decoder.
    /// Added to make decoder able to have variable row and columns for each page.
    /// </summary>
    public AnsiDecoder Decoder { get; private set; }

    /// <summary>
    /// For cursor save/restore
    /// </summary>
    internal StatusType Save;

    /// <summary>
    /// Top of scrolling region
    /// </summary>
    internal short ScrollTop;

    /// <summary>
    /// Bottom of scrolling reg
    /// </summary>
    internal short ScrollBottom;

    /// <summary>
    /// Returns a deep copy of this PageType object.
    /// </summary>
    /// <returns>deep copy of this PageType object.</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a deep copy of this PageType object.
    /// </summary>
    /// <returns>deep copy of this PageType object.</returns>
    public PageType Clone()
    {
        PageType newPage = new PageType(Page, Encoder, Decoder, Status, Save);
        newPage.ScrollTop = ScrollTop;      /* top of scrolling region */
        newPage.ScrollBottom = ScrollBottom;      /* bottom of scrolling reg */
        newPage.LastMod = LastMod;
        return newPage;
    }

    private PageType(CharType[] page, AnsiEncoder enc, AnsiDecoder dec, StatusType status, StatusType save)
    {
        Encoder = enc.Clone();
        Decoder = dec.Clone();

        this.Page = new CharType[page.Length];
        for (int i = 0; i < page.Length; i++)
            this.Page[i] = page[i].Clone();

        this.Status = status.Clone();
        this.Save = save.Clone();  /* for cursor save/restore */
    }

    /// <summary>
    /// Instantiates a new page type.
    /// </summary>
    ///
    /// <param name="_nNumberOfRows">the n number of rows</param>
    /// <param name="_nNumberOfColumns">the n number of columns</param>
    public PageType(short _nNumberOfRows, short _nNumberOfColumns)
    {
        Decoder = new AnsiDecoder();
        Decoder.QaSetColumns(_nNumberOfColumns);
        Decoder.QaSetRows(_nNumberOfRows);
        Decoder.QaSetEndOfRow((short)(_nNumberOfColumns + 1));
        Decoder.QaSetScrollBottom((short)(_nNumberOfRows - 1));
        Encoder = new AnsiEncoder();

        Page = new CharType[_nNumberOfRows * _nNumberOfColumns];

        for (int i = 0; i < _nNumberOfRows * _nNumberOfColumns; i++)
        {
            Page[i] = new CharType();
        }

        Status = new StatusType();
        Save = new StatusType();

        ScrollBottom = (short)(Decoder.QaPageRows() - 1);
        ScrollTop = 1;
        LastMod = -1;
    }
}
