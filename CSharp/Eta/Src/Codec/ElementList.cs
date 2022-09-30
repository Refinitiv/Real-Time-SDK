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
	/// The <seealso cref="ElementList"/> is a self-describing container type that can contain
	/// zero to N^23 <seealso cref="ElementEntry"/> element entries, where zero indicates an
	/// empty element list. Each entry, known as an <seealso cref="ElementEntry"/>, contains an
	/// element name, dataType enumeration, and value.An element list is equivalent
	/// to <seealso cref="FieldList"/>, where name and type information is present in each
	/// element entry instead of optimized via a field dictionary.
	/// </summary>
	/// <para>
	/// <b>ElementList Encoding Example</b>
	/// </para>
	/// <para>
	/// The following example demonstrates how to encode an <seealso cref="ElementList"/> and
	/// encodes four <seealso cref="ElementEntry"/> values:
	/// <ul>
	/// <li>
	/// The first encodes an entry from a primitive <seealso cref="Time"/> type.</li>
	/// <li>
	/// The second encodes from a pre-encoded buffer containing an encoded
	/// <seealso cref="UInt"/>.</li>
	/// <li>
	/// The third encodes as a blank <seealso cref="Real"/> value.</li>
	/// <li>
	/// The fourth encodes as an <seealso cref="ElementList"/> container type.</li>
	/// <para>
	/// The pattern used to encode the fourth entry can be used to encode any
	/// container type into an <seealso cref="ElementEntry"/>. This example demonstrates error
	/// handling for the initial encode method. However, additional error handling
	/// is omitted to simplify the example. This example shows the encoding of
	/// standard name, dataType, and value data.
	/// </para>
	/// <para>
	/// 
	/// <pre>
	/// 
	/// EncodeIterator encIter = new EncodeIterator();
	/// ElementList elementList = new ElementList();
	/// // indicate that standard data will be encoded and that elementListNum is
	/// // included
	/// elementList.ApplyHasStandardData();
	/// elementList.ElementListNum = 5;
	/// if (elementList.EncodeInit(encIter, null, 0) &lt; CodecReturnCode.SUCCESS)
	/// {
	///     // error condition
	/// }
	/// else
	/// {
	///     // elementList.encodeInit was successful
	/// 
	///     // create a single ElementEntry and reuse for each entry
	///     ElementEntry elementEntry = new ElementEntry();
	/// 
	///     Buffer name = new Buffer();
	/// 
	///     // FIRST Element Entry: encode entry from the Time primitive type
	///     name.Data(&quot;Element1 - Primitive&quot;);
	///     elementEntry.Name = name;
	///     elementEntry.DataType = DataTypes.TIME;
	/// 
	///     // create and populate time
	///     Time time = new Time();
	///     time.hour(11);
	///     time.minute(30);
	///     time.second(15);
	///     time.millisecond(7);
	/// 
	///     retVal = elementEntry.Encode(encIter, time);
	/// 
	///     // SECOND Element Entry: encode entry from preencoded buffer containing an
	///     // encoded UInt type
	///     elementEntry.Clear();
	///     name.Data(&quot;Element2 - Primitive&quot;);
	///     elementEntry.Name = name;
	///     elementEntry.DataType = DataTypes.UINT;
	///     UInt uint =new UInt();
	///     uint.value(1234);
	///     retVal = elementEntry.Encode(encIter, uint);
	/// 
	///     // THIRD Element Entry: encode entry as a blank Real primitive
	///     // need to ensure that ElementEntry is appropriately cleared clearing will
	///     // ensure that encData is properly emptied
	///     elementEntry.Clear();
	///     name.Data(&quot;Element3 - Blank&quot;);
	///     elementEntry.Name = name;
	///     retVal = elementEntry.EncodeBlank(encIter);
	/// 
	///     // FOURTH Element Entry: encode entry as a container type, ElementList
	///     // need to ensure that ElementEntry is appropriately cleared clearing will
	///     // ensure that encData is properly emptied
	///     elementEntry.Clear();
	///     name.Data(&quot;Element4 - Container&quot;);
	///     elementEntry.Name = name;
	///     elementEntry.DataType = DataTypes.ELEMENT_LIST;
	///     retVal = elementEntry.EncodeInit(encIter, 0);
	///     // has standard data
	///     retVal = elementList.EncodeInit(encIter, null, 50);
	///     name.Data(&quot;string type&quot;);
	///     elementEntry.Name = name;
	///     elementEntry.DataType = DataTypes.ASCII_STRING;
	///     stringBuffer.Data(&quot;ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL&quot;);
	///     retVal = elementEntry.Encode(encIter, stringBuffer);
	///     retVal = elementList.EncodeComplete(encIter, true);
	///     // complete encoding of complex element entry. If any element list encoding
	///     // failed, success is false
	///     retVal = elementEntry.EncodeComplete(encIter, success);
	/// }
	/// // complete elementList encoding. If success parameter is true, this will
	/// // finalize encoding.
	/// // If success parameter is false, this will roll back encoding prior to
	/// // elementList.encodeInit
	/// retval = elementList.EncodeComplete(encIter, success);
	/// </pre>
	/// </para>
	/// <para>
	/// <b>ElementList decoding example</b>
	/// </para>
	/// <para>
	/// The following sample demonstrates how to decode an ElementList and is
	/// structured to decode each entry to its contained value. This example uses a
	/// switch statement to invoke the specific decoder for the contained type,
	/// however for sample clarity, necessary cases have been omitted. This example
	/// uses the same <seealso cref="DecodeIterator"/> when calling the primitive decoder
	/// method. An application could optionally use a new <seealso cref="DecodeIterator"/> by
	/// setting the encData on a new iterator. For simplification, the example omits
	/// some error handling.
	/// 
	/// </para>
    /// </ul> 
	/// </para>
    /// <para>
	/// <ul class="blockList">
	/// <li class="blockList">
	/// 
	/// <pre>
	/// //decode into the element list structure
	/// if(retVal  = elementList.Decode(decIter, localSetDefs)) >= CodecReturnCode.SUCCESS)
	/// {
	///  //decode each element entry
	///  while((retVal = elementEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER))
	///  {
	///      if (retVal &lt; CodecReturnCode.SUCCESS)
	///      {
	/// 
	///      }
	///      else
	///      {
	///          //elemEntry.dataType to call correct primitive decode method
	///          switch (elemEntry.DataType)
	///          {
	///              case DataTypes.REAL:
	///                  retVal = real.Decode(decIter);
	///              break;
	///              case DataTypes.DATE:
	///                  retVal = date.Decode(decIter);
	///              break;
	///              //switch statement omitted to shorten sample code
	///          }
	///      }
	///  }
	/// }
	/// else
	/// {
	///      //decoding failure tends to be unrecoverable
	/// }
	/// 
	/// </pre>
	/// 
	/// </li>
	/// </ul>
    /// </para>
	/// <seealso cref="ElementEntry"/>
	/// <seealso cref="ElementListFlags"/>
	public sealed class ElementList : IXMLDecoder
    {
		internal readonly Buffer _encodedSetData = new Buffer();
		internal readonly Buffer _encodedEntries = new Buffer();

        /// <summary>
        /// Creates <seealso cref="ElementList"/>.
        /// </summary>
        /// <returns> <seealso cref="ElementList"/> object
        /// </returns>
        /// <seealso cref="ElementList"/>
        public ElementList()
        {
        }

        /// <summary>
		/// Clears <seealso cref="ElementList"/> object. Useful for object reuse during encoding.
		/// While decoding, <seealso cref="ElementList"/> object can be reused without using <seealso cref="ElementList.Clear()"/>.
		/// </summary>
		public void Clear()
		{
			Flags = 0;
			ElementListNum = 0;
			SetId = 0;
			_encodedSetData.Clear();
			_encodedEntries.Clear();
		}

        /// <summary>
        /// Prepares element list for Element List for encoding.
        /// 
        /// Typical use:<para />
        /// 1. Call encodeInit()<para />
        /// 2. Call one or more element encoding methods using the same buffer<para />
        /// 3. Call encodeComplete()<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="setDb"> A set definition database to refer use, if encoding set data in the list. </param>
        /// <param name="setEncodingMaxSize"> Maximum amount of space for the set definition database.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="LocalElementSetDefDb"/>
        public CodecReturnCode EncodeInit(EncodeIterator iter, LocalElementSetDefDb setDb, int setEncodingMaxSize)
		{
			return Encoders.EncodeElementListInit(iter, this, setDb, setEncodingMaxSize);
		}

        /// <summary>
		/// Completes elementList encoding.
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
			return Encoders.EncodeElementListComplete(iter, success);
		}

        /// <summary>
		/// Initialize decoding iterator for a element list.
		/// 
		/// Typical use:<para />
		/// 1. Call decode() on the list<para />
		/// 2. Call decode() for each element in the list<para />
		/// </summary>
		/// <param name="iter"> The iterator used to parse the element list. </param>
		/// <param name="localSetDb"> The local set database.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="LocalElementSetDefDb"/>
		public CodecReturnCode Decode(DecodeIterator iter, LocalElementSetDefDb localSetDb)
		{
			return Decoders.DecodeElementList(iter, this, localSetDb);
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
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.ELEMENT_LIST, null, null, null, iter);
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
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.ELEMENT_LIST, null, dictionary, null, iter);
        }

        /// <summary>
		/// Checks the presence of the Information presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <seealso cref="ElementList.Flags"/>.
		/// </summary>
		/// <seealso cref="ElementList.Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasInfo()
		{
			return ((Flags & ElementListFlags.HAS_ELEMENT_LIST_INFO) > 0 ? true : false);
		}

        /// <summary>
		/// Checks the presence of the Standard Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <seealso cref="ElementList.Flags"/>.
		/// </summary>
		/// <seealso cref="ElementList.Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasStandardData()
		{
			return ((Flags & ElementListFlags.HAS_STANDARD_DATA) > 0 ? true : false);
		}

        /// <summary>
		/// Checks the presence of the Set Id presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <seealso cref="ElementList.Flags"/>.
		/// </summary>
		/// <seealso cref="ElementListFlags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasSetId()
		{
			return ((Flags & ElementListFlags.HAS_SET_ID) > 0 ? true : false);
		}

        /// <summary>
		/// Checks the presence of the Set Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <seealso cref="ElementList.Flags"/>.
		/// </summary>
		/// <seealso cref="ElementList.Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasSetData()
		{
			return ((Flags & ElementListFlags.HAS_SET_DATA) > 0 ? true : false);
		}

        /// <summary>
        /// Applies the Information presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <seealso cref="ElementList.Flags"/>.
        /// </summary>
        /// <seealso cref="ElementList.Flags"/>
        public void ApplyHasInfo()
		{
			Flags |= ElementListFlags.HAS_ELEMENT_LIST_INFO;
		}

        /// <summary>
		/// Applies the Standard Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="ElementList.Flags"/>.
		/// </summary>
		/// <seealso cref="ElementList.Flags"/>
		public void ApplyHasStandardData()
		{
			Flags |= ElementListFlags.HAS_STANDARD_DATA;
		}

        /// <summary>
		/// Applies the Set Id presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="ElementList.Flags"/>.
		/// </summary>
		/// <seealso cref="ElementList.Flags"/>
		public void ApplyHasSetId()
		{
			Flags |= ElementListFlags.HAS_SET_ID;
		}

        /// <summary>
		/// Applies the Set Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="ElementList.Flags"/>.
		/// </summary>
		/// <seealso cref="ElementList.Flags"/>
		public void ApplyHasSetData()
		{
			Flags |= ElementListFlags.HAS_SET_DATA;
		}

        /// <summary>
		/// Gets or sets an external element-list template, also known as a record template.
		/// A record template contains information about all possible entries contained
		/// in the stream and is typically used by caching mechanisms to pre-allocate
		/// storage. Must be in the range of -32768 - 32767.
		/// </summary>
		public int ElementListNum { get; set; }

        /// <summary>
        /// Gets or sets Corresponds to the set definition used for encoding or decoding the
        /// set-defined data in this <seealso cref="ElementList"/>. When encoding, this is
        /// the set definition used to encode any set-defined content. When decoding,
        /// this is the set definition used for decoding any set-defined content.
        /// If a setId value is not present on a message containing set-defined data,
        /// a setId of 0 is implied. Must be in the range of 0 - 32767.
        /// </summary>
		public int SetId { get; set; }

        /// <summary>
		/// Gets or sets encoded set-defined data, if any, contained in
		/// the message. If populated, contents are described by the set definition
		/// associated with the setId member.
		/// </summary>
		public Buffer EncodedSetData
		{
            get
            {
                return _encodedSetData;
            }

            set
            {
                (_encodedSetData).CopyReferences(value);
            }
		}

        /// <summary>
		/// Gets encoded element name, dataType, value encoded data, if any, contained
		/// in the message. This would refer to encoded <seealso cref="ElementList"/> payload and length information.
		/// </summary>
		/// <returns> the encodedEntries </returns>
		public Buffer EncodedEntries
		{
            get
            {
                return _encodedEntries;
            }
		}

        /// <summary>
        /// Gets or sets all the flags applicable to this element list
        /// flags ia an integer containing all the flags applicable to this element list
        /// </summary>
        public ElementListFlags Flags { get; set; }
	
	}

}