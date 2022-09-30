/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{
    /// <summary>
	/// An entry for ETA <seealso cref="ElementList"/> that can house any <seealso cref="DataTypes"/>,
	/// including primitive types, set-defined types, or container types.
	/// If <seealso cref="ElementEntry"/> is a part of updating information and contains a
	/// primitive type, any previously stored or displayed data is replaced.
	/// If the <seealso cref="ElementEntry"/> contains another container type, action values
	/// associated with that type indicate how to modify data.
	/// </summary>
	/// <seealso cref="ElementList"/>
	sealed public class ElementEntry
	{
		internal readonly Buffer _name = new Buffer();
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
        /// Creates <seealso cref="ElementEntry"/>.
        /// </summary>
        /// <returns> <seealso cref="ElementEntry"/> object
        /// </returns>
        /// <seealso cref="ElementEntry"/>
        public ElementEntry()
        {
        }

        /// <summary>
        /// Clears <seealso cref="ElementEntry"/> object. Useful for object reuse during
        /// encoding. While decoding, <seealso cref="ElementEntry"/> object can be reused
        /// without using <seealso cref="ElementEntry.Clear()"/>.
        /// </summary>
        public void Clear()
		{
			_name.Clear();
			DataType = 0;
			_encodedData.Clear();
		}

        /// <summary>
		/// Encodes <seealso cref="ElementEntry"/> from pre-encoded data.
		/// 
		/// This method expects the same <seealso cref="EncodeIterator"/> that was used with
		/// <seealso cref="ElementList.EncodeInit(EncodeIterator, LocalElementSetDefDb, int)"/>
		/// <seealso cref="ElementEntry.Name"/> and <seealso cref="ElementEntry.DataType"/> must be
		/// properly populated.
		/// 
		/// Set encodedData with pre-encoded data before calling this method.
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeElementEntry(iter, this, null);
		}

        /// <summary>
		/// Encode a single element as blank.
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeBlank(EncodeIterator iter)
		{
			_encodedData.Clear();

			return Encoders.EncodeElementEntry(iter, this, null);
		}

        /// <summary>
		/// Encode a single element as <seealso cref="Int"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Int"/>
		public CodecReturnCode Encode(EncodeIterator iter, Int data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <seealso cref="UInt"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="UInt"/>
		public CodecReturnCode Encode(EncodeIterator iter, UInt data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <seealso cref="Float"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Float"/>
        public CodecReturnCode Encode(EncodeIterator iter, Float data)
        {
            return Encoders.EncodeElementEntry(iter, this, data);
        }

        /// <summary>
        /// Encode a single element as as <seealso cref="Double"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Double"/>
        public CodecReturnCode Encode(EncodeIterator iter, Double data)
        {
            return Encoders.EncodeElementEntry(iter, this, data);
        }

        /// <summary>
		/// Encode a single element as <seealso cref="Real"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Real"/>
		public CodecReturnCode Encode(EncodeIterator iter, Real data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <seealso cref="Date"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Date"/>
		public CodecReturnCode Encode(EncodeIterator iter, Date data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <seealso cref="Time"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Time"/>
		public CodecReturnCode Encode(EncodeIterator iter, Time data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <see cref="DateTime"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="DateTime"/>
		public CodecReturnCode Encode(EncodeIterator iter, DateTime data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <see cref="Qos"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.</param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Qos"/>
		public CodecReturnCode Encode(EncodeIterator iter, Qos data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <see cref="State"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="State"/>
		public CodecReturnCode Encode(EncodeIterator iter, State data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <see cref="Enum"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Enum"/>
		public CodecReturnCode Encode(EncodeIterator iter, Enum data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
		/// Encode a single element as <see cref="Buffer"/>.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Buffer"/>
		public CodecReturnCode Encode(EncodeIterator iter, Buffer data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encodes an <seealso cref="ElementEntry"/> from a complex type, such as a container type or an array.
        /// 
        /// Used for aggregate elements, such as element lists.
        /// 
        /// Typical use:<para />
        /// 1. Call EncodeInit()<para />
        /// 2. Call one or more encoding methods for the complex type using the
        /// same buffer<para />
        /// 3. Call EncodeComplete()<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="encodingMaxSize"> max expected encoding size of element data
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        public CodecReturnCode EncodeInit(EncodeIterator iter, int encodingMaxSize)
		{
			return Encoders.EncodeElementEntryInit(iter, this, encodingMaxSize);
		}

        /// <summary>
		/// Completes encoding an <see cref="ElementEntry"/> for a complex type.
		/// 
		/// Typical use:<para />
		/// 1. Call EncodeInit()<para />
		/// 2. Call one or more encoding methods for the complex type using the
		/// same buffer<para />
		/// 3. Call EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="success"> If true - successfully complete the aggregate, if false -
		///            remove the aggregate from the buffer.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeElementEntryComplete(iter, success);
		}

        /// <summary>
		/// Decode a single element.
		/// </summary>
		/// <param name="iter"> The iterator used to parse the element list.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeElementEntry(iter, this);
		}

        /// <summary>
		/// Gets or sets <seealso cref="Buffer"/> containing the name associated with this
		/// <seealso cref="ElementEntry"/>. Element names are defined outside of ETA, typically
		/// as part of a domain model specification or dictionary. It Is possible for
		/// a name to be empty; however this provides no identifying information for
		/// the element.
		/// </summary>
        public Buffer Name
        {
            get
            {
                return _name;
            }

            set
            {
                (_name).CopyReferences(value);
            }
        }


        /// <summary>
		/// Gets or sets the <see cref="DataTypes"/> of this <see cref="ElementEntry"/>'s contents.
		/// Must be in the range of <see cref="DataTypes.INT"/> - 255.
		/// <ul>
		/// <li>
		/// While encoding, set this to the enumerated value of the target type.</li>
		/// <li>
		/// While decoding, dataType describes the type of contained data so that the
		/// correct decoder can be used. If set-defined data is used, dataType will
		/// indicate any specific <see cref="DataTypes"/> information as defined in the set
		/// definition.</li>
		/// </ul>
		/// </summary>
		public int DataType { get; set; }

        /// <summary>
		/// Gets or sets encoded content in this <see cref="ElementEntry"/>. If populated on encode
		/// methods, this indicates that data is pre-encoded and encData copies while encoding.
		/// While decoding, this refers to the encoded ElementEntry's payload.
		/// </summary>
		public Buffer EncodedData
		{
            get
            {
                return _encodedData;
            }

            set
            {
                (_encodedData).CopyReferences(value);
            }
		}
	}
}