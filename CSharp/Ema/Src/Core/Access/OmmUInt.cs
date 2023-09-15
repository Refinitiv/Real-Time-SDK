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
/// OmmUInt represents ulong value in Omm.
/// </summary>
///
/// <remarks>
/// <example>
/// void decodeData( Data rcvdData )
/// {
/// 	if(rcvdData.Code != Data.DataCode.BLANK)
/// 	{
/// 		switch(rcvdData.DataType)
/// 		{
/// 		    case DataType.DataTypes.UINT:
/// 			    ulong val = ((OmmUInt)rcvdData).Value;
/// 			break;
/// 		}
/// 	}
/// }
/// </example>
///
/// <para>
/// OmmUInt is a read only class.</para>
///
/// <para>
/// The usage of this class is limited to downcast operation only.</para>
///
/// <para>
/// All methods in this class are SingleThreaded.</para>
/// </remarks>
/// <seealso cref="Data"/>
public sealed class OmmUInt : Data
{

    #region Public members

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => Access.DataType.DataTypes.UINT; }

    /// <summary>
    /// Extracts contained unsigned long value.
    /// </summary>
    public ulong Value { get => (ulong)m_UInt.ToLong(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmUInt"/> instance.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_UInt.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmUInt() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
    {
        if (CodecReturnCode.SUCCESS == m_UInt.Decode(dIter))
            Code = DataCode.NO_CODE;
        else
            Code = DataCode.BLANK;

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.UInt m_UInt = new();

    #endregion
}
