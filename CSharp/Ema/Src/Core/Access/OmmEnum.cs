/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmEnum represents <see cref="ushort"/> value in Omm. The enumeration is the meaning of the ushort value.
/// </summary>
/// <remarks>
/// OmmEnum is a read only class.<br/>
/// The usage of this class is limited to downcast operation only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows example decoding usage:<br/>
/// <code>
/// void DecodeData( Data rcvdData )
/// {
/// 	if ( rcvdData.Code != Data.DataCode.BLANK )
/// 		switch ( rcvdData.DataType )
/// 		{
/// 		case DataTypes.ENUM:
/// 			ushort val = ((OmmEnum)rcvdData).Value;
/// 			break;
/// 		}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
///
public sealed class OmmEnum : Data
{

    #region Public members

    /// <summary>
    /// Extracts contained ushort value.
    /// </summary>
    public ushort Value { get => (ushort)m_Enum.ToInt(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmEnum"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Enum.ToString();
    }

    #endregion

    #region Implementation details
    

    internal OmmEnum()
    {
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmEnum;
        m_dataType = Access.DataType.DataTypes.ENUM;
    }

    [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
    internal CodecReturnCode DecodeOmmEnum(DecodeIterator dIter)
    {
        if (m_Enum.Decode(dIter) == CodecReturnCode.SUCCESS)
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

    private LSEG.Eta.Codec.Enum m_Enum = new();

    #endregion
}
