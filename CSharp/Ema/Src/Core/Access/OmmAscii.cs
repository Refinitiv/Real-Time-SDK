/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;

namespace LSEG.Ema.Access;


/// <summary>
/// OmmAscii represents Ascii string value in Omm.
/// </summary>
/// <remarks>
/// OmmAscii is a read only class.<br/>
/// The usage of this class is limited to downcast operation only.<br/>
/// All methods in this class are \ref single threaded.<br/>
/// <example>
/// The following code snippet is shows example decoding usage:<br/>
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
/// </example>
/// </remarks>
public sealed class OmmAscii : Data
{
    #region Public members

    /// <summary>
    /// Ascii value
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
    /// <returns>string representing current <see cref="OmmAscii"/> object.</returns>
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

    internal OmmAscii() 
    {
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeAscii;
        m_dataType = Access.DataType.DataTypes.ASCII;
    }

    internal CodecReturnCode DecodeAscii(DecodeIterator dIter)
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
