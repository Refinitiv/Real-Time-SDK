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
///
/// <example>
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
/// </example>
///
/// OmmBuffer is a read only class.
/// The usage of this class is limited to downcast operation only.
/// All methods in this class are \ref SingleThreaded.
///
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
///
public sealed class OmmBuffer : Data
{
    #region Public members

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => DataTypes.BUFFER; }

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
    /// <returns>string representing current <see cref="OmmBuffer"/> instance.</returns>
    public override string ToString()
    {
        if (Code == DataCode.BLANK)
            return BLANK_STRING;

        return EmaUtility.AsHexString(AsHex());
    }

    #endregion

    #region Implementation details

    internal OmmBuffer() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
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
