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
	/// An entry in a <see cref="Series"/> container that can house other container types
	/// only. <see cref="Series"/> is a uniform type, where <see cref="Series"/>.containerType
	/// indicates the single type housed in each entry. As entries are received, they
	/// are appended to any previously received entries.
	/// </summary>
	/// <seealso cref="Series"/>
    sealed public class SeriesEntry
    {
        internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
        /// Creates <see cref="SeriesEntry"/>.
        /// </summary>
        /// <returns> SeriesEntry object
        /// </returns>
        /// <seealso cref="SeriesEntry"/>
        public SeriesEntry()
        {
        }

        /// <summary>
		/// Clears <see cref="SeriesEntry"/> object. Useful for object reuse during
		/// encoding. While decoding, <see cref="SeriesEntry"/> object can be reused
		/// without using <see cref="Clear()"/>.
		/// </summary>
        public void Clear()
        {
            _encodedData.Clear();
        }

        /// <summary>
		/// Encodes single series item, "moving" to the next row if necessary. Must
		/// be called after EncodeSetDefsComplete() and/or
		/// EncodeSummaryDataComplete().
		/// 
		/// Typical use:<para />
		/// 1. Call Series.EncodeInit()<para />
		/// 2. Call SeriesEntry.Encode() for each entry using the same buffer<para />
		/// 3. Call Series.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
        public CodecReturnCode Encode(EncodeIterator iter)
        {
            return Encoders.EncodeSeriesEntry(iter, this);
        }

        /// <summary>
		/// Prepare a single series item for encoding. Must be called after
		/// EncodeSetDefsComplete() and/or EncodeSummaryDataComplete().
		/// 
		/// Typical use:<para />
		/// 1. Call Series.EncodeInit()<para />
		/// 2. Call SeriesEntry.EncodeInit()..SeriesEntry.EncodeComplete() for each
		/// entry using the same buffer<para />
		/// 3. Call Series.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator </param>
		/// <param name="maxEncodingSize"> max encoding size of the data
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
        public CodecReturnCode EncodeInit(EncodeIterator iter, int maxEncodingSize)
        {
            return Encoders.EncodeSeriesEntryInit(iter, this, maxEncodingSize);
        }

        /// <summary>
		/// Completes a series element encoding.
		/// 
		/// Typical use:<para />
		/// 1. Call Series.EncodeInit()<para />
		/// 2. Call SeriesEntry.EncodeInit()..SeriesEntry.EncodeComplete() for each
		/// entry using the same buffer<para />
		/// 3. Call Series.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="success"> If true - successfully complete the aggregate,
		///                if false - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
        public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
        {
            return Encoders.EncodeSeriesEntryComplete(iter, success);
        }

        /// <summary>
		/// Decode a series row.
		/// </summary>
		/// <param name="iter"> Decode iterator
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
        public CodecReturnCode Decode(DecodeIterator iter)
        {
            return Decoders.DecodeSeriesEntry(iter, this);
        }

        /// <summary>
		/// Gets or sets encoded content of this SeriesEntry. If set, this indicates that data is
		/// pre-encoded and encodedData will be copied while encoding.
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