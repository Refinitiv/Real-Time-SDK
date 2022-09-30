/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;

namespace Refinitiv.Eta.Codec
{

    /// <summary>
	/// ETA entry for <seealso cref="Array"/> that can house only simple primitive types
	/// such as <seealso cref="Int"/>, <seealso cref="Real"/>, or <seealso cref="Date"/>.
	/// </summary>
	/// <seealso cref="Array" />
	sealed public class ArrayEntry
	{
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
		/// Creates <seealso cref="ArrayEntry"/>.
		/// </summary>
		/// <returns> ArrayEntry object
		/// </returns>
		/// <seealso cref="ArrayEntry"/>
        public ArrayEntry()
        {
        }

        /// <summary>
		/// Clears <seealso cref="ArrayEntry"/> object. Useful for object reuse during encoding.
		/// While decoding, <seealso cref="ArrayEntry"/> object can be reused without using <seealso cref="ArrayEntry.Clear()"/>.
		/// </summary>
        public void Clear()
		{
			_encodedData.Clear();
		}

        /// <summary>
		/// Decode an array entry.
		/// </summary>
		/// <param name="iter"> Decode iterator
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeArrayEntry(iter, this);
		}

        /// <summary>
        /// Encodes an <seealso cref="ArrayEntry"/> from pre-encoded data. This method
        /// expects the same <seealso cref="EncodeIterator"/> that was used with
        /// <seealso cref="Array.EncodeInit(EncodeIterator)"/>.
        /// 
        /// Typical use:<para />
        /// 1. Call Array.EncodeInit()<para />
        /// 2. Call ArrayEntry.Encode() for each entry using the same buffer<para />
        /// 3. Call Array.EncodeComplete()<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodePreencodedArrayEntry(iter, _encodedData);
		}

        /// <summary>
		/// Perform array item encoding of blank data.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeBlank(EncodeIterator iter)
		{
			_encodedData.Clear();

			return Encoders.EncodePreencodedArrayEntry(iter, _encodedData);
		}

        /// <summary>
		/// Perform array item encoding of an <seealso cref="Int"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="Int"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Int"/>
		public CodecReturnCode Encode(EncodeIterator iter, Int data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of a <seealso cref="UInt"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="UInt"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="UInt"/>
		public CodecReturnCode Encode(EncodeIterator iter, UInt data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <seealso cref="Float"/>.<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The Float.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Float"/>
        public CodecReturnCode Encode(EncodeIterator iter, Float data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <seealso cref="Double"/>.<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The Double.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Double"/>
        public CodecReturnCode Encode(EncodeIterator iter, Double data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of a <seealso cref="Real"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="Real"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Real"/>
		public CodecReturnCode Encode(EncodeIterator iter, Real data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of a <seealso cref="Date"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="Date"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Date"/>
		public CodecReturnCode Encode(EncodeIterator iter, Date data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of a <seealso cref="Time"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="Time"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Time"/>
		public CodecReturnCode Encode(EncodeIterator iter, Time data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of a <seealso cref="DateTime"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="DateTime"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="DateTime"/>
		public CodecReturnCode Encode(EncodeIterator iter, DateTime data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of a <seealso cref="Qos"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="Qos"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Qos"/>
		public CodecReturnCode Encode(EncodeIterator iter, Qos data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of a <seealso cref="State"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="State"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="State"/>
		public CodecReturnCode Encode(EncodeIterator iter, State data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of an <seealso cref="Enum"/>.<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="Enum"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Enum"/>
		public CodecReturnCode Encode(EncodeIterator iter, Enum data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
		/// Perform array item encoding of a <seealso cref="Buffer"/>.<para />
		/// 
		/// The length of the buffer should be the same as the itemLength of the array;
		/// only the shorter of the two will be copied.
		/// The entry is encoded as blank when the data is null or when the data length is 0.
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The <seealso cref="Buffer"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Buffer"/>
		public CodecReturnCode Encode(EncodeIterator iter, Buffer data)
		{
			if ((data == null) || (data.Length == 0))
			{
				_encodedData.Clear();
				return Encoders.EncodeArrayEntry(iter, _encodedData);
			}
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Gets or sets raw data contents of the array entry. Set by Decode() method during
        /// decode or set with encoded content and used with the Encode(iter) method.
        /// </summary>
        /// <returns> encodedData </returns>
        public Buffer EncodedData
        {
            get
            {
                return _encodedData;
            }

            set
            {
                Debug.Assert(value != null, "encodedData must be non-null");

                _encodedData.CopyReferences(value);
            }
        }
	}
}