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
/// OmmInt represents <see cref="long"/> value in Omm.
/// </summary>
///
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
///
/// OmmInt is a read only class.
///
/// The usage of this class is limited to downcast operation only.
///
/// All methods in this class are SingleThreaded.
///
/// <seealso cref="Data"/>
///	<seealso cref="EmaBuffer"/>
///
public sealed class OmmInt : Data
{
    #region Public members

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => DataTypes.INT; }

    /// <summary>
    /// Extracts contained long value.
    /// </summary>
    public long Value { get => m_Int.ToLong(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmInt"/> instance.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Int.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmInt() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
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
