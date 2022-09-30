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
    /// A ETA Array is a uniform primitive type that can contain 0 to N simple primitive entries,
    /// where zero entries indicates an empty Array.
    /// <para>
    /// Each Array entry can house only simple primitive types such as <seealso cref="Int"/>,
    /// <seealso cref="Real"/>, or <seealso cref="Date"/>. An array entry cannot house any container
    /// types or other <seealso cref="Array"/> types. This is a uniform type, where
    /// <seealso cref="Array.PrimitiveType"/> indicates the single, simple primitive type of
    /// each entry.
    /// 
    /// </para>
    /// <para>
    /// Array uses simple replacement rules for change management. When new entries
    /// are added, or any array entry requires a modification, all entries must be
    /// sent with the <seealso cref="Array"/>. This new <seealso cref="Array"/> entirely replaces any
    /// previously stored or displayed data.
    /// 
    /// </para>
    /// <para>
    /// An Array entry can be encoded from pre-encoded data or by encoding individual
    /// pieces of data as provided. The <seealso cref="Array"/> does not use a specific entry
    /// structure. When encoding, the application passes a pointer to the primitive
    /// type value (when data is not encoded) or a Buffer (containing the
    /// pre-encoded primitive).
    /// 
    /// </para>
    /// <para>
    /// <b>Example: Encoding of Array</b>
    /// </para>
    /// <para>
    /// The following code example demonstrates how to encode an <seealso cref="Array"/>. The
    /// array is set to encode unsigned integer entries, where the entries have a
    /// fixed length of two bytes each. The example encodes two array entries. The
    /// first entry is encoded from a primitive <seealso cref="UInt"/> type; the second entry
    /// is encoded from a <seealso cref="Buffer"/> containing a pre-encoded <seealso cref="UInt"/> type.
    /// 
    /// <ul class="blockList">
    /// <li class="blockList">
    /// 
    /// <pre>
    /// int retval;
    /// // Create Array
    /// Array array = new Array();
    /// 
    /// // Populate array prior to calling encodeInit()
    /// array.PrimitiveType = DataTypes.UINT;
    /// array.ItemLength = 2;
    /// 
    /// if (array.EncodeInit(encIter) &lt; CodecReturnCode.SUCCESS)
    /// {
    ///     // error condition
    /// }
    /// else
    /// {
    ///     // encode first entry from a UInt from a primitive type
    ///     retval = uint.Encode(encIter);
    /// 
    ///     // encode second entry from a UInt from pre-encoded integer contained in a
    ///     // buffer. This buffer.data should point to encoded int and the length
    ///     // should be number of bytes encoded.
    ///     retval = preencodedUint.Encode(encIter);
    /// }
    /// 
    /// // Complete array encoding. If success parameter is true, this will finalize
    /// // encoding.
    /// // If success parameter is false, this will roll back encoding prior to
    /// // encodeArrayInit
    /// 
    /// retval = array.EncodeComplete(encIter, success);
    /// </pre>
    /// 
    /// </li>
    /// </ul>
    /// 
    /// 
    /// </para>
    /// <para>
    /// <b>Example: Decoding of Array</b>
    /// </para>
    /// <para>
    /// The following example decodes an <seealso cref="Array"/> and each of its entries to the
    /// primitive value. This sample code assumes the contained primitive type is a
    /// <seealso cref="UInt"/>. Typically an application invokes the specific primitive decoder
    /// for the contained type or uses a switch statement to allow for a more generic
    /// array entry decoder. This example uses the same <seealso cref="DecodeIterator"/> when
    /// calling the primitive decoder method. An application could optionally use a
    /// new <seealso cref="DecodeIterator"/> by setting the encoded entry buffer on a new
    /// iterator. To simplify the example, some error handling is omitted.
    /// 
    /// <ul class="blockList">
    /// <li class="blockList">
    /// 
    /// <pre>
    /// int retval;
    /// 
    ///  //decode array
    /// if (array.Decode(decIter) >= <seealso cref="CodecReturnCode.SUCCESS"/>)
    /// {
    ///    //decode array entry
    ///    while((retval = arrayentry.Decode(decIter, entrybuffer)) != <seealso cref="CodecReturnCode.END_OF_CONTAINER"/>)
    ///    {
    ///          if(retval &lt; <seealso cref="CodecReturnCode.SUCCESS"/>)
    ///          {
    ///              //decoding failure tends to be unrecoverable
    ///          }
    ///          else
    ///          {
    ///              // decode array entry into primitive type 
    ///              // we can use the same decode iterator, or set the encoded
    ///              // entry buffer onto a new iterator 
    ///              retval = uInt.Decode(decIter);
    ///          }
    ///    }
    /// }
    /// 
    /// </pre>
    /// 
    /// </li>
    /// </ul>
    /// </para>
    /// </summary>
    sealed public class Array : IXMLDecoder
    {
		internal int _primitiveType;
		internal int _itemLength;
		internal readonly Buffer _encodedData = new Buffer(); // this member variable is set by internal implementation

        /// <summary>
		/// Creates <seealso cref="Array"/>.
		/// </summary>
		/// <returns> Array object
		/// </returns>
		/// <seealso cref="Array"/>
        public Array()
        {
        }

        /// <summary>
		/// Sets all members in <seealso cref="Array"/> to an initial value. Useful for object
		/// reuse during encoding. While decoding, <seealso cref="Array"/> object can be reused
		/// without using <seealso cref="Clear()"/>.
		/// </summary>
		public void Clear()
		{
			_primitiveType = 0;
			_itemLength = 0;
			_encodedData.Clear();
		}

        /// <summary>
        /// Is Array blank.
        /// </summary>
		public bool IsBlank { get; private set; }

        /// <summary>
        /// Prepares array for encoding.
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
        public CodecReturnCode EncodeInit(EncodeIterator iter)
		{
			if (!IsBlank)
			{
				return Encoders.EncodeArrayInit(iter, this);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

        /// <summary>
        /// Completes array encoding.
        /// 
        /// Typical use:<para />
        /// 1. Call Array.EncodeInit()<para />
        /// 2. Call ArrayEntry.Encode() for each entry using the same buffer<para />
        /// 3. Call Array.EncodeComplete()<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="success"> If true - successfully complete the aggregate,
        ///                if false - remove the aggregate from the buffer.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			if (!IsBlank)
			{
				return Encoders.EncodeArrayComplete(iter, success);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

        /// <summary>
        /// Decodes to XML the data next in line to be decoded with the iterator.
        /// Data that require dictionary lookups are decoded to hexadecimal strings.
        /// </summary>
        /// <param name="iter"> Decode iterator
        /// </param>
        /// <returns> The XML representation of the container
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeArray(iter, this);
		}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter">The iterator</param>
        /// <returns></returns>
        public string DecodeToXml(DecodeIterator iter)
        {
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.ARRAY, null, null, null, iter);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="iter"></param>
        /// <param name="dictionary"></param>
        /// <returns></returns>
        public string DecodeToXml(DecodeIterator iter, DataDictionary dictionary)
        {
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.ARRAY, null, dictionary, null, iter);
        }

        /// <summary>
		/// Gets or sets Primitive type for all items in the array. Primitive type must be from the
		/// <seealso cref="DataTypes"/> enumeration and must be greater than <seealso cref="DataTypes.UNKNOWN"/>
		/// and less than or equals to <seealso cref="DataTypes.BASE_PRIMITIVE_MAX"/>.
		/// primitiveType cannot be <seealso cref="DataTypes.ARRAY"/>.
		/// </summary>
		/// <seealso cref="DataTypes" />
		public int PrimitiveType
        {
            get => _primitiveType;
            
            set
            {
                Debug.Assert((value > DataTypes.UNKNOWN && value <= DataTypes.BASE_PRIMITIVE_MAX),
                    "primitiveType must be from the DataTypes enumeration and greater than UNKNOWN and less than or equal to BASE_PRIMITIVE_MAX.");

                Debug.Assert(value != DataTypes.ARRAY, "primitiveType in array can't be DataTypes.ARRAY.");

                _primitiveType = value;
                IsBlank = false;
            }
		}

        /// <summary>
        /// Gets or sets the number of items in the array
		/// If items are fixed length populate length here - otherwise make 0 for
		/// length specified item encoding. Must be in the range of 0 - 65535.
		/// </summary>
        public int ItemLength
        {
            get => _itemLength;
            
            set
            {
                Debug.Assert((value >= 0 && value <= System.UInt16.MaxValue), "itemLength is out of range (0-65535)");
                _itemLength = value;
                IsBlank = false;
            }
        }

        /// <summary>
		/// Raw data contents of the array. Set by Decode() method during decode.
		/// </summary>
		/// <returns> the encodedData
		/// </returns>
		/// <seealso cref="Buffer"/>
		public Buffer EncodedData()
		{
			return _encodedData;
		}

        internal void Blank()
        {
            Clear();
            IsBlank = true;
        }
    }

}