/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using LSEG.Eta.Ansi;

namespace LSEG.Eta.AnsiPage;

/// <summary>
/// The <c>Page</c> interface is the heart of the AnsiPage API.  This interface
/// encapsulates a list of cells defined by the <see cref="PageCell"/> interface into one entity,
/// forming the page image.
/// </summary>
///
/// <remarks>
/// <para>
/// The <c>Page</c> interface also provides the ability to generate and interpret ANSI
/// encoded update streams, as well as the ability to reset the <c>Page</c> and a list of
/// <see cref="PageUpdate"/> objects back to their default state.  The encoding process analyzes
/// the list of updates and the corresponding page cells to produce an ANSI update stream.
/// This stream may then, for example, be sent to a different application for decoding.</para>
///
/// <para>
/// The decoding process parses the contents of an ANSI encoded update stream into the
/// list of updates and the corresponding page cells.</para>
/// </remarks>
///
/// <seealso cref="PageCell"/>
/// <seealso cref="PageUpdate"/>
public class Page : ICloneable
{
    private ListType? m_UpdateList = null;
    private PageType m_Page;


    /// <summary>
    /// The Page constructor is used to create a <c>Page</c> object.
    /// </summary>
    ///
    /// <param name="nNumberOfRows">Number of rows that the Page object will contain. Default: 25</param>
    /// <param name="nNumberOfColumns">Number of columns that the Page object will contain. Default: 80</param>
    public Page(short nNumberOfRows, short nNumberOfColumns)
    {
        //If constructor is passed negative values (or zero), use default values
        RowsCount = (nNumberOfRows <= 0) ? (short)25 : nNumberOfRows;
        ColumnsCount = (nNumberOfColumns <= 0) ? (short)80 : nNumberOfColumns;
        m_UpdateList = new ListType();
        m_Page = new PageType(nNumberOfRows, nNumberOfColumns);
        m_Page.Status.InitStatus();
    }

    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Makes a copy of this <c>Page</c>. Changes to the copy will not affect the original and vice versa.
    /// </summary>
    ///
    /// <returns>the deep copy of this <c>Page</c></returns>
    public Page Clone()
    {
        Page page = new Page(RowsCount, ColumnsCount);
        page.m_Page = m_Page.Clone();
        page.m_UpdateList = m_UpdateList?.Clone();
        return page;
    }

    /// <summary>
    /// Analyzes and parses the ANSI encoded update stream and updates the
    /// <see cref="PageUpdate"/> list as well as the corresponding <see cref="PageCell"/>
    /// objects of the <c>Page</c> image.
    /// </summary>
    ///
    /// <remarks>
    /// <para>Decoding an ANSI stream is accomplished by using a finite state machine.  Each
    /// of the ANSI escape sequences is parsed according to the state into which the
    /// previous characters have put this machine.</para>
    ///
    /// <para>As the sequences are recognized, the page image is modified. This allows the
    /// update list to be sized without knowledge of the maximum amount of update
    /// information that a source can communicate.  The page image should not be altered
    /// between calls to the <c>Decode()</c> method.</para>
    ///
    /// <list type="bullet">
    /// <item>As <c>Decode()</c> is called, entries are added to the update list for the
    /// following reasons:</item>
    /// <item>For each repositioning of the cursor an entry with zero length is generated
    /// for that point.  Carriage Return plus Line Feed is treated as a single sequence.
    /// </item>
    /// <item>Each character written increases the length of the current entry by one.  If
    /// it wraps around at the end of a line, it gets treated as cursor movement and a new
    /// entry is added as above.
    /// </item>
    /// <item>Scrolling generates an entry for each row scrolled, and one for the new cursor
    /// position following the scroll.</item>
    /// </list>
    /// </remarks>
    ///
    /// <param name="instream"><c>Stream</c> that contains the ANSI encoded update
    /// stream to be decoded.</param>
    /// <param name="pageUpdateList">Vector of <see cref="PageUpdate"/> objects.  This
    /// list will be updated by the <c>Decode()</c> method based upon the contents of the
    /// ANSI encoded update string.</param>
    ///
    /// <returns>the actual number of characters that were successfully parsed.</returns>
    public long Decode(Stream instream, List<PageUpdate> pageUpdateList)
    {
        if (m_UpdateList != null)
        {
            m_UpdateList = null;
        }
        m_UpdateList = new ListType();
        long nNumCharsParsed = m_Page.Decoder.QaDecode(m_Page, instream, m_UpdateList);
        pageUpdateList.Clear();
        for (int i = 0; i < m_UpdateList.Index + 1; i++)
        {
            PageUpdate pageUpdate = new PageUpdate();
            pageUpdate.SetRow(m_UpdateList.UpdList[i].Row);
            pageUpdate.SetBeginningColumn(m_UpdateList.UpdList[i].UpdateColBegin);
            pageUpdate.SetEndingColumn(m_UpdateList.UpdList[i].UpdateColEnd);
            pageUpdateList.Add(pageUpdate);
        }
        return nNumCharsParsed;
    }

    ///  <summary>
    ///  Analyzes the list of provided <c>PageUpdate</c> objects and the corresponding
    ///  <c>PageCell</c> objects in the Page image and produces an ANSI encoded update
    ///  stream.
    ///  </summary>
    ///
    ///  <param name="bFadeEnable">Flag used to enable or disable the use of fading.</param>
    ///  <param name="pageUpdate">Vector of <see cref="PageUpdate"/> objects.  This array
    ///  will be analyzed by the <c>Encode()</c> method to produce the ANSI encoded update
    ///  stream.</param>
    ///  <param name="outstream"><c>Stream</c> that will be filled in by the
    ///  encode() method.  This <c>Stream</c> will contain the ANSI encoded update
    ///  string when the encoding process is complete.</param>
    ///
    ///  <returns><c>true</c> if all of the updates defined by the update list pointed to
    ///  by pageUpdate have been successfully encoded. Returns <c>false</c>, if there is
    ///  any errors in encoding.</returns>
    public bool Encode(bool bFadeEnable, List<PageUpdate> pageUpdate, Stream outstream)
    {
        if (m_UpdateList != null)
        {
            m_UpdateList = null;
        }
        m_UpdateList = new ListType();
        m_UpdateList.Index = (short)(pageUpdate.Count - 1);
        for (int i = 0; i < pageUpdate.Count; i++)
        {
            PageUpdate update = pageUpdate[i];
            m_UpdateList.UpdList[i].Row = update.GetRow();
            m_UpdateList.UpdList[i].UpdateColBegin = update.GetBeginningColumn();
            m_UpdateList.UpdList[i].UpdateColEnd = update.GetEndingColumn();
        }

        long nRetVal = m_Page.Encoder.QaEncode(m_Page, outstream,
            bFadeEnable, m_UpdateList, RowsCount, ColumnsCount);
        if (nRetVal == 0)
        {
            // Since the encoding of the current update has completed, reset last_mod so
            // that the next time Encode() is called for this page, it will know it is for
            // a brand new update.
            m_Page.LastMod = 0;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Output the content of each <see cref="PageCell"/> to a <c>String</c> object.
    /// </summary>
    ///
    /// <returns><c>String</c> object that contains the character is each <see cref="PageCell"/></returns>
    ///
    /// <seealso cref="PageCell"/>
    public override string ToString()
    {
        StringBuilder buf = new StringBuilder(RowsCount * ColumnsCount);
        for (short j = 1; j <= RowsCount; j++)
        {
            for (short k = 1; k <= ColumnsCount; k++)
            {
                buf.Append(GetChar(j, k));
            }
            buf.Append('\n');
        }
        return buf.ToString();
    }

    /// <summary>
    /// Output the content of each <see cref="PageCell"/> to a <c>String</c> object.  If the
    /// <c>drawBorder</c> is set, the content will have a border.
    /// </summary>
    ///
    /// <param name="drawBorder">the draw border</param>
    /// <returns><c>String</c> object that contains the character is each <see cref="PageCell"/></returns>
    public String ToString(bool drawBorder)
    {
        StringBuilder buf = new StringBuilder(RowsCount * ColumnsCount);
        if (drawBorder)
        {
            for (int i = 1; i <= ColumnsCount; i++)
                buf.Append('-');
        }
        buf.Append('\n');
        for (short j = 1; j <= RowsCount; j++)
        {
            if (drawBorder)
            {
                buf.Append('|');
            }
            for (short k = 1; k <= ColumnsCount; k++)
            {
                buf.Append(GetChar(j, k));
            }
            if (drawBorder)
            {
                buf.Append('|');
            }
            buf.Append('\n');
        }
        if (drawBorder)
        {
            for (int i = 0; i < ColumnsCount; i++)
                buf.Append('-');
        }
        buf.Append('\n');
        return buf.ToString();
    }

    /// <summary>
    /// Used to determine the number of columns contained in the <c>Page</c> object.
    /// </summary>
    ///
    /// <remarks>
    /// The number of columns specified when the <c>Page</c> object was created.  If no
    /// default value was specified the default value is returned.
    /// </remarks>
    public short ColumnsCount { get; private set; } = 80;

    /// <summary>
    /// Used to determine the number of rows contained in the Page object.
    /// </summary>
    ///
    /// <remarks>
    /// The number of rows specified when the <c>Page</c> object was created.  If no
    /// default value was specified the default value is returned.
    /// </remarks>
    public short RowsCount { get; private set; } = 25;

    /// <summary>
    /// The contents of a particular cell.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired char</param>
    /// <param name="nColumn">Column of the desired char</param>
    ///
    /// <returns>char of a particular cell within the page.  If the row and column are out
    /// of range, '\0' is returned.</returns>
    public char GetChar(short nRow, short nColumn)
    {
        if (IsOutOfBounds(nRow, nColumn))
            return '\0';

        return GetCharType(nRow, nColumn).Ch;
    }

    /// <summary>
    /// The <c>CellColor</c> of a particular cell's background.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired background color</param>
    /// <param name="nColumn">Column of the desired background color</param>
    ///
    /// <returns>background color of a particular cell within the page.  If the row and
    /// column are out of range, <see cref="CellColor.Mono"/> is returned.</returns>
    ///
    /// <seealso cref="CellColor"/>
    public CellColor GetBackgroundColor(short nRow, short nColumn)
    {
        if (IsOutOfBounds(nRow, nColumn))
            return CellColor.Mono;

        CharType cell = GetCharType(nRow, nColumn);
        return CellColor.GetCellColor(cell.ColorAttrib >> 4);
    }


    /// <summary>
    /// The fade <c>CellColor</c> of a particular cell's background.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired background fade color</param>
    /// <param name="nColumn">Column of the desired background fade color</param>
    ///
    /// <returns>background fade color of a particular cell within the page.  If the row
    /// and column are out of range, <see cref="CellColor.Mono"/> is returned.</returns>
    ///
    /// <seealso cref="CellColor"/>
    public CellColor GetBackgroundFadeColor(short nRow, short nColumn)
    {
        if (IsOutOfBounds(nRow, nColumn))
            return CellColor.Mono;

        CharType cell = GetCharType(nRow, nColumn);
        return CellColor.GetCellColor(cell.ColorFadeAttr >> 4);
    }

    /// <summary>
    /// Checks for style.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired CellStyle</param>
    /// <param name="nColumn">Column of the desired CellStyle</param>
    /// <param name="style">style to look for in page</param>
    ///
    /// <returns>true if any of the styles in <c>style</c> is enabled for the given
    /// cell</returns>
    public bool HasStyle(short nRow, short nColumn, CellStyle style)
    {
        if (IsOutOfBounds(nRow, nColumn))
            return false;

        CharType cell = GetCharType(nRow, nColumn);
        return (cell.Attr & style.ToByte()) != 0;
    }

    /// <summary>
    /// Checks for fade style.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired fade style</param>
    /// <param name="nColumn">Column of the desired style CellStyle</param>
    /// <param name="style">style to look for in page</param>
    ///
    /// <returns>true if any of the styles in <c>style</c> is enabled for the given
    /// cell</returns>
    public bool HasFadeStyle(short nRow, short nColumn, CellStyle style)
    {
        if (IsOutOfBounds(nRow, nColumn))
            return false;

        CharType cell = GetCharType(nRow, nColumn);
        return (cell.FadeAttr & style.ToByte()) != 0;
    }

    /// <summary>
    /// The <c>CellColor</c> of a particular cell's foreground.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired foreground color</param>
    /// <param name="nColumn">Column of the desired foreground color</param>
    ///
    /// <returns>foreground color of a particular cell within the page.  If the row and
    /// column are out of range, <see cref="CellColor.Mono"/> is returned.</returns>
    ///
    /// <seealso cref="CellColor"/>
    public CellColor GetForegroundColor(short nRow, short nColumn)
    {
        if (IsOutOfBounds(nRow, nColumn))
            return CellColor.Mono;

        CharType cell = GetCharType(nRow, nColumn);
        return CellColor.GetCellColor(cell.ColorAttrib & 0x0f);
    }

    /// <summary>
    /// The fade <c>CellColor</c> of a particular cell's foreground.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired foreground fade color</param>
    /// <param name="nColumn">Column of the desired foreground fade color</param>
    ///
    /// <returns>foreground fade color of a particular cell within the page.  If the row
    /// and column are out of range, <see cref="CellColor.Mono"/> is returned.</returns>
    ///
    /// <seealso cref="CellColor"/>
    public CellColor GetForegroundFadeColor(short nRow, short nColumn)
    {
        if (IsOutOfBounds(nRow, nColumn))
            return CellColor.Mono;

        CharType cell = GetCharType(nRow, nColumn);
        return CellColor.GetCellColor(cell.ColorFadeAttr & 0x0f);
    }

    /// <summary>
    /// The <c>GraphicSet</c> of a particular cell within the Page object.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired GraphicSet</param>
    /// <param name="nColumn">Column of the desired GraphicSet</param>
    ///
    /// <returns><c>GraphicSet</c> of a particular cell within the page.  If the row and
    /// column are out of range, <see cref="GraphicSet.Unknown"/> is returned.</returns>
    ///
    /// <seealso cref="GraphicSet"/>
    public GraphicSet GetGraphicSet(short nRow, short nColumn)
    {
        if (IsOutOfBounds(nRow, nColumn))
            return GraphicSet.Unknown;

        CharType cell = GetCharType(nRow, nColumn);
        return GraphicSet.GetGraphicSet(cell.GS);
    }

    /// <summary>
    /// Used to reset the <c>Page</c> object and the <paramref name="pageUpdateList"/>
    /// back to their default state.
    /// </summary>
    ///
    /// <param name="pageUpdateList">the list of desired cells to modify.</param>
    ///
    /// <seealso cref="PageUpdate"/>
    public void Reset(List<PageUpdate> pageUpdateList)
    {
        if (m_UpdateList != null)
            m_UpdateList = null;
        m_UpdateList = new ListType();
        m_UpdateList.Index = 0;
        m_Page.Decoder.QaReset(m_Page, m_UpdateList);

        for (int i = 0; i < pageUpdateList.Count; i++)
        {
            PageUpdate update = pageUpdateList[i];
            update.SetRow((short)0);
            update.SetBeginningColumn((short)0);
            update.SetEndingColumn((short)0);
        }
    }

    /// <summary>
    /// Used to modify the contents of a particular cell within the <c>Page</c> object.
    /// </summary>
    ///
    /// <param name="nRow">Row of the desired cell to modify.</param>
    /// <param name="nColumn">Column of the desired cell to modify.</param>
    /// <param name="rPagecell">constant PageCell object that will be copied into the
    /// desired cell of the Page object.</param>
    ///
    /// <seealso cref="PageCell"/>
    public void SetPageCell(short nRow, short nColumn, in PageCell rPagecell)
    {
        // Check to make sure that the specified cell is within the page bounds before
        // updating the cell.
        if ((nRow > 0 && nRow <= RowsCount) &&
            (nColumn > 0 && nColumn <= ColumnsCount))
        {
            // Assign the element to the offset of the page image determined by the row
            // and column specified
            m_Page.Page[((((nRow) - 1) * ColumnsCount) + ((nColumn) - 1))] = rPagecell.ToCharType().Clone();
        }
    }

    /// <summary>
    /// Default constructor that constructs a 25 row by 80 column page object.
    /// </summary>
    public Page()
    {
        RowsCount = 25;
        ColumnsCount = 80;
        m_UpdateList = new ListType();
        m_Page = new PageType((short)25, (short)80);
        m_Page.Status.InitStatus();
    }

    /// <summary>
    /// Gets the char type.
    /// </summary>
    ///
    /// <param name="nRow">the n row</param>
    /// <param name="nColumn">the n column</param>
    ///
    /// <returns>the char type</returns>
    private CharType GetCharType(short nRow, short nColumn)
    {
        return m_Page.Page[((((nRow) - 1) * ColumnsCount) + ((nColumn) - 1))];
    }

    /// <summary>
    /// Checks if the specified cell is out of bounds.
    /// </summary>
    ///
    /// <param name="nRow">the n row</param>
    /// <param name="nColumn">the n column</param>
    ///
    /// <returns>true, if out of bounds</returns>
    private bool IsOutOfBounds(short nRow, short nColumn)
    {
        return nRow > RowsCount ||
            nColumn > ColumnsCount ||
            nRow <= 0 ||
            nColumn <= 0;
    }
}
