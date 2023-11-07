/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmTime represents Time info in Omm.
/// </summary>
/// <remarks>
/// OmmTime encapsulates hour, minute, second, millisecond, microsecond and nanosecond information.
/// OmmTimeis a read only class.<br/>
/// This class is used for extraction of OmmTime info only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows setting of time in ElementList:<br/>
/// <code>
/// ElementList eList = new ElementList();
/// eList.AddTime( "my time", 23, 59, 59, 999, 999, 999 ).Complete();
/// </code>
/// The following code snippet sows extraction of time from ElementList.<br/>
/// <code>
/// void DecodeElementList( ElementList eList )
/// {
///     var eListIt = eList.GetEnumerator();
/// 	while(eListIt.MoveNext())
/// 	{
/// 		ElementEntry eEntry = eListIt.Current;
///
/// 		if (eEntry.Code != Data.DataCode.BLANK)
/// 		{
/// 			switch ( eEntry.getDataType() )
/// 			{
/// 				case DataType.DataTypes.TIME:
/// 					OmmTime ommTime = eEntry.OmmTimeValue();
/// 					int hour = ommTime.Hour;
/// 				break;
/// 			}
/// 		}
/// 	}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
public sealed class OmmTime : Data
{
    #region Public members

    /// <summary>
    /// Gets Hour. Range is 0 - 23 where 255 indicates blank.
    /// </summary>
    public int Hour { get => m_Time.Hour(); }

    /// <summary>
    /// Gets Minute. Range is 0 - 59 where 255 indicates blank.
    /// </summary>
    public int Minute { get => m_Time.Minute(); }

    /// <summary>
    /// Gets Second. Range is 0 - 60 where 255 indicates blank and 60 is to account for leap second.
    /// </summary>
    public int Second { get => m_Time.Second(); }

    /// <summary>
    /// Gets Millisecond. Range is 0 - 999 where 65535 indicates blank.
    /// </summary>
    public int Millisecond { get => m_Time.Millisecond(); }

    /// <summary>
    /// Gets Microsecond. Range is 0 - 999 where 2047 indicates blank.
    /// </summary>
    public int Microsecond { get => m_Time.Microsecond(); }

    /// <summary>
    /// Gets Nanosecond. Range is 0 - 999 where 2047 indicates blank.
    /// </summary>
    public int Nanosecond { get => m_Time.Nanosecond(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmTime"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Time.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmTime() 
    { 
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmTime;
        m_dataType = Access.DataType.DataTypes.TIME;
    }

    [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
    internal CodecReturnCode DecodeOmmTime(DecodeIterator dIter)
    {
        if (m_Time.Decode(dIter) == CodecReturnCode.SUCCESS)
        {
            Code = DataCode.NO_CODE;
            return CodecReturnCode.SUCCESS;
        }
        Code = DataCode.BLANK;
        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.Time m_Time = new();

    #endregion
}
