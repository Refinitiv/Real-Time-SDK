/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access;

/// <summary>
/// Data class is a parent abstract class defining common interfaces for all Data type classes.
/// </summary>
///
/// <remarks>
/// <para>
/// All classes representing OMM Data inherit from this class.</para>
///
/// <para>
/// Objects of this class are intended to be short lived or rather transitional.</para>
///
/// <para>
/// Objects of this class are not cache-able.</para>
///
/// <para>
/// All methods in this class are SingleThreaded.</para>
///
/// </remarks>
public abstract class Data
{

    #region Public members

    /// <summary>
    /// DataCode represents special state of Data.
    /// </summary>
    public enum DataCode
    {
        /// <summary>
        /// Indicates no special code.  An application typically processes a valid
        /// <see cref="Data.DataType"/> value.
        /// </summary>
        NO_CODE = 0,


        /// <summary>
        /// Indicates the value is unspecified.  An application typically sets the blank
        /// code when needing to initialize or clear a field.
        /// </summary>
        BLANK = 1
    };

    ///	<summary>
    ///	The DataType, which is the type of Omm data.
    ///	</summary>
    public abstract int DataType { get; }

    /// <summary>
    /// The Code, which indicates a special state of a DataType.
    /// </summary>
    public DataCode Code { get; internal set; }

    /// <summary>
    /// Returns the <see cref="Code"/> value in a string format.
    /// </summary>
    /// <returns>textual representation of the <see cref="Code"/> value</returns>
    public string CodeAsString()
    {
        return Code switch
        {
            DataCode.NO_CODE => NOCODE_STRING,
            DataCode.BLANK => BLANK_STRING,
            _ => DEFAULTCODE_STRING + Code
        };
    }

    /// <summary>
    /// Returns a buffer that in turn provides an alphanumeric null-terminated
    /// hexadecimal string representation.
    /// </summary>
    /// <returns>EmaBuffer with this object's hex representation</returns>
    public EmaBuffer AsHex()
    {
        Span<byte> bytesInBuffer = new Span<byte>(m_bodyBuffer!.Data().Contents, m_bodyBuffer.Position, m_bodyBuffer.Length);
        if (_asHex == null)
            _asHex = new EmaBuffer(bytesInBuffer);
        else
            _asHex.CopyFrom(bytesInBuffer);

        return _asHex;
    }

    #endregion

    #region Implementation details

    internal const int ENCODE_RSSL_BUFFER_INIT_SIZE = 4096;

    internal const string NOCODE_STRING = "NoCode";

    internal const string BLANK_STRING = "(blank data)";

    internal const string DEFAULTCODE_STRING = "Unknown DataCode value ";

    internal StringBuilder m_ToString = new StringBuilder();

    internal DataCode m_DataCode = DataCode.NO_CODE;

    internal EmaBuffer? _asHex;

    internal DecodeIterator m_decodeIterator = new();

    internal DecodeIterator? _rsslNestedMsgDecodeIter;

    internal int m_MajorVersion = Codec.MajorVersion();

    internal int m_MinorVersion = Codec.MinorVersion();


    internal bool m_hasDecodedDataSet = false;

    internal Buffer? m_bodyBuffer;

    internal EmaObjectManager? m_objectManager;
    internal bool m_inPool = false;
    internal bool m_ownedByPool = false;

    internal abstract CodecReturnCode Decode(DecodeIterator dIter);

    internal virtual CodecReturnCode Decode(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
    {
        return CodecReturnCode.FAILURE;
    }

    internal Encoder? Encoder { get; set; }

    internal abstract string ToString(int indent);

    internal virtual void ClearInt()
    {
        m_bodyBuffer = null;
        m_decodeIterator.Clear();
        m_hasDecodedDataSet = false;
        if (Encoder != null)
            Encoder!.Clear();
    }

    internal void ReturnToPool()
    {
        ClearInt();
        if (m_ownedByPool && !m_inPool)
        {
            m_inPool = true;
            m_objectManager!.ReturnToPool(this);
        }
    }

    #endregion
}
