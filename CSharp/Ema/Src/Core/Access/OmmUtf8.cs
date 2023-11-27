/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using System;

namespace LSEG.Ema.Access;


/// <summary>
/// OmmUtf8 represents Utf8 string value in Omm.
/// </summary>
/// <remarks>
/// OmmUtf8 is a read only class.<br/>
/// The usage of this class is limited to downcast operation only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows example decoding:<br/>
/// <code>
/// void DecodeData( Data rcvdData )
/// {
/// 	if ( rcvdData.Code != Data.DataCode.BLANK )
/// 		switch ( rcvdData.DataType )
/// 		{
/// 		case DataType.DataTypes.UTF8:
/// 			EmaBuffer val = ((OmmUtf8)rcvdData).Value;
/// 			break;
/// 		}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
public sealed class OmmUtf8 : Data
{
    #region Public Interface

    /// <summary>
    /// Extracts contained EmaBuffer value.
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
    /// <returns>string representing current <see cref="OmmUtf8"/> object.</returns>
    public string AsString()
    {
        if (m_Buffer.Length == 0)
            return string.Empty;
        else
            return m_Buffer.ToString();
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmUtf8"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
        {
            if (m_bodyBuffer == null || m_bodyBuffer.Length == 0)
                return string.Empty;
            else
                return m_bodyBuffer.ToString();
        }
    }

    #endregion

    #region Implementation details

    internal OmmUtf8() 
    { 
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmUtf8;
        m_dataType = Access.DataType.DataTypes.UTF8;
    }

    internal CodecReturnCode DecodeOmmUtf8(DecodeIterator dIter)
    {
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
