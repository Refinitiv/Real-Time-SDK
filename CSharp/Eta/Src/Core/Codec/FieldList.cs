/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// A container of field identifier - value paired entries, each known as a <see cref="FieldEntry"/>.
    /// </summary>
    /// <remarks>
    ///
    /// <para>A field identifier, also referred to as a fieldId, is a
    /// value that refers to specific name and type information defined by an external
    /// field dictionary, such as the RDMFieldDictionary. A field list can contain zero
    /// to N entries, where zero indicates an empty field list.</para>
    ///
    /// <para>
    /// <b>FieldList Encoding Example</b>
    /// </para>
    /// <para>
    /// The following example demonstrates how to encode a <see cref="FieldList"/> and
    /// encodes four <see cref="FieldEntry"/> values:
    /// <ul>
    /// <li>
    /// The first encodes an entry from a primitive <see cref="Date"/> type.</li>
    /// <li>
    /// The second from a pre-encoded buffer containing an encoded <see cref="UInt"/>.</li>
    /// <li>
    /// The third as a blank <see cref="Real"/> value.</li>
    /// <li>
    /// The fourth as an <see cref="Array"/> complex type. The pattern followed while
    /// encoding the fourth entry can be used for encoding of any container type into
    /// a <see cref="FieldEntry"/></li>
    /// </ul>
    /// </para>
    /// <para>
    /// This example demonstrates error handling for the initial encode method. To
    /// simplify the example, additional error handling is omitted (though it should
    /// be performed). This example shows encoding of standard fieldId-value data.
    ///
    /// </para>
    ///
    /// <code>
    /// FieldList fieldList = new FieldList();
    ///
    /// // create a single FieldEntry and reuse for each entry
    /// FieldEntry fieldEntry = new FieldEntry();
    ///
    /// EncodeIterator encIter = new ncodeIterator();
    ///
    /// // NOTE: the fieldId, dictionaryId and fieldListNum values used for this example
    /// // do not correspond to actual id values
    /// // indicate that standard data will be encoded and that dictionaryId and
    /// // fieldListNum are included
    /// fieldList.ApplyHasInfo();
    /// fieldList.DictionaryId = 2;
    /// fieldList.FieldListNum = 3;
    /// fieldList.ApplyHasStandardData();
    ///
    /// // begin encoding of field list - assumes that encIter is already populated with
    /// // buffer and version information, store return value to determine success or
    /// // failure
    ///
    /// if (fieldList.EncodeInit(encIter, null, 40) &lt; CodecReturnCode.SUCCESS)
    /// {
    ///     // error condition - switch our success value to false so we can roll back
    ///     success = false;
    /// }
    /// else
    /// {
    ///     // fieldListInit encoding was successful
    ///     // FIRST Field Entry: encode entry from the Date primitive type
    ///     Date date = new Date();
    ///     date.Day(23);
    ///     date.Month(5);
    ///     date.Year(2012);
    ///     fieldEntry.FieldId = 16;
    ///     fieldEntry.DataType = DataTypes.DATE;
    ///     retVal = fieldEntry.Encode(encIter, date);
    ///
    ///     // SECOND Field Entry: encode entry from preencoded buffer containing an
    ///     // encoded UInt type
    ///     // populate and encode field entry with fieldId and dataType information for
    ///     // this field
    ///     // because we are re-populating all values on FieldEntry, there is no need
    ///     // to clear it.
    ///     fieldEntry.Clear();
    ///     fieldEntry.FieldId = 16;
    ///     fieldEntry.DataType = DataTypes.UINT;
    ///     // assuming encUInt is a Buffer with length and data properly populated
    ///     fieldEntry.EncodedData(encUInt);
    ///     retVal = fieldEntry.Encode(encIter, date);
    ///
    ///     // THIRD Field Entry: encode entry as a blank Real primitive type
    ///     fieldEntry.Clear();
    ///     fieldEntry.FieldId = 22;
    ///     fieldEntry.DataType = DataTypes.REAL;
    ///     retVal = fieldEntry.EncodeBlank(encIter);
    ///
    ///     // FOURTH Field Entry: encode entry as a complex type, Array primitive
    ///     Array array = new Array();
    ///     fieldEntry.Clear();
    ///     fieldEntry.FieldId = 1021;
    ///     fieldEntry.DataType = DataTypes.ARRAY;
    ///     // begin complex field entry encoding, we are not sure of the approximate
    ///     // max encoding length
    ///     retVal = fieldEntry.EncodeInit(encIter, 0);
    ///
    ///     // now encode nested container using its own specific encode methods
    ///     // encode Real values into the array
    ///     array.PrimitiveType = DataTypes.REAL;
    ///
    ///     // values are variable length
    ///     array.ItemLength = 0;
    ///
    ///     // begin encoding of array - using same encIterator as field list
    ///     if ((retVal = array.EncodeInit(encIter)) &lt; CodecReturnCode.SUCCESS)
    ///         // Continue encoding array entries
    ///
    ///         // Complete nested container encoding
    ///         retVal = encode.ArrayComplete(encIter, success);
    ///
    ///     // complete encoding of complex field entry. If any array encoding failed,
    ///     // success is false
    ///     retVal = fieldEntry.EncodeComplete(encIter, success);
    /// }
    ///
    /// // complete fieldList encoding. If success parameter is true, this will finalize
    /// // encoding.
    /// // If success parameter is false, this will roll back encoding prior to
    /// // FieldList.EncodeInit()
    ///
    /// retVal = fieldList.EncodeComplete(encIter, success);
    /// </code>
    ///
    /// <para>
    /// <b>FieldList Decoding Example</b>
    /// </para>
    /// <para>
    /// The following example demonstrates how to decode a <see cref="FieldList"/> and is
    /// structured to decode each entry to the contained value. This example uses a
    /// switch statement to invoke the specific decoder for the contained type,
    /// however to simplify the example, necessary cases and some error handling are
    /// omitted. This example uses the same <see cref="DecodeIterator"/> when calling the
    /// primitive decoder method. An application could optionally use a new
    /// <see cref="DecodeIterator"/> by setting the encData on a new iterator.
    ///
    /// </para>
    ///
    /// <code>
    /// // decode into the field list structure
    /// if ((retVal = fieldList.Decode(decIter, localSetDefs)) &gt; CodecReturnCode.SUCCESS)
    /// {
    ///     // decode each field entry
    ///     while ((retVal = fieldEntry.Decode(decIter)) != CodecReturnCode.END_OF_CONTAINER)
    ///     {
    ///         if (retVal &lt; CodecReturnCode.SUCCESS)
    ///         {
    ///             // decoding failure tends to be unrecoverable
    ///         }
    ///         else
    ///         {
    ///             // look up type in field dictionary and call correct primitive
    ///             // decode method
    ///             switch (fieldDict.Entry(fieldEntry.FieldId).rwfType)
    ///             {
    ///                 case DataTypes.REAL:
    ///                     retVal = real.Decode(decIter);
    ///                     break;
    ///                 case DataTypes.DATE:
    ///                     retVal = date.Decode(decIter);
    ///                     break;
    ///             // full switch statement omitted to shorten sample code
    ///             }
    ///         }
    ///     }
    /// }
    /// else
    /// {
    ///     // decoding failure tends to be unrecoverable
    /// }
    /// </code>
    ///
    /// </remarks>
    /// <seealso cref="FieldListFlags"/>
    /// <seealso cref="FieldEntry"/>
    sealed public class FieldList : IXMLDecoder
    {
		internal readonly Buffer _encodedSetData = new Buffer();
		internal readonly Buffer _encodedEntries = new Buffer();

        /// <summary>
		/// Creates <see cref="FieldList"/>.
		/// </summary>
		/// <seealso cref="FieldList"/>
        public FieldList()
        {

        }

		/// <summary>
		/// Clears <see cref="FieldList"/> object. Useful for object reuse during encoding.
		/// While decoding, <see cref="FieldList"/> object can be reused without using <see cref="Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			Flags = 0;
			DictionaryId = 0;
			FieldListNum = 0;
			SetId = 0;
			_encodedSetData.Clear();
			_encodedEntries.Clear();
		}

        /// <summary>
        /// Initializes List for encoding.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
        /// <item> Call <c>FieldEntry.Encode()</c> or
        /// <see cref="FieldEntry.EncodeInit(EncodeIterator, int)"/>..<see cref="FieldEntry.EncodeComplete(EncodeIterator, bool)"/>
        /// for each field in the list using the same buffer</item>
        /// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="setDb"> The set definition database to be used, if encoding set data. </param>
        /// <param name="setEncodingMaxSize"> Max encoding size for field list (If Unknown set to zero).
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="LocalFieldSetDefDb"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter, LocalFieldSetDefDb setDb, int setEncodingMaxSize)
		{
			return Encoders.EncodeFieldListInit(iter, this, setDb, setEncodingMaxSize);
		}

		/// <summary>
		/// Completes List encoding.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() or
		/// <see cref="FieldEntry.EncodeInit(EncodeIterator, int)"/>..<see cref="FieldEntry.EncodeComplete(EncodeIterator, bool)"/>
        /// for each field in the list using the same buffer</item>
		/// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="success"> If <c>true</c> - successfully complete the aggregate,
		///                if <c>false</c> - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeFieldListComplete(iter, success);
		}

		/// <summary>
		/// Initialize decoding iterator for a field list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="Decode(DecodeIterator, LocalFieldSetDefDb)"/></item>
		/// <item> Call <see cref="FieldEntry.Decode(DecodeIterator)"/> for each field in the list.</item>
        /// </list>
        /// </remarks>
		/// <param name="iter"> The iterator used to parse the field list. </param>
		/// <param name="localFieldSetDef"> The local set database. </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="LocalFieldSetDefDb"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter, LocalFieldSetDefDb localFieldSetDef)
		{
			return Decoders.DecodeFieldList(iter, this, localFieldSetDef);
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
        public string DecodeToXml(DecodeIterator iter)
        {
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.FIELD_LIST, null, null, null, iter);
        }

        /// <summary>
		/// Decodes to XML the data next in line to be decoded with the iterator.
		/// </summary>
		/// <param name="iter"> Decode iterator </param>
		/// <param name="dictionary"> Data dictionary </param>
		/// <returns> The XML representation of the container
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="DataDictionary"/>
        public string DecodeToXml(DecodeIterator iter, DataDictionary dictionary)
        {
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.FIELD_LIST, null, dictionary, null, iter);
        }

		/// <summary>
		/// Checks the presence of the Information presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasInfo()
		{
			return (Flags & FieldListFlags.HAS_FIELD_LIST_INFO) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Standard Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasStandardData()
		{
			return (Flags & FieldListFlags.HAS_STANDARD_DATA) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Set Id presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasSetId()
		{
			return (Flags & FieldListFlags.HAS_SET_ID) > 0;
		}

		/// <summary>
		/// Checks the presence of the Set Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasSetData()
		{
			return (Flags & FieldListFlags.HAS_SET_DATA) > 0;
		}
		/// <summary>
		/// Applies the Information presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasInfo()
		{
            Flags |= FieldListFlags.HAS_FIELD_LIST_INFO;
		}

		/// <summary>
		/// Applies the Standard Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasStandardData()
		{
            Flags |= FieldListFlags.HAS_STANDARD_DATA;
		}

		/// <summary>
		/// Applies the Set Id presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasSetId()
		{
            Flags |= FieldListFlags.HAS_SET_ID;
		}

		/// <summary>
		/// Applies the Set Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasSetData()
		{
            Flags |= FieldListFlags.HAS_SET_DATA;
		}

        /// <summary>
		/// Gets or sets all the flags applicable to this field list.
		/// Must be in the range of 0 - 255.
		/// </summary>
		/// <seealso cref="FieldListFlags"/>
		public FieldListFlags Flags { get; set; }

        /// <summary>
        /// Gets or sets Dictionary id. Refers to an external field dictionary, such as
        /// RDMFieldDictionary.
        /// </summary>
        /// <value> the dictionaryId
        /// </value>
        public int DictionaryId { get; set; }

        /// <summary>
		/// Refers to an external fieldlist template, also known as a record template.
		/// The record template contains information about all possible fields in a
		/// stream and is typically used by caching implementations to pre-allocate
		/// storage. Must be in the range of -32768 - 32767.
		/// </summary>
		public int FieldListNum { get; set; }

        /// <summary>
        /// Gets or sets set id. Corresponds to the Set Definition used for encoding or decoding
        /// the set defined data in this <see cref="FieldList"/>. Must be in the range of 0 - 32767.
        /// </summary>
        public int SetId { get; set; }

        /// <summary>
		/// Gets or sets encoded set data. If populated, contents are described by the set
		/// definition associated with the setId member.
		/// </summary>
		public Buffer EncodedSetData
		{
            get
            {
                return _encodedSetData;
            }

            set
            {
                _encodedSetData.CopyReferences(value);
            }
		}

        /// <summary>
		/// Gets encoded fieldId-value pair encoded data, if
		/// any, contained in the message. This would refer to encoded
		/// <see cref="FieldList"/> payload and length information.
		/// </summary>
		public Buffer EncodedEntries
		{
            get
            {
                return _encodedEntries;
            }

            set
            {
                _encodedEntries.CopyReferences(value);
            }
		}
	}
}