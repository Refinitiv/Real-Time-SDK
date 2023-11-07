/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using System;
using LSEG.Eta.Codec;

namespace LSEG.Ema.Access;


/// <summary>
/// OmmBuffer represents a binary buffer value in Omm.
/// </summary>
/// <remarks>
/// OmmBuffer is a read only class.<br/>
/// The usage of this class is limited to downcast operation only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// Example usage:<br/>
/// <code>
/// void DecodeData(Data rcvdData )
/// {
/// 	if ( rcvdData.Code != Data.DataCode.BLANK )
/// 		switch ( rcvdData.DataType )
/// 		{
/// 		    case DataType.DataTypes.BUFFER:
/// 			    EmaBuffer value = ((OmmBuffer)rcvdData).Buffer;
/// 			break;
/// 		}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
///
public sealed class OmmBuffer : Data
{
    #region Public members

    /// <summary>
    /// Gets the underlying <see cref="EmaBuffer"/> 
    /// </summary>
    public EmaBuffer Value
    {
        get => m_EmaBuffer.AssignFrom(
            m_Buffer.Data().Contents,
            m_Buffer.Position,
            m_Buffer.Length);
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmBuffer"/> object.</returns>
    public override string ToString()
    {
        if (Code == DataCode.BLANK)
            return BLANK_STRING;

        return EmaUtility.AsHexString(AsHex());
    }

    #endregion

    #region Implementation details

    internal OmmBuffer() 
    {
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmBuffer;
        m_dataType = DataTypes.BUFFER;
    }

    internal CodecReturnCode DecodeOmmBuffer(DecodeIterator dIter)
    {
        m_bodyBuffer = m_Buffer;
        if (m_Buffer.Decode(dIter) == CodecReturnCode.SUCCESS)
            Code = DataCode.NO_CODE;
        else
            Code = DataCode.BLANK;

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.Buffer m_Buffer = new();
    private EmaBuffer m_EmaBuffer = new();

    #endregion
}
