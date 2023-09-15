/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{

    /// <summary>
    /// A ETA Array is a uniform primitive type that can contain 0 to N simple primitive entries,
    /// where zero entries indicates an empty Array.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Each Array entry can house only simple primitive types such as <see cref="Int"/>,
    /// <see cref="Real"/>, or <see cref="Date"/>. An array entry cannot house any container
    /// types or other <see cref="Array"/> types. This is a uniform type, where
    /// <see cref="Array.PrimitiveType"/> indicates the single, simple primitive type of
    /// each entry.
    /// 
    /// </para>
    /// <para>
    /// Array uses simple replacement rules for change management. When new entries
    /// are added, or any array entry requires a modification, all entries must be
    /// sent with the <see cref="Array"/>. This new <see cref="Array"/> entirely replaces any
    /// previously stored or displayed data.
    /// 
    /// </para>
    /// <para>
    /// An Array entry can be encoded from pre-encoded data or by encoding individual
    /// pieces of data as provided. The <see cref="Array"/> does not use a specific entry
    /// structure. When encoding, the application passes a pointer to the primitive
    /// type value (when data is not encoded) or a Buffer (containing the
    /// pre-encoded primitive).
    /// 
    /// </para>
    /// <para>
    /// <b>Example: Encoding of Array</b>
    /// </para>
    /// <para>
    /// The following code example demonstrates how to encode an <see cref="Array"/>. The
    /// array is set to encode unsigned integer entries, where the entries have a
    /// fixed length of two bytes each. The example encodes two array entries. The
    /// first entry is encoded from a primitive <see cref="UInt"/> type; the second entry
    /// is encoded from a <see cref="Buffer"/> containing a pre-encoded <see cref="UInt"/> type.
    /// </para>
    /// 
    /// <code>
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
    /// </code>
    /// 
    /// <para>
    /// <b>Example: Decoding of Array</b>
    /// </para>
    /// <para>
    /// The following example decodes an <see cref="Array"/> and each of its entries to the
    /// primitive value. This sample code assumes the contained primitive type is a
    /// <see cref="UInt"/>. Typically an application invokes the specific primitive decoder
    /// for the contained type or uses a switch statement to allow for a more generic
    /// array entry decoder. This example uses the same <see cref="DecodeIterator"/> when
    /// calling the primitive decoder method. An application could optionally use a
    /// new <see cref="DecodeIterator"/> by setting the encoded entry buffer on a new
    /// iterator. To simplify the example, some error handling is omitted.
    /// </para>
    /// 
    /// <code>
    /// int retval;
    /// 
    ///  //decode array
    /// if (array.Decode(decIter) >= CodecReturnCode.SUCCESS)
    /// {
    ///    //decode array entry
    ///    while((retval = arrayentry.Decode(decIter, entrybuffer)) != CodecReturnCode.END_OF_CONTAINER)
    ///    {
    ///          if(retval &lt; CodecReturnCode.SUCCESS)
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
    /// </code>
    /// 
    /// </remarks>
    /// 
    /// <seealso cref="ArrayEntry"/>
    /// <seealso cref="Buffer"/>
    /// <seealso cref="CodecReturnCode"/>
    /// <seealso cref="DecodeIterator"/>
    /// <seealso cref="EncodeIterator"/>
    sealed public class Array : IXMLDecoder
    {
		internal int _primitiveType;
		internal int _itemLength;
		internal readonly Buffer _encodedData = new Buffer(); // this member variable is set by internal implementation

        /// <summary>
		/// Creates <see cref="Array"/>.
		/// </summary>
		/// <seealso cref="Array"/>
        public Array()
        {
        }

        /// <summary>
		/// Sets all members in <see cref="Array"/> to an initial value. Useful for object
		/// reuse during encoding. While decoding, <see cref="Array"/> object can be reused
		/// without using <see cref="Clear()"/>.
		/// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        /// </summary>
        /// <remarks>
        /// Typical use:<para />
        /// <list type="number">
        /// <item>Call <see cref="EncodeInit(EncodeIterator)"/></item>
        /// <item>Call <see cref="ArrayEntry.Encode(EncodeIterator)"/> for each entry using the same buffer</item>
        /// <item>Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> The encoder iterator.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        /// </summary>
        /// 
        /// <remarks>
        /// Typical use:<para />
        /// <list type="number">
        /// <item>Call <see cref="EncodeInit(EncodeIterator)"/></item>
        /// <item>Call <see cref="ArrayEntry.Encode(EncodeIterator)"/> for each entry using the same buffer</item>
        /// <item>Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator.</param>
        /// <param name="success"> If <c>true</c> - successfully complete the aggregate,
        ///                if <c>false</c> - remove the aggregate from the buffer.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        /// <returns> The XML representation of the container.
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeArray(iter, this);
		}

        /// <summary>
        /// Decodes to XML the data next in line to be decoded with the iterator.
        /// Data that require dictionary lookups are decoded to hexadecimal strings.
        /// </summary>
        /// <param name="iter">Decode iterator</param>
        /// <returns> The XML representation of the array.
        /// </returns>
        public string DecodeToXml(DecodeIterator iter)
        {
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.ARRAY, null, null, null, iter);
        }

        /// <summary>
        /// Decodes to XML the data next in line to be decoded with the iterator.
        /// </summary>
        /// <param name="iter">Decode iterator</param>
        /// <param name="dictionary">Data dictionary</param>
        /// <returns> The XML representation of the array.
        /// </returns>
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
                Debug.Assert(value > DataTypes.UNKNOWN && value <= DataTypes.BASE_PRIMITIVE_MAX,
                    "primitiveType must be from the DataTypes enumeration and greater than UNKNOWN and less than or equal to BASE_PRIMITIVE_MAX.");

                Debug.Assert(value != DataTypes.ARRAY, "primitiveType in array can't be DataTypes.ARRAY.");

                _primitiveType = value;
                IsBlank = false;
            }
		}

        /// <summary>
        /// Gets or sets the number of items in the array.
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
		/// Raw data contents of the array. Set by <see cref="Decode(DecodeIterator)"/> method during decode.
		/// </summary>
		/// <returns> the encodedData.
		/// </returns>
		/// <seealso cref="Buffer"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Buffer EncodedData()
		{
			return _encodedData;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Blank()
        {
            Clear();
            IsBlank = true;
        }
    }
}