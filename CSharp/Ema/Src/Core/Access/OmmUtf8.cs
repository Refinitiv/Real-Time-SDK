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
/// OmmUtf8 represents Utf8 string value in Omm.
/// </summary>
/// <remarks>
/// <example>
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
/// </example>
///
/// <para>
/// OmmUtf8 is a read only class.</para>
///
/// <para>
/// The usage of this class is limited to downcast operation only.</para>
///
/// <para>
/// All methods in this class are SingleThreaded.</para>
///
/// </remarks>
///
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
///
public sealed class OmmUtf8 : Data
{
    #region Public Interface

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => Access.DataType.DataTypes.UTF8; }

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
    /// 
    /// </summary>
    /// <returns></returns>
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
    /// <returns>string representing current <see cref="OmmUtf8"/> instance.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
        {
            if (m_bodyBuffer is null || m_bodyBuffer.Length == 0)
                return string.Empty;
            else
                return m_bodyBuffer.ToString();
        }
    }

    #endregion

    #region Implementation details

    internal OmmUtf8() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
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
