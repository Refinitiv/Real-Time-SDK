/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access;

///  <summary>
///  OmmFloat represents float value in Omm.
///  </summary>
///
///  <code>
///  void DecodeData( Data rcvdData )
///  {
///  	if ( rcvdData.Code != Data.DataCode.BLANK )
///  		switch ( rcvdData.DataType )
///  		{
///  		case DataTypes.FLOAT:
///  			float value = ((OmmFloat)rcvdData).Value;
///  			break;
///  		}
///  }
///  </code>
///
///  \remark OmmFloat is a read only class.
///  \remark The usage of this class is limited to downcast operation only.
///  \remark All methods in this class are \ref SingleThreaded.
///
///  <seealso cref="Data"/>
///  <seealso cref="EmaBuffer"/>
///
public sealed class OmmFloat : Data
{
    #region Public members

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => Access.DataType.DataTypes.FLOAT; }

    /// <summary>
    /// Extracts contained float value.
    /// </summary>
    public float Value { get => m_Float.ToFloat(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmFloat"/> instance.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Float.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmFloat() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
    {
        if (CodecReturnCode.SUCCESS == m_Float.Decode(dIter))
            Code = DataCode.NO_CODE;
        else
            Code = DataCode.BLANK;

        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.Float m_Float = new();

    #endregion
}
