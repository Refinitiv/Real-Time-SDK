/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmUInt represents ulong value in Omm.
/// </summary>
/// <remarks>
/// OmmUInt is a read only class.<br/>
/// The usage of this class is limited to downcast operation only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows example decoding:<br/>
/// <code>
/// void decodeData( Data rcvdData )
/// {
/// 	if(rcvdData.Code != Data.DataCode.BLANK)
/// 	{
/// 		switch(rcvdData.DataType)
/// 		{
/// 		    case DataType.DataTypes.UINT:
/// 			    ulong val = ((OmmUInt)rcvdData).Value;
/// 			break;
/// 		}
/// 	}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
public sealed class OmmUInt : Data
{

    #region Public members

    /// <summary>
    /// Extracts contained unsigned long value.
    /// </summary>
    public ulong Value { get => (ulong)m_UInt.ToLong(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmUInt"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_UInt.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmUInt()
    {
        ReturnToPoolInternal = () => { m_objectManager.ReturnToPool(this); };
        DecodePrimitiveType = DecodeUInt;
        m_dataType = Access.DataType.DataTypes.UINT;
    }

    internal CodecReturnCode DecodeUInt(DecodeIterator dIter)
    {
        if (m_UInt.Decode(dIter) == CodecReturnCode.SUCCESS)
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

    private LSEG.Eta.Codec.UInt m_UInt = new();

    #endregion
}
