/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;


namespace LSEG.Ema.Access;


/// <summary>
/// OmmRmtes represents Rmtes string value in Omm.
/// </summary>
/// <remarks>
/// OmmRmtes is a read only class.<br/>
/// This class is used for extraction of RMTES info only.<br/>
/// All methods in this class are single threaded.<br/>
/// </remarks>
/// <seealso cref="Data"/>
public sealed class OmmRmtes : Data
{
    #region Public members

    /// <summary>
    /// Extracts contained RmtesBuffer value.
    /// </summary>
    public RmtesBuffer Value { get => m_RmtesBuffer; }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmRmtes"/> object.</returns>
    public override string ToString()
    {
        if (Code == DataCode.BLANK)
            return BLANK_STRING;
        else
            return m_RmtesBuffer.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmRmtes()
    {
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmRmtes;
        m_dataType = Access.DataType.DataTypes.RMTES;
    }

    internal CodecReturnCode DecodeOmmRmtes(DecodeIterator dIter)
    {
        if (m_RmtesBuffer.m_ApplyToCache)
            m_RmtesBuffer.Clear();

        if (m_RmtesBuffer.m_Buffer.Decode(dIter) == CodecReturnCode.SUCCESS)
            Code = DataCode.NO_CODE;
        else
            Code = DataCode.BLANK;

        m_bodyBuffer = m_RmtesBuffer.m_Buffer;

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private RmtesBuffer m_RmtesBuffer = new RmtesBuffer();

    #endregion

}
