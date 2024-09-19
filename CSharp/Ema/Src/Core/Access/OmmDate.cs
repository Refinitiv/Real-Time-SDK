/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmDate represents Date info in Omm.
/// </summary>
/// <remarks>
/// OmmDate encapsulates year, month and day information.<br/>
/// OmmDate is a read only class.<br/>
/// This class is used for extraction of Date info only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows setting of date in ElementList;<br/>
/// <code>
/// ElementList eList;
/// eList.AddDate( "my date", 1999, 12, 31 ).Complete();
/// </code>
/// The following code snippet shows extraction of date from ElementList.<br/>
/// <code>
/// void DecodeElementList( ElementList eList )
/// {
///         foreach(ElementEntry eEntry in eList)
///         {
///             if ( eEntry.Code != Data.DataCode.BLANK )
///             switch ( eEntry.LoadType )
///             {
///                 case DataTypes.DATE:
///                     OmmDate ommDate = eEntry.OmmDateValue();
///                     int year = ommDate.Year;
///                 break;
///             }
///         }
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
public sealed class OmmDate : Data
{
    #region Public members

    /// <summary>
    /// Gets Year. Range is 0 - 4095 where 0 indicates blank.
    /// </summary>
    public int Year { get => m_Date.Year(); }

    /// <summary>
    /// Gets Month. Range is 0 - 12 where 0 indicates blank.
    /// </summary>
    public int Month { get => m_Date.Month(); }

    /// <summary>
    /// Gets Day. Range is 0 - 31 where 0 indicates blank.
    /// </summary>
    public int Day { get => m_Date.Day(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmDate"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Date.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmDate() 
    {
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmDate;
        m_dataType = Access.DataType.DataTypes.DATE;
    }

    [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
    internal CodecReturnCode DecodeOmmDate(DecodeIterator dIter)
    {
        CodecReturnCode decodeRetValue = m_Date.Decode(dIter);
        if (CodecReturnCode.SUCCESS == decodeRetValue)
            Code = DataCode.NO_CODE;
        else if (CodecReturnCode.INCOMPLETE_DATA == decodeRetValue)
            return CodecReturnCode.INCOMPLETE_DATA;
        else
        {
            Code = DataCode.BLANK;
            m_Date.Blank();
        }

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private Eta.Codec.Date m_Date = new();

    #endregion
}
