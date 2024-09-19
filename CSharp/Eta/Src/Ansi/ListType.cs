/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Maintains a list of updated regions.
/// </summary>
/// <seealso cref="AnsiDecoder.QaDecode(LSEG.Eta.Ansi.PageType, Stream, LSEG.Eta.Ansi.ListType)"/>
/// <seealso cref="UpdateType"/>
public sealed class ListType : ICloneable
{
    /// <summary>
    /// Num of entries in <see cref="UpdList"/>
    /// </summary>
    public short Index;

    /// <summary>
    /// An actual list of updated regions.
    /// </summary>
    public UpdateType[] UpdList;

    /// <summary>
    /// Instantiates a new list type.
    /// </summary>
    public ListType()
    {
        UpdList = new UpdateType[256];
        for (int i = 0; i < 256; i++)
            UpdList[i] = new UpdateType();
    }

    private ListType(short i)
    {
        UpdList = new UpdateType[256];
        Index = i;
    }

    /// <summary>
    /// Returns a copy of this list type.
    /// </summary>
    /// <returns>a deep copy of this list type</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a copy of this list type.
    /// </summary>
    /// <returns>a deep copy of this list type</returns>
    public ListType Clone()
    {
        ListType newList = new ListType(Index);
        for (int i = 0; i < 256; i++)
        {
            newList.UpdList[i] = UpdList[i].Clone();
        }
        return newList;
    }
}
