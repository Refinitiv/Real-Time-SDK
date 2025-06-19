/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access;


/// <summary>
/// OmmDateTime represents DateTime info in Omm.
/// </summary>
/// <remarks>
/// OmmDateTime encapsulates year, month, day, hour, minute, second, millisecond,
/// microsecond and nanosecond information.<br/>
/// /// OmmDateTime is a read only class.<br/>
/// This class is used for extraction of DateTime info only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows extraction of DateTime from ElementList.<br/>
/// <code>
/// void DecodeElementList( ElementList eList )
/// {
///     var eListIt = eList.GetEnumerator();
/// 	while ( eList.MoveNext() )
/// 	{
/// 		ElementEntry eEntry = eListIt.Current;
///
/// 		if ( eEntry.Code != Data.DataCode.BLANK )
/// 			switch ( eEntry.LoadType )
/// 			{
/// 				case DataType.DataTypes.DATETIME:
/// 					OmmDateTime ommDateTime = eEntry.OmmDateTimeValue();
/// 					int year = ommDateTime.Year;
/// 					...
/// 					int hour = ommDateTime.Hour;
/// 					break;
/// 			}
/// 	}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
///
public sealed class OmmDateTime : Data
{
    #region Public members

    /// <summary>
    /// Gets Year. Range is 0 - 4095 where 0 indicates blank.
    /// </summary>
    public int Year { get => m_DateTime.Year(); }

    /// <summary>
    /// Gets Month. Range is 0 - 12 where 0 indicates blank.
    /// </summary>
    public int Month { get => m_DateTime.Month(); }

    /// <summary>
    /// Gets Day. Range is 0 - 31 where 0 indicates blank.
    /// </summary>
    public int Day { get => m_DateTime.Day(); }

    /// <summary>
    /// Gets Hour. Range is 0 - 23 where 255 indicates blank.
    /// </summary>
    public int Hour { get => m_DateTime.Hour(); }

    /// <summary>
    /// Gets Minute. Range is 0 - 59 where 255 indicates blank.
    /// </summary>
    public int Minute { get => m_DateTime.Minute(); }

    /// <summary>
    /// Gets Second. Range is 0 - 60 where 255 indicates blank and 60 is to account for leap second.
    /// </summary>
    public int Second { get => m_DateTime.Second(); }

    /// <summary>
    /// Gets Millisecond. Range is 0 - 999 where 65535 indicates blank.
    /// </summary>
    public int Millisecond { get => m_DateTime.Millisecond(); }

    /// <summary>
    /// Gets Microsecond. Range is 0 - 999 where 2047 indicates blank.
    /// </summary>
    public int Microsecond { get => m_DateTime.Microsecond(); }

    /// <summary>
    /// Gets Nanosecond. Range is 0 - 999 where 2047 indicates blank.
    /// </summary>
    public int Nanosecond { get => m_DateTime.Nanosecond(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmDateTime"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_DateTime.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmDateTime() 
    { 
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmDateTime;
        m_dataType = Access.DataType.DataTypes.DATETIME;
    }

    [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
    internal CodecReturnCode DecodeOmmDateTime(DecodeIterator dIter)
    {
        CodecReturnCode decodeRetValue = m_DateTime.Decode(dIter);

        if (CodecReturnCode.SUCCESS == decodeRetValue)
            Code = DataCode.NO_CODE;
        else if (CodecReturnCode.INCOMPLETE_DATA == decodeRetValue)
            return CodecReturnCode.INCOMPLETE_DATA;
        else
        {
            Code = DataCode.BLANK;
            m_DateTime.Blank();
        }

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private Eta.Codec.DateTime m_DateTime = new();

    #endregion
}
