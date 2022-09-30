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
	/// An entry for the ETA <see cref="Vector"/> that can house other Container Types
	/// only. <see cref="Vector"/> is a uniform type, where <see cref="Vector.ContainerType"/>
	/// indicates the single type housed in each entry. Each entry has an associated
	/// action which informs the user of how to apply the data contained in the entry
	/// </summary>
	/// <seealso cref="VectorEntryActions"/>
	/// <seealso cref="VectorEntryFlags"/>
	/// <seealso cref="Vector"/>
	sealed public class VectorEntry
	{
		internal readonly int MAX_INDEX = 0x3fffffff;
		internal System.UInt32 _index;
		internal readonly Buffer _permData = new Buffer();
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
        /// Creates <see cref="VectorEntry"/>.
        /// </summary>
        /// <returns> VectorEntry object
        /// </returns>
        /// <seealso cref="VectorEntry"/>
        public VectorEntry()
        {
        }

        /// <summary>
		/// Clears <see cref="VectorEntry"/> object. Useful for object reuse during
		/// encoding. While decoding, <see cref="VectorEntry"/> object can be reused
		/// without using <see cref="Clear()"/>.
		/// </summary>
		public void Clear()
		{
			Flags = 0;
			Action = 0;
			_index = 0;
			_permData.Clear();
			_encodedData.Clear();
		}

        /// <summary>
		/// Encodes single vector item, "moving" to the next row if necessary. Must
		/// be called after EncodeSetDefsComplete() and/or
		/// EncodeSummaryDataComplete().
		/// 
		/// Typical use:<para />
		/// 1. Call Vector.EncodeInit()<para />
		/// 2. Call VectorEntry.Encode() for each entry using the same buffer<para />
		/// 3. Call Vector.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeVectorEntry(iter, this);
		}

        /// <summary>
		/// Prepare a single vector item for encoding. Must be called after
		/// EncodeSetDefsComplete() and/or EncodeSummaryDataComplete().
		/// 
		/// Typical use:<para />
		/// 1. Call Vector.EncodeInit()<para />
		/// 2. Call VectorEntry.EncodeInit()..VectorEntry.EncodeComplete() for each
		/// entry using the same buffer<para />
		/// 3. Call Vector.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator </param>
		/// <param name="maxEncodingSize"> max encoding size of the data
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeInit(EncodeIterator iter, int maxEncodingSize)
		{
			return Encoders.EncodeVectorEntryInit(iter, this, maxEncodingSize);
		}

        /// <summary>
		/// Completes a vector element encoding.
		/// 
		/// Typical use:<para />
		/// 1. Call Vector.EncodeInit()<para />
		/// 2. Call VectorEntry.EncodeInit()..VectorEntry.EncodeComplete() for each
		/// entry using the same buffer<para />
		/// 3. Call Vector.EncodeComplete()<para />
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
			return Encoders.EncodeVectorEntryComplete(iter, success);
		}

        /// <summary>
		/// Decode a vector row.
		/// </summary>
		/// <param name="iter"> Decode iterator
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeVectorEntry(iter, this);
		}

        /// <summary>
		/// Checks the presence of the Permission Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> true - if present; false - if not present </returns>
		public bool CheckHasPermData()
		{
			return (Flags & VectorEntryFlags.HAS_PERM_DATA) > 0 ? true : false;
		}

        /// <summary>
		/// Applies the Permission Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		public void ApplyHasPermData()
		{
			Flags |= VectorEntryFlags.HAS_PERM_DATA;
		}

        /// <summary>
		/// Gets or sets all the flags applicable to this vector entry
		/// </summary>
		public VectorEntryFlags Flags { get; set; }

        /// <summary>
		/// Sets or sets Vector action (helps to manage change processing rules and informs the
		/// consumer of how to apply the information contained in the entry).
		/// Must be in the range of 0 - 15.
		/// </summary>
		public VectorEntryActions Action { get; set; }

        /// <summary>
		/// Gets or sets 0-base entry index. Must be in the range of 0 - 1073741823.
		/// </summary>
		public System.UInt32 Index
		{
            get
            {
                return _index;
            }

            set
            {
                Debug.Assert(value >= 0 && value <= 1073741823, "index is out of range (0-1073741823)");

                _index = value;
            }
		}

        /// <summary>
		/// Gets or sets permission expression for this entry.
		/// </summary>
		public Buffer PermData
		{
            get
            {
                return _permData;
            }

            set
            {
                (_permData).CopyReferences(value);
            }
		}

        /// <summary>
		/// Gets or sets encoded data to be applied for the entry.
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