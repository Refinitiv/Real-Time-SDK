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
/// OmmEnum represents <see cref="ushort"/> value in Omm. The enumeration is the meaning of the ushort value.
/// </summary>
///
/// <example>
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
/// </example>
///
/// OmmEnum is a read only class.
///
/// The usage of this class is limited to downcast operation only.
///
/// All methods in this class are SingleThreaded.
///
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
///
public sealed class OmmEnum : Data
{

    #region Public members

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => Access.DataType.DataTypes.ENUM; }

    /// <summary>
    /// Extracts contained ushort value.
    /// </summary>
    public ushort Value { get => (ushort)m_Enum.ToInt(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmEnum"/> instance.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Enum.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmEnum() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
    {
        if (CodecReturnCode.SUCCESS == m_Enum.Decode(dIter))
            Code = DataCode.NO_CODE;
        else
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
