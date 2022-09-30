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
    /// 
    /// </summary>
	sealed public class MapEntry
	{
		internal readonly Buffer _permData = new Buffer();
		internal readonly Buffer _encodedKey = new Buffer();
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
		/// Creates <see cref="MapEntry"/>.
		/// </summary>
		/// <returns> MapEntry object
		/// </returns>
		/// <seealso cref="MapEntry"/>
        public MapEntry()
        {
        }

        /// <summary>
        /// Clears to default values.
        /// </summary>
		public void Clear()
		{
			Flags = 0;
			Action = 0;
			_permData.Clear();
			_encodedKey.Clear();
			_encodedData.Clear();
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeMapEntry(iter, this, null);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Int keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, UInt keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Real keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Date keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Time keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, DateTime keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Qos keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, State keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Enum keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Buffer keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, null, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Int keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, UInt keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Real keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Date keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Time keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, DateTime keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Qos keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, State keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Enum keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Buffer keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="success"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeMapEntryComplete(iter, this, success);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Decode(DecodeIterator iter, object keyData)
		{
			return Decoders.DecodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
		public bool CheckHasPermData()
		{
			return ((Flags & MapEntryFlags.HAS_PERM_DATA) > 0 ? true : false);
		}

        /// <summary>
        /// 
        /// </summary>
		public void ApplyHasPermData()
		{
			Flags = (Flags | MapEntryFlags.HAS_PERM_DATA);
		}

        /// <summary>
        /// 
        /// </summary>
		public MapEntryActions Action { get; set; }

        /// <summary>
        /// 
        /// </summary>
		public MapEntryFlags Flags { get; set; }

        /// <summary>
        /// 
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
        /// 
        /// </summary>
		public Buffer EncodedKey
		{
            get
            {
                return _encodedKey;
            }

            set
            {
                (_encodedKey).CopyReferences(value);
            }
		}

        /// <summary>
        /// Gets or sets encoded data
        /// </summary>
        /// <value>The <see cref="Buffer"/> containing encoded data</value>
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

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Float keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <returns></returns>
		public CodecReturnCode Encode(EncodeIterator iter, Double keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Float keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="keyData"></param>
        /// <param name="maxEncodingSize"></param>
        /// <returns></returns>
		public CodecReturnCode EncodeInit(EncodeIterator iter, Double keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}
	}

}