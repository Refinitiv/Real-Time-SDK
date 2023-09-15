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
/// OmmAscii represents Ascii string value in Omm.
/// </summary>
///
/// <code>
///
/// void DecodeData(Data rcvdData )
/// {
/// 	if ( rcvdData.Code != Data.DataCode.BLANK )
/// 		switch ( rcvdData.DataType )
/// 		{
/// 		    case DataType.DataTypes.ASCII:
/// 			    string value = ((OmmAscii)rcvdData).Ascii;
/// 			break;
/// 		}
/// }
/// </code>
///
/// \remark OmmAscii is a read only class.
/// \remark The usage of this class is limited to downcast operation only.
/// \remark All methods in this class are \ref SingleThreaded.
///
public sealed class OmmAscii : Data
{
    #region Public members

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => Access.DataType.DataTypes.ASCII; }

    /// <summary>
    /// Extracts contained ASCII string value.
    /// </summary>
    public string Value
    {
        get =>
            (m_Ascii.Length > 0)
            ? m_Ascii.ToString()
            : string.Empty;
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmAscii"/> instance.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return Data.BLANK_STRING;
        else
        {
            if (m_Ascii.Length == 0)
                return string.Empty;
            else
                return m_Ascii.ToString();
        }
    }

    #endregion

    #region Implementation details

    internal OmmAscii() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
    {
        if (CodecReturnCode.SUCCESS == m_Ascii.Decode(dIter))
            Code = DataCode.NO_CODE;
        else
            Code = DataCode.BLANK;

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.Buffer m_Ascii = new();

    #endregion
}
