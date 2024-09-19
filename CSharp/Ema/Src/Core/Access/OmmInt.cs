/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmInt represents <see cref="long"/> value in Omm.
/// </summary>
/// <remarks>
/// OmmInt is a read only class.<br/>
/// The usage of this class is limited to downcast operation only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows example decoding usage:<br/>
/// <code>
/// void DecodeData( Data rcvdData )
/// {
///	 if ( rcvdData.Code != Data.DataCode.BLANK )
/// 		switch ( rcvdData.DataType )
///		{
///		case DataTypes.INT:
///			long value = ((OmmInt)rcvdData).Value;
///			break;
///		}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
///	<seealso cref="EmaBuffer"/>
///
public sealed class OmmInt : Data
{
    #region Public members

    /// <summary>
    /// Extracts contained long value.
    /// </summary>
    public long Value { get => m_Int.ToLong(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmInt"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Int.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmInt()
    {
        ReturnToPoolInternal = () => { m_objectManager.ReturnToPool(this); };
        DecodePrimitiveType = DecodeOmmInt;
        m_dataType = DataTypes.INT;
    }

    internal CodecReturnCode DecodeOmmInt(DecodeIterator dIter)
    {
        if (CodecReturnCode.SUCCESS == m_Int.Decode(dIter))
            Code = DataCode.NO_CODE;
        else
            Code = DataCode.BLANK;

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.Int m_Int = new();

    #endregion
}
