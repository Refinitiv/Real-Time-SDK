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
/// OmmDouble represents double in Omm.
/// </summary>
///
/// <example>
/// void DecodeData( Data rcvdData )
/// {
/// 	if ( rcvdData.Code != Data.DataCode.BLANK )
/// 		switch ( rcvdData.DataType )
/// 		{
/// 		case Access.DataType.DataTypes.DOUBLE :
/// 			double val = ((OmmDouble)rcvdData).Value;
/// 			break;
/// 		}
/// }
/// </example>
///
/// OmmDouble is a read only class.
///
/// The usage of this class is limited to downcast operation only.
///
/// All methods in this class are SingleThreaded.
///
/// <seealso cref="OmmArray"/>
/// <seealso cref="EmaBuffer"/>
///
public sealed class OmmDouble : Data
{
    #region Public members

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => Access.DataType.DataTypes.DOUBLE; }

    /// <summary>
    /// Extracts contained double value.
    /// </summary>
    public double Value { get => m_Double.ToDouble(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmDouble"/> instance.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Double.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmDouble() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
    {
        if (CodecReturnCode.SUCCESS == m_Double.Decode(dIter))
            Code = DataCode.NO_CODE;
        else
            Code = DataCode.BLANK;

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.Double m_Double = new();

    #endregion
}
