/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// ANSI Decoder update list entry. Defines row and column where update occurs.
/// </summary>
/// <seealso cref="AnsiDecoder.QaDecode(LSEG.Eta.Ansi.PageType, Stream, LSEG.Eta.Ansi.ListType)"/>
public sealed class UpdateType : ICloneable
{
    /// <summary>
    /// Row on which the update appears.
    /// </summary>
    public short Row;

    /// <summary>
    /// Beginning column of update.
    /// </summary>
    public short UpdateColBegin;

    /// <summary>
    /// Ending column of update.
    /// </summary>
    public short UpdateColEnd;

    /// <summary>
    /// Returns a deep copy of this UpdateType object.
    /// </summary>
    /// <returns>deep copy of this UpdateType object.</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a deep copy of this UpdateType object.
    /// </summary>
    /// <returns>deep copy of this UpdateType object.</returns>
    public UpdateType Clone()
    {
        UpdateType update = new UpdateType();
        update.Row = Row;
        update.UpdateColBegin = UpdateColBegin;
        update.UpdateColEnd = UpdateColEnd;
        return update;
    }
}
