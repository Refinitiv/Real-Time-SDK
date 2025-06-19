/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.AnsiPage;

/// <summary>
/// The <see cref="PageUpdate"/> interface provide the ability to track updates that are made to
/// the <see cref="PageCell"/> of an ANSI page.
/// </summary>
///
/// <remarks>
/// <para>
/// This, among other things, facilitates the capability for a source application to
/// publish only the portions of the page image that have been modified. Each PageUpdate
/// consists of three numeric values.</para>
///
/// <para>
/// The first designates the row on which the update has occurred, and the remaining two
/// identify the beginning and ending columns of the update.  More specifically, the
/// column of the first character in the update marks the beginning column, while the
/// ending column is marked as one column beyond the last character in the update.</para>
///
/// <para>
/// For example, an update beginning on column 9 and continuing through column 13 would
/// have a value of 9 for the beginning column and a value of 14 for the ending column.
/// Subtracting the ending column value from the beginning column value yields the total
/// number of characters in the update; therefore, this update would be 5 characters long.
/// </para>
///
/// </remarks>
///
/// <seealso cref="Page"/>
/// <seealso cref="PageCell"/>
public sealed class PageUpdate : ICloneable
{
    private short m_nRow;
    private short m_nBeginningColumn;
    private short m_nEndingColumn;

    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Makes a copy of this PageUpdate. Changes to the copy will not affect the original and vice versa.
    /// </summary>
    ///
    /// <returns>copy of this PageUpdate.</returns>
    public PageUpdate Clone()
    {
        return new PageUpdate(m_nRow, m_nBeginningColumn, m_nEndingColumn);
    }

    /// <summary>
    /// PageUpdate() constructor is used to create a PageUpdate object
    /// </summary>
    ///
    /// <param name="nRow">Row that the PageUpdate object will be initialized to.</param>
    /// <param name="nBeginningColumn">Beginning column to initialize the PageUpdate object.</param>
    /// <param name="nEndingColumn">Ending column to initialize the PageUpdate object.</param>
    public PageUpdate(short nRow, short nBeginningColumn, short nEndingColumn)
    {
        m_nRow = nRow;
        m_nBeginningColumn = nBeginningColumn;
        m_nEndingColumn = nEndingColumn;
    }

    /// <summary>
    /// Default constructor
    /// </summary>
    public PageUpdate()
    {
        m_nRow = 0;
        m_nBeginningColumn = 0;
        m_nEndingColumn = 0;
    }

    /// <summary>
    /// Used to determine the column where the update begins.
    /// </summary>
    ///
    /// <returns>the column where the update begins.</returns>
    public short GetBeginningColumn()
    {
        return m_nBeginningColumn;
    }

    /// <summary>
    /// Used to determine the column where the update ends.
    /// </summary>
    ///
    /// <returns>the column which is one beyond where the update ends.</returns>
    public short GetEndingColumn()
    {
        return m_nEndingColumn;
    }

    /// <summary>
    /// Used to determine the row.
    /// </summary>
    ///
    /// <returns>the row of the PageUpdate object.</returns>
    public short GetRow()
    {
        return m_nRow;
    }

    /// <summary>
    /// Used to modify the column on which the update begins.
    /// </summary>
    ///
    /// <param name="nBeginningColumn">Column the update begins on.</param>
    public void SetBeginningColumn(short nBeginningColumn)
    {
        m_nBeginningColumn = nBeginningColumn;
    }

    /// <summary>
    /// Used to modify the column on which the update ends.
    /// </summary>
    ///
    /// <param name="nEndingColumn">One column beyond where the update ends on.</param>
    public void SetEndingColumn(short nEndingColumn)
    {
        m_nEndingColumn = nEndingColumn;
    }

    /// <summary>
    /// Used to modify the row on which the update appears.
    /// </summary>
    ///
    /// <param name="nRow">Row the update appears on.</param>
    public void SetRow(short nRow)
    {
        m_nRow = nRow;
    }
}
