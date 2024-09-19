/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */


using System;

using LSEG.Eta.Codec;


namespace LSEG.Ema.Access;

/// <summary>
/// OmmFloat represents float value in Omm.
/// </summary>
/// <remarks>
/// OmmFloat is a read only class.<br/>
/// The usage of this class is limited to downcast operation only.<br/>
/// All methods in this class are \ref single threaded.<br/>
/// <example>
/// The following code snippet shows example decoding usage:<br/>
/// <code>
/// void DecodeData( Data rcvdData )
/// {
/// 	if ( rcvdData.Code != Data.DataCode.BLANK )
/// 		switch ( rcvdData.DataType )
/// 		{
/// 		case DataTypes.FLOAT:
/// 			float value = ((OmmFloat)rcvdData).Value;
/// 			break;
/// 		}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
public sealed class OmmFloat : Data
{
    #region Public members

    /// <summary>
    /// Extracts contained float value.
    /// </summary>
    public float Value { get => m_Float.ToFloat(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmFloat"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Float.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmFloat()
    {
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmFloat;
        m_dataType = Access.DataType.DataTypes.FLOAT;
    }

    internal CodecReturnCode DecodeOmmFloat(DecodeIterator dIter)
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
