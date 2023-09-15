/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Defines interface for an ANSI page parser/decoder.
/// </summary>
public interface IAnsi
{

    /// <summary>
    /// Decodes an ANSI string.
    /// </summary>
    ///
    /// <remarks>
    /// <para>
    /// Decoding an ANSI string is accomplished using a finite state machine.
    /// Each of the ANSI escape sequences is parsed according to the state into
    /// which the previous characters have the machine.</para>
    ///
    /// <para>
    /// As the sequences are recognized, the page image is modified.</para>
    ///
    /// <para>
    /// An update list is maintained to make identification of changed regions
    /// easier.  Each entry has the row, the start column and the end column of
    /// the portion of the screen that was modified.  Entries where the start
    /// column and the end column are equal indicate that no update has occured
    /// at this location.</para>
    ///
    /// <para>
    /// The update list is maintained according to the following rules:</para>
    ///
    /// <list type="number">
    /// <item>
    /// Printable Characters written to the page increment the end col.</item>
    ///
    /// <item>
    /// When the end of the line is reached, if line wrap is on, a new entry in
    /// the list is created, the row being the next row, the start and end
    /// column being set to 1.  If wrap is off, the character overwrites the one
    /// currently at the 80th column and the end column field in the update list
    /// is not modified.</item>
    ///
    /// <item>
    /// When scrolling occurs, an entry is made in the update list for each row
    /// that was modified.</item>
    ///
    /// <item>
    /// Any sequence or character that moves the cursor position causes a new
    /// entry to be made in the list.  This includes cursor positioning
    /// sequences, backspace, linefeed, etc.</item>
    ///
    /// <item>
    /// Erase commands cause new entries to be made for each row that is
    /// affected.</item>
    /// </list>
    /// </remarks>
    ///
    /// <param name="page">base Page image</param>
    /// <param name="inStream">stream with the ANSI encoded update stream to be
    ///   decoded.</param>
    /// <param name="u_list">page updates created based on the decoded stream.</param>
    ///
    /// <returns>
    /// Returns the number of characters in text it has successfully parsed.  It
    /// will not return a length which would cause an escape sequence to be
    /// broken.  QaDecode may be called as many times as necessary until all the
    /// text is parsed.  If QaDecode has completed, the length which was passed
    /// to it is returned.
    /// </returns>
    int QaDecode(PageType page, Stream inStream, ListType u_list);

    /// <summary>
    /// Returns current end of row.
    /// </summary>
    /// <returns>current end of row</returns>
    /// <seealso cref="QaSetEndOfRow(short)"/>
    short QaEndOfRow();

    /// <summary>
    /// Columns count in this page.
    /// </summary>
    /// <returns>columns count</returns>
    /// <seealso cref="QaSetColumns(short)"/>
    short QaPageColumns();

    /// <summary>
    /// Rows count in this page.
    /// </summary>
    /// <returns>rows count</returns>
    /// <seealso cref="QaSetRows(short)"/>
    short QaPageRows();

    /// <summary>
    /// Returns scroll region top row number.
    /// </summary>
    /// <returns>scroll region top row number</returns>
    /// <seealso cref="QaSetScrollBottom(short)"/>
    short QaScrollBottom();

    /// <summary>
    /// Set the column count on the page.
    /// </summary>
    /// <param name="cl">new column count value</param>
    /// <seealso cref="QaPageColumns()"/>
    void QaSetColumns(short cl);

    /// <summary>
    /// Where the row ends.
    /// </summary>
    /// <param name="rw">new end of row value</param>
    void QaSetEndOfRow(short rw);

    /// <summary>
    /// Rows count on a page.
    /// </summary>
    /// <param name="rw">new rows count</param>
    /// <seealso cref="QaPageRows()"/>
    void QaSetRows(short rw);

    /// <summary>
    /// Set top row of the scroll region.
    /// </summary>
    /// <param name="cl">new scroll region row number</param>
    /// <seealso cref="QaScrollBottom()"/>
    void QaSetScrollBottom(short cl);

    /// <summary>
    /// Current major version.
    /// </summary>
    /// <seealso cref="MinorVersion"/>
    const int MajorVersion = 1;

    /// <summary>
    /// Current minor version.
    /// </summary>
    /// <seealso cref="MajorVersion"/>
    const int MinorVersion = 0;
}
