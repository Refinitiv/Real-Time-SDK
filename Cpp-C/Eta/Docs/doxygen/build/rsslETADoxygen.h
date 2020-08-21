/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

 /**
 *  @defgroup RSSLIteratorInterfaces Overview: Transport API Encode/Decode Iterator Use
 *  @{
 *
 *  The following sections describe high level usage of the Transport API Encode and Decode Iterators.  Specific iterator structure and function definitions can be found in \ref RsslIteratorGroup.
 *
 *	@section IterOverview Transport API Iterator Overview
 *	When encoding or decoding RWF content with Transport API, the user leverages an iterator to manage the encoding or decoding process.
 *  The Transport API defines a single encode iterator type (\ref RsslEncodeIterator) and a single decode iterator type (\ref RsslDecodeIterator).  A single instance of this iterator
 *  can manage the full depth and breadth of the encoding or decoding process.  Alternatly, multiple iterator instances can be used to individually manage separate portions of the encode
 *  or decode process.  <BR><BR>
 *  The Transport API encoder/decoder does not provide any inherent threading or locking capability.  Separate iterator and type instances do not cause contention and do not share resources between instances.
 *  Any needed threading, locking, or thread-model implementation is at the discretion of the application.  Different application threads can encode or decode different messages without requiring a lock;
 *  thus each thread must use its own iterator instance and each message should be encoded or decoded using unique and independent buffers.  
 *  @note Though possible, Refinitiv recommends that you do not encode or decode related messages (messages flowing on the same stream) on different threads as this can impact the delivery order.
 *
 *	@subsection IterVers Iterator RWF Protocol Versioning 
 *	The Transport API iterators help the user to manage version information associated with the RWF content being exchanged.  When using the Transport API 
 *  Transport the protocol type and version information can be exchanged and negotiated on the connection (via the RsslConnectOptions or RsslBindOptions).  
 *  The Transport API will reject any connection establishment when the protocol type does not match across the connection.  If the protocol type does match,
 *  an appropriate major and minor version will be determined and this should be the version of RWF encoded or decoded when using this connection.
 *  After the connection becomes active, this negotiated version information is available and can then be provided to the iterator to ensure that the proper version is encoded or decoded.
 *  If not using the Transport API, the user can determine the desired version of RWF to encode and specify this information on their iterator.
 *  Transport API provides RWF protocol type and protocol version values in @header "rtr\rsslIterators.h" for this purpose.  
 *  @note Specifying appropriate version information on \ref RsslDecodeIterator and \ref RsslEncodeIterator is important to future-proof applications and avoid incompatability if new RWF functionality is added.
 *
 *  See \ref RsslIteratorVersion for more information about Iterator Versioning.
 *  
 *  @section DecodeIter Transport API Decode Iterator Overview
 *  All RWF decoding requires the use of an \ref RsslDecodeIterator, where a single iterator can manage the full decoding process.  
 *  For instance, if the application is decoding a message containing an RsslFieldList, the same \ref RsslDecodeIterator can be used
 *  to decode the RsslFieldList, all RsslFieldEntry, and all types (primitive types or container types) housed in the entries. 
 *  Seperate iterators can be used as well, depending on the needs of the application.  Following the same example, one 
 *  \ref RsslDecodeIterator can be used to decode the message information (up to the beginning of the RsslFieldList payload).  
 *  Another \ref RsslDecodeIterator reference can be used to decode the RsslFieldList and entries, and if desired, other 
 *  iterators can be used to decode the contents of each RsslFieldEntry.<BR><BR>
 *  Before decoding begins, the iterator should be initialized to ready it for decoding.  Initialization consists of several steps.  
 *  The rsslClearDecodeIterator() function can be used to initialize (or re-initialize for reuse) the \ref RsslDecodeIterator.  
 *  After clearing, an \ref RsslBuffer containing the content to decode should be associated with the \ref RsslDecodeIterator.
 *  In addition, RWF version information should be provided to the \ref RsslDecodeIterator so the desired version of RWF is 
 *  decoded.<BR><BR>
 *  See \ref RsslDecodeIteratorType for more information and code examples.<BR><BR><BR>
 *  
 *  @section EncodeIter Transport API Encode Iterator Overview
 *  All RWF encoding requires the use of an \ref RsslEncodeIterator, where a single iterator can manage the full encoding process.
 *  For instance, if the application is encoding a message containing an RsslFieldList, the same \ref RsslEncodeIterator can be used
 *  to encode the message header information, the RsslFieldList information, each RsslFieldEntry, and the content of each entry 
 *  (primitive types or container types).  Seperate iterators can be used as well, depending on the needs of the application.  
 *  Following the same example, one \ref RsslEncodeIterator can be used to pre-encode an entry's content.  This pre-encoded content
 *  can then be set on the RsslFieldEntry and encoded using the \ref RsslEncodeIterator that is encoding the RsslFieldList.  This
 *  encoded field list content can then be set on the RsslMsg and yet another \ref RsslEncodeIterator can be used to encode the message
 *  and its pre-encoded payload.<BR><BR>
 *  Before encoding begins, the iterator should be initialized to ready it for the encoding process.  Initialization consists of several steps.
 *  The rsslClearEncodeIterator() function can be used to initialize (or re-initalize for reuse) the \ref RsslEncodeIterator.
 *  After clearing, an \ref RsslBuffer with ample memory should be associated with the iterator; this will be the buffer that
 *  content is encoded into (if using with the Transport API, this is often a buffer obtained from the rsslGetBuffer() function so it can be immediatley written after encoding completes).
 *  In addition, RWF version information should be provided to the \ref RsslEncodeIterator so the desired version of RWF is encoded.
 *  @note When encoding is complete, the user should query the length of encoded content, setting this information in the \ref RsslBuffer::length.  
 *  If encoding fails, the \ref RsslEncodeIterator can be used to roll back content to the last successful encoding point.  <BR><BR>
 *  See \ref RsslEncodeIteratorType for more information and code examples.<BR><BR><BR>
 *
 * @}
 */


/**
 *	@defgroup RSSLRetCodes Overview: Transport API Return Codes and Meaning
 *	@{
 *
 *	This section describes each Transport API return code and provides high level meaning and handling information.  The codes are used to communicate errors and other information to the user.  
 *  These are returned from both encoder/decoder functions as well as the transport functions. 
 *
 *	@}
 */

/**
 *	@defgroup RSSLTypeEnums Overview: Transport API Primitive and Container Type Enumerations
 *	@{
 *
 *	This section describes each Transport API primitive and container type enumerated value and refers to each types specific section. These enumerated values are used to indicate which type is being sent or received. 
 *
 *	@}
 */
 

/**
 *  @defgroup RSSLDataPrimitiveInterfaces Overview: Transport API Primitive Type Encode/Decode Interface Functions
 *  @{
 *
 *  This section contains a listing of the Transport API encode and decode functions available for Primitive Types.
 * 
 *  @section IntOverview Signed Integer Encoding/Decoding Interface Function List
 *  The \ref RsslInt primitive type allows the user to represent signed integers ranging from -2<sup>63</sup> to 2<sup>63</sup> - 1.<BR>
 *  <ul>
 *  <li>rsslEncodeInt() - Encodes an \ref RsslInt primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeInt() - Decodes an \ref RsslInt primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslIntGroup for more information.<BR><BR><BR>
 *
 *  @section UIntOverview Unsigned Integer Encoding/Decoding Interface Function List
 *  The \ref RsslUInt primitive type allows the user to represent signed integers ranging from 0 to 2<sup>64</sup> - 1.<BR>
 *  <ul>
 *  <li>rsslEncodeUInt() - Encodes an \ref RsslUInt primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeUInt() - Decodes an \ref RsslUInt primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslUIntGroup for more information.<BR><BR><BR>
 *
 *  @section FloatOverview Float Encoding/Decoding Interface Function List
 *  The \ref RsslFloat primitive type allows the user to represent 4-byte IEEE-754 floating point values.<BR>
 *  <ul>
 *  <li>rsslEncodeFloat() - Encodes an \ref RsslFloat primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeFloat() - Decodes an \ref RsslFloat primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslFloatGroup for more information.<BR><BR><BR>
 *
 *  @section DoubleOverview Double Encoding/Decoding Interface Function List
 *  The \ref RsslDouble primitive type allows the user to represent 8-byte IEEE-754 floating point values.<BR>
 *  <ul>
 *  <li>rsslEncodeDouble() - Encodes an \ref RsslDouble primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeDouble() - Decodes an \ref RsslDouble primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslDoubleGroup for more information.<BR><BR><BR>
 *
 *  @section BufferOverview Buffer and String Encoding/Decoding Interface Function List
 *  The \ref RsslBuffer primitive type allows the user to represent length specified content.  This includes various string types (ASCII, RMTES, UTF8) or non-string opaque content.<BR>
 *  <ul>
 *  <li>rsslEncodeBuffer() - Encodes an \ref RsslBuffer primitive type.  This will copy contents into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeBuffer() - Decodes an \ref RsslBuffer primitive type.  This will point to the contents in the buffer referred to by the \ref RsslDecodeIterator.  For performance purposes, contents will not be copied.</li><BR><BR>
 *  </ul>
 *  See \ref RsslBufferGroup for more information.<BR><BR><BR>
 *
 *  @section DateOverview Date Encoding/Decoding Interface Function List
 *  The RsslDate primitive type allows the user to represent a date containing a month, day, and year.<BR>
 *  <ul>
 *  <li>rsslEncodeDate() - Encodes an RsslDate primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeDate() - Decodes an RsslDate primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslDateGroup for more information.<BR><BR><BR>
 *
 *  @section TimeOverview Time Encoding/Decoding Interface Function List
 *  The RsslTime primitive type allows the user to represent a time containing hour, minute, second, and millisecond information.<BR>
 *  <ul>
 *  <li>rsslEncodeTime() - Encodes an RsslTime primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeTime() - Decodes an RsslTime primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslTimeGroup for more information.<BR><BR><BR>
 *
 *  @section DateTimeOverview DateTime Encoding/Decoding Interface Function List
 *  The RsslDateTime primitive type allows the user to represent a date and time containing month, day, year, hour, minute, second, and millisecond information.<BR>
 *  <ul>
 *  <li>rsslEncodeDateTime() - Encodes an RsslDateTime primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeDateTime() - Decodes an RsslDateTime primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslDateTimeGroup for more information.<BR><BR><BR>
 *
 *  @section EnumOverview Enum Encoding/Decoding Interface Function List
 *  The \ref RsslEnum primitive type allows the user to represent unsigned 2-byte value, typically corresponding to an enumerated value that can be cross-referenced with an enum dictionary.<BR>
 *  <ul>
 *  <li>rsslEncodeEnum() - Encodes an \ref RsslEnum primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeEnum() - Decodes an \ref RsslEnum primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslEnumGroup for more information.<BR><BR><BR>
 *
 *  @section ArrayOverview Array Encoding/Decoding Interface Function List
 *  The RsslArray is a uniform primitive type that can contain multiple simple primitive entries.  Entries can be either fixed length or variable length.<BR>
 *  <ul>
 *  <li>rsslEncodeArrayInit() - Begins array encoding, initial function to call when encoding an array type. </li><BR>
 *  <li>rsslEncodeArrayComplete() - Completes encoding of array, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired. </li><BR>
 *  <li>rsslEncodeArrayEntry() - Encodes entry into an array.  Can be primitive representation or pre-encoded. </li><BR>
 *  <li>rsslDecodeArray() - Decodes an array container and provides access to buffer containing all encoded array entries. </li><BR>
 *  <li>rsslDecodeArrayEntry() - Decodes array entries and provides access to individual array entries.  Primitive decode functions can be used to decode individual array entry into primitive representation. </li><BR><BR>
 *  </ul>
 *  See \ref RsslArrayGroup for more information.<BR><BR><BR>
 *
 *  @section QosOverview QoS Encoding/Decoding Interface Function List
 *  The \ref RsslQos primitive type allows the user to represent quality of service information, including timeliness (data age) and rate (data's period of change), associated with the content being sent or received.
 *  <ul>
 *  <li>rsslEncodeQos() - Encodes an \ref RsslQos primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeQos() - Decodes an \ref RsslQos primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslQosGroup for more information.<BR><BR><BR>
 *
 *  @section StateOverview State Encoding/Decoding Interface Function List
 *  The \ref RsslState primitive type allows the user to convey data and stream health information.<BR>
 *  <ul>
 *  <li>rsslEncodeState() - Encodes an \ref RsslState primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 *  <li>rsslDecodeState() - Decodes an \ref RsslState primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR><BR>
 *  </ul>
 *  See \ref RsslStateGroup for more information.<BR><BR><BR>
 *
 *	@section RmtesOverview RMTES Decoding Interface Function List
 *  The RMTES decoding interface allows the user to decode and print RMTES encoded data. These functions take in an RsslBuffer which contains encoded RMTES data, and converts the data to a user-readable UTF-8 or UCS2 unicode encoded string.<BR>
 *  <ul>
 *  <li>rsslRMTESApplyToCache() - Copies the data provided to the cache location.  This also will apply any marketfeed update logic to the cache(if present).</li><BR>
 *  <li>rsslRMTESToUTF8() - Decodes the provided RsslRmtesCacheBuffer to a UTF-8 encoded 8-bit character string.</li><BR>
 *  <li>rsslRMTESToUCS2() - Decodes the provided RsslRmtesCacheBuffer to a UCS2 encoded 16-bit integer string.</li><BR><BR>
 *  </ul>
 *  See \ref RsslRmtesGroup for more information.<BR><BR><BR>
 *  @}
 */

/**
 *  @defgroup RSSLDataContainerInterfaces Overview: Transport API Container Type Encode/Decode Interface Functions
 *  @{
 *
 *  This section contains a listing of the Transport API encode and decode functions available for Container Types.
 *
 *  @section FieldListOverview RsslFieldList Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeFieldListInit() - Begins RsslFieldList encoding, initial function to call when encoding an RsslFieldList type.</li><BR>
 *  <li>rsslEncodeFieldListComplete() - Completes encoding of an RsslFieldList, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeFieldList() - Decodes an RsslFieldList container and provides access to buffer containing all encoded field entries.</li><BR>
 *  </ul>
 *  
 *  @subsection FieldEntryOverview RsslFieldEntry Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeFieldEntry() - Encodes RsslFieldEntry into an RsslFieldList, where the RsslFieldEntry is being encoded from a base primitive type passed in via void*, has pre-encoded content set in RsslFieldEntry::encData, or is blank where void* is NULL and RsslFieldEntry::encData is empty.  </li> <BR>
 *  <li>rsslEncodeFieldEntryInit() - Begins RsslFieldEntry encoding, initial function to call when encoding an RsslFieldEntry that houses a non pre-encoded container. </li>  <BR>
 *  <li>rsslEncodeFieldEntryComplete() - Completes encoding of an RsslFieldEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeFieldEntry() - Decodes RsslFieldList entries and provides access to individual field entry content.  If RsslFieldEntry::dataType is ::RSSL_DT_UNKNOWN, type should be determined by looking up RsslFieldEntry::fieldId in appropriate field dictionary file.  If RsslFieldEntry::dataType is populated, this decoder should be used to continue to decode individual RsslFieldEntry contents.</li> <BR>
 *  </ul>
 *
 *  @section ElementListOverview RsslElementList Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeElementListInit() - Begins RsslElementList encoding, initial function to call when encoding an RsslElementList type.</li><BR>
 *  <li>rsslEncodeElementListComplete() - Completes encoding of an RsslElementList, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeElementList() - Decodes an RsslElementList container and provides access to buffer containing all encoded element entries.</li><BR>
 *  </ul>
 *  
 *  @subsection ElementEntryOverview RsslElementEntry Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeElementEntry() - Encodes RsslElementEntry into an RsslElementList, where the RsslElementEntry is being encoded from a base primitive type passed in via void*, has pre-encoded content set in RsslElementEntry::encData, or is blank where void* is NULL and RsslElementEntry::encData is empty.  </li> <BR>
 *  <li>rsslEncodeElementEntryInit() - Begins RsslElementEntry encoding, initial function to call when encoding an RsslElementEntry that houses a non pre-encoded container. </li>  <BR>
 *  <li>rsslEncodeElementEntryComplete() - Completes encoding of an RsslElementEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeElementEntry() - Decodes RsslElementList entries and provides access to individual element entry content.  RsslElementEntry::dataType decoder should be used to continue to decode individual RsslElementEntry contents.</li> <BR>
 *  </ul>
 *
 *  @section MapOverview RsslMap Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeMapInit() - Begins RsslMap encoding, initial function to call when encoding an RsslMap type.</li><BR>
 *  <li>rsslEncodeMapComplete() - Completes encoding of an RsslMap, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslEncodeMapSetDefsComplete() - Completes encoding of set definitions on an RsslMap, called after set definitions (if any) are encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be encoded and this function should be called prior to any summary data encoding.</li> <BR>
 *  <li>rsslEncodeMapSummaryDataComplete() - Completes encoding of summary data on an RsslMap, called after summary data is encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be completed first, followed by any summary data encoding and this function.</li> <BR>
 *  <li>rsslDecodeMap() - Decodes an RsslMap container and provides access to buffer containing all encoded map entries.</li><BR>
 *  </ul>
 *  
 *  @subsection MapEntryOverview RsslMapEntry Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeMapEntry() - Encodes RsslMapEntry into an RsslMap, where the RsslMapEntry has no payload or has pre-encoded content set in RsslMapEntry::encData.  Entry key can be passed in and encoded via this function or can be populated in RsslMapEntry::encKey if preencoded.</li> <BR>
 *  <li>rsslEncodeMapEntryInit() - Begins RsslMapEntry encoding, initial function to call when encoding an RsslMapEntry without pre-encoded payload. Entry key can be passed in and encoded via this function or can be populated in RsslMapEntry::encKey if preencoded.</li>  <BR>
 *  <li>rsslEncodeMapEntryComplete() - Completes encoding of an RsslMapEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeMapEntry() - Decodes RsslMap entries and provides access to individual map entries and their keys.  RsslMap::containerType decode functions can be used to decode individual RsslMapEntry contents.</li> <BR>
 *  </ul>
 *
 *  @section VectorOverview RsslVector Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeVectorInit() - Begins RsslVector encoding, initial function to call when encoding an RsslVector type.</li><BR>
 *  <li>rsslEncodeVectorComplete() - Completes encoding of an RsslVector, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslEncodeVectorSetDefsComplete() - Completes encoding of set definitions on an RsslVector, called after set definitions (if any) are encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be encoded and this function should be called prior to any summary data encoding.</li> <BR>
 *  <li>rsslEncodeVectorSummaryDataComplete() - Completes encoding of summary data on an RsslVector, called after summary data is encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be completed first, followed by any summary data encoding and this function.</li> <BR>
 *  <li>rsslDecodeVector() - Decodes an RsslVector container and provides access to buffer containing all encoded vector entries.</li><BR>
 *  </ul>
 *  
 *  @subsection VectorEntryOverview RsslVectorEntry Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeVectorEntry() - Encodes RsslVectorEntry into an RsslVector, where the RsslVectorEntry has no payload or has pre-encoded content set in RsslVectorEntry::encData.</li> <BR>
 *  <li>rsslEncodeVectorEntryInit() - Begins RsslVectorEntry encoding, initial function to call when encoding an RsslVectorEntry without pre-encoded payload. </li>  <BR>
 *  <li>rsslEncodeVectorEntryComplete() - Completes encoding of an RsslVectorEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeVectorEntry() - Decodes RsslVector entries and provides access to individual map entries and their indexes.  RsslVector::containerType decode functions can be used to decode individual RsslVectorEntry contents.</li> <BR>
 *  </ul>
 * 
 *  @section SeriesOverview RsslSeries Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeSeriesInit() - Begins RsslSeries encoding, initial function to call when encoding an RsslSeries type.</li><BR>
 *  <li>rsslEncodeSeriesComplete() - Completes encoding of an RsslSeries, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslEncodeSeriesSetDefsComplete() - Completes encoding of set definitions on an RsslSeries, called after set definitions (if any) are encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be encoded and this function should be called prior to any summary data encoding.</li> <BR>
 *  <li>rsslEncodeSeriesSummaryDataComplete() - Completes encoding of summary data on an RsslSeries, called after summary data is encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be completed first, followed by any summary data encoding and this function.</li> <BR>
 *  <li>rsslDecodeSeries() - Decodes an RsslSeries container and provides access to buffer containing all encoded series entries.</li><BR>
 *  </ul>
 *  
 *  @subsection SeriesEntryOverview RsslSeriesEntry Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeSeriesEntry() - Encodes RsslSeriesEntry into an RsslSeries, where the RsslSeriesEntry has no payload or has pre-encoded content set in RsslSeriesEntry::encData.</li> <BR>
 *  <li>rsslEncodeSeriesEntryInit() - Begins RsslSeriesEntry encoding, initial function to call when encoding an RsslSeriesEntry without pre-encoded payload. </li>  <BR>
 *  <li>rsslEncodeSeriesEntryComplete() - Completes encoding of an RsslSeriesEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeSeriesEntry() - Decodes RsslSeries entries and provides access to individual series entries.  RsslSeries::containerType decode functions can be used to decode individual RsslSeriesEntry contents.</li> <BR>
 *  </ul>
 * 
 *  @section FilterOverview RsslFilterList Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeFilterListInit() - Begins RsslFilterList encoding, initial function to call when encoding an RsslFilterList type.</li><BR>
 *  <li>rsslEncodeFilterListComplete() - Completes encoding of an RsslFilterList, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeFilterList() - Decodes an RsslFilterList container and provides access to buffer containing all encoded filter entries.</li><BR>
 *  </ul>
 *  
 *  @subsection FilterEntryOverview RsslFilterEntry Encoding/Decoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeFilterEntry() - Encodes RsslFilterEntry into an RsslFilter, where the RsslFilterEntry has no payload or has pre-encoded content set in RsslFilterEntryEntry::encData.</li> <BR>
 *  <li>rsslEncodeFilterEntryInit() - Begins RsslFilterEntry encoding, initial function to call when encoding an RsslFilterEntry without pre-encoded payload. </li>  <BR>
 *  <li>rsslEncodeFilterEntryComplete() - Completes encoding of an RsslFilterEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired.</li><BR>
 *  <li>rsslDecodeFilterEntry() - Decodes RsslFilter entries and provides access to individual filter entries.  RsslFilter::containerType or RsslFilterEntry::containerType decode functions can be used to decode individual RsslFilterEntry contents.</li> <BR>
 *  </ul>
 *
 * @}
*/

 /**
 *  @defgroup RSSLMessageInterfaces Overview: Transport API Message Encode/Decode Interface Functions
 *  @{
 *
 *  This section contains a listing of the Transport API encode and decode functions available for the Transport API Messages.
 *
 *	@section MessageEncodeFunctions RsslMsg Encoding Interface Function List
 *  <ul>
 *  <li>rsslEncodeMsg() - Encodes an RsslMsg from a fully populated message structure.  If any message key attributes (\ref RsslMsgKey::encAttrib), \ref RsslMsgBase::extendedHeader, or payload (\ref RsslMsgBase::encDataBody) are required, these must be set prior to this function call.</li><BR>
 *  <li>rsslEncodeMsgInit() - Encodes an RsslMsg, where header encoding will return control to the user so any message key attributes (\ref RsslMsgKey::encAttrib), \ref RsslMsgBase::extendedHeader, or payload (\ref RsslMsgBase::encDataBody) content can be encoded.  When encoding is complete, the user must call rsslEncodeMsgComplete().  </li><BR>
 *  <li>rsslEncodeMsgKeyAttribComplete() - Used invoking rsslEncodeMsgInit(), if the application indicated that message key attributes (\ref RsslMsgKey::encAttrib) should be encoded but did not provide them as pre-encoded. After rsslEncodeMsgInit() returns and the application has completed their attribute encoding, this function is called to complete attribute encoding in the message and allow for encoding any further payload or extended header content. </li><BR>
 *  <li>rsslEncodeExtendedHeaderComplete() - Used invoking rsslEncodeMsgInit(), if the application indicated that the extended header should be encoded but did not provide it as pre-encoded. After rsslEncodeMsgInit() (and rsslEncodeMsgKeyAttribComplete() if needing to encode attributes) returns and the application has completed their extended header encoding, this function is called to complete extended header encoding in the message and allow for encoding any further payload. </li><BR>
 *  <li>rsslEncodeMsgComplete() - Completes RsslMsg encoding.  This is performed after completing all other portions of message encoding (e.g., after rsslEncodeMsgInit() completes and, if needed, rsslEncodeMsgKeyAttribComplete() and/or rsslEncodeExtendedHeaderComplete() complete.</li> <BR>
 *  <BR><BR>
 *  </ul>
 * 
 *	@section MessageDecodeFunctions RsslMsg Decoding Interface Function List
 *  <ul>
 *  <li>rsslDecodeMsg() - Decodes encoded message information into an RsslMsg.  Any message key attribute (\ref RsslMsgKey::encAttrib),  extended header, and payload (\ref RsslMsgBase::encDataBody) will not be decoded by this function call. </li><BR>
 *  <li>rsslDecodeMsgKeyAttrib() - If intending to decode any message key attribute (\ref RsslMsgKey::encAttrib) using the same \ref RsslDecodeIterator used when calling rsslDecodeMsg(), this function ensures the iterator is in the proper state to decode the attributes.  This is necessary when using the same iterator because the attribute and the payload may have the same type, which introduces ambiguity in the users intent.   </li><BR>
 *  <BR><BR>
 *  </ul>
 *
 * @}
*/

/* Start of Transport Group */ 

/**
 *	@defgroup RSSLTransport Transport API
 *	@{
 *	@}
 */
 
/* End of Transport Group */

/** 
 *	@defgroup RSSLWF Transport API Wire Format Codec API
 *	\brief The Transport API Wire Format Encoder/Decoder Package manages the binary representation of OMM content, ranging from simple primitive 
 *		   types up through more robust container and message types. This package is required by any application encoding or decoding OMM content.
 *	@{
 */






/**
 *	@defgroup RSSLWFCommon Transport API Codec Common Structures
 *	@brief The Transport API Codec Common Structures are used to encode and decode all OMM Messages, Primitives, and Containers.
 *  @{
 */
 
 /* Start of Iterator Group */

/**
 * @defgroup RsslIteratorGroup Transport API Iterator Reference Group
 *
 * @brief Transport API encode and decode iterators are used to manage the process for all RWF encoding and decoding.
 *
 * The Transport API Iterator functionality is defined in @header "rtr/rsslIterators.h"
 *
 *	@section RsslIterOverview Transport API Iterator Overview
 *	When encoding or decoding RWF content with the Transport API, the user leverages an iterator to manage the encoding or decoding process.
 *  The Transport API defines a single encode iterator type (\ref RsslEncodeIterator) and a single decode iterator type (\ref RsslDecodeIterator).  A single instance of this iterator
 *  can manage the full depth and breadth of the encoding or decoding process.  Alternatly, multiple iterator instances can be used to individually manage separate portions of the encode
 *  or decode process.  <BR><BR>
 *  The Transport API encoder/decoder does not provide any inherent threading or locking capability.  Separate iterator and type instances do not cause contention and do not share resources between instances.
 *  Any needed threading, locking, or thread-model implementation is at the discretion of the application.  Different application threads can encode or decode different messages without requiring a lock;
 *  thus each thread must use its own iterator instance and each message should be encoded or decoded using unique and independent buffers.  
 *  @note Though possible, Refinitiv recommends that you do not encode or decode related messages (messages flowing on the same stream) on different threads as this can impact the delivery order.
 *
 *  See \ref RsslIteratorVersion for information about encoding or decoding the correct version of RWF with the iterator.<BR>
 *  See \ref RsslDecodeIteratorType for information and examples related to the \ref RsslDecodeIterator.<BR>
 *  See \ref RsslEncodeIteratorType for information and examples related to the \ref RsslEncodeIterator.<BR>
 *
 * @{
 */

/**
 *	@defgroup RsslIteratorVersion Iterator RWF Protocol Versioning 
 *  @brief Detailed information about RWF versioning and the iterator.
 *  @{
 * 
 *	The Transport API iterators help the user to manage version information associated with the RWF content being exchanged.  When using the Transport API 
 *  Transport the protocol type and version information can be exchanged and negotiated on the connection (via the RsslConnectOptions or RsslBindOptions).  
 *  The Transport API will reject any connection establishment when the protocol type does not match across the connection.  If the protocol type does match,
 *  an appropriate major and minor version will be determined and this should be the version of RWF encoded or decoded when using this connection.
 *  After the connection becomes active, this negotiated version information is available and can then be provided to the iterator to ensure that the proper version is encoded or decoded.
 *  If not using the Transport API, the user can determine the desired version of RWF to encode and specify this information on their iterator.
 *  The Transport API provides RWF protocol type and protocol version values in @header "rtr\rsslIterators.h" for this purpose.  
 *  @note Specifying appropriate version information on \ref RsslDecodeIterator and \ref RsslEncodeIterator is important to future-proof applications and avoid incompatability if new RWF functionality is added.
 *
 *  @section IterVersUseExample Transport API Iterator Versioning Examples
 *  The following code example demonstrates associating version information with a Transport API iterator.
 *	@code
 *	//Create RsslEncodeIterator on the stack (this is the same process for RsslDecodeIterator)
 *	RsslEncodeIterator encIter;
 *	//Clear iterator
 *	rsslClearEncodeIterator(&encIter);
 *	//Associate RWF version (note that the clear function sets this to the 
 *	//default version - this only needs to be set if desired encode or decode version is different.
 *	if (rsslSetEncodeIteratorRWFVersion(&encIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION) < RSSL_RET_FAILURE)
 *	{
 *		//Handle error appropriatley 
 *		return 0;
 *	}
 *
 *	//--------------------------------------------------------
 *
 *	//Version information is typically negotiated by the Transport API and can be
 *	//obtained from the RsslChannel after it becomes ACTIVE.
 *	if (rsslSetEncodeIteratorRWFVersion(&encIter, pChnl->majorVersion, pChnl->minorVersion) < RSSL_RET_FAILURE)
 *	{
 *		//Handle error appropriatley 
 *		return 0;
 *	}
 *
 *	@endcode
 *  @}
 */


/**
 *	@defgroup RsslDecodeIteratorType Transport API Decode Iterator
 *  @brief Detailed information about the RsslDecodeIterator
 *  @{
 *
 *  All RWF decoding requires the use of an \ref RsslDecodeIterator, where a single iterator can manage the full decoding process.  
 *  For instance, if the application is decoding a message containing an RsslFieldList, the same \ref RsslDecodeIterator can be used
 *  to decode the RsslFieldList, all RsslFieldEntry, and all types (primitive types or container types) housed in the entries. 
 *  Seperate iterators can be used as well, depending on the needs of the application.  Following the same example, one 
 *  \ref RsslDecodeIterator can be used to decode the message information (up to the beginning of the RsslFieldList payload).  
 *  Another \ref RsslDecodeIterator reference can be used to decode the RsslFieldList and entries, and if desired, other 
 *  iterators can be used to decode the contents of each RsslFieldEntry.<BR><BR>
 *  Before decoding begins, the iterator should be initialized to ready it for decoding.  Initialization consists of several steps.  
 *  The rsslClearDecodeIterator() function can be used to initialize (or re-initialize for reuse) the \ref RsslDecodeIterator.  
 *  After clearing, an \ref RsslBuffer containing the content to decode should be associated with the \ref RsslDecodeIterator.
 *  In addition, RWF version information should be provided to the \ref RsslDecodeIterator so the desired version of RWF is 
 *  decoded.<BR><BR>
 *
 *  @section IterDecodeExample RsslDecodeIterator Example
 *  The following code example demonstrates creation of the Transport API decode iterator and associating with buffer to decode.
 *	@code
 *	//Create RsslDecodeIterator on the stack 
 *	RsslDecodeIterator decIter;
 *	//Clear iterator, using default wire format version (if not using default version 
 *	//this should be set using rsslSetDecodeIteratorRWFVersion()
 *	rsslClearDecodeIterator(&decIter);
 *	//Associate buffer to decode with the iterator
 *	if (rsslSetDecodeIteratorBuffer(&decIter, &decBuf) < RSSL_RET_FAILURE)
 *	{
 *		//Handle error appropriatley 
 *		return 0;
 *	}
 *	
 *
 *	@endcode
 *  @}
 */

/**
 *	@defgroup RsslEncodeIteratorType Transport API Encode Iterator
 *  @brief Detailed information about the RsslEncodeIterator
 *  @{ 
 *
 *  All RWF encoding requires the use of an \ref RsslEncodeIterator, where a single iterator can manage the full encoding process.
 *  For instance, if the application is encoding a message containing an RsslFieldList, the same \ref RsslEncodeIterator can be used
 *  to encode the message header information, the RsslFieldList information, each RsslFieldEntry, and the content of each entry 
 *  (primitive types or container types).  Seperate iterators can be used as well, depending on the needs of the application.  
 *  Following the same example, one \ref RsslEncodeIterator can be used to pre-encode an entry's content.  This pre-encoded content
 *  can then be set on the RsslFieldEntry and encoded using the \ref RsslEncodeIterator that is encoding the RsslFieldList.  This
 *  encoded field list content can then be set on the RsslMsg and yet another \ref RsslEncodeIterator can be used to encode the message
 *  and its pre-encoded payload.<BR><BR>
 *
 *  Before encoding begins, the iterator should be initialized to ready it for the encoding process.  Initialization consists of several steps.
 *  The rsslClearEncodeIterator() function can be used to initialize (or re-initalize for reuse) the \ref RsslEncodeIterator.
 *  After clearing, an \ref RsslBuffer with ample memory should be associated with the iterator; this will be the buffer that
 *  content is encoded into (if using with the Transport API, this is often a buffer obtained from the rsslGetBuffer() function so it can be immediatley written after encoding completes).
 *  In addition, RWF version information should be provided to the \ref RsslEncodeIterator so the desired version of RWF is encoded.
 *  @note When encoding is complete, the user should query the length of encoded content, setting this information in the \ref RsslBuffer::length.  
 *  If encoding fails, the \ref RsslEncodeIterator can be used to roll back content to the last successful encoding point.  <BR><BR>
 *
 *  @section IterEncodeExample RsslEncodeIterator Example
 *  The following code example demonstrates creation of the Transport API encode iterator and associating with buffer to encode into.
 *	@code
 *	//Create RsslEncodeIterator on the stack 
 *	RsslEncodeIterator encIter;
 *	//Clear iterator, using default wire format version (if not using default version 
 *	//this should be set using rsslSetEncodeIteratorRWFVersion()
 *	rsslClearEncodeIterator(&decIter);
 *	//Associate buffer to encode into with the iterator
 *	if (rsslSetEncodeIteratorBuffer(&encIter, &encBuf) < RSSL_RET_FAILURE)
 *	{
 *		//Handle error appropriatley 
 *		return 0;
 *	}
 *
 *	//Do encoding using iterator
 *
 *	//Check encoded length and set on buffer after encoding is complete
 *	encBuf.length = rsslGetEncodedBufferLength(&encIter);
 *
 *	@endcode
 *  @}
 */

/**  
 *  @}
 * 
 */

 /* End of Iterator Group */
 
 
/**
 *	@}
 */
 
/**
 *	@defgroup RSSLWFData Transport API Wire Format Data Package
 *	@brief The Transport API Data Package manages the binary representation of OMM data payload, ranging from simple primitive types 
 *		   through comprehensive hierarchal container types. This package is required by any application encoding or decoding OMM data payloads.
 *	@{
 */

/**
 *	@defgroup RSSLWFDataCommon Transport API Data Common 
 *	@brief The Transport API Data Package's common structures contain the necessary structures and helper functions to encode or decode any OMM formatted data.
 *	@{
 */

/**
 * @defgroup RSSLWFDataCommonHelpers Common Data Helper Functions
 *  @brief Detailed information about common data utility helper functions.
 * @{
 */
 
/**
 *	@}
 */

/**
 *	@defgroup RSSLWFDataConv Common Data Conversion Helper Functions
 *  @brief Detailed information about common data conversion helper functions.
 * @{
 *	@}
 */
 
/**
 * @defgroup DataPkgVersion Data Package Library Version
 * @brief This allows the user to programmatically extract version information about the library.
 * @{
 * 
 * @}
 */

/**
 *	@}
 */
 
/**
 *	@defgroup RSSLWFDataPrimitives Transport API Data Primitive Types
 *	@brief The Transport API Primitive Types are common typedef and structure definitions used to represent simple content like integers or length specified buffers.  These are available for use by other packages and applications alike and are required by the Transport API Data, Transport API Message, Transport API Dictionary and Transport API Transport packages.
 *	@{
 */
 

/**
 *	@defgroup DataPrimitive System Types
 *	@brief The system types are platform independent representations of system types (e.g. 8 bit signed integer, 16 bit unsigned integer, etc.).  
 *	@{
 *
 *	These definitions leverage some common platform type defines contained in @header "rtr/os.h" which is a header file used to streamline cross-platform
 *	development.
 *  The system types are provided for use with Transport API interfaces and internal implementation.  
 *  @note Some system types may not have a specific corresponding RWF type for encoding or decoding.
 *
 */

/**
 *	@}
 */
 

/**
 *	@defgroup RSSLPrimitiveEncoders Transport API System Primitive Encoder Functions
 *	@brief These functions are used to encode selected system primitive types using an RsslEncodeIterator .
 *	@{
 *	@}
 */
  


/* Start of RsslGeneralUtils */

/**
 *	@defgroup RsslGeneralPrimitiveTypes Generic Primitive Type Utilities
 *  @brief Detailed information about generic functions that can help encode, decode, or convert many primitive types
 *
 * This functionality is defined in @header "rtr/rsslPrimitiveEncoders.h" and @header "rtr/rsslPrimitiveDecoders.h"
 *
 *
 * @section GeneralEnc Generic Primitive Type Encoding Functions
 * The following function can be used to encode nearly any primitive type.<BR>  
 * See \ref RsslGeneralEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodePrimitiveType() - Encodes any primitive type other than RsslArray.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section GeneralDec Generic Primitive Type Decoding Functions
 * The following function can be used to decode nearly any primitive type.<BR>
 * See \ref RsslGeneralDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodePrimitiveType() - Decodes any primitive type other than RsslArray.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section GeneralUtils Generic Primitive Type Utility & Conversion Functions
 * The Transport API C Edition provides several common conversion helpers for primitive types.<BR>
 * See \ref RsslGeneralUtils for more detailed information.<BR><BR>
 * <BR><BR>
 *
 * @{
 */


/**
 *  @defgroup RsslGeneralEnc Generic Primitive Type Encoding
 *  @brief Detailed information about generic primitive type encoding functions.
 *  @{
 * 
 *  Details for encoding a specific primitive type can be found in that type's respective section.
 *  
 *  @section GeneralEncExample Generic Primitive Type Encoding Example
 *	The following example demonstrates generic encoding of an RsslReal.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslReal with 654.23
 *	RsslReal rReal = {RSSL_FALSE, RSSL_RH_EXPONENT_2, 65423};
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodePrimitiveType() function to encode content
 *	if ((retVal = rsslEncodePrimitiveType(&encIter, RSSL_DT_REAL, (void*)&rReal)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodePrimitiveType.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslGeneralDec Generic Primitive Type Decoding
 *  @brief Detailed information about generic primitive type decoding functions.
 *  @{
 *
 *	Details for decoding a specific primitive type can be found in that type's respective section.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned and the value is populated based on the blanking rules defined in each type's respective section.
 * 
 *  @section GeneralDecExample Generic Primitive Type Decoding Example
 *  The following example demonstrates generic decoding.  This assumes the type was already known or looked up in a field dictionary.
 *  @code
 *	RsslInt rInt; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	retVal = rsslDecodePrimitiveType(&decIter, RSSL_DT_INT, &rInt);
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodePrimitiveType.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */

/**
 *
 *	@defgroup RsslGeneralUtils Generic Primitive Type Utility & Conversion Functions
 *  @brief Detailed information about generic primitive type utility and conversion helper functions
 *  @{
 *
 *  There are several helper and conversion functions provided for use with the Transport API.  
 * 
 *  @}
 *
 */

/**  
 *  @}
 * 
 */

/* End of RsslGeneralUtils */




/* Start of RsslInt */

/**
 * @defgroup RsslIntGroup RsslInt Reference Group
 *
 * @brief The integer primitive type allows the user to represent signed integer content ranging from -2<sup>63</sup> to 2<sup>63</sup> - 1. 
 *
 * The \ref RsslInt functionality is defined in @header "rtr/rsslTypes.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section IntType RsslInt Primitive Type Overview
 * The \ref RsslInt primitive type allows the user to represent signed integers ranging from -2<sup>63</sup> to 2<sup>63</sup> - 1.  
 * This is defined as a type definition to the system's native 64-bit signed type.<BR>
 * See \ref RsslIntType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section IntEnc RsslInt Encoding Functions
 * The following function is used while encoding an \ref RsslInt.<BR>  
 * See \ref RsslIntEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeInt() - Encodes an \ref RsslInt primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section IntDec RsslInt Decoding Functions
 * The following function is used while decoding an \ref RsslInt.<BR>
 * See \ref RsslIntDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeInt() - Decodes an \ref RsslInt primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslIntType RsslInt Primitive Type
 *  @brief Detailed information about \ref RsslInt type
 *  @{
 *
 * The \ref RsslInt primitive type allows the user to represent signed integers ranging from from -2<sup>63</sup> to 2<sup>63</sup> - 1.  
 * This is defined as a type definition to the system's native 64-bit signed type.<BR>
 * @note When represented as standard data (e.g. not set defined) in an RsslFieldList or RsslElementList, this type can be represented as blank.  When 
 * encoding as set defined this cannot be blank.  See documentation of those types for detailed information about how to encode or decode blank entries.
 *
 *  @section IntUseExample RsslInt Creation Examples
 *  The following code example demonstrates stack creation and population of an \ref RsslInt.  Because this is a simple primitive type, there is no initializer or clear function required.
 *	@code
 *	//Create RsslInt on the stack
 *	RsslInt rInt = 42;
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslIntEnc RsslInt Encoding
 *  @brief Detailed information about \ref RsslInt encoding
 *  @{
 * 
 *  When encoding an \ref RsslInt, only the number of bytes necessary to represent the contained value will be encoded.
 *  
 *  @section IntEncExample RsslInt Encoding Example
 *	The following example demonstrates encoding an \ref RsslInt.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslInt
 *	RsslInt rInt = -123456;
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeInt() function to encode content
 *	if ((retVal = rsslEncodeInt(&encIter, &rInt)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeInt.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslIntDec RsslInt Decoding
 *  @brief Detailed information about \ref RsslInt decoding
 *  @{
 *
 *  When decoding an \ref RsslInt, although fewer bytes may be on the wire, the full signed integer value is reconstructed into the \ref RsslInt type.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned.
 * 
 *  @section IntDecExample RsslInt Decoding Example
 *  @code
 *	RsslInt rInt; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeInt(&decIter, &rInt)) < RSSL_RET_SUCCESS)
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeInt.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslInt */


/* Start of RsslUInt */

/**
 * @defgroup RsslUIntGroup RsslUInt Reference Group
 *
 * @brief The unsigned integer primitive type allows the user to represent unsigned integer values ranging from 0 to 2<sup>64</sup> - 1. 
 *
 * The \ref RsslUInt functionality is defined in @header "rtr/rsslTypes.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section UIntType RsslUInt Primitive Type Overview
 * The \ref RsslUInt primitive type allows the user to represent unsigned integers ranging from from 0 to 2<sup>64</sup> - 1.  
 * This is defined as a type definition to the system's native 64-bit unsigned type.<BR>
 * See \ref RsslUIntType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section UIntEnc RsslUInt Encoding Functions
 * The following function is used while encoding an \ref RsslUInt.<BR>  
 * See \ref RsslUIntEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeUInt() - Encodes an \ref RsslUInt primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section UIntDec RsslUInt Decoding Functions
 * The following function is used while decoding an \ref RsslUInt.<BR>
 * See \ref RsslUIntDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeUInt() - Decodes an \ref RsslUInt primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslUIntType RsslUInt Primitive Type
 *  @brief Detailed information about \ref RsslUInt type
 *  @{
 *
 * The \ref RsslUInt primitive type allows the user to represent unsigned integers ranging from from 0 to 2<sup>63</sup> - 1.  
 * This is defined as a type definition to the system's native 64-bit signed type.<BR>
 * @note When represented as standard data (e.g. not set defined) in an RsslFieldList or RsslElementList, this type can be represented as blank.  When 
 * encoding as set defined this cannot be blank.  See documentation of those types for detailed information about how to encode or decode blank entries.
 *
 *  @section UIntUseExample RsslUInt Creation Examples
 *  The following code example demonstrates stack creation and population of an \ref RsslUInt.  Because this is a simple primitive type, there is no initializer or clear function required.
 *	@code
 *	//Create RsslUInt on the stack
 *	RsslUInt rUInt = 14002;
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslUIntEnc RsslUInt Encoding
 *  @brief Detailed information about \ref RsslUInt encoding
 *  @{
 * 
 *  When encoding an \ref RsslUInt, only the number of bytes necessary to represent the contained value will be encoded.
 *  
 *  @section UIntEncExample RsslUInt Encoding Example
 *	The following example demonstrates encoding an \ref RsslUInt.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslUInt
 *	RsslUInt rUInt = 65535;
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeUInt() function to encode content
 *	if ((retVal = rsslEncodeUInt(&encIter, &rUInt)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeUInt.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslUIntDec RsslUInt Decoding
 *  @brief Detailed information about \ref RsslUInt decoding
 *  @{
 *
 *  When decoding an \ref RsslUInt, although fewer bytes may be on the wire, the full unsigned integer value is reconstructed into the \ref RsslUInt type.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned.
 * 
 *  @section UIntDecExample RsslUInt Decoding Example
 *  @code
 *	RsslInt rUInt; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeUInt(&decIter, &rUInt)) < RSSL_RET_SUCCESS)
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeUInt.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslUInt */


/* Start of RsslReal */

/**
 * @defgroup RsslRealGroup RsslReal Reference Group
 *
 * @brief The \ref RsslReal Primitive Type is an optimized wire representation of a decimal or fractional value.  
 *
 * The \ref RsslReal functionality is defined in @header "rtr/rsslReal.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section RealType RsslReal Primitive Type Overview
 * The \ref RsslReal Primitive Type is an optimized wire representation of a decimal or fractional value.  Through the use of a formatting hint that conveys 
 * decimal point location or fractional denominator information, the \ref RsslReal generally requires less bytes on the wire than float or double types.  
 * The \ref RsslReal primitive type allows the user to represent signed values ranging from -2<sup>63</sup> to 2<sup>63</sup> - 1. This can be combined
 * with hint values to add or remove up to seven trailing zeros, fourteen decimal places, or fractional denominators up to 256.<BR>
 * See \ref RsslRealType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section RealEnc RsslReal Encoding Functions
 * The following function is used while encoding an \ref RsslReal.<BR>  
 * See \ref RsslRealEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeReal() - Encodes an \ref RsslReal primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section RealDec RsslReal Decoding Functions
 * The following function is used while decoding an \ref RsslReal.<BR>
 * See \ref RsslRealDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeReal() - Decodes an \ref RsslReal primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section RealUtils RsslReal Utility & Conversion Functions
 * The Transport API C Edition provides several common conversion helpers for the \ref RsslReal.  These can help with converting between \ref RsslReal and double, float, and numeric strings.<BR>
 * See \ref RsslRealUtils for more detailed information.<BR><BR>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslRealType RsslReal Primitive Type
 *  @brief Detailed information about \ref RsslReal type
 *  @{
 *
 * The \ref RsslReal Primitive Type is an optimized wire representation of a decimal or fractional value.  Through the use of a formatting hint that conveys 
 * decimal point location or fractional denominator information, the \ref RsslReal generally requires less bytes on the wire than float or double types.  
 * The \ref RsslReal primitive type allows the user to represent signed values ranging from -2<sup>63</sup> to 2<sup>63</sup> - 1. This can be combined
 * with hint values to add or remove up to seven trailing zeros, fourteen decimal places, or fractional denominators up to 256.<BR>
 * @note <ul><li>When using the \ref RsslReal initializer or clear functions, the RsslReal::hint is set to <b>14</b>, which corresponds to ::RSSL_RH_EXPONENT0. The user should ensure that
 * the RsslReal::hint is populated appropriatley in correspondance to the value being encoded.</li>
 * <li>The \ref RsslReal type can be represented as blank when sent as standard and set-defined data when contained in an RsslFieldList or RsslElementList.</li></ul>
 *
 *  @section RealUseExample RsslReal Creation Examples
 *  The following code example demonstrates stack creation, initialization, and population of an \ref RsslReal.
 *	@code
 *	//Create RsslReal on the stack
 *	RsslReal rReal = RSSL_INIT_REAL;
 *	//Populate RsslReal with value 123.45
 *	rReal.value = 12345;
 *	rReal.hint = RSSL_RH_EXPONENT_2;
 *	
 *	//------------------------------------------
 *
 *	//We can clear the RsslReal for initial use (or reuse) using the clear function instead of static initializer
 *	RsslReal rReal;
 *	rsslClearReal(&rReal);
 *	//Populate RsslReal with value 29/32
 *	rReal.value = 29;
 *	rReal.hint = RSSL_RH_FRACTION_32;
 *
 *	//--------------------------------------------
 * 
 *	//Alternatly, we can create and populate the RsslReal in a single line
 *	//Populate real with 2956000
 *	RsslReal rReal = {RSSL_FALSE, RSSL_RH_EXPONENT3, 2956 };
 *
 *	//--------------------------------------------
 *
 *	//To send an RsslReal as blank, set the isBlank Boolean
 *	RsslReal rReal = RSSL_INIT_REAL;
 *	rReal.isBlank = RSSL_TRUE;
 *
 *	//--------------------------------------------
 *
 *	//or use the provided blank helper function
 *	RsslReal rReal;
 *	rsslBlankReal(&rReal);
 *	
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslRealEnc RsslReal Encoding
 *  @brief Detailed information about \ref RsslReal encoding
 *  @{
 * 
 *  When encoding an \ref RsslReal, only the number of bytes necessary to represent the contained value will be encoded.
 *	@note If sending \ref RsslReal as blank, set the RsslReal::isBlank to RSSL_TRUE.
 *  
 *  @section RealEncExample RsslReal Encoding Example
 *	The following example demonstrates encoding an \ref RsslReal.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslReal with 654.23
 *	RsslReal rReal = {RSSL_FALSE, RSSL_RH_EXPONENT_2, 65423};
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeReal() function to encode content
 *	if ((retVal = rsslEncodeReal(&encIter, &rReal)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeReal.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslRealDec RsslReal Decoding
 *  @brief Detailed information about \ref RsslReal decoding
 *  @{
 *
 *  When decoding an \ref RsslReal, although fewer bytes may be on the wire, the full value is reconstructed into the \ref RsslReal type.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned and the \ref RsslReal::isBlank is set to RSSL_TRUE.
 * 
 *  @section RealDecExample RsslReal Decoding Example
 *  @code
 *	RsslReal rReal; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	retVal = rsslDecodeReal(&decIter, &rReal);
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeReal.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */

/**
 *
 *	@defgroup RsslRealUtils RsslReal Utility & Conversion Functions
 *  @brief Detailed information about \ref RsslReal utility and conversion helper functions
 *  @{
 *
 *  There are several helper and conversion functions provided for use with the RsslReal.  These can help determine equality of two RsslReal structures and aid in
 *  conversion between RsslReal and double, float, and numeric strings. A user can also use the RsslReal::value and RsslReal::hint to manually perform the conversion calculation.  
 *  See the \ref RealConvExample for details.
 * 
 *  @section RealConvExample RsslReal Conversion Example
 *  This example assumes that the RsslReal is not blank and is populated with an RsslReal::value and RsslReal::hint that corresponds to one of the \ref RsslRealHints.  
 *  @note The conversion calculations shown require use of the <b>pow</b> and <b>floor</b> function, included in the math system library.  
 *
 *  @code
 *	//To convert from a populated RsslReal to a double, the following calculation can be performed:
 *	double outputValue;
 *	if (rsslReal.hint < RSSL_RH_FRACTION_1)
 *	{
 *		//RsslReal represents a decimal value
 *		//multiply the RsslReal::value by 10^(scaled RsslReal::hint)
 *		outputValue = rsslReal.value*(pow(10, (rsslReal.hint - RSSL_RH_EXPONENT0)));
 *	}
 *	else
 *	{
 *		//RsslReal represents a fractional value
 *		//divide the RsslReal::value by 2^(scaled RsslReal::hint)
 *		outputValue = rsslReal.value/(pow(2, (rsslReal.hint - RSSL_RH_FRACTION_1)));
 *	}
 *
 *	//To convert from a double to an RsslReal, the following calculation can be performed:
 *	RsslReal outReal = RSSL_INIT_REAL;
 *	//Note that this conversion requires the RsslReal::hint to be known and does not determine the best hint to use
 *	outReal.hint = inHint;
 *	if (inHint < RSSL_RH_FRACTION_1)
 *	{
 *		//RsslReal will represent a decimal value
 *		//divide the double by 10^(scaled RsslReal::hint), add 0.5 and take the floor
 *		outReal.value = floor((doubleVal/(pow(10, (inHint - RSSL_RH_EXPONENT0)))) + 0.5);
 *	}
 *	else
 *	{
 *		//RsslReal will represent a fractional value
 *		//multiply the double by 2^(scaled RsslReal::hint)
 *		outReal.value = doubleVal*(pow(2, (inHint - RSSL_RH_FRACTION_1)));
 *	}
 *
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslReal */


/* Start of RsslFloat */

/**
 * @defgroup RsslFloatGroup RsslFloat Reference Group
 *
 * @brief The float primitive type allows the user to represent 4-byte IEEE-754 floating point values. 
 *
 * The \ref RsslFloat functionality is defined in @header "rtr/rsslTypes.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section FloatType RsslFloat Primitive Type Overview
 * The \ref RsslFloat primitive type allows the user to represent IEEE-754 floating point values.
 * This is defined as a type definition to the system's native float type.<BR>
 * See \ref RsslFloatType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section FloatEnc RsslFloat Encoding Functions
 * The following function is used while encoding an \ref RsslFloat.<BR>  
 * See \ref RsslFloatEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeFloat() - Encodes an \ref RsslFloat primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section FloatDec RsslFloat Decoding Functions
 * The following function is used while decoding an \ref RsslFloat.<BR>
 * See \ref RsslFloatDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeFloat() - Decodes an \ref RsslFloat primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslFloatType RsslFloat Primitive Type
 *  @brief Detailed information about \ref RsslFloat type
 *  @{
 *
 * The \ref RsslFloat primitive type allows the user to represent IEEE-754 floating point values.
 * This is defined as a type definition to the system's native float type.<BR>
 * @note When represented as standard data (e.g. not set defined) in an RsslFieldList or RsslElementList, this type can be represented as blank.  When 
 * encoding as set defined this cannot be blank.  See documentation of those types for detailed information about how to encode or decode blank entries.
 *
 *  @section FloatUseExample RsslFloat Creation Examples
 *  The following code example demonstrates stack creation and population of an \ref RsslFloat.  Because this is a simple primitive type, there is no initializer or clear function required.
 *	@code
 *	//Create RsslFloat on the stack
 *	RsslFloat rFloat = 3.14159;
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslFloatEnc RsslFloat Encoding
 *  @brief Detailed information about \ref RsslFloat encoding
 *  @{
 * 
 *	The \ref RsslFloat follows the IEEE-754 specification.
 *  
 *  @section FloatEncExample RsslFloat Encoding Example
 *	The following example demonstrates encoding an \ref RsslFloat.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslFloat
 *	RsslFloat rFloat = 6.02;
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeFloat() function to encode content
 *	if ((retVal = rsslEncodeFloat(&encIter, &rFloat)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeFloat.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslFloatDec RsslFloat Decoding
 *  @brief Detailed information about \ref RsslFloat decoding
 *  @{
 *
 *  When decoding an \ref RsslFloat, the floating point value on the wire is reconstructured into an \ref RsslFloat.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned.
 * 
 *  @section FloatDecExample RsslFloat Decoding Example
 *  @code
 *	RsslFloat rFloat; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeFloat(&decIter, &rFloat)) < RSSL_RET_SUCCESS)
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeFloat.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslFloat */


/* Start of RsslDouble */

/**
 * @defgroup RsslDoubleGroup RsslDouble Reference Group
 *
 * @brief The double primitive type allows the user to represent 8-byte IEEE-754 floating point values. 
 *
 * The \ref RsslDouble functionality is defined in @header "rtr/rsslTypes.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section DoubleType RsslDouble Primitive Type Overview
 * The \ref RsslDouble primitive type allows the user to represent 8-byte IEEE-754 floating point values.
 * This is defined as a type definition to the system's native double type.<BR>
 * See \ref RsslDoubleType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section DoubleEnc RsslDouble Encoding Functions
 * The following function is used while encoding an \ref RsslDouble.<BR>  
 * See \ref RsslDoubleEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeDouble() - Encodes an \ref RsslDouble primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section DoubleDec RsslDouble Decoding Functions
 * The following function is used while decoding an \ref RsslDouble.<BR>
 * See \ref RsslDoubleDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeDouble() - Decodes an \ref RsslDouble primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslDoubleType RsslDouble Primitive Type
 *  @brief Detailed information about \ref RsslDouble type
 *  @{
 *
 * The \ref RsslDouble primitive type allows the user to represent 8-byte IEEE-754 floating point values.
 * This is defined as a type definition to the system's native double type.<BR>
 * @note When represented as standard data (e.g. not set defined) in an RsslFieldList or RsslElementList, this type can be represented as blank.  When 
 * encoding as set defined this cannot be blank.  See documentation of those types for detailed information about how to encode or decode blank entries.
 *
 *  @section DoubleUseExample RsslDouble Creation Examples
 *  The following code example demonstrates stack creation and population of an \ref RsslDouble.  Because this is a simple primitive type, there is no initializer or clear function required.
 *	@code
 *	//Create RsslDouble on the stack
 *	RsslFloat rFloat = 867.5309;
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslDoubleEnc RsslDouble Encoding
 *  @brief Detailed information about \ref RsslDouble encoding
 *  @{
 * 
 *	The \ref RsslDouble follows the IEEE-754 specification.
 *  
 *  @section DoubleEncExample RsslDouble Encoding Example
 *	The following example demonstrates encoding an \ref RsslDouble.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslDouble
 *	RsslDouble rDouble = 12345.6789;
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeDouble() function to encode content
 *	if ((retVal = rsslEncodeDouble(&encIter, &rDouble)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeDouble.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslDoubleDec RsslDouble Decoding
 *  @brief Detailed information about \ref RsslDouble decoding
 *  @{
 *
 *  When decoding an \ref RsslDouble, the floating point value on the wire is reconstructured into an \ref RsslDouble.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned.
 * 
 *  @section DoubleDecExample RsslDouble Decoding Example
 *  @code
 *	RsslDouble rDouble; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeDouble(&decIter, &rDouble)) < RSSL_RET_SUCCESS)
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeDouble.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslDouble */


/* Start of RsslBuffer */

/**
 * @defgroup RsslBufferGroup RsslBuffer Reference Group
 *
 * @brief The buffer primitive type allows the user to represent length specified content. 
 *
 * The RsslBuffer functionality is defined in @header "rtr/rsslTypes.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section BufStruct RsslBuffer Structure Overview
 * The \ref RsslBuffer primitive type allows the user to represent length specified content.  This includes various string types (ASCII, RMTES, UTF8) or non-string opaque content.
 * The buffer consists of a length and a pointer, where the pointer is used to refer to the content and the length indicates the number of bytes referred to.<BR>
 * See \ref RsslBufferStruct for more detailed information. <BR>
 * <BR><BR>
 *
 * @section BufEnc RsslBuffer Encoding Functions
 * The following function is used while encoding an \ref RsslBuffer.<BR>  
 * See \ref RsslBufferEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeBuffer() - Encodes an \ref RsslBuffer primitive type.  This will copy contents into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section BufDec RsslBuffer Decoding Functions
 * The following function is used while decoding an \ref RsslBuffer.<BR>
 * See \ref RsslBufferDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeBuffer() - Decodes an \ref RsslBuffer primitive type.  This will point to the contents in the buffer referred to by the \ref RsslDecodeIterator.  For performance purposes, contents will not be copied.</li><BR>
 * </ul>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslBufferStruct RsslBuffer Primitive Type
 *  @brief Detailed information about RsslBuffer structure and helper functions
 *  @{
 *
 * The \ref RsslBuffer primitive type allows the user to represent length specified content.  This includes various string types (ASCII, RMTES, UTF8) or non-string opaque content.
 * The buffer consists of a length and a pointer, where the pointer is used to refer to the content and the length indicates the number of bytes referred to.  
 * @note <ul><li>When sending string types, a NULL terminator is typically not sent or included in the length.  The length should be used to determine where the buffer contents end.</li>
 * <li>The \ref RsslBuffer type can be represented as blank when sent as standard and set-defined data when contained in an RsslFieldList or RsslElementList.</li></ul>
 *
 *
 *  @section BufferUseExample RsslBuffer Creation Examples
 *  The following code example demonstrates stack creation and initialization of an \ref RsslBuffer along with use of the clear function to reuse the array.
 *  This is typical usage prior to encoding an \ref RsslBuffer.  When decoding, the \ref RsslBuffer will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	//Create RsslBuffer on the stack and use static initializer
 *	RsslBuffer buffer = RSSL_INIT_BUFFER;
 *	//Populate buffer
 *	buffer.data = "ABCD";
 *	buffer.length = 4;
 *	
 *	//------------------------------------------
 *
 *	//We can clear the buffer for initial use (or reuse) using the clear function instead of static initializer
 *	RsslBuffer buffer;
 *	rsslClearBuffer(&buffer);
 *	//Populate buffer
 *	buffer.data = "ABCD";
 *	buffer.length = 4;
 *
 *	//--------------------------------------------
 * 
 *	//Alternatly, we can create and populate the buffer in a single line
 *	RsslBuffer buffer = {4, "ABCD"};
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslBufferEnc RsslBuffer Encoding
 *  @brief Detailed information about RsslBuffer encoding
 *  @{
 * 
 *  When encoding an \ref RsslBuffer, the \ref RsslBuffer::data should point to content to be encoded and the 
 *  \ref RsslBuffer::length should indicate the number of bytes being pointed to.  
 *  \note Encoding an \ref RsslBuffer will result in the contents being copied into the buffer referred to by the \ref RsslEncodeIterator.  
 * 
 *  @section BufEncExample RsslBuffer Encoding Example
 *	The following example demonstrates encoding an \ref RsslBuffer.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the buffer, no need to clear since setting both members.
 *	RsslBuffer buf = {3, "TRI"};
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeBuffer() function to encode content
 *	if ((retVal = rsslEncodeBuffer(&encIter, &buf)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeBuffer.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslBufferDec RsslBuffer Decoding
 *  @brief Detailed information about RsslBuffer decoding
 *  @{
 *
 *  When decoding an \ref RsslBuffer, the \ref RsslBuffer::data will point to the decoded content 
 *  and the \ref RsslBuffer::length will indicate the number of bytes pointed to.
 *  \note <ul><li>When decoding an RsslBuffer, contents are not copied for performance purposes.  If content needs to be modified or
 *  requires a longer life span, users can copy content.</li>
 *	<li>When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned and the \ref RsslBuffer::length is set to 0.</li></ul>
 * 
 *  @section BufDecExample RsslBuffer Decoding Example
 *  @code
 *	RsslBuffer buf; //No need to clear since we will decode into this
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeBuffer(&decIter, &buf)) < RSSL_RET_SUCCESS)
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeBuffer.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslBuffer */


/* Start of RsslDate */

/**
 * @defgroup RsslDateGroup RsslDate Reference Group
 *
 * @brief The RsslDate Primitive Type is an optimized wire representation of date that can contain month, day, and year.  
 *
 * The RsslDate functionality is defined in @header "rtr/rsslDateTime.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section DateType RsslDate Primitive Type Overview
 * The RsslDate Primitive Type is an optimized wire representation of a date containing month, day, and year.  
 * This type allows for representing the entire date or individual portions as blank, for example having a valid month and year but a blank day.
 * See \ref RsslDateType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section DateEnc RsslDate Encoding Functions
 * The following function is used while encoding an RsslDate.<BR>  
 * See \ref RsslDateEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeDate() - Encodes an RsslDate primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section DateDec RsslDate Decoding Functions
 * The following function is used while decoding an RsslDate.<BR>
 * See \ref RsslDateDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeDate() - Decodes an RsslDate primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section DateUtils RsslDate Utility & Conversion Functions
 * The Transport API C Edition provides several common conversion helpers for the RsslDate.  These can help with comparing and converting between RsslDate and various date strings.<BR>
 * See \ref RsslDateUtils for more detailed information.<BR><BR>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslDateType RsslDate Primitive Type
 *  @brief Detailed information about RsslDate type
 *  @{
 *
 * The RsslDate Primitive Type is an optimized wire representation of a date containing month, day, and year.  
 * This type allows for representing the entire date or individual portions as blank, for example having a valid month and year but a blank day.
 * @note <ul><li>When using the RsslDate initializer or clear functions, all values are set to <b>0</b>, essentially setting the RsslDate to blank. 
 * The user should ensure that the RsslDate is populated appropriatley in correspondance to the actual date value being encoded, if not sending a blank date.</li>
 * <li>The RsslDate type can be represented as blank when sent as standard and set-defined data when contained in an RsslFieldList or RsslElementList.</li></ul>
 *
 *  @section DateUseExample RsslDate Creation Examples
 *  The following code example demonstrates stack creation, initialization, and population of an RsslDate.
 *	@code
 *	//Create RsslDate on the stack
 *	RsslDate rDate = RSSL_INIT_DATE;
 *	//Populate RsslDate with a month, day, and year
 *	rDate.month = 10;
 *	rDate.day = 21;
 *	rDate.year = 1978;
 *	
 *	//------------------------------------------
 *
 *	//We can clear the RsslDate for initial use (or reuse) using the clear function instead of static initializer
 *	RsslDate rDate;
 *	rsslClearDate(&rDate);
 *	//Populate RsslDate with just a month and year, day was cleared and set to 0 so it is already blank
 *	rDate.month = 3;
 *	rDate.year = 2012;
 *
 *	//--------------------------------------------
 * 
 *	//Alternatly, we can create and populate the RsslDate in a single line
 *	//Populate RsslDate with day, month, and year
 *	RsslDate rDate = { 25, 12, 2012 };
 *
 *	//--------------------------------------------
 *
 *	//To send an RsslDate as blank, we can use the static blank initializer
 *	RsslDate rDate = RSSL_BLANK_DATE;
 *
 *	//--------------------------------------------
 *
 *	//or use the provided blank helper function
 *	RsslDate rDate;
 *	rsslBlankDate(&rDate);
 *	
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslDateEnc RsslDate Encoding
 *  @brief Detailed information about RsslDate encoding
 *  @{
 * 
 *  When encoding an RsslDate, only the number of bytes necessary to represent the contained value will be encoded.
 *	@note If sending RsslDate as blank, use one of the blank initializers.  If sending only a portion of the date as blank (e.g. blank day), set the corresponding member to <b>0</b>.
 *  
 *  @section DateEncExample RsslDate Encoding Example
 *	The following example demonstrates encoding an RsslDate.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslDate with a day, month, and year.
 *	RsslDate rDate = {9, 7, 2011};
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeDate() function to encode content
 *	if ((retVal = rsslEncodeDate(&encIter, &rDate)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeDate.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslDateDec RsslDate Decoding
 *  @brief Detailed information about RsslDate decoding
 *  @{
 *
 *  When decoding an RsslDate, although fewer bytes may be on the wire, the full value is reconstructed into the RsslDate type.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned and the RsslDate::day, RsslDate::month, and RsslDate::year are set to <b>0</b>
 * 
 *  @section DateDecExample RsslDate Decoding Example
 *  @code
 *	RsslDate rDate; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	retVal = rsslDecodeDate(&decIter, &rDate);
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeDate.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */

/**
 *
 *	@defgroup RsslDateUtils RsslDate Utility & Conversion Functions
 *  @brief Detailed information about RsslDate utility and conversion helper functions
 *  @{
 *
 *  There are several helper and conversion functions provided for use with the RsslDate.  
 *  These can help with comparing and converting between RsslDate and various date strings.
 *
 * @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslDate */


/* Start of RsslTime */

/**
 * @defgroup RsslTimeGroup RsslTime Reference Group
 *
 * @brief The RsslTime Primitive Type is an optimized wire representation of time that can contain hour, minute, second, and millisecond.  
 *
 * The RsslTime functionality is defined in @header "rtr/rsslDateTime.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section TimeType RsslTime Primitive Type Overview
 * The RsslTime Primitive Type is an optimized wire representation of a time containing hour, minute, second, and millisecond.  
 * This type allows for representing the entire time or individual portions as blank, for example having a valid hour and minute but blank second and millisecond.
 * See \ref RsslTimeType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section TimeEnc RsslTime Encoding Functions
 * The following function is used while encoding an RsslTime.<BR>  
 * See \ref RsslTimeEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeTime() - Encodes an RsslTime primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section TimeDec RsslTime Decoding Functions
 * The following function is used while decoding an RsslTime.<BR>
 * See \ref RsslTimeDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeTime() - Decodes an RsslTime primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section TimeUtils RsslTime Utility & Conversion Functions
 * The Transport API C Edition provides several common conversion helpers for the RsslTime.  These can help with comparing and converting between RsslTime and various time strings.<BR>
 * See \ref RsslTimeUtils for more detailed information.<BR><BR>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslTimeType RsslTime Primitive Type
 *  @brief Detailed information about RsslTime type
 *  @{
 *
 * The RsslTime Primitive Type is an optimized wire representation of a time containing hour, minute, second, and millisecond.  
 * This type allows for representing the entire time or individual portions as blank, for example having a hour and minute but blank second and millisecond.
 * @note <ul><li>When using the RsslTime initializer or clear functions, all values are set to <b>0</b>. This differs from a blank time where hour, minute, and second are set to <b>255</b> and millisecond is set to <b>65535</b>.
 * The user should ensure that the RsslTime is populated appropriatley in correspondance to the actual time value being encoded, if not sending a blank time.</li>
 * <li>The RsslTime type can be represented as blank when sent as standard and set-defined data when contained in an RsslFieldList or RsslElementList.</li></ul>
 *
 *  @section TimeUseExample RsslTime Creation Examples
 *  The following code example demonstrates stack creation, initialization, and population of an RsslTime.
 *	@code
 *	//Create RsslTime on the stack
 *	RsslTime rTime = RSSL_INIT_TIME;
 *	//Populate RsslTime with hour and minute
 *	rTime.hour = 9;
 *	rTime.minute = 20;
 *	
 *	//------------------------------------------
 *
 *	//We can clear the RsslTime for initial use (or reuse) using the clear function instead of static initializer
 *	RsslTime rTime;
 *	rsslClearTime(&rTime);
 *	//Populate RsslTime with hour, minute, and second.
 *	rTime.hour = 11;
 *	rTime.minute = 45;
 *	rTime.second = 3;
 *
 *	//--------------------------------------------
 * 
 *	//Alternatly, we can create and populate the RsslTime in a single line
 *	//Populate RsslTime with hour, minute, second, and millisecond
 *	RsslTime rTime = { 10, 15, 52, 0 };
 *
 *	//--------------------------------------------
 *
 *	//To send an RsslTime as blank, we can use the static blank initializer
 *	RsslTime rTime = RSSL_BLANK_TIME;
 *
 *	//--------------------------------------------
 *
 *	//or use the provided blank helper function
 *	RsslTime rTime;
 *	rsslBlankTime(&rTime);
 *	
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslTimeEnc RsslTime Encoding
 *  @brief Detailed information about RsslTime encoding
 *  @{
 * 
 *  When encoding an RsslTime, only the number of bytes necessary to represent the contained value will be encoded.  Any trailing <b>0</b> values (e.g. RsslTime::millisecond is set to <b>0</b>) will be inferred.
 *	@note If sending RsslTime as blank, use one of the blank initializers.  If sending only a portion of the time as blank (e.g. blank day), set the corresponding member to its appropriate blank value (RsslTime::hour, RsslTime::minute, RsslTime::second use <b>255</b> to indicate blank, RsslTime::millisecond uses <b>65535</b>).
 *  
 *  @section TimeEncExample RsslTime Encoding Example
 *	The following example demonstrates encoding an RsslTime.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslTime with an hour and minute.
 *	RsslTime rTime = {10, 21, 0, 0};
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeTime() function to encode content
 *	if ((retVal = rsslEncodeTime(&encIter, &rTime)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeTime.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslTimeDec RsslTime Decoding
 *  @brief Detailed information about RsslTime decoding
 *  @{
 *
 *  When decoding an RsslTime, although fewer bytes may be on the wire, the full value is reconstructed into the RsslTime type.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned and the RsslTime::hour, RsslTime::minute, and RsslTime::second are set to <b>255</b> and RsslTime::millisecond is set to <b>65535</b>
 * 
 *  @section TimeDecExample RsslTime Decoding Example
 *  @code
 *	RsslTime rTime; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	retVal = rsslDecodeTime(&decIter, &rTime);
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeTime.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */

/**
 *
 *	@defgroup RsslTimeUtils RsslTime Utility & Conversion Functions
 *  @brief Detailed information about RsslTime utility and conversion helper functions
 *  @{
 *
 *  There are several helper and conversion functions provided for use with the RsslTime.  
 *  These can help with comparing and converting between RsslTime and various time strings.
 *
 * @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslTime */


/* Start of RsslDateTime */

/**
 * @defgroup RsslDateTimeGroup RsslDateTime Reference Group
 *
 * @brief The RsslDateTime Primitive Type is an optimized wire representation of time that can contain month, day, year, hour, minute, second, and millisecond.  
 *
 * The RsslDateTime functionality is defined in @header "rtr/rsslDateTime.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section DateTimeType RsslDateTime Primitive Type Overview
 * The RsslDateTime Primitive Type is an optimized wire representation of a date and time containing month, day, year, hour, minute, second, and millisecond.  
 * This type allows for representing the entire date and time or individual portions as blank, for example having a valid month, year, hour and minute but blank day, second and millisecond.
 * See \ref RsslDateTimeType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section DateTimeEnc RsslDateTime Encoding Functions
 * The following function is used while encoding an RsslDateTime.<BR>  
 * See \ref RsslDateTimeEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeDateTime() - Encodes an RsslDateTime primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section DateTimeDec RsslDateTime Decoding Functions
 * The following function is used while decoding an RsslDateTime.<BR>
 * See \ref RsslDateTimeDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeDateTime() - Decodes an RsslDateTime primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section DateTimeUtils RsslDateTime Utility & Conversion Functions
 * The Transport API C Edition provides several common conversion helpers for the RsslDateTime.  These can help with comparing and converting between RsslDateTime and various date and time strings.<BR>
 * See \ref RsslDateTimeUtils for more detailed information.<BR><BR>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslDateTimeType RsslDateTime Primitive Type
 *  @brief Detailed information about RsslDateTime type
 *  @{
 *
 * The RsslDateTime Primitive Type is an optimized wire representation of a date and time containing day, month, year, hour, minute, second, and millisecond.  
 * This type allows for representing the entire date and time or individual portions as blank, for example having a month, year, hour and minute but blank day, second and millisecond.
 * @note <ul><li>When using the RsslDateTime initializer or clear functions, all values are set to <b>0</b>. This differs from a blank date and time where month, day, and year are set to <b>0</b>,  hour, minute, and second are set to <b>255</b> and millisecond is set to <b>65535</b>.
 * The user should ensure that the RsslDateTime is populated appropriatley in correspondance to the actual date and time value being encoded, if not sending a blank date and time.</li>
 * <li>The RsslDateTime type can be represented as blank when sent as standard and set-defined data when contained in an RsslFieldList or RsslElementList.</li></ul>
 *
 *  @section DateTimeUseExample RsslDateTime Creation Examples
 *  The following code example demonstrates stack creation, initialization, and population of an RsslDateTime.
 *	@code
 *	//Create RsslDateTime on the stack
 *	RsslDateTime rDateTime = RSSL_INIT_DATETIME;
 *	//Populate RsslDateTime with month, day, year, hour and minute
 *	rDateTime.date.month = 10;
 *	rDateTime.date.day = 21;
 *	rDateTime.date.year = 2012;
 *	rDateTime.time.hour = 9;
 *	rDateTime.time.minute = 20;
 *	
 *	//------------------------------------------
 *
 *	//We can clear the RsslDateTime for initial use (or reuse) using the clear function instead of static initializer
 *	RsslDateTime rDateTime;
 *	rsslClearDateTime(&rDateTime);
 *	//Populate RsslDateTime with day, month, year, hour, minute, and second.
 *	rDateTime.date.month = 3;
 *	rDateTime.date.day = 25;
 *	rDateTime.date.year = 2012;
 *	rDateTime.time.hour = 11;
 *	rDateTime.time.minute = 45;
 *	rDateTime.time.second = 3;
 *
 *	//--------------------------------------------
 * 
 *	//Alternatly, we can create and populate the RsslDateTime in a single line
 *	//Populate RsslDateTime with day, month, year, hour, minute, second, and millisecond
 *	RsslDateTime rDateTime = { { 27, 7, 2013}, {10, 15, 52, 0} };
 *
 *	//--------------------------------------------
 *
 *	//To send an RsslDateTime as blank, we can use the static blank initializer
 *	RsslDateTime rDateTime = RSSL_BLANK_DATETIME;
 *
 *	//--------------------------------------------
 *
 *	//or use the provided blank helper function
 *	RsslDateTime rDateTime;
 *	rsslBlankDateTime(&rDateTime);
 *	
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslDateTimeEnc RsslDateTime Encoding
 *  @brief Detailed information about RsslDateTime encoding
 *  @{
 * 
 *  When encoding an RsslDateTime, only the number of bytes necessary to represent the contained value will be encoded.  Any trailing <b>0</b> values (e.g. \ref RsslDateTime::time::millisecond is set to <b>0</b>) will be inferred.
 *	@note If sending RsslDateTime as blank, use one of the blank initializers.  If sending only a portion of the date and time as blank (e.g. blank day), set the corresponding member to its appropriate blank value (\ref RsslDateTime::date::day, \ref RsslDateTime::date::month, \ref RsslDateTime::date::year use <b>0</b>, \ref RsslDateTime::time::hour, \ref RsslDateTime::time::minute, \ref RsslDateTime::time::second use <b>255</b> to indicate blank, \ref RsslDateTime::time::millisecond uses <b>65535</b>).
 *  
 *  @section DateTimeEncExample RsslDateTime Encoding Example
 *	The following example demonstrates encoding an RsslDateTime.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslDateTime with a month, day, year, hour and minute.
 *	RsslDateTime rDateTime = { {21, 10, 2013}, {11, 24, 0, 0} };
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeDateTime() function to encode content
 *	if ((retVal = rsslEncodeTime(&encIter, &rDateTime)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeDateTime.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslDateTimeDec RsslDateTime Decoding
 *  @brief Detailed information about RsslDateTime decoding
 *  @{
 *
 *  When decoding an RsslDateTime, although fewer bytes may be on the wire, the full value is reconstructed into the RsslDateTime type.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned and the \ref RsslDateTime::time::hour, \ref RsslDateTime::time::minute, \ref RsslDateTime::time::second are set to <b>255</b>, \ref RsslDateTime::time::millisecond is set to <b>65535</b>, \ref RsslDateTime::date::day, \ref RsslDateTime::date::month, \ref RsslDateTime::date::year are set to <b>0</b>
 * 
 *  @section DateTimeDecExample RsslDateTime Decoding Example
 *  @code
 *	RsslDateTime rDateTime; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	retVal = rsslDecodeDateTime(&decIter, &rDateTime);
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeDateTime.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */

/**
 *
 *	@defgroup RsslDateTimeUtils RsslDateTime Utility & Conversion Functions
 *  @brief Detailed information about RsslDateTime utility and conversion helper functions
 *  @{
 *
 *  There are several helper and conversion functions provided for use with the RsslDateTime.  
 *  These can help with comparing and converting between RsslDateTime and various date and time strings.
 *
 * @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslDateTime */


/* Start of RsslEnum */

/**
 * @defgroup RsslEnumGroup RsslEnum Reference Group
 *
 * @brief The enum primitive type allows the user to represent an unsigned 2-byte value, typically corresponding to an enumerated value that can be cross-referenced with an enum dictionary.
 *
 * The \ref RsslEnum functionality is defined in @header "rtr/rsslTypes.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section EnumType RsslEnum Primitive Type Overview
 * The \ref RsslEnum primitive type allows the user to represent an unsigned 2-byte value, typically corresponding to an enumerated value that can be cross-referenced with an enum dictionary.
 * This is defined as a type definition to the system's native unsigned short type.<BR>
 * See \ref RsslEnumType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section EnumEnc RsslEnum Encoding Functions
 * The following function is used while encoding an \ref RsslEnum.<BR>  
 * See \ref RsslEnumEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeEnum() - Encodes an \ref RsslEnum primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section EnumDec RsslEnum Decoding Functions
 * The following function is used while decoding an \ref RsslEnum.<BR>
 * See \ref RsslEnumDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeEnum() - Decodes an \ref RsslEnum primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslEnumType RsslEnum Primitive Type
 *  @brief Detailed information about \ref RsslEnum type
 *  @{
 *
 * The \ref RsslEnum primitive type allows the user to represent an unsigned 2-byte value, typically corresponding to an enumerated value that can be cross-referenced with an enum dictionary.
 * This is defined as a type definition to the system's native unsigned short type.<BR>
 * @note When represented as standard data (e.g. not set defined) in an RsslFieldList or RsslElementList, this type can be represented as blank.  When 
 * encoding as set defined this cannot be blank.  See documentation of those types for detailed information about how to encode or decode blank entries.
 *
 *  @section EnumUseExample RsslEnum Creation Examples
 *  The following code example demonstrates stack creation and population of an \ref RsslEnum.  Because this is a simple primitive type, there is no initializer or clear function required.
 *	@code
 *	//Create RsslEnum on the stack
 *	RsslEnum rEnum = 127;
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslEnumEnc RsslEnum Encoding
 *  @brief Detailed information about \ref RsslEnum encoding
 *  @{
 * 
 *	The \ref RsslEnum is a 2-byte unsigned short.
 *  
 *  @section EnumEncExample RsslEnum Encoding Example
 *	The following example demonstrates encoding an \ref RsslEnum.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslEnum
 *	RsslEnum rEnum = 3500;
 *	RsslRet retVal;
 *
 * 	//Invoke rsslEncodeEnum() function to encode content
 *	if ((retVal = rsslEncodeEnum(&encIter, &rEnum)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeEnum.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslEnumDec RsslEnum Decoding
 *  @brief Detailed information about \ref RsslEnum decoding
 *  @{
 *
 *  When decoding an \ref RsslEnum, the enum value on the wire is reconstructured into an \ref RsslEnum.
 *  @note When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned.
 * 
 *  @section EnumDecExample RsslEnum Decoding Example
 *  @code
 *	RsslEnum rEnum; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeEnum(&decIter, &rEnum)) < RSSL_RET_SUCCESS)
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeEnum.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslEnum */




/* Start of RsslArray */

/**
 *  @defgroup RsslArrayGroup RsslArray Reference Group
 *
 *	@brief The Array primitive type allows the user to represent a uniform list of simple primitive types.
 *  
 *  To use RsslArray functionality include @header "rtr/rsslArray.h"
 * 
 *  @section ArrStruct RsslArray Structure Overview
 *  The RsslArray primitive type allows the user to represent a uniform list of simple primitive types (e.g. all primitives with the exception of RsslArray).  
 *  All primitive types contained in an RsslArray are the same, with the type information specified in the RsslArray::primitiveType.  The RsslArray can house 
 *  variable length content, indicated with an RsslArray::itemLength of <b>0</b>, or fixed length content, indicated with an RsslArray::itemLength 
 *  that is <b>non-zero</b>.<BR>
 *  See \ref ArrayStruct for more detailed information. <BR>
 *  <BR><BR>
 *	
 *  @section ArrEnc RsslArray Encoding Functions 
 *  The following functions are used while encoding an RsslArray and its entries.  Entries can be encoded from a native primitive representation (e.g. \ref RsslReal, \ref RsslInt) or from pre-encoded primitive content.<BR>
 *  See \ref ArrayEncoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslEncodeArrayInit() - Begins array encoding, initial function to call when encoding an array type.</li><BR>
 *  <li>rsslEncodeArrayComplete() - Completes encoding of array, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired. </li><BR>
 *  <li>rsslEncodeArrayEntry() - Encodes entry into an array.  Can be primitive representation or pre-encoded.</li><BR>
 *  </ul>
 *  <BR><BR>
 *  @section ArrDec RsslArray Decoding Functions 
 *  The following functions are used while decoding an RsslArray and its entries.  Once at the individual entry level, a primitive type representation can be obtained by 
 *  calling the specific primitive decode function.<BR>
 *  See \ref ArrayDecoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslDecodeArray() - Decodes an array container and provides access to buffer containing all encoded array entries.</li><BR>
 *  <li>rsslDecodeArrayEntry() - Decodes array entries and provides access to individual array entries.  Primitive decode functions can be used to decode individual array entry into primitive representation. </li><BR>
 *  </ul>
 *  
 *  @{
 */

/**
 *	@defgroup ArrayStruct RsslArray Primitive Type
 *  @brief Detailed information about RsslArray structure and helper functions
 *  @{
 *
 *  The RsslArray primitive type allows the user to represent a uniform list of simple primitive types (e.g. all primitives with the exception of RsslArray).  
 *  All primitive types contained in an RsslArray are the same, with the type information specified in the RsslArray::primitiveType.  The RsslArray can house 
 *  variable length content, indicated with an RsslArray::itemLength of <b>0</b>, or fixed length content, indicated with an RsslArray::itemLength 
 *  that is <b>non-zero</b>.
 *  @note When represented as standard data (e.g. not set defined) in an RsslFieldList or RsslElementList, this type can be represented as blank.  This type has no 
 *  set defined data counterpart.  See documentation of those types for detailed information about how to encode or decode blank entries.
 *
 *  @section ArrUseExample RsslArray Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslArray along with use of the clear function to reuse the array.
 *  This is typical usage prior to encoding an RsslArray.  When decoding, the RsslArray will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	//Create RsslArray on the stack and use static initializer
 *	RsslArray rsslArray = RSSL_INIT_ARRAY;
 *	//Set primitive type and leave length as 0 to encode as variable length
 *	rsslArray.primitiveType = RSSL_DT_QOS;
 *	//Now invoke other array encoding functions
 *	
 *	//------------------------------------------
 *
 *	//We can clear the array for initial use (or reuse) using the clear function instead of static initializer
 *	RsslArray rsslArray;
 *	rsslClearArray(&rsslArray);
 *	//Set primitive type and set length to positive integer to encode as that fixed length
 *	rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
 *	rsslArray.itemLength = 3; //Will result in fixed length, 3 byte ASCII strings
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup ArrayEncoding RsslArray Encoding
 *  @brief Detailed information about RsslArray encoding
 *  @{
 * 
 *  When encoding an RsslArray, the encoding process begins with a call to rsslEncodeArrayInit().  Entries can be encoded from a native primitive representation or from pre-encoded primitive content by calling rsslEncodeArrayEntry().  
 *  If encoding from a native primitive representation (e.g. \ref RsslReal, \ref RsslInt), pass NULL as the pEncData \ref RsslBuffer and provide pointer to primitive type as void* parameter.
 *  If encoding an entry from pre-encoded content, populate the pEncData \ref RsslBuffer and pass NULL as the void*. When all entries are encoded, calling rsslEncodeArrayComplete() will finish RsslArray encoding.  
 *  @section ArrEncExample RsslArray Encoding Example
 *	The following example demonstrates encoding a fixed-length RsslArray that contains unsigned integers.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create the success parameter for completing the array.  
 *	//If any error occurs, this becomes false so rsslEncodeArrayComplete will roll back to last successful encoding
 *	RsslBool success = RSSL_TRUE;
 *	RsslRet retVal;
 *	//Create and initialize array structure
 *	RsslArray rsslArray = RSSL_INIT_ARRAY;
 *	//Set desired primitive type that array will contain
 *	rsslArray.primitiveType = RSSL_DT_UINT;
 *	//Set up array to send integers as fixed length of 2 bytes each.  
 *	//If set to 0, array contents are variable length and each integer would be sent as number of bytes required to represent it's value 
 *	rsslArray.itemLength = 2;
 *
 * 	//Initialize array encoding, handle any error
 *	if ((retVal = rsslEncodeArrayInit(&encIter, &rsslArray)) < RSSL_RET_SUCCESS)
 *	{
 *		success = RSSL_FALSE;
 *		printf("Error %s (%d) encountered with rsslEncodeArrayInit.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *	else
 *	{
 *		//Create and populate integer, pass it as void* parameter since it is not pre-encoded
 *		RsslUInt uInt = 23456;
 *		retVal = rsslEncodeArrayEntry(&encIter, NULL, &uInt); 
 * 		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeArrayEntry.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *		}
 *	
 *		//Encode second entry.
 *		uInt = 61585;
 *		retVal = rsslEncodeArrayEntry(&encIter, NULL, &uInt); 
 * 		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeArrayEntry.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *		}
 *	
 *		//Encode third entry.  Because the RsslArray is using a fixed length encoding,
 *		//this will be sent as 2 bytes even though the value can be represented in less than 2 bytes.
 *		uInt = 123;
 *		retVal = rsslEncodeArrayEntry(&encIter, NULL, &uInt); 
 * 		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeArrayEntry.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *		}
 *
 *	}
 *	retVal = rsslEncodeArrayComplete(&encIter, success);
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup ArrayDecoding RsslArray Decoding
 *  @brief Detailed information about RsslArray decoding
 *  @{
 *
 *  When decoding an RsslArray, each entry is returned as an \ref RsslBuffer containing
 *  the encoded content.  Further decoding of an entry's content can be achieved by using the appropriate 
 *  primitive type decoder (e.g. rsslDecodeReal()).  If desired, decoding of an entry's content can be skipped by 
 *  invoking the rsslDecodeArrayEntry() function to move to the next entry. 
 *  @section ArrDecExample RsslArray Decoding Example
 *  @code
 *	RsslArray rsslArray; //No need to clear since we will decode into this
 *	RsslBuffer entryBuffer;	//Buffer to expose individual array entry contents through
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeArray(&decIter, &rsslArray)) >= RSSL_RET_SUCCESS)
 *	{
 *		//decode each array entry
 *		while ((retVal = rsslDecodeArrayEntry(&decIter, &entryBuffer)) != RSSL_RET_END_OF_CONTAINER)
 *		{
 *			//handle failure cases
 *			if (retVal < RSSL_RET_SUCCESS)
 *			{
 *				//handle error as needed
 *				printf("Error %s (%d) encountered while decoding array entries.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			}
 *			else
 *			{
 *				//now decode primitive types
 *				switch(rsslArray.primitiveType)
 *				{
 *					//handle appropriate primitive types and do something with data
  *					case RSSL_DT_UINT:
 *						RsslUInt uInt;
 *						retVal = rsslDecodeInt(&decIter, &uInt);
 *					break;
 *					...
 *				}
 *			}
 *		}
 *	}
 *	else
 *	{
 *		//initial array decode failed
 *		printf("Error %s (%d) encountered while decoding array.  Error text: %s\n",
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslArray */

 
/* Start of RsslQos */

/**
 * @defgroup RsslQosGroup RsslQos Reference Group
 *
 * @brief The quality-of-service primitive type can be used to indicate the quality of associated content using timliness and rate characteristics.
 *
 * The \ref RsslQos functionality is defined in @header "rtr/rsslQos.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section QosType RsslQos Primitive Type Overview
 * The RsslQos primitive type can be used to indicate the quality of service associated with content.  This is broken into two dimensions: 
 * <ul><li>Timeliness (defined by \ref RsslQosTimeliness) indicates information about the age of content, for example whether it is real time or delayed.</li>
 * <li>Rate (defined by \ref RsslQosRates) indicates the content's period of change, for example whether it is tick-by-tick or conflated.</li></ul>
 * See \ref RsslQosType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section QosEnc RsslQos Encoding Functions
 * The following function is used while encoding an RsslQos.<BR>  
 * See \ref RsslQosEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeQos() - Encodes an RsslQos primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section QosDec RsslQos Decoding Functions
 * The following function is used while decoding an RsslQos.<BR>
 * See \ref RsslQosDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeQos() - Decodes an RsslQos primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section QosUtils RsslQos Utility & Conversion Functions
 * The Transport API C Edition provides several common conversion helpers for the RsslQos.<BR>
 * See \ref RsslQosUtils for more detailed information.<BR><BR>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslQosType RsslQos Primitive Type
 *  @brief Detailed information about RsslQos type
 *  @{
 *
 * The RsslQos primitive type can be used to indicate the quality of service associated with content.  This is broken into two dimensions: 
 * <ul><li>Timeliness (defined by \ref RsslQosTimeliness) indicates information about the age of content, for example whether it is real time or delayed.</li>
 * <li>Rate (defined by \ref RsslQosRates) indicates the content's period of change, for example whether it is tick-by-tick or conflated.</li></ul>
 * @note When represented as standard data (e.g. not set defined) in an RsslFieldList or RsslElementList, this type can be represented as blank.  When 
 * encoding as set defined this cannot be blank.  See documentation of those types for detailed information about how to encode or decode blank entries.
 *
 *  @section QosUseExample RsslQos Creation Examples
 *  The following code example demonstrates stack creation and population of an RsslQos. 
 *  @note <ul><li>When using the RsslQos initializer or clear functions, all values are set to <b>0</b>. This results in having an unspecified QoS, which is not supported.
 *  The user should ensure that the RsslQos is populated appropriatley in correspondance to the actual quality of service information being encoded.</li>
 *  <li>The RsslQos type can be represented as blank when sent only as standard data when contained in an RsslFieldList or RsslElementList.</li></ul>
 *
 *  @section QosUseExample RsslQos Creation Examples
 *  The following code example demonstrates stack creation, initialization, and population of an RsslQos.
 *	@code
 *	//Create RsslQos on the stack
 *	RsslQos rQos = RSSL_INIT_QOS;
 *	//Populate RsslQos with a real time/tick-by-tick qos.  No rate or time info.
 *	rQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
 *	rQos.timeliness = RSSL_QOS_TIME_REALTIME;
 *	
 *	//------------------------------------------
 *
 *	//We can clear the RsslQos for initial use (or reuse) using the clear function instead of static initializer
 *	RsslQos rQos;
 *	rsslClearQos(&rQos);
 *	//Populate RsslQos with delayed, tick by tick data.  RsslQos::timeInfo indicates delay information.
 *	rQos.rate = RSSL_QOS_RATE_TICK_BY_TICK; // delayed data is still full tick
 *	rQos.timeliness = RSSL_QOS_TIME_DELAYED;
 *	rQos.timeInfo = 15;
 *
 *	//--------------------------------------------
 * 
 *	//Alternatly, we can create and populate the RsslQos in a single line
 *	//Populate RsslQos with timeliness, rate, dynamic, timeInfo, rateInfo
 *	RsslQos rQos = { RSSL_QOS_TIME_REALTIME, RSSL_QOS_RATE_JIT_CONFLATED, RSSL_FALSE, 0, 0 };
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslQosEnc RsslQos Encoding
 *  @brief Detailed information about RsslQos encoding
 *  @{
 * 
 *  When encoding an RsslQos, only the number of bytes necessary to represent the contained value will be encoded.
 *  @note <ul><li>Encoding of an RsslQos containing ::RSSL_QOS_TIME_UNSPECIFIED or ::RSSL_QOS_RATE_UNSPECIFIED is not supported.  These values are intended as structure initializer values and not for providing or consuming.</li>
 *  <li>RsslQos::timeInfo is applicable only when specifying an RsslQos::timeliness value of ::RSSL_QOS_TIME_DELAYED.</li>
 *  <li>RsslQos::rateInfo is applicable only when specifying an RsslQos::rate value of ::RSSL_QOS_RATE_TIME_CONFLATED.</li></ul>
 *
 *  
 *  @section QosEncExample RsslQos Encoding Example
 *	The following example demonstrates encoding an RsslQos.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslQos
 *	RsslQos rQos = RSSL_INIT_QOS;
 *	RsslRet retVal;
 *
 *	rQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
 *	rQos.timeliness = RSSL_QOS_TIME_REALTIME;
 *
 * 	//Invoke rsslEncodeQos() function to encode content
 *	if ((retVal = rsslEncodeQos(&encIter, &rQos)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeQos.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslQosDec RsslQos Decoding
 *  @brief Detailed information about RsslQos decoding
 *  @{
 *
 *  When decoding an RsslQos, although fewer bytes may be on the wire, the full signed integer value is reconstructed into the \ref RsslQos type.
 *  @note <ul><li>When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned.</li>
 *	<li>If an unspecified RsslQos is received (e.g. sent by a version of a component that was released prior to unspecified QoS being restricted on the wire), this will be interpredted by the RsslQos decode functionality as a QoS of ::RSSL_QOS_TIME_REALTIME and ::RSSL_QOS_RATE_TICK_BY_TICK</li></ul>
 * 
 *  @section QosDecExample RsslQos Decoding Example
 *  @code
 *	RsslQos rQos; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeQos(&decIter, &rInt)) < RSSL_RET_SUCCESS)
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeQos.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */

/**
 *
 *	@defgroup RsslQosUtils RsslQos Utility & Conversion Functions
 *  @brief Detailed information about RsslQos utility and conversion helper functions
 *  @{
 *
 *  There are several helper and conversion functions provided for use with the RsslQos.  
 *
 * @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslQos */


/* Start of RsslState */

/**
 * @defgroup RsslStateGroup RsslState Reference Group
 *
 * @brief The state primitive type can be used to convey data and stream health information.
 *
 * The \ref RsslState functionality is defined in @header "rtr/rsslState.h", @header "rtr/rsslPrimitiveEncoders.h", and @header "rtr/rsslPrimitiveDecoders.h"
 *
 * @section StateType RsslState Primitive Type Overview
 * The RsslState primitive type can be used to convey data and stream health information.  
 * <ul><li>Data State (defined by \ref RsslDataStates) indicates whether the data is ok or suspect.</li>
 * <li>Stream State (defined by \ref RsslStreamStates) indicates whether the stream is open, closed, closed but recoverable, redirected, or non-streaming.</li></ul>
 * See \ref RsslStateType for more detailed information. <BR>
 * <BR><BR>
 *
 * @section StateEnc RsslState Encoding Functions
 * The following function is used while encoding an RsslState.<BR>  
 * See \ref RsslStateEnc for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslEncodeState() - Encodes an RsslState primitive type.  This will perform any necessary byte swapping when encoding into the buffer referred to by the \ref RsslEncodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section StateDec RsslState Decoding Functions
 * The following function is used while decoding an RsslState.<BR>
 * See \ref RsslStateDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslDecodeState() - Decodes an RsslState primitive type.  This will perform any necessary byte swapping when decoding from the buffer referred to by the \ref RsslDecodeIterator.</li><BR>
 * </ul>
 * <BR><BR>
 * @section StateUtils RsslState Utility & Conversion Functions
 * The Transport API C Edition provides several common conversion helpers for the RsslState.<BR>
 * See \ref RsslStateUtils for more detailed information.<BR><BR>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslStateType RsslState Primitive Type
 *  @brief Detailed information about RsslState type
 *  @{
 *
 * The RsslState primitive type can be used to convey data and stream health information.  
 * <ul><li>Data State (defined by \ref RsslDataStates) indicates whether the data is ok or suspect.</li>
 * <li>Stream State (defined by \ref RsslStreamStates) indicates whether the stream is open, closed, closed but recoverable, redirected, or non-streaming.</li></ul>
 * @note When represented as standard data (e.g. not set defined) in an RsslFieldList or RsslElementList, this type can be represented as blank.  When 
 * encoding as set defined this cannot be blank.  See documentation of those types for detailed information about how to encode or decode blank entries.
 *
 *  @section StateUseExample RsslState Creation Examples
 *  The following code example demonstrates stack creation and population of an RsslState. 
 *  @note <ul><li>When using the RsslState initializer or clear functions, all values are set to <b>0</b>. This results in having an unspecified stream state, which is not supported.
 *  The user should ensure that the RsslState is populated appropriatley in correspondance to the actual state information being encoded.</li>
 *  <li>The RsslState type can be represented as blank when sent only as standard data when contained in an RsslFieldList or RsslElementList.</li></ul>
 *
 *  @section StateUseExample RsslState Creation Examples
 *  The following code example demonstrates stack creation, initialization, and population of an RsslState.
 *	@code
 *	//Create RsslState on the stack
 *	RsslState rState = RSSL_INIT_STATE;
 *	//Populate RsslState with an open/ok state and no state code.
 *	rState.streamState = RSSL_STREAM_OPEN;
 *	rState.dataState = RSSL_DATA_OK;
 *	
 *	//------------------------------------------
 *
 *	//We can clear the RsslState for initial use (or reuse) using the clear function instead of static initializer
 *	RsslState rState;
 *	rsslClearState(&rState);
 *	//Populate RsslState with Closed Recover, Suspect, and state code
 *	rState.streamState = RSSL_STREAM_CLOSED_RECOVER;
 *	rState.dataState = RSSL_DATA_SUSPECT;
 *	rState.code = RSSL_SC_TOO_MANY_ITEMS;
 *	rState.text.data = "Too many open items.";
 *	rState.text.length = 20;
 *
 *	//--------------------------------------------
 * 
 *	//Alternatly, we can create and populate the RsslState in a single line
 *	//Populate RsslState with stream state, data state, state code, and text
 *	RsslState rState = { RSSL_STREAM_OPEN, RSSL_DATA_OK, RSSL_SC_NONE, {0, 0} };
 *
 *	@endcode
 *  @}
 *
 */

/**
 *  @defgroup RsslStateEnc RsslState Encoding
 *  @brief Detailed information about RsslState encoding
 *  @{
 * 
 *  When encoding an RsslState, only the number of bytes necessary to represent the contained value will be encoded.
 *  @note Encoding of an RsslState containing ::RSSL_STREAM_UNSPECIFIED is not supported.  This value is intended as a structure initializer value and not for providing or consuming.
 *  
 *  @section StateEncExample RsslState Encoding Example
 *	The following example demonstrates encoding an RsslState.
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create and populate the RsslState
 *	RsslState rState = RSSL_INIT_STATE;
 *	RsslRet retVal;
 *
 *	rState.stream = RSSL_STREAM_OPEN;
 *	rState.data = RSSL_DATA_OK;
 *
 * 	//Invoke rsslEncodeState() function to encode content
 *	if ((retVal = rsslEncodeState(&encIter, &rState)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered with rsslEncodeState.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup RsslStateDec RsslState Decoding
 *  @brief Detailed information about RsslState decoding
 *  @{
 *
 *  When decoding an RsslState, although fewer bytes may be on the wire, the full signed integer value is reconstructed into the \ref RsslState type.
 *  @note <ul><li>When decoding a blank value, ::RSSL_RET_BLANK_DATA is returned.</li>
 *	<li>If an unspecified RsslQos is received (e.g. sent by a version of a component that was released prior to unspecified QoS being restricted on the wire), this will be interpredted by the RsslQos decode functionality as a QoS of ::RSSL_QOS_TIME_REALTIME and ::RSSL_QOS_RATE_TICK_BY_TICK</li></ul>
 * 
 *  @section StateDecExample RsslState Decoding Example
 *  @code
 *	RsslState rState; //No need to initialize since we will decode into this
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeState(&decIter, &rInt)) < RSSL_RET_SUCCESS)
 *	{
 *		//handle error as needed
 *		switch(retVal)
 *		{
 *			case RSSL_RET_BLANK_DATA:
 *				//Decoded content was blank
 *			break;
 *			case RSSL_RET_SUCCESS:
 *				//Decoding was successful, do something with content
 *				...
 *			break;
 *			default:
 *				printf("Error %s (%d) encountered with rsslDecodeState.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}					
 *  @endcode
 *  @}
 *
 */

/**
 *
 *	@defgroup RsslStateUtils RsslState Utility & Conversion Functions
 *  @brief Detailed information about RsslState utility and conversion helper functions
 *  @{
 *
 *  There are several helper and conversion functions provided for use with the RsslState.  
 *
 * @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslState */

/* Start of RsslRmtesGroup */

/**
 * @defgroup RsslRmtesGroup RMTES Decoder Reference Group
 *
 * @brief The RMTES(Reuters Multilingual Text Encoding Standard) Decoder can be used to decode RMTES content into UTF8 or UCS2 Unicode strings for display or machine parsing purposes.
 *
 * The RMTES Decoding functionality is defined in @header "rtr/rsslRmtes.h".
 *
 * @section CacheBufStruct RsslRmtesCacheBuffer Structure Overview
 * The \ref RsslRmtesCacheBuffer primitive type allows the user to cache an encoded RMTES string for decoding into UTF8 or UCS2 Uncode.
 * See \ref RsslRmtesCacheBuffer for more detailed information. <BR>
 * The buffer consists of an allocated length, a used length, and a pointer.  The pointer is used to refer to the content.  The allocated length is used to represent the total allocated length of the buffer, and the used length is used to represent the total amount of used data in the cache buffer.<BR>
 * <BR><BR>
 *
 * @section CacheBufStore RsslRmtesCacheBuffer Storage Functions
 * The following function is used to fill an \ref RsslRmtesCacheBuffer's data.<BR>  
 * See \ref RsslRmtesDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslRMTESApplyToCache() - This will apply the encoded RMTES data to the RsslRmtesCacheBuffer. In addition, this will apply any partial update commands contained within the given RsslBuffer to the RsslRmtesCacheBuffer. </li><BR>
 * </ul>
 * <BR><BR>
 *
 * @section U16BufStruct RsslU16Buffer Structure Overview
 * The \ref RsslU16Buffer primitive type allows the user to store a UCS2 Unicode string.
 * See \ref RsslU16BufferStruct for more detailed information. <BR>
 * The buffer consists of a length, and a pointer.  The pointer is used to refer to the content.  The length is used to represent the total length of the data contained in the buffer.<BR>
 * <BR><BR>
 *
 * @section U16BufStructFunc RsslU16Buffer Usage Functions
 * The following function is used to fill an \ref RsslU16Buffer's data.<BR>  
 * See \ref RsslRmtesDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslRMTESToUCS2() - This will convert the RMTES data contained in an RsslRmtesCacheBuffer to UCS2 Unicode.</li><BR>
 * </ul>
 * <BR><BR>
 *
 * @section RMTESHelper Rssl RMTES Helper Functions
 * The following function is used to determine if an RMTES-encoded string contains a partial update command.<BR>  
 * See \ref RsslRmtesDec for more detailed information.<BR><BR>
 * <ul>
 * <li>rsslHasPartialRMTESUpdate() - Checks the given RsslBuffer for the presence of a partial update command. </li><BR>
 * </ul>
 * <BR><BR>
 *
 * @{
 */

/**
 *	@defgroup RsslRmtesCacheStruct RsslRmtesCacheBuffer Primitive Type
 *  @brief Detailed information about RsslRmtesCacheBuffer structure and helper functions
 *  @{
 *
 * The \ref RsslRmtesCacheBuffer is used to store RMTES-encoded data for translation to UCS-2 or UTF-8 unicode strings.  This is used by the RMTES Decoder interface to apply any Marketfeed semantics to the stored cache.
 * The buffer consists of an used length, an allocated length, and a pointer.  The pointer is used to refer to the content.  The allocated length is used to represent the lengh of the cache's character buffer, and the used length is used to represent the length of the used data in the cache buffer.
 *
 *  @section RsslRmtesCacheBufferExample RsslRmtesCacheBuffer Creation Example
 *  The following code example demonstrates stack creation and initialization of an \ref RsslRmtesCacheBuffer along with use of the clear function to reuse the array.
 *  This is typical usage prior to encoding an \ref RsslRmtesCacheBuffer.  After rsslRMTESApplyToCache() is called, the data will be copied into the RsslRmtesCacheBuffer's data pointer.
 *	@code
 *	//We can clear the buffer for initial use (or reuse) using the clear function
 *	char cacheMem[100];
 *	RsslRmtesCacheBuffer buffer;
 *	RsslRmtesCacheBuffer(&buffer);
 *	//Populate buffer
 *	buffer.data = cacheMem;
 *	// The total used length is 0 after initialization.
 *	buffer.length = 0;		
 *	buffer.allocatedLength = 100;
 *	@endcode
 *  @}
 */
 
/**
 *	@defgroup RsslU16BufferStruct RsslU16Buffer Primitive Type
 *  @brief Detailed information about RsslBuffer structure and helper functions
 *  @{
 *
 * The \ref RsslU16Buffer primitive type allows the user to represent length specified unsigned 16-bit wide character strings. This is used by the RMTES Decoder interface to provider the user with a UCS2 unicode string, and is not sent on the wire.
 * The buffer consists of a length and a pointer, where the pointer is used to refer to the content and the length indicates the number of unsigned 16-bit integers referred to.
 * @note <ul><li>The RMTES decoder interface does not apply a NULL terminator after decoding to a UCS2 string.  The length should be used to determine where the buffer contents end.</li>
 *
 *  @section U16BufferUseExample RsslBuffer Creation Examples
 *  The following code example demonstrates stack creation and initialization of an \ref RsslU16Buffer along with use of the clear function to reuse the array.
 *  This is typical usage prior to decoding RMTES data into an \ref RsslU16Buffer.
 *	@code
 *
 *	//We can clear the buffer for initial use (or reuse) using the clear function
 *	RsslUInt16 cacheMem[100];
 *	RsslU16Buffer buffer;
 *	rsslClearU16Buffer(&buffer);
 *	//Populate buffer
 *	buffer.data = cacheMem;
 *	buffer.length = 100;
 *
 *	@endcode
 *
 *  @}
 */

/**
 *
 *	@defgroup RsslRmtesDec Rssl RMTES Decoding Functions and Helpers
 *  @brief Detailed information about the Rssl RMTES decoding interface
 *  @{
 *
 *	The decoding process for RMTES data starts after receiving an RMTES-encoded string.  The data is copied into an RsslRmtesCacheBuffer by rsslRMTESApplyToCache(), which applies any marketfeed semantics to the buffer.  Then, the user calls either rsslRMTESToUCS2() or rsslRMTESToUTF8() to convert the string to the appropriate unicode format.
 * 
 *  @section RmtesUsageExample Rssl RMTES Decoding Example
 *	The Rssl RMTES Decoder is capable of decoding an encoded RMTES string into UCS2 or UTF8 encoded data.  The first code snippet illustrates decoding RMTES data into UCS2 Unicode.<br>
 * 
 *  @code
 *	RsslBuffer inputBuf; //Input buffer, already has been populated.
 *	char cache[100];
 *	RsslRmtesCacheBuffer rmtesCache; // Cache buffer, used to apply any partial updates
 *	RsslUInt16 ucsString[100];
 *	RsslU16Buffer U16StringBuf;	//Unsigned 16-bit integer buffer, used to get the UCS2 encoded string
 *	RsslRet retVal;
 *
 *	// Setup the cache and U16 string buffers
 *	rmtesCache.data = cache;
 *	rmtesCache.length = 0;	//This is the used length of the buffer.  Since the cache is empty, this is 0
 *	rmtesCache.allocatedLength = 100;
 *
 *	U16StringBuf.data = ucsString;
 *	U16StringBuf.length = 100;
 *
 *	if ((retVal = rsslRMTESApplyToCache(&inputBuf, &rmtesCache)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered while applying buffer to RMTES cache.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *
 *	if ((retVal = rsslRMTESToUCS2(&rmtesCache, &U16StringBuf)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered while converting buffer from RMTES encoded data to UCS2.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *  @endcode
 *
 *	This second code snippet illustrates decoding RMTES data into UTF8:	
 *
 *  @code
 *	RsslBuffer inputBuf; //Input buffer, already has been populated.
 *	char cache[100];
 *	RsslRmtesCacheBuffer rmtesCache; // Cache buffer, used to apply any partial updates
 *	unsigned char utfString[100];
 *	RsslBuffer charStringBuf;	//character buffer, used to get the UTF8 encoded string
 *	RsslRet retVal;
 *
 *	// Setup the cache and U16 string buffers
 *	rmtesCache.data = cache;
 *	rmtesCache.length = 0;	//This is the used length of the buffer.  Since the cache is empty, this is 0
 *	rmtesCache.allocatedLength = 100;
 *
 *	charStringBuf.data = utfString;
 *	charStringBuf.length = 100;
 *
 *	if ((retVal = rsslRMTESApplyToCache(&inputBuf, &rmtesCache)) < RSSL_RET_SUCCESS)
 *		{
 *		printf("Error %s (%d) encountered while applying buffer to RMTES cache.  Error text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *
 *	if ((retVal = rsslRMTESToUTF8(&rmtesCache, &charStringBuf)) < RSSL_RET_SUCCESS)
 *	{
 *		printf("Error %s (%d) encountered while converting buffer from RMTES encoded data to UTF8.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslRmtes Group */
 
/**
 *	@}
 */
 
/**
 *	@defgroup RSSLWFDataContainers Transport API Data Container Structures
 *	@brief The Transport API Data Package's container structures are required to encode or decode OMM binary data.  These structures are used to encode primitive and container Transport API data structures.
 *	@{
 */

 
/* Start of RsslFieldList */

/**
 *  @defgroup RsslFieldListGroup RsslFieldList Reference Group
 *
 *	@brief The Field List container type allows the user to represent a collection of field identifier - value pairs, where the value can be a container type  or a primitive type.  The field identifier can be cross referenced with a field dictionary to determine the specific type of the content.  
 *  @{  
 *
 *  To use RsslFieldList functionality include @header "rtr/rsslFieldList.h"
 * 
 *  @section FieldListStruct RsslFieldList and RsslFieldEntry Structure Overview
 *  The RsslFieldList container type allows the user to represent a collection of field identifier - value pairs, where the value can be a container type  or a primitive type.  The field identifier can be cross referenced with a field dictionary to determine the specific type of the content.  
 *  See \ref RsslFieldListStruct for more detailed information on the RsslFieldList and RsslFieldEntry. <BR>
 *  <BR><BR>
 *	
 *  @section FieldListEnc RsslFieldList Encoding Functions 
 *  The following functions are used while encoding an RsslFieldList and its entries.  Entries content can be encoded from a native primitive representation (e.g. \ref RsslReal, \ref RsslInt), from pre-encoded primitive or containers, from non pre-encoded primitive or containers, or as blank.<BR>
 *  See \ref FieldListEncoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslEncodeFieldListInit() - Begins RsslFieldList encoding, initial function to call when encoding an RsslFieldList type.</li><BR>
 *  <li>rsslEncodeFieldListComplete() - Completes encoding of an RsslFieldList, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired<BR>
 *  <li>rsslEncodeFieldEntry() - Encodes RsslFieldEntry into an RsslFieldList, where the RsslFieldEntry has native primitive payload passed in via void*, pre-encoded content set in RsslFieldEntry::encData, or blank (where void* is NULL and RsslFieldEntry::encData is empty). </li> <BR>
 *  <li>rsslEncodeFieldEntryInit() - Begins RsslFieldEntry encoding, initial function to call when encoding an RsslFieldEntry without pre-encoded payload. </li>  <BR>
 *  <li>rsslEncodeFieldEntryComplete() - Completes encoding of an RsslFieldEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired. </li><BR>
 *  <BR><BR>
 *  </ul>
 *  @section FieldListDec RsslFieldList Decoding Functions 
 *  The following functions are used while decoding an RsslFieldList and its entries.  Once at the individual entry level, the entries content can be left as pre-encoded content or decoded to it's native primitive type or container type representation by cross referencing RsslFieldEntry::fieldId with field dictionary and invoking correct type decoders. <BR>
 *  See \ref FieldListDecoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslDecodeFieldList() - Decodes an RsslFieldList container and provides access to buffer containing all encoded field entries.</li><BR>
 *  <li>rsslDecodeFieldEntry() - Decodes RsslFieldList entries and provides access to each individual RsslFieldEntry and their content.  If further content decoding is desired, RsslFieldEntry::fieldId can be cross referenced with a field dictionary to determine the type and invoke the correct decode functions.  </li> <BR>
 *  </ul>
 *  
 */


/**
 *	@defgroup RsslFieldListStruct RsslFieldList Container Type
 *  @brief Detailed information about RsslFieldList and RsslFieldEntry structures, initializers, flag values, and helper functions
 *  @{
 *
 *  The RsslFieldList container type allows the user to represent a collection of field identifier - value pairs, where the value can be a container type  or a primitive type.  The field identifier can be cross referenced with a field dictionary to determine the specific type of the content.  
 *  @note An RsslFieldList can contain standard data and/or set defined data.  When set defined data is contained, a set definition is required to decode this content - if not present, all set data decoding will be skipped. 
 *
 * 
 *  @section FieldListUseExample RsslFieldList Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslFieldList and RsslFieldEntry.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslFieldList.  When decoding, the RsslFieldList and any entries will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	//Create RsslFieldList and RsslFieldEntry on the stack and use static initializer
 *	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;
 *	RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;
 *  
 *	//Populate RsslFieldList with relevant information
 *	//Set flags to indicate field list has standard data and dictionary Id
 *	fieldList.flags = RSSL_FLF_HAS_FIELD_LIST_INFO | RSSL_FLF_HAS_STANDARD_DATA;
 *	fieldList.dictionaryId = 1;
 *	fieldList.fieldListNum = 10;
 *	//Now invoke RsslFieldList encoding functions (e.g. rsslEncodeFieldListInit). 
 *  
 *	//Populate RsslFieldEntry with relevant information (set type to encode and field ID)
 *	fieldEntry.dataType = RSSL_DT_UINT;
 *	fieldEntry.fieldId = 1;
 *	//Now invoke RsslFieldEntry encode functions, passing in the primitive type payload as unencoded data (e.g. rsslEncodeFieldEntry)
 *  
 *	//------------------------------------------
 *  
 *	//We can also use the clear function for initial use (or reuse) using the clear function instead of static initializer
 *	RsslFieldList fieldList;
 *	RsslFieldEntry fieldEntry;
 *  
 *	//Clear RsslFieldList and populate with relevant information
 *	rsslClearFieldList(&fieldList);
 *	//Set flags to indicate field list has standard data, set data, set Id (for decoding and encoding set data), and a dictionary Id 
 *	fieldList.flags = RSSL_FLF_HAS_FIELD_LIST_INFO | RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID;
 *	//Populate relevant members on RsslFieldList
 *	fieldList.dictionaryId = 1;
 *	fieldList.fieldListNum = 10;
 *	fieldList.setId = 3;
 *	//Now invoke RsslFieldList encoding functions, ensuring to pass in set definition (e.g. rsslEncodeFieldListInit).
 * 
 *	//Clear RsslFieldEntry and populate with relevant information
 *	fieldEntry.fieldId = 6;
 *	fieldEntry.dataType = RSSL_DT_REAL;
 *	//Now invoke RsslFieldEntry encode functions (e.g. rsslEncodeFieldEntry, rsslEncodeFieldEntryInit)
 *
 *	@endcode
 *
 *  @section TheFieldListStruct RsslFieldList Structure, Initializers and Helpers
 *  @defgroup FStruct RsslFieldList Structure, Initializers, Flag values, and Helpers
 *  @brief Detailed information about the RsslFieldList Structure, Initializers, and Helper methods
 *  @{
 *
 *  @}
 *
 *
 *  @section TheFieldEntryStruct RsslFieldEntry Structure, Initializers, and Helpers
 *  @defgroup FEStruct RsslFieldEntry Structure, Initalizers and Helpers
 *  @brief Detailed information about the RsslFieldEntry Structure, Initializers, and Helper methods
 *  @{
 * 
 *  @}
 */
 
/**
 *  @}
 *
 */

/**
 *  @defgroup FieldListEncoding RsslFieldList and RsslFieldEntry Encoding
 *  @brief Detailed information about RsslFieldList and RsslFieldEntry encoding
 *  @{
 * 
 *  When encoding an RsslFieldList, the encoding process begins with a call to rsslEncodeFieldListInit().  Entries can be encoded by using rsslEncodeFieldEntry(), where the RsslFieldEntry is populated with pre-encodede-encoded content (or blank), 
 *  or using rsslEncodeFieldEntryInit(), the appropriate encode functions based on the type described by the RsslFieldEntry::fieldId, and rsslEncodeFieldEntryComplete().  
 *  If encoding RsslFieldEntry content from a native primitive representation (e.g. \ref RsslReal, \ref RsslInt), pass an empty RsslFieldEntry::encData \ref RsslBuffer and provide pointer to primitive type as void* parameter on rsslEncodeFieldEntry().
 *  If encoding the RsslFieldEntry content from pre-encoded content, populate the RsslFieldEntry::encData \ref RsslBuffer and pass NULL as the void*. 
 *	If encoding the RsslFieldEntry as blank, pass NULL as the void* and pass an empty RsslFieldEntry::encData \ref RsslBuffer.
 *  When all entries are encoded, calling rsslEncodeFieldListComplete() will finish RsslFieldList encoding.  
 *  @section FieldListEncExample RsslFieldList Encoding Example
 *	The following example demonstrates encoding an RsslFieldList.  
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create the success parameter for completing the RsslFieldList and RsslFieldEntry encoding.  
 *	//If any error occurs, this becomes false so rsslEncodeFieldEntryComplete() or rsslEncodeFieldListComplete() will roll back to last successful encoding
 *	RsslBool success = RSSL_TRUE;
 *	RsslRet retVal;
 *	//Create and initialize RsslFieldList structure
 *	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;
 *	//Set desired flags on field list
 *	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
 *	fieldList.dictionaryId = 1;
 *	fieldList.fieldListNum = 25;
 *
 *	//Initialize RsslFieldList encoding, handle any error.  Last two parameters are 0 because there is no 
 *	//set definitions necessary and no set data is being encoded.
 *	if ((retVal = rsslEncodeFieldListInit(&encIter, &fieldList, 0, 0)) < RSSL_RET_SUCCESS)
 *	{
 *		success = RSSL_FALSE;
 *		printf("Error %s (%d) encountered with rsslEncodeFieldListInit().  Error Text: %s\n", 
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *	else
 *	{
 *		//Create the RsslFieldEntry structure and payload primitives, this will be reused so we will clear it between uses.
 *		RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;
 *		uIntKey = 23456;
 *		fieldEntry.fieldId = 1;
 *		//Begin entry encoding.  Pass in payload as void* for this entry.  
 *		retVal = rsslEncodeFieldEntry(&encIter, &fieldEntry, &uIntKey); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *		}
 *  
 *		//Now encode a container type as content using rsslEncodeFieldEntryInit() and rsslEncodeFieldEntryComplete().  
 *
 *		fieldEntry.fieldId = -20526;
 *		//Begin entry encoding.  Pass in length hint for expected size
 *		retVal = rsslEncodeFieldEntryInit(&encIter, &fieldEntry, 350);
 *		//Now call respective encode functions for type being housed in the RsslFieldEntry
 *		retVal = rsslEncodeFieldEntryComplete(&encIter, success);
 *	}
 *	//Done with entries, complete the field list encoding
 *	retVal = rsslEncodeFieldListComplete(&encIter, success);
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup FieldListDecoding RsslFieldList and RsslFieldEntry Decoding
 *  @brief Detailed information about RsslFieldList and RsslFieldEntry decoding
 *  @{
 *
 *  When decoding an RsslFieldList, each entry is returned as an \ref RsslFieldEntry containing
 *  the encoded payload content.  Further decoding of an entry's content can be achieved by using the appropriate 
 *  primitive type or container type decoder.  If the RsslFieldEntry::dataType is ::RSSL_DT_UNKNOWN, applications can cross reference
 *	the fid with a field dictionary to determine the type.  
 *  If desired, decoding of an entry's content can be skipped by 
 *  invoking the rsslDecodeFieldEntry() function to move to the next entry. 
 *  @section FieldListDecExample RsslFieldList Decoding Example
 *  @code
 *	RsslFieldList fieldList; //No need to clear since we will decode into this
 *	RsslFieldEntry fieldEntry;	//RsslFieldEntry to expose individual entry contents through
 *	RsslRet retVal;
 *
 *	//Third parameter is 0 because we do not have set definitions in this example
 *	if ((retVal = rsslDecodeFieldList(&decIter, &fieldList, 0)) >= RSSL_RET_SUCCESS)
 *	{
 *		while ((retVal = rsslDecodeFieldEntry(&decIter, &fieldEntry)) != RSSL_RET_END_OF_CONTAINER)
 *		{
 *			//handle failure cases
 *			if (retVal < RSSL_RET_SUCCESS)
 *			{
 *				//handle error as needed
 *				printf("Error %s (%d) encountered while decoding field entries.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			}
 *			else
 *			{
 *				//now decode field entries contents type
 *				//look up type in dictionary (dictionary functionality can be found in rsslDataDictionary.h
 *				switch((getDictionaryEntry(pDataDict, fieldEntry.fieldId))->rwfType)
 *				{
 *					//handle appropriate primitive or container types to decode
 *					...
 *				}
 *			}
 *		}
 *	}
 *	else
 *	{
 *		//initial field list decode failed
 *		printf("Error %s (%d) encountered while decoding RsslFieldList.  Error text: %s\n",
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *					
 *  @endcode
 *  @}
 *
 */

/**
 * @defgroup FieldListSetInfo Field List Set Defined Data Usage, Functions, and Structures
 * @{
 * @}
 */

/**  
 *  @}
 * 
 */

/* End of RsslFieldList */


/* Start of RsslElementList */

/**
 *  @defgroup RsslElementListGroup RsslElementList Reference Group
 *
 *	@brief The Element List container type allows the user to represent a collection of name - type - value triples, where the value can be a container type  or a primitive type.  The name and type information is similar to the content that would be cross referenced with a field dictionary when using an RsslFieldList. No dictionary is required to decode RsslElementEntry contents.
 *  
 *  To use RsslElementList functionality include @header "rtr/rsslElementList.h"
 * 
 *  @section ElementListStruct RsslElementList and RsslElementEntry Structure Overview
 *  The RsslElementList container type allows the user to represent a collection of name - type - value triples, where the value can be a container type  or a primitive type.  The name and type information is similar to the content that would be cross referenced with a field dictionary when using an RsslFieldList.  
 *  See \ref RsslElementListStruct for more detailed information on the RsslElementList and RsslElementEntry. <BR>
 *  <BR><BR>
 *	
 *  @section ElementListEnc RsslElementList Encoding Functions 
 *  The following functions are used while encoding an RsslElementList and its entries.  Entries content can be encoded from a native primitive representation (e.g. \ref RsslReal, \ref RsslInt), from pre-encoded primitive or containers, from non pre-encoded primitive or containers, or as blank.<BR>
 *  See \ref ElementListEncoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslEncodeElementListInit() - Begins RsslElementList encoding, initial function to call when encoding an RsslElementList type.</li><BR>
 *  <li>rsslEncodeElementListComplete() - Completes encoding of an RsslElementList, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired<BR>
 *  <li>rsslEncodeElementEntry() - Encodes RsslElementEntry into an RsslElementList, where the RsslElementEntry has native primitive payload passed in via void*, pre-encoded content set in RsslElementEntry::encData, or blank (where void* is NULL and RsslElementEntry::encData is empty). </li> <BR>
 *  <li>rsslEncodeElementEntryInit() - Begins RsslElementEntry encoding, initial function to call when encoding an RsslElementEntry without pre-encoded payload. </li>  <BR>
 *  <li>rsslEncodeElementEntryComplete() - Completes encoding of an RsslElementEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired. </li><BR>
 *  <BR><BR>
 *  </ul>
 *  @section ElementListDec RsslElementList Decoding Functions 
 *  The following functions are used while decoding an RsslElementList and its entries.  Once at the individual entry level, the entries content can be left as pre-encoded content or decoded to it's native primitive type or container type representation by invoking the RsslElementEntry::dataType decode functions.  <BR>
 *  See \ref ElementListDecoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslDecodeElementList() - Decodes an RsslElementList container and provides access to buffer containing all encoded element entries.</li><BR>
 *  <li>rsslDecodeElementEntry() - Decodes RsslElementList entries and provides access to each individual RsslElementEntry and their content.  If further content decoding is desired, the application can call the decoder that corresponds to RsslElementEntry::dataType.  </li><BR>
 *  </ul>
 *  
 *  @{
 */

/**
 *	@defgroup RsslElementListStruct RsslElementList Container Type
 *  @brief Detailed information about RsslElementList and RsslElementEntry structures, initializers, flag values, and helper functions
 *  @{
 *
 *  The RsslElementList container type allows the user to represent a collection of name - type - value triples, where the value can be a container type  or a primitive type.  The name and type information is similar to the content that would be cross referenced with a field dictionary when using an RsslFieldList. No dictionary is required to decode RsslElementEntry contents.
 *  @note An RsslElementList can contain standard data and/or set defined data.  When set defined data is contained, a set definition is required to decode this content - if not present, all set data decoding will be skipped. 
 *
 * 
 *  @section ElementListUseExample RsslElementList Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslElementList and RsslElementEntry.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslElementList.  When decoding, the RsslElementList and any entries will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	//Create RsslElementList and RsslElementEntry on the stack and use static initializer
 *	RsslElementList elementList = RSSL_INIT_ELEMENT_LIST;
 *	RsslElementEntry elementEntry = RSSL_INIT_ELEMENT_ENTRY;
 *  
 *	//Populate RsslElementList with relevant information
 *	//Set flags to indicate element list has standard data
 *	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
 *	//Now invoke RsslElementList encoding functions (e.g. rsslEncodeElementListInit). 
 *  
 *	//Populate RsslElementEntry with relevant information (set type to encode and name)
 *	elementEntry.dataType = RSSL_DT_UINT;
 *	elementEntry.name.length = 3;
 *	elementEntry.name.data = "TRI";
 *	//Now invoke RsslElementEntry encode functions, passing in the primitive type payload as unencoded data (e.g. rsslEncodeElementEntry)
 *  
 *	//------------------------------------------
 *  
 *	//We can also use the clear function for initial use (or reuse) using the clear function instead of static initializer
 *	RsslElementList elementList;
 *	RsslElementEntry elementEntry;
 *  
 *	//Clear RsslElementList and populate with relevant information
 *	rsslClearElementList(&elementList);
 *	//Set flags to indicate element list has standard data, set data, set Id (for decoding and encoding set data)
 *	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_SET_DATA | RSSL_ELF_HAS_SET_ID;
 *	//Populate relevant members on RsslElementList
 *	elementList.setId = 3;
 *	//Now invoke RsslElementList encoding functions, ensuring to pass in set definition (e.g. rsslEncodeElementListInit).
 * 
 *	//Clear RsslElementEntry and populate with relevant information
 *	elementEntry.name.length = 3;
 *	elementEntry.name.data = "TRI";
 *	elementEntry.dataType = RSSL_DT_REAL;
 *	//Now invoke RsslElementEntry encode functions (e.g. rsslEncodeElementEntry, rsslEncodeElementEntryInit)
 *
 *	@endcode
 *
 *  @section TheElementListStruct RsslElementList Structure, Initializers and Helpers
 *  @defgroup EStruct RsslElementList Structure, Initializers, Flag values, and Helpers
 *  @brief Detailed information about the RsslElementList Structure, Initializers, and Helper methods
 *  @{
 *
 *  @}
 *
 *
 *  @section TheElementEntryStruct RsslElementEntry Structure, Initializers, and Helpers
 *  @defgroup EEStruct RsslElementEntry Structure, Initalizers and Helpers
 *  @brief Detailed information about the RsslElementEntry Structure, Initializers, and Helper methods
 *  @{
 * 
 *  @}
 *
 *  @}
 *
 */

/**
 *  @defgroup ElementListEncoding RsslElementList and RsslElementEntry Encoding
 *  @brief Detailed information about RsslElementList and RsslElementEntry encoding
 *  @{
 * 
 *  When encoding an RsslElementList, the encoding process begins with a call to rsslEncodeElementListInit().  Entries can be encoded by using rsslEncodeElementEntry(), where the RsslElementEntry is populated with pre-encodede-encoded content (or blank), 
 *  or using rsslEncodeElementEntryInit(), the appropriate encode functions based on the type described by the RsslElementEntry::dataType, and rsslEncodeElementEntryComplete().  
 *  If encoding RsslElementEntry content from a native primitive representation (e.g. \ref RsslReal, \ref RsslInt), pass an empty RsslElementEntry::encData \ref RsslBuffer and provide pointer to primitive type as void* parameter on rsslEncodeElementEntry().
 *  If encoding the RsslElementEntry content from pre-encoded content, populate the RsslElementEntry::encData \ref RsslBuffer and pass NULL as the void*. 
 *	If encoding the RsslElementEntry as blank, pass NULL as the void* and pass an empty RsslElementEntry::encData \ref RsslBuffer.
 *  When all entries are encoded, calling rsslEncodeElementListComplete() will finish RsslElementList encoding.  
 *  @section ElementListEncExample RsslElementList Encoding Example
 *	The following example demonstrates encoding an RsslElementList.  
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create the success parameter for completing the RsslElementList and RsslElementEntry encoding.  
 *	//If any error occurs, this becomes false so rsslEncodeElementEntryComplete() or rsslEncodeElementListComplete() will roll back to last successful encoding
 *	RsslBool success = RSSL_TRUE;
 *	RsslRet retVal;
 *	//Create and initialize RsslElementList structure
 *	RsslElementList elementList = RSSL_INIT_ELEMENT_LIST;
 *	//Set desired flags on element list
 *	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
 *
 *	//Initialize RsslElementList encoding, handle any error.  Last two parameters are 0 because there is no 
 *	//set definitions necessary and no set data is being encoded.
 *	if ((retVal = rsslEncodeElementListInit(&encIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
 *	{
 *		success = RSSL_FALSE;
 *		printf("Error %s (%d) encountered with rsslEncodeElementListInit().  Error Text: %s\n", 
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *	else
 *	{
 *		//Create the RsslElementEntry structure and payload primitives, this will be reused so we will clear it between uses.
 *		RsslElementEntry elementEntry = RSSL_INIT_ELEMENT_ENTRY;
 *		uIntKey = 23456;
 *		elementEntry.name.length = 3;
 *		elementEntry.name.data = "BID";
 *		elementEntry.dataType = RSSL_DT_UINT;
 *		//Begin entry encoding.  Pass in payload as void* for this entry.  
 *		retVal = rsslEncodeElementEntry(&encIter, &elementEntry, &uIntKey); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeElementEntry().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *		}
 *  
 *		//Now encode a container type as content using rsslEncodeElementEntryInit() and rsslEncodeElementEntryComplete().  
 *
 *		elementEntry.name.length = 3;
 *		elementEntry.name.data = "MAP";
 *		elementEntry.dataType = RSSL_DT_MAP;
 *		//Begin entry encoding.  Pass in length hint for expected size
 *		retVal = rsslEncodeElementEntryInit(&encIter, &elementEntry, 350);
 *		//Now call respective encode functions for type being housed in the RsslElementEntry
 *		retVal = rsslEncodeElementEntryComplete(&encIter, success);
 *	}
 *	//Done with entries, complete the element list encoding
 *	retVal = rsslEncodeElementListComplete(&encIter, success);
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup ElementListDecoding RsslElementList and RsslElementEntry Decoding
 *  @brief Detailed information about RsslElementList and RsslElementEntry decoding
 *  @{
 *
 *  When decoding an RsslElementList, each entry is returned as an \ref RsslElementEntry containing
 *  the encoded payload content.  Further decoding of an entry's content can be achieved by using the appropriate 
 *  primitive type or container type decoder as specified by RsslElementEntry::dataType.
 *  If desired, decoding of an entry's content can be skipped by 
 *  invoking the rsslDecodeElementEntry() function to move to the next entry. 
 *  @section ElementListDecExample RsslElementList Decoding Example
 *  @code
 *	RsslElementList elementList; //No need to clear since we will decode into this
 *	RsslElementEntry elementEntry;	//RsslElementEntry to expose individual entry contents through
 *	RsslRet retVal;
 *
 *	//Third parameter is 0 because we do not have set definitions in this example
 *	if ((retVal = rsslDecodeElementList(&decIter, &elementList, 0)) >= RSSL_RET_SUCCESS)
 *	{
 *		while ((retVal = rsslDecodeElementEntry(&decIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
 *		{
 *			//handle failure cases
 *			if (retVal < RSSL_RET_SUCCESS)
 *			{
 *				//handle error as needed
 *				printf("Error %s (%d) encountered while decoding element entries.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			}
 *			else
 *			{
 *				//now decode element entries contents type
 *				switch(elementEntry.dataType)
 *				{
 *					//handle appropriate primitive or container types to decode
 *					...
 *				}
 *			}
 *		}
 *	}
 *	else
 *	{
 *		//initial element list decode failed
 *		printf("Error %s (%d) encountered while decoding RsslElementList.  Error text: %s\n",
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *					
 *  @endcode
 *  @}
 *
 */

/**
 * @defgroup ElementListSetInfo Element List Set Defined Data Usage, Functions, and Structures
 * @{
 * @}
 */

/**  
 *  @}
 * 
 */

/* End of RsslElementList */

 

/* Start of RsslMap*/

/**
 *  @defgroup RsslMapGroup RsslMap Reference Group
 *
 *	@brief The Map container type allows the user to represent a primitiveType key indexed collection of uniform entries (e.g. each entry houses the same containerType).
 *  
 *  To use RsslMap functionality include @header "rtr/rsslMap.h"
 * 
 *  @section MapStruct RsslMap and RsslMapEntry Structure Overview
 *  The RsslMap container type allows the user to represent primitive type key indexed collection of uniform entries (e.g. each entry houses the same container type).  
 *  Each RsslMapEntry::encKey is a primitive type, with the type information specified in the RsslMap::keyPrimitiveType.  All contained RsslMapEntry house the same container type, 
 *  with the type information specified in the RsslMap::containerType.  <BR>
 *  See \ref RsslMapStruct for more detailed information on the RsslMap and RsslMapEntry. <BR>
 *  <BR><BR>
 *	
 *  @section MapEnc RsslMap Encoding Functions 
 *  The following functions are used while encoding an RsslMap and its entries.  Entries key can be encoded from a native primitive representation (e.g. \ref RsslReal, \ref RsslInt) or from pre-encoded primitive content.<BR>
 *  See \ref MapEncoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslEncodeMapInit() - Begins RsslMap encoding, initial function to call when encoding an RsslMap type.</li><BR>
 *  <li>rsslEncodeMapComplete() - Completes encoding of an RsslMap, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired<BR>
 *  <li>rsslEncodeMapSetDefsComplete() - Completes encoding of set definitions on an RsslMap, called after set definitions (if any) are encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be encoded and this function should be called prior to any summary data encoding.</li> <BR>
 *  <li>rsslEncodeMapSummaryDataComplete() - Completes encoding of summary data on an RsslMap, called after summary data is encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be completed first, followed by any summary data encoding and this function.</li> <BR>
 *  <li>rsslEncodeMapEntry() - Encodes RsslMapEntry into an RsslMap, where the RsslMapEntry has no payload or has pre-encoded content set in RsslMapEntry::encData.  Entry key can be passed in and encoded via this function or can be populated in RsslMapEntry::encKey if preencoded.</li> <BR>
 *  <li>rsslEncodeMapEntryInit() - Begins RsslMapEntry encoding, initial function to call when encoding an RsslMapEntry without pre-encoded payload. Entry key can be passed in and encoded via this function or can be populated in RsslMapEntry::encKey if preencoded.</li>  <BR>
 *  <li>rsslEncodeMapEntryComplete() - Completes encoding of an RsslMapEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired. </li><BR>
 *  <BR><BR>
 *  </ul>
 *  @section MapDec RsslMap Decoding Functions 
 *  The following functions are used while decoding an RsslMap and its entries.  Once at the individual entry level, the entries key can be decoded to it's native primitive type representation (e.g. \ref RsslReal, \ref RsslInt) or left as pre-encoded primitive content. <BR>
 *  See \ref MapDecoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslDecodeMap() - Decodes an RsslMap container and provides access to buffer containing all encoded map entries.</li><BR>
 *  <li>rsslDecodeMapEntry() - Decodes RsslMap entries and provides access to individual map entries and their keys.  RsslMap::containerType decode functions can be used to decode individual RsslMapEntry contents.</li> <BR>
 *  </ul>
 *  
 *  @{
 */

/**
 *	@defgroup RsslMapStruct RsslMap Container Type
 *  @brief Detailed information about RsslMap and RsslMapEntry structures, initializers, flag values, and helper functions
 *  @{
 *
 *  The RsslMap container type allows the user to represent primitive type key indexed collection of uniform entries (e.g. each entry houses the same container type).  
 *  Each RsslMapEntry::encKey is a primitive type, with the type information specified in the RsslMap::keyPrimitiveType.  All contained RsslMapEntry house the same container type, 
 *  with the type information specified in the RsslMap::containerType.
 *  @note An RsslMap can contain standard data and set defined data when the RsslMap::containerType is RsslFieldList or RsslElementList.  
 *
 * 
 *  @section MapUseExample RsslMap Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslMap and RsslMapEntry.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslMap.  When decoding, the RsslMap and any entries will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	//Create RsslMap and RsslMapEntry on the stack and use static initializer
 *	RsslMap rsslMap = RSSL_INIT_MAP;
 *	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
 *  
 *	//Populate RsslMap with relevant information
 *	rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_TOTAL_COUNT_HINT;
 *	//Set RsslMap::containerType and RsslMap::keyPrimitiveType
 *	rsslMap.containerType = RSSL_DT_FIELD_LIST;
 *	rsslMap.keyPrimitiveType = RSSL_DT_REAL;
 *	//If summary data is preencoded, can set encoded data and length directly on RsslMap prior to encoding
 *	rsslMap.encSummaryData.data = encSummaryData.data;
 *	rsslMap.encSummaryData.length = encSummaryData.length;
 *	//Set approximate total count hint of the number of map entries across all parts of the map.
 *	rsslMap.totalCountHint = 128;
 *	//Now invoke RsslMap encoding functions (e.g. rsslEncodeMapInit). 
 *  
 *	//Populate RsslMapEntry with relevant information
 *	//Flags are cleared, no need to set RSSL_MPEF_NONE
 *	//Set Action
 *	mapEntry.action = RSSL_MPEA_ADD_ENTRY;
 *	//Now invoke RsslMapEntry encode functions, passing in the key as unencoded data (e.g. rsslEncodeMapEntryInit, rsslEncodeMapEntryComplete)
 *  
 *	//------------------------------------------
 *  
 *	//We can also use the clear function for initial use (or reuse) using the clear function instead of static initializer
 *	RsslMap rsslMap;
 *	RsslMapEntry mapEntry;
 *  
 *	//Clear RsslMap and populate with relevant information
 *	rsslClearMap(&rsslMap);
 *	rsslMap.flags = RSSL_MPF_HAS_KEY_FIELD_ID | RSSL_MPF_HAS_PER_ENTRY_PERM_DATA;
 *	rsslMap.keyFieldId = 25;
 *	rsslMap.keyPrimitiveType = RSSL_DT_REAL;
 *	rsslMap.containerType = RSSL_DT_FIELD_LIST;
 *	//Now invoke RsslMap encoding functions (e.g. rsslEncodeMapInit).
 * 
 *	//Clear RsslMapEntry and populate with relevant information
 *	mapEntry.flags = RSSL_MPEF_HAS_PERM_DATA;
 *	// Populate perm data on RsslMapEntry::permData.  This example sets it to buffer that is assumed to contain lock 
 *	mapEntry.permData.data = permDataBuffer.data;
 *	mapEntry.permData.length = permDataBuffer.length;
 *	//If key is pre-encoded, can set this on RsslMapEntry::encKey
 *	mapEntry.encKey.data = preEncReal.data;
 *	mapEntry.encKey.length = preEncReal.length;
 *	//Set action 
 *	mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
 *	//Now invoke RsslMapEntry encode functions (e.g. rsslEncodeMapEntry)
 *
 *	@endcode
 *
 *  @section TheMapStruct RsslMap Structure, Initializers and Helpers
 *  @defgroup MStruct RsslMap Structure, Initializers, Flag values, and Helpers
 *  @brief Detailed information about the RsslMap Structure, Initializers, and Helper methods
 *  @{
 *
 *  @}
 *
 *
 *  @section TheMapEntryStruct RsslMapEntry Structure, Initializers, and Helpers
 *  @defgroup MEStruct RsslMapEntry Structure, Initalizers and Helpers
 *  @brief Detailed information about the RsslMapEntry Structure, Initializers, Flag values, and Helper methods
 *  @{
 * 
 *  @}
 *
 *  @}
 *
 */

/**
 *  @defgroup MapEncoding RsslMap and RsslMapEntry Encoding
 *  @brief Detailed information about RsslMap and RsslMapEntry encoding
 *  @{
 * 
 *  When encoding an RsslMap, the encoding process begins with a call to rsslEncodeMapInit().  Entries can be encoded by using rsslEncodeMapEntry(), where the RsslMapEntry is populated with pre-encodede-encoded content, 
 *  or using rsslEncodeMapEntryInit(), the specified RsslMap::containerType encode functions, and rsslEncodeMapEntryComplete().  
 *  If encoding the key from a native primitive representation (e.g. \ref RsslReal, \ref RsslInt), pass an empty RsslMapEntry::encKey \ref RsslBuffer and provide pointer to primitive type as void* parameter on rsslEncodeMapEntry() or rsslEncodeMapEntryInit().
 *  If encoding the key from pre-encoded content, populate the RsslMapEntry::encKey \ref RsslBuffer and pass NULL as the void*. When all entries are encoded, calling rsslEncodeMapComplete() will finish RsslMap encoding.  
 *  @section MapEncExample RsslMap Encoding Example
 *	The following example demonstrates encoding an RsslMap.  
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create the success parameter for completing the RsslMap and RsslMapEntry encoding.  
 *	//If any error occurs, this becomes false so rsslEncodeMapEntryComplete() or rsslEncodeMapComplete() will roll back to last successful encoding
 *	RsslBool success = RSSL_TRUE;
 *	RsslRet retVal;
 *	//Create and initialize RsslMap structure
 *	RsslMap rsslMap = RSSL_INIT_MAP;
 *	//Create the RsslMapEntry structure and key primitive, this will be reused so we will clear it between uses.
 *	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
 *	RsslUInt uIntKey;
 *	//Set desired key primitive type that each entry will use
 *	rsslMap.keyPrimitiveType = RSSL_DT_UINT;
 *	//Set desired container type that each entry will contain
 *	rsslMap.containerType = RSSL_DT_FIELD_LIST;
 *	//Set desired flags on map
 *	rsslMap.flags = RSSL_MPF_HAS_TOTAL_COUNT_HINT | RSSL_MPF_HAS_SUMMARY_DATA;
 *	rsslMap.totalCountHint = 2;
 *
 *	//Initialize RsslMap encoding, handle any error.  Last two parameters are 0 because there is no 
 *	//set data or summary data being used in this example.  
 *	if ((retVal = rsslEncodeMapInit(&encIter, &rsslMap, 0, 0)) < RSSL_RET_SUCCESS)
 *	{
 *		success = RSSL_FALSE;
 *		printf("Error %s (%d) encountered with rsslEncodeMapInit().  Error Text: %s\n", 
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *	else
 *	{
 *		//Encode Summary Data contents.  This should match the type specified in RsslMap::containerType
 *		...
 *		//When summary data content is completed, finish it with rsslEncodeMapSummaryDataComplete()
 *		retVal = rsslEncodeMapSummaryDataComplete(&encIter, success)
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeMapSummaryDataComplete().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Roll back entire map or start encoding over again. 
 *			rsslEncodeMapComplete(&encIter, success);
 *		}
 *		//Now populate RsslMapEntry and its key value (passing key as void* parameter since it is not pre-encoded)
 *		//Populate key
 *		uIntKey = 23456;
 *		//Populate entry with desired content.  No need to set flags since they are cleared and not adding perm data.
 *		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
 *		//Begin entry encoding.  Pass in key as void* for this entry.  Expected content length is 
 *		//less than 100 bytes, so we are providing the maxEncodingSize hint.
 *		retVal = rsslEncodeMapEntryInit(&encIter, &mapEntry, &uIntKey, 100); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeMapEntryInit().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Can roll back this entry or entire map
 *			rsslEncodeMapEntryComplete(&encIter, success);
 *		}
 *  
 *		//Now encode RsslFieldList content.  See field list encoding example at \ref FieldListEncExample
 *
 *		//When field list encoding completes, finish the RsslMapEntry
 *		retVal = rsslEncodeMapEntryComplete(&encIter, success);
 *	
 *		//Encode second entry. This time, assume pre-encoded key and pre-encoded payload are contained in buffers.
 *		//Clear RsslMapEntry for reuse
 *		rsslClearMapEntry(&mapEntry);
 *		//Populate entry with pre-encoded content
 *		mapEntry.encKey.data = encKeyBuffer.data;
 *		mapEntry.encKey.length = encKeyBuffer.length;
 *		mapEntry.encData.data = encDataBuffer.data;
 *		mapEntry.encData.length = encDataBuffer.length;
 *		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
 *  
 *		//Because key is pre-encoded and populated on entry, pass in NULL for void*
 *		retVal = rsslEncodeMapEntry(&encIter, &mapEntry, NULL); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeMapEntry().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Non Init/Complete calls will automatically roll back entry when failure occurs.  
 *			//Can choose to roll back entire map if user wants *         
 *		}
 *	}
 *	//Done with entries, complete the map encoding
 *	retVal = rsslEncodeMapComplete(&encIter, success);
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup MapDecoding RsslMap and RsslMapEntry Decoding
 *  @brief Detailed information about RsslMap and RsslMapEntry decoding
 *  @{
 *
 *  When decoding an RsslMap, each entry is returned as an \ref RsslMapEntry containing
 *  the encoded key and payload content.  Further decoding of an entry's content can be achieved by using the appropriate 
 *  container type decoder (e.g. if RsslMap::containerType is ::RSSL_DT_FIELD_LIST, call rsslDecodeFieldList()). Further decoding of an entry's key content can be achieved by passing in 
 *  the correct primitive on the rsslDecodeMapEntry() function or by calling the correct primitive type decoder (e.g. if RsslMap::keyPrimitiveType is ::RSSL_DT_UINT call rsslDecodeUInt()).
 *  If desired, decoding of an entry's content can be skipped by 
 *  invoking the rsslDecodeMapEntry() function to move to the next entry. 
 *  @section MapDecExample RsslMap Decoding Example
 *  @code
 *	RsslMap rsslMap; //No need to clear since we will decode into this
 *	RsslMapEntry mapEntry;	//RsslMapEntry to expose individual entry contents through
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeMap(&decIter, &rsslMap)) >= RSSL_RET_SUCCESS)
 *	{
 *		//If key is an unsigned integer, we can pass in to have key decoded 
 *		//during call to rsslDecodeMapEntry().  If NULL is passed in, only
 *		//RsslMapEntry::encKey will be available, but primitive type decoder can 
 *		//be used to later decode the key. 
 *		RsslUInt keyUInt;
 *		while ((retVal = rsslDecodeMapEntry(&decIter, &mapEntry, &keyUInt)) != RSSL_RET_END_OF_CONTAINER)
 *		{
 *			//handle failure cases
 *			if (retVal < RSSL_RET_SUCCESS)
 *			{
 *				//handle error as needed
 *				printf("Error %s (%d) encountered while decoding map entries.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			}
 *			else
 *			{
 *				//now decode container type
 *				switch(rsslMap.containerType)
 *				{
 *					//handle appropriate container types and do something with data
 *					...
 *				}
 *			}
 *		}
 *	}
 *	else
 *	{
 *		//initial map decode failed
 *		printf("Error %s (%d) encountered while decoding RsslMap.  Error text: %s\n",
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslMap */

 
/* Start of RsslSeries */

/**
 *  @defgroup RsslSeriesGroup RsslSeries Reference Group
 *
 *	@brief The Series container type allows the user to represent an implicitly indexed collection of uniform entries (e.g. each entry houses the same containerType).  This is well suited for sending infrequently or non-updating table like content, where each RsslSeriesEntry represents an additional row in a table.  
 *  
 *  To use RsslSeries functionality include @header "rtr/rsslSeries.h"
 * 
 *  @section SeriesStruct RsslSeries and RsslSeriesEntry Structure Overview
 *  The RsslSeries container type allows the user to represent an implicitly indexed collection of uniform entries (e.g. each entry houses the same container type).  
 *  This is well suited for sending infrequently or non-updating table like content, where each RsslSeriesEntry represents an additional row in a table.  
 *  All contained RsslSeriesEntry house the same container type, with the type information specified in the RsslSeries::containerType.  <BR>
 *  See \ref RsslSeriesStruct for more detailed information on the RsslSeries and RsslSeriesEntry. <BR>
 *  <BR><BR>
 *	
 *  @section SerEnc RsslSeries Encoding Functions 
 *  The following functions are used while encoding an RsslSeries and its entries.<BR>
 *  See \ref SeriesEncoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslEncodeSeriesInit() - Begins RsslSeries encoding, initial function to call when encoding an RsslSeries type.</li><BR>
 *  <li>rsslEncodeSeriesComplete() - Completes encoding of an RsslSeries, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired<BR>
 *  <li>rsslEncodeSeriesSetDefsComplete() - Completes encoding of set definitions on an RsslSeries, called after set definitions (if any) are encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be encoded and this function should be called prior to any summary data encoding.</li> <BR>
 *  <li>rsslEncodeSeriesSummaryDataComplete() - Completes encoding of summary data on an RsslSeries, called after summary data is encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be completed first, followed by any summary data encoding and this function.</li> <BR>
 *  <li>rsslEncodeSeriesEntry() - Encodes RsslSeriesEntry into an RsslVector, where the RsslSeriesEntry has no payload or has pre-encoded content set in RsslSeriesEntry::encData.</li> <BR>
 *  <li>rsslEncodeSeriesEntryInit() - Begins RsslSeriesEntry encoding, initial function to call when encoding an RsslSeriesEntry without pre-encoded payload.</li>  <BR>
 *  <li>rsslEncodeSeriesEntryComplete() - Completes encoding of an RsslSeriesEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired. </li><BR>
 *  <BR><BR>
 *  </ul>
 *  @section SerDec RsslSeries Decoding Functions 
 *  The following functions are used while decoding an RsslSeries and its entries.<BR>
 *  See \ref SeriesDecoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslDecodeSeries() - Decodes an RsslSeries container and provides access to buffer containing all encoded series entries.</li><BR>
 *  <li>rsslDecodeSeriesEntry() - Decodes RsslSeries entries and provides access to individual series entries.  RsslSeries::containerType decode functions can be used to decode individual RsslSeriesEntry contents.</li> <BR>
 *  </ul>
 *  
 *  @{
 */

/**
 *	@defgroup RsslSeriesStruct RsslSeries Container Type
 *  @brief Detailed information about RsslSeries and RsslSeriesEntry structures, initializers, flag values, and helper functions
 *  @{
 *
 *  The RsslSeries container type allows the user to represent an implicitly indexed collection of uniform entries (e.g. each entry houses the same container type).  
 *  All contained RsslSeriesEntry house the same container type, with the type information specified in the RsslSeries::containerType.
 *  This is well suited for sending infrequently or non-updating table like content, where each RsslSeriesEntry represents an additional row in a table.  
 *  @note An RsslSeries can contain standard data and set defined data when the RsslSeries::containerType is RsslFieldList or RsslElementList.  
 *
 * 
 *  @section SeriesUseExample RsslSeries Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslSeries and RsslSeriesEntry.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslSeries.  When decoding, the RsslSeries and any entries will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	//Create RsslSeries and RsslSeriesEntry on the stack and use static initializer
 *	RsslSeries rsslSeries = RSSL_INIT_SERIES;
 *	RsslSeriesEntry seriesEntry = RSSL_INIT_SERIES_ENTRY;
 *  
 *	//Populate RsslSeries with relevant information
 *	rsslSeries.flags = RSSL_SRF_HAS_SUMMARY_DATA | RSSL_SRF_HAS_TOTAL_COUNT_HINT;
 *	//Set RsslSeries::containerType
 *	rsslSeries.containerType = RSSL_DT_ELEMENT_LIST;
 *	//If summary data is preencoded, can set encoded data and length directly on RsslSeries prior to encoding
 *	rsslSeries.encSummaryData.data = encSummaryData.data;
 *	rsslSeries.encSummaryData.length = encSummaryData.length;
 *	//Set approximate total count hint of the number of series entries across all parts of the series.
 *	rsslSeries.totalCountHint = 5120;
 *	//Now invoke RsslSeries encoding functions (e.g. rsslEncodeSeriesInit()). 
 *  
 *	//Populate RsslSeriesEntry with relevant information
 *	//Now invoke RsslSeriesEntry encode functions (e.g. rsslEncodeSeriesEntryInit(), rsslEncodeSeriesEntryComplete())
 *  
 *	//------------------------------------------
 *  
 *	//We can also use the clear function for initial use (or reuse) using the clear function instead of static initializer
 *	RsslVector rsslSeries;
 *	RsslSeriesEntry seriesEntry;
 *  
 *	//Clear RsslSeries and populate with relevant information
 *	rsslClearSeries(&rsslSeries);
 *	rsslSeries.containerType = RSSL_DT_ELEMENT_LIST;
 *	//Now invoke RsslSeries encoding functions (e.g. rsslEncodeSeriesInit()).
 * 
 *	//Clear RsslSeriesEntry and populate with relevant information
 *	//Now invoke RsslSeriesEntry encode functions (e.g. rsslEncodeSeriesEntry())
 *
 *	@endcode
 *
 *  @section TheSeriesStruct RsslSeries Structure, Initializers and Helpers
 *  @defgroup SStruct RsslSeries Structure, Initializers, Flag values, and Helpers
 *  @brief Detailed information about the RsslSeries Structure, Initializers, and Helper methods
 *  @{
 *
 *  @}
 *
 *
 *  @section TheSeriesEntryStruct RsslSeriesEntry Structure, Initializers, and Helpers
 *  @defgroup SEStruct RsslSeriesEntry Structure, Initalizers and Helpers
 *  @brief Detailed information about the RsslSeriesEntry Structure, Initializers, Flag values, and Helper methods
 *  @{
 * 
 *  @}
 *
 *  @}
 *
 */

/**
 *  @defgroup SeriesEncoding RsslSeries and RsslSeriesEntry Encoding
 *  @brief Detailed information about RsslSeries and RsslSeriesEntry encoding
 *  @{
 * 
 *  When encoding an RsslSeries, the encoding process begins with a call to rsslEncodeSeriesInit().  Entries can be encoded by using rsslEncodeSeriesEntry(), where the RsslSeriesEntry is populated with pre-encodede-encoded content, 
 *  or using rsslEncodeSeriesEntryInit(), the specified RsslSeries::containerType encode functions, and rsslEncodeSeriesEntryComplete().  
 *  When all entries are encoded, calling rsslEncodeSeriesComplete() will finish RsslSeries encoding.  
 *  @section SeriesEncExample RsslSeries Encoding Example
 *	The following example demonstrates encoding an RsslSeries.  
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create the success parameter for completing the RsslSeries and RsslSeriesEntry encoding.  
 *	//If any error occurs, this becomes false so rsslEncodeSeriesEntryComplete() or rsslEncodeSeriesComplete() will roll back to last successful encoding
 *	RsslBool success = RSSL_TRUE;
 *	RsslRet retVal;
 *	//Create and initialize RsslSeries structure
 *	RsslSeries rsslSeries = RSSL_INIT_SERIES;
 *	//Create the RsslSeriesEntry structure, this will be reused so we will clear it between uses.
 *	RsslSeriesEntry seriesEntry = RSSL_INIT_SERIES_ENTRY;
 *	//Set desired container type that each entry will contain
 *	rsslSeries.containerType = RSSL_DT_ELEMENT_LIST;
 *	//Set desired flags on series
 *	rsslSeries.flags = RSSL_SRF_HAS_TOTAL_COUNT_HINT | RSSL_SRF_HAS_SUMMARY_DATA;
 *	rsslSeries.totalCountHint = 2;
 *
 *	//Initialize RsslSeries encoding, handle any error.  This example expects summary data to be less than 50 bytes, so encoding length hint is provided.  Last parameter is 0 because there is no set data.   
 *	if ((retVal = rsslEncodeSeriesInit(&encIter, &rsslSeries, 50, 0)) < RSSL_RET_SUCCESS)
 *	{
 *		success = RSSL_FALSE;
 *		printf("Error %s (%d) encountered with rsslEncodeSeriesInit().  Error Text: %s\n", 
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *	else
 *	{
 *		//Encode Summary Data contents.  This should match the type specified in RsslSeries::containerType
 *		...
 *		//When summary data content is completed, finish it with rsslEncodeSeriesSummaryDataComplete()
 *		retVal = rsslEncodeSeriesSummaryDataComplete(&encIter, success)
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeSeriesSummaryDataComplete().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Roll back entire series or start encoding over again. 
 *			rsslEncodeSeriesComplete(&encIter, success);
 *		}
 *		//Now populate RsslSeriesEntry 
 *		//Populate entry with desired content.  No need to set flags since they are cleared and not adding perm data.
 *		
 *		retVal = rsslEncodeSeriesEntryInit(&encIter, &seriesEntry, 100); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeSeriesEntryInit().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Can roll back this entry or entire series
 *			rsslEncodeSeriesEntryComplete(&encIter, success);
 *		}
 *  
 *		//Now encode RsslElementList content.  See element list encoding example at \ref ElementListEncExample
 *
 *		//When element list encoding completes, finish the RsslSeriesEntry
 *		retVal = rsslEncodeSeriesEntryComplete(&encIter, success);
 *	
 *		//Encode second entry. This time, assume pre-encoded payload is contained in buffer.
 *		//Clear RsslSeriesEntry for reuse
 *		rsslClearSeriesEntry(&seriesEntry);
 *		//Populate entry with pre-encoded content
 *		seriesEntry.encData.data = encDataBuffer.data;
 *		seriesEntry.encData.length = encDataBuffer.length;
 *  
 *		retVal = rsslEncodeSeriesEntry(&encIter, &seriesEntry); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeSeriesEntry().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Non Init/Complete calls will automatically roll back entry when failure occurs.  
 *			//Can choose to roll back entire series if user wants *         
 *		}
 *	}
 *	//Done with entries, complete the series encoding
 *	retVal = rsslEncodeSeriesComplete(&encIter, success);
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup SeriesDecoding RsslSeries and RsslSeriesEntry Decoding
 *  @brief Detailed information about RsslSeries and RsslSeriesEntry decoding
 *  @{
 *
 *  When decoding an RsslSeries, each entry is returned as an \ref RsslSeriesEntry containing
 *  the encoded payload content.  Further decoding of an entry's content can be achieved by using the appropriate 
 *  container type decoder (e.g. if RsslSeries::containerType is ::RSSL_DT_ELEMENT_LIST, call rsslDecodeElementList()).
 *  If desired, decoding of an entry's content can be skipped by 
 *  invoking the rsslDecodeSeriesEntry() function to move to the next entry. 
 *  @section SeriesDecExample RsslSeries Decoding Example
 *  @code
 *	RsslSeries rsslSeries; //No need to clear since we will decode into this
 *	RsslSeriesEntry seriesEntry; //RsslSeriesEntry to expose individual entry contents through
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeSeries(&decIter, &rsslSeries)) >= RSSL_RET_SUCCESS)
 *	{
 *		while ((retVal = rsslDecodeSeriesEntry(&decIter, &seriesEntry)) != RSSL_RET_END_OF_CONTAINER)
 *		{
 *			//handle failure cases
 *			if (retVal < RSSL_RET_SUCCESS)
 *			{
 *				//handle error as needed
 *				printf("Error %s (%d) encountered while decoding series entries.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			}
 *			else
 *			{
 *				//now decode container type
 *				switch(rsslSeries.containerType)
 *				{
 *					//handle appropriate container types and do something with data
 *					...
 *				}
 *			}
 *		}
 *	}
 *	else
 *	{
 *		//initial series decode failed
 *		printf("Error %s (%d) encountered while decoding RsslSeries.  Error text: %s\n",
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslSeries */

 
/* Start of RsslVector */

/**
 *  @defgroup RsslVectorGroup RsslVector Reference Group
 *
 *	@brief The Vector container type allows the user to represent a \ref RsslUInt indexed collection of uniform entries (e.g. each entry houses the same containerType).  This container type can optionally support sorting, where the consumer of content is responsible for applying actions in the appropriate manner to maintain ordering.
 *  
 *  To use RsslVector functionality include @header "rtr/rsslVector.h"
 * 
 *  @section VectorStruct RsslVector and RsslVectorEntry Structure Overview
 *  The RsslVector container type allows the user to represent a \ref RsslUInt indexed collection of uniform entries (e.g. each entry houses the same container type).  
 *  This container type can optionally support sorting through the use of the ::RSSL_VTF_SUPPORTS_SORTING flag and specific actions, where the consumer of content is responsible for applying actions in the appropriate manner to maintain ordering. 
 *  All contained RsslVectorEntry house the same container type, with the type information specified in the RsslVector::containerType.  <BR>
 *  See \ref RsslVectorStruct for more detailed information on the RsslVector and RsslVectorEntry. <BR>
 *  <BR><BR>
 *	
 *  @section VecEnc RsslVector Encoding Functions 
 *  The following functions are used while encoding an RsslVector and its entries.<BR>
 *  See \ref VectorEncoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslEncodeVectorInit() - Begins RsslVector encoding, initial function to call when encoding an RsslVector type.</li><BR>
 *  <li>rsslEncodeVectorComplete() - Completes encoding of an RsslVector, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired<BR>
 *  <li>rsslEncodeVectorSetDefsComplete() - Completes encoding of set definitions on an RsslVector, called after set definitions (if any) are encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be encoded and this function should be called prior to any summary data encoding.</li> <BR>
 *  <li>rsslEncodeVectorSummaryDataComplete() - Completes encoding of summary data on an RsslVector, called after summary data is encoded or encoding fails and encoding rollback is desired.  If both set definitions and summary data are present and neither are pre-encoded, set definitions should be completed first, followed by any summary data encoding and this function.</li> <BR>
 *  <li>rsslEncodeVectorEntry() - Encodes RsslVectorEntry into an RsslVector, where the RsslVectorEntry has no payload or has pre-encoded content set in RsslVectorEntry::encData.</li> <BR>
 *  <li>rsslEncodeVectorEntryInit() - Begins RsslVectorEntry encoding, initial function to call when encoding an RsslVectorEntry without pre-encoded payload.</li>  <BR>
 *  <li>rsslEncodeVectorEntryComplete() - Completes encoding of an RsslVectorEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired. </li><BR>
 *  <BR><BR>
 *  </ul>
 *  @section VecDec RsslVector Decoding Functions 
 *  The following functions are used while decoding an RsslVector and its entries.<BR>
 *  See \ref VectorDecoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslDecodeVector() - Decodes an RsslVector container and provides access to buffer containing all encoded vector entries.</li><BR>
 *  <li>rsslDecodeVectorEntry() - Decodes RsslVector entries and provides access to individual vector entries.  RsslVector::containerType decode functions can be used to decode individual RsslVectorEntry contents.</li> <BR>
 *  </ul>
 *  
 *  @{
 */

/**
 *	@defgroup RsslVectorStruct RsslVector Container Type
 *  @brief Detailed information about RsslVector and RsslVectorEntry structures, initializers, flag values, and helper functions
 *  @{
 *
 *  The RsslVector container type allows the user to represent \ref RsslUInt indexed collection of uniform entries (e.g. each entry houses the same container type).  
 *  All contained RsslVectorEntry house the same container type, with the type information specified in the RsslVector::containerType.
 *  @note An RsslVector can contain standard data and set defined data when the RsslVector::containerType is RsslFieldList or RsslElementList.  
 *
 * 
 *  @section VectorUseExample RsslVector Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslVector and RsslVectorEntry.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslVector.  When decoding, the RsslVector and any entries will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	//Create RsslVector and RsslVectorEntry on the stack and use static initializer
 *	RsslVector rsslVector = RSSL_INIT_VECTOR;
 *	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
 *  
 *	//Populate RsslVector with relevant information
 *	rsslVector.flags = RSSL_VTF_HAS_SUMMARY_DATA | RSSL_VTF_HAS_TOTAL_COUNT_HINT;
 *	//Set RsslVector::containerType
 *	rsslVector.containerType = RSSL_DT_FIELD_LIST;
 *	//If summary data is preencoded, can set encoded data and length directly on RsslVector prior to encoding
 *	rsslVector.encSummaryData.data = encSummaryData.data;
 *	rsslVector.encSummaryData.length = encSummaryData.length;
 *	//Set approximate total count hint of the number of vector entries across all parts of the vector.
 *	rsslVector.totalCountHint = 210;
 *	//Now invoke RsslVector encoding functions (e.g. rsslEncodeVectorInit()). 
 *  
 *	//Populate RsslVectorEntry with relevant information
 *	//Flags are cleared, no need to set RSSL_VTEF_NONE
 *	//Set Action and index
 *	vectorEntry.action = RSSL_VTEA_ADD_ENTRY;
 *	vectorEntry.index = 0;
 *	//Now invoke RsslVectorEntry encode functions (e.g. rsslEncodeVectorEntryInit(), rsslEncodeVectorEntryComplete())
 *  
 *	//------------------------------------------
 *  
 *	//We can also use the clear function for initial use (or reuse) using the clear function instead of static initializer
 *	RsslVector rsslVector;
 *	RsslVectorEntry vectorEntry;
 *  
 *	//Clear RsslVector and populate with relevant information
 *	rsslClearVector(&rsslVector);
 *	rsslVector.flags = RSSL_VTF_HAS_PER_ENTRY_PERM_DATA;
 *	rsslVector.containerType = RSSL_DT_FIELD_LIST;
 *	//Now invoke RsslVector encoding functions (e.g. rsslEncodeVectorInit()).
 * 
 *	//Clear RsslVectorEntry and populate with relevant information
 *	vectorEntry.flags = RSSL_VTEF_HAS_PERM_DATA;
 *	// Populate perm data on RsslVectorEntry::permData.  This example sets it to buffer that is assumed to contain lock 
 *	vectorEntry.permData.data = permDataBuffer.data;
 *	vectorEntry.permData.length = permDataBuffer.length;
 *	//Set action and index
 *	vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
 *	vectorEntry.index = 0;
 *	//Now invoke RsslVectorEntry encode functions (e.g. rsslEncodeVectorEntry())
 *
 *	@endcode
 *
 *  @section TheVectorStruct RsslVector Structure, Initializers and Helpers
 *  @defgroup VStruct RsslVector Structure, Initializers, Flag values, and Helpers
 *  @brief Detailed information about the RsslVector Structure, Initializers, and Helper methods
 *  @{
 *
 *  @}
 *
 *
 *  @section TheVectorEntryStruct RsslVectorEntry Structure, Initializers, and Helpers
 *  @defgroup VEStruct RsslVectorEntry Structure, Initalizers and Helpers
 *  @brief Detailed information about the RsslVectorEntry Structure, Initializers, Flag values, and Helper methods
 *  @{
 * 
 *  @}
 *
 *  @}
 *
 */

/**
 *  @defgroup VectorEncoding RsslVector and RsslVectorEntry Encoding
 *  @brief Detailed information about RsslVector and RsslVectorEntry encoding
 *  @{
 * 
 *  When encoding an RsslVector, the encoding process begins with a call to rsslEncodeVectorInit().  Entries can be encoded by using rsslEncodeVectorEntry(), where the RsslVectorEntry is populated with pre-encodede-encoded content, 
 *  or using rsslEncodeVectorEntryInit(), the specified RsslVector::containerType encode functions, and rsslEncodeVectorEntryComplete().  
 *  When all entries are encoded, calling rsslEncodeVectorComplete() will finish RsslVector encoding.  
 *  @section VectorEncExample RsslVector Encoding Example
 *	The following example demonstrates encoding an RsslVector.  
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create the success parameter for completing the RsslVector and RsslVectorEntry encoding.  
 *	//If any error occurs, this becomes false so rsslEncodeVectorEntryComplete() or rsslEncodeVectorComplete() will roll back to last successful encoding
 *	RsslBool success = RSSL_TRUE;
 *	RsslRet retVal;
 *	//Create and initialize RsslVector structure
 *	RsslVector rsslVector = RSSL_INIT_VECTOR;
 *	//Create the RsslVectorEntry structure, this will be reused so we will clear it between uses.
 *	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
 *	//Set desired container type that each entry will contain
 *	rsslVector.containerType = RSSL_DT_FIELD_LIST;
 *	//Set desired flags on vector
 *	rsslVector.flags = RSSL_VTF_HAS_TOTAL_COUNT_HINT | RSSL_VTF_HAS_SUMMARY_DATA;
 *	rsslVector.totalCountHint = 2;
 *
 *	//Initialize RsslVector encoding, handle any error.  This example expects summary data to be less than 50 bytes, so encoding length hint is provided.  Last parameter is 0 because there is no set data.   
 *	if ((retVal = rsslEncodeVectorInit(&encIter, &rsslVector, 50, 0)) < RSSL_RET_SUCCESS)
 *	{
 *		success = RSSL_FALSE;
 *		printf("Error %s (%d) encountered with rsslEncodeVectorInit().  Error Text: %s\n", 
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *	else
 *	{
 *		//Encode Summary Data contents.  This should match the type specified in RsslVector::containerType
 *		...
 *		//When summary data content is completed, finish it with rsslEncodeVectorSummaryDataComplete()
 *		retVal = rsslEncodeVectorSummaryDataComplete(&encIter, success)
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeVectorSummaryDataComplete().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Roll back entire vector or start encoding over again. 
 *			rsslEncodeVectorComplete(&encIter, success);
 *		}
 *		//Now populate RsslVectorEntry 
 *		//Populate entry with desired content.  No need to set flags since they are cleared and not adding perm data.
 *		vectorEntry.action = RSSL_VTEA_ADD_ENTRY;
 *		vectorEntry.index = 0;
 *		//Begin entry encoding.  Expected content length is 
 *		//less than 100 bytes, so we are providing the maxEncodingSize hint.
 *		retVal = rsslEncodeVectorEntryInit(&encIter, &vectorEntry, 100); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeVectorEntryInit().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Can roll back this entry or entire vector
 *			rsslEncodeVectorEntryComplete(&encIter, success);
 *		}
 *  
 *		//Now encode RsslFieldList content.  See field list encoding example at \ref FieldListEncExample
 *
 *		//When field list encoding completes, finish the RsslVectorEntry
 *		retVal = rsslEncodeVectorEntryComplete(&encIter, success);
 *	
 *		//Encode second entry. This time, assume pre-encoded payload is contained in buffer.
 *		//Clear RsslVectorEntry for reuse
 *		rsslClearVectorEntry(&vectorEntry);
 *		//Populate entry with pre-encoded content
 *		vectorEntry.encData.data = encDataBuffer.data;
 *		vectorEntry.encData.length = encDataBuffer.length;
 *		vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
 *		vectorEntry.index = 1;
 *  
 *		retVal = rsslEncodeVectorEntry(&encIter, &vectorEntry); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeVectorEntry().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Non Init/Complete calls will automatically roll back entry when failure occurs.  
 *			//Can choose to roll back entire vector if user wants *         
 *		}
 *	}
 *	//Done with entries, complete the vector encoding
 *	retVal = rsslEncodeVectorComplete(&encIter, success);
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup VectorDecoding RsslVector and RsslVectorEntry Decoding
 *  @brief Detailed information about RsslVector and RsslVectorEntry decoding
 *  @{
 *
 *  When decoding an RsslVector, each entry is returned as an \ref RsslVectorEntry containing
 *  the encoded payload content.  Further decoding of an entry's content can be achieved by using the appropriate 
 *  container type decoder (e.g. if RsslVector::containerType is ::RSSL_DT_FIELD_LIST, call rsslDecodeFieldList()).
 *  If desired, decoding of an entry's content can be skipped by 
 *  invoking the rsslDecodeVectorEntry() function to move to the next entry. 
 *  @section VectorDecExample RsslVector Decoding Example
 *  @code
 *	RsslVector rsslVector; //No need to clear since we will decode into this
 *	RsslVectorEntry vectorEntry; //RsslVectorEntry to expose individual entry contents through
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeVector(&decIter, &rsslVector)) >= RSSL_RET_SUCCESS)
 *	{
 *		while ((retVal = rsslDecodeVectorEntry(&decIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
 *		{
 *			//handle failure cases
 *			if (retVal < RSSL_RET_SUCCESS)
 *			{
 *				//handle error as needed
 *				printf("Error %s (%d) encountered while decoding vector entries.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			}
 *			else
 *			{
 *				//now decode container type
 *				switch(rsslVector.containerType)
 *				{
 *					//handle appropriate container types and do something with data
 *					...
 *				}
 *			}
 *		}
 *	}
 *	else
 *	{
 *		//initial vector decode failed
 *		printf("Error %s (%d) encountered while decoding RsslVector.  Error text: %s\n",
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslVector */

 
/* Start of RsslFilterList */

/**
 *  @defgroup RsslFilterGroup RsslFilterList Reference Group
 *
 *	@brief The Filter List container type allows the user to represent an 8-bit ID indexed collection of non-uniform entries (e.g. each entry can house different container types).  
 *  
 *  To use RsslFilterList functionality include @header "rtr/rsslFilterList.h"
 * 
 *  @section FilterListStruct RsslFilterList and RsslFilterEntry Structure Overview
 *  The RsslFilterList container type allows the user to represent an 8-bit ID indexed collection of non-uniform entries (e.g. each entry can house different container types).<BR>
 *  Each contained RsslFilterEntry can house a different container type.  If RsslFilterEntry::containerType is present, this indicates the type housed in the entry.  Otherwise, the entry houses the type indicated by RsslFilterList::containerType.<BR>
 *  See \ref RsslFilterListStruct for more detailed information on the RsslFilterList and RsslFilterEntry. <BR>
 *  <BR><BR>
 *	
 *  @section FilterEnc RsslFilterList Encoding Functions 
 *  The following functions are used while encoding an RsslFilterList and its entries.<BR>
 *  See \ref FilterListEncoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslEncodeFilterListInit() - Begins RsslFilterList encoding, initial function to call when encoding an RsslFilterList type.</li><BR>
 *  <li>rsslEncodeFilterListComplete() - Completes encoding of an RsslFilterList, called after all entries (if any) are encoded or encoding fails and encoding rollback is desired<BR>
 *  <li>rsslEncodeFilterEntry() - Encodes RsslFilterEntry into an RsslFilterList, where the RsslFilterEntry has no payload or has pre-encoded content set in RsslFilterEntry::encData.</li> <BR>
 *  <li>rsslEncodeFilterEntryInit() - Begins RsslFilterEntry encoding, initial function to call when encoding an RsslFilterEntry without pre-encoded payload.</li>  <BR>
 *  <li>rsslEncodeFilterEntryComplete() - Completes encoding of an RsslFilterEntry, called after the entries contents are encoded or encoding fails and encoding rollback is desired. </li><BR>
 *  <BR><BR>
 *  </ul>
 *  @section FilterDec RsslFilterList Decoding Functions 
 *  The following functions are used while decoding an RsslFilterList and its entries.<BR>
 *  See \ref FilterListDecoding for more detailed information.<BR><BR>
 *  <ul>
 *  <li>rsslDecodeFilterList() - Decodes an RsslFilterList container and provides access to buffer containing all encoded filter entries.</li><BR>
 *  <li>rsslDecodeFilterEntry() - Decodes RsslFilterList entries and provides access to individual vector entries.  RsslFilterList::containerType or RsslFilterEntry::containerType decode functions can be used to decode individual RsslFilterEntry contents.</li> <BR>
 *  </ul>
 *  
 *  @{
 */

/**
 *	@defgroup RsslFilterListStruct RsslFilterList Container Type
 *  @brief Detailed information about RsslFilterList and RsslFilterEntry structures, initializers, flag values, and helper functions
 *  @{
 *
 *  The RsslFilterList container type allows the user to represent an 8-bit ID indexed collection of non-uniform entries (e.g. each entry can house a different container type).
 *  Each RsslFilterEntry contains an 8-bit id corresponding to one of 32 possible bit-value identifiers.  These identifiers
 *  are typically defined by a domain model specification and can be used to indicate interest or presence of specific
 *  entries through the inclusion of the id in the RsslMsgKey::filter. 
 *  Each contained RsslFilterEntry can house a different container type.  If RsslFilterEntry::containerType is present, this indicates the type housed in the entry.  Otherwise, the entry houses the type indicated by RsslFilterList::containerType.<BR>
 *
 * 
 *  @section FilterListUseExample RsslFilterList Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslFilterList and RsslFilterEntry.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslFilterList.  When decoding, the RsslFilterList and any entries will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	//Create RsslFilterList and RsslFilterEntry on the stack and use static initializer
 *	RsslFilterList rsslFilterList = RSSL_INIT_FILTER_LIST;
 *	RsslFilterEntry filterEntry = RSSL_INIT_FILTER_ENTRY;
 *  
 *	//Populate RsslFilterList with relevant information
 *	rsslFilterList.flags = RSSL_FTF_HAS_TOTAL_COUNT_HINT;
 *	//Set RsslFilterList::containerType to the type that represents the most entries
 *	rsslFilterList.containerType = RSSL_DT_MAP;
 *	//Set approximate total count hint of the number of filter entries.
 *	rsslVector.totalCountHint = 6;
 *	//Now invoke RsslFilterList encoding functions (e.g. rsslEncodeFilterListInit()). 
 *  
 *	//Populate RsslFilterEntry with relevant information
 *	//If wanting to override RsslFilterList::containerType, indicate so on the RsslFilterEntry
 *	filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
 *	filterEntry.containerType = RSSL_DT_ELEMENT_LIST;
 *	//Set Action and ID 
 *	filterEntry.action = RSSL_FTEA_SET_ENTRY;
 *	vectorEntry.id = 1;
 *	//Now invoke RsslFilterEntry encode functions (e.g. rsslEncodeFilterEntryInit(), rsslEncodeFilterEntryComplete())
 *  
 *	//------------------------------------------
 *  
 *	//We can also use the clear function for initial use (or reuse) using the clear function instead of static initializer
 *	RsslFilterList rsslFilterList;
 *	RsslFilterEntry filterEntry;
 *  
 *	//Clear RsslFilterList and populate with relevant information
 *	rsslClearFilterList(&rsslFilterList);
 *	rsslFilterList.flags = RSSL_FTF_HAS_PER_ENTRY_PERM_DATA;
 *	rsslFilterList.containerType = RSSL_DT_MAP;
 *	//Now invoke RsslFilterList encoding functions (e.g. rsslEncodeFilterListInit()).
 * 
 *	//Clear RsslFilterEntry and populate with relevant information
 *	filterEntry.flags = RSSL_FTEF_HAS_PERM_DATA;
 *	// Populate perm data on RsslFilterEntry::permData.  This example sets it to buffer that is assumed to contain lock 
 *	filterEntry.permData.data = permDataBuffer.data;
 *	filterEntry.permData.length = permDataBuffer.length;
 *	//Set action and id
 *	filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
 *	vectorEntry.index = 2;
 *	//Now invoke RsslFilterEntry encode functions (e.g. rsslEncodeFilterEntry())
 *
 *	@endcode
 *
 *  @section TheFilterListStruct RsslFilterList Structure, Initializers and Helpers
 *  @defgroup FLStruct RsslFilterList Structure, Initializers, Flag values, and Helpers
 *  @brief Detailed information about the RsslFilterList Structure, Initializers, and Helper methods
 *  @{
 *
 *  @}
 *
 *
 *  @section TheFilterEntryStruct RsslFilterEntry Structure, Initializers, and Helpers
 *  @defgroup FLEStruct RsslFilterEntry Structure, Initalizers and Helpers
 *  @brief Detailed information about the RsslFilterEntry Structure, Initializers, Flag values, and Helper methods
 *  @{
 * 
 *  @}
 *
 *  @}
 *
 */

/**
 *  @defgroup FilterListEncoding RsslFilterList and RsslFilterEntry Encoding
 *  @brief Detailed information about RsslFilterList and RsslFilterEntry encoding
 *  @{
 * 
 *  When encoding an RsslFilterList, the encoding process begins with a call to rsslEncodeFilterListInit().  Entries can be encoded by using rsslEncodeFilterEntry(), where the RsslFilterEntry is populated with pre-encodede-encoded content, 
 *  or using rsslEncodeFilterEntryInit(), the specified RsslFilterList::containerType or RsslFilterEntry::containerType encode functions, and rsslEncodeFilterEntryComplete().  
 *  When all entries are encoded, calling rsslEncodeFilterListComplete() will finish RsslFilterList encoding.  
 *  @section FilterListEncExample RsslFilterList Encoding Example
 *	The following example demonstrates encoding an RsslFilterList.  
 * 
 *  Creation and initialization of the \ref RsslEncodeIterator is not shown in the example, however
 *  see the example in \ref RsslEncodeIteratorType for more information.  The example assumes that
 *  the encIter was properly cleared and has an appropriate size buffer and version information
 *  already set.
 *
 *  @code
 *	//Create the success parameter for completing the RsslFilterList and RsslFilterEntry encoding.  
 *	//If any error occurs, this becomes false so rsslEncodeFilterEntryComplete() or rsslEncodeFilterListComplete() will roll back to last successful encoding
 *	RsslBool success = RSSL_TRUE;
 *	RsslRet retVal;
 *	//Create and initialize RsslFilterList structure
 *	RsslFilterList rsslFilterList = RSSL_INIT_FILTER_LIST;
 *	//Create the RsslFilterEntry structure, this will be reused so we will clear it between uses.
 *	RsslFilterEntry filterEntry = RSSL_INIT_FILTER_ENTRY;
 *	//Set most common container type that entries will contain
 *	rsslFilterList.containerType = RSSL_DT_MAP;
 *	//Set desired flags on filter list
 *	rsslFilterList.flags = RSSL_FTF_HAS_TOTAL_COUNT_HINT;
 *	rsslFilterList.totalCountHint = 3;
 *
 *	//Initialize RsslFilterList encoding, handle any error.  
 *	if ((retVal = rsslEncodeFilterListInit(&encIter, &rsslFilterList)) < RSSL_RET_SUCCESS)
 *	{
 *		success = RSSL_FALSE;
 *		printf("Error %s (%d) encountered with rsslEncodeFilterListInit().  Error Text: %s\n", 
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *	else
 *	{
 *		//Now populate RsslFilterEntry 
 *		//Populate entry with desired content.  No need to set flags; sending same container as specified in RsslFilterList::containerType and not adding perm data.
 *		filterEntry.action = RSSL_FTEA_SET_ENTRY;
 *		filterEntry.id = 1;
 *		//Begin entry encoding.  Expected content length is 
 *		//less than 100 bytes, so we are providing the maxEncodingSize hint.
 *		retVal = rsslEncodeFilterEntryInit(&encIter, &filterEntry, 100); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeFilterEntryInit().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Can roll back this entry or entire vector
 *			rsslEncodeFilterEntryComplete(&encIter, success);
 *		}
 *  
 *		//Now encode RsslMap content.  See map encoding example at \ref MapEncExample
 *
 *		//When map encoding completes, finish the RsslFilterEntry
 *		retVal = rsslEncodeFilterEntryComplete(&encIter, success);
 *	
 *		//Encode second entry. This time, assume pre-encoded RsslMap payload is contained in buffer.
 *		//Clear RsslFilterEntry for reuse
 *		rsslClearFilterEntry(&filterEntry);
 *		//Populate entry with pre-encoded content
 *		filterEntry.encData.data = encMapBuffer.data;
 *		filterEntry.encData.length = encMapBuffer.length;
 *		filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
 *		filterEntry.index = 2;
 *  
 *		retVal = rsslEncodeFilterEntry(&encIter, &filterEntry); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeFilterEntry().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Non Init/Complete calls will automatically roll back entry when failure occurs.  
 *			//Can choose to roll back entire vector if user wants *         
 *		}
 * 
 *		//Encode third entry.  Different pre-encoded container type this time
 *		//Clear RsslFilterEntry for reuse
 *		rsslClearFilterEntry(&filterEntry);
 *		//set container type and flag
 *		filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
 *		filterEntry.containerType = RSSL_DT_FIELD_LIST;
 *		//Populate entry with pre-encoded content
 *		filterEntry.encData.data = encFieldListBuffer.data;
 *		filterEntry.encData.length = encFieldListBuffer.length;
 *		filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
 *		filterEntry.index = 4;
 *  
 *		retVal = rsslEncodeFilterEntry(&encIter, &filterEntry); 
 *		if (retVal < RSSL_RET_SUCCESS)
 *		{
 *			printf("Error %s (%d) encountered with rsslEncodeFilterEntry().  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			success = RSSL_FALSE;
 *			//Non Init/Complete calls will automatically roll back entry when failure occurs.  
 *			//Can choose to roll back entire vector if user wants *         
 *		}
 *	}
 *	//Done with entries, complete the filter list encoding
 *	retVal = rsslEncodeFilterListComplete(&encIter, success);
 *  @endcode
 *  @}
 */

/**
 *
 *	@defgroup FilterListDecoding RsslFilterList and RsslFilterEntry Decoding
 *  @brief Detailed information about RsslFilterList and RsslFilterEntry decoding
 *  @{
 *
 *  When decoding an RsslFilterList, each entry is returned as an \ref RsslFilterEntry containing
 *  the encoded payload content.  Further decoding of an entry's content can be achieved by using the appropriate 
 *  container type decoder. If the RsslFilterEntry::containerType is present, indicated by ::RSSL_FTEA_HAS_CONTAINER_TYPE, that container type decode 
 *  functionality should be used.  Otherwise, the RsslFilterList::containerType decode functionality should be used.  
 *  If desired, decoding of an entry's content can be skipped by 
 *  invoking the rsslDecodeFilterEntry() function to move to the next entry. 
 *  @section FilterListDecExample RsslFilterList Decoding Example
 *  @code
 *	RsslFilterList rsslFilterList; //No need to clear since we will decode into this
 *	RsslFilterEntry filterEntry; //RsslFilterEntry to expose individual entry contents through
 *	RsslRet retVal;
 *
 *	if ((retVal = rsslDecodeFilterList(&decIter, &rsslFilterList)) >= RSSL_RET_SUCCESS)
 *	{
 *		while ((retVal = rsslDecodeFilterEntry(&decIter, &filterEntry)) != RSSL_RET_END_OF_CONTAINER)
 *		{
 *			//handle failure cases
 *			if (retVal < RSSL_RET_SUCCESS)
 *			{
 *				//handle error as needed
 *				printf("Error %s (%d) encountered while decoding filter entries.  Error text: %s\n", 
 *						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			}
 *			else
 *			{
 *				RsslContainerType type;
 *				//Determine correct type to decode as
 *				if (filterEntry.flags & RSSL_FTEA_HAS_CONTAINER_TYPE)
 *					type = filterEntry.containerType;
 *				else
 *					type = rsslFilterList.containerType;
 *				//now decode container type
 *				switch(type)
 *				{
 *					//handle appropriate container types and do something with data
 *					...
 *				}
 *			}
 *		}
 *	}
 *	else
 *	{
 *		//initial filter list decode failed
 *		printf("Error %s (%d) encountered while decoding RsslFilterList.  Error text: %s\n",
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *					
 *  @endcode
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslFilterList */

 
/**
 *	@}
 */
 
/**
 *	@}
 */


/**
 *	@defgroup RSSLWFMessage Transport API Wire Format Message Package
 *	@brief The Transport API Message Package manages the binary representation of OMM messages, ranging from requests and responses to bi-directional generic messages.  
 *	@{
 */

/**
 *	@defgroup RSSLWFMessages Transport API Message Structures
 *	@brief The Transport API Message Package's message structures are required to encode or decode OMM messages.
 *	@{
 */


/**
 * @defgroup MsgBase Transport API Message Base
 * @brief The RsslMsgBase contains information that is shared across all Transport API Message structures.
 * @{
 *
 *  To use RsslMsgBase functionality include @header "rtr/rsslMsgBase.h"
 *
 *	See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *
 *  @section TheMsgBaseStruct RsslMsgBase Structure, Initializers and Helpers
 *  @defgroup MsgBaseStruct RsslMsgBase Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslMsgBase Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 */
 
/**
 *	@}
 */

/**
 * @defgroup MsgKey Transport API Message Key
 *	@brief The RsslMsgKey structure contains attributes that are used to identify the contents of a particular stream.  
 *	This information, as well as the domain type and Quality of Service information can be used to uniquely identify a stream.
 * @{
 *
 *	The RsslMsgKey is used in RsslRequestMsg to specify the item stream that the consumer wishes to open. 
 *	After the provider has processed the request, any subsequent RsslResponseMsg should contain the same Stream ID and RsslMsgKey information that the consumer specified. Updates typically will not contain an RsslMsgKey unless the consumer specifies it in the initial RsslRequestMsg.
 *
 *	To use RsslMsgKey functionality include @header "rtr/rsslMsgKey.h"
 *
 *  
 *
 *	See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section TheMsgKeyStruct RsslMsgKey Structure, Initializers and Helpers
 *  @defgroup ReqMsgStruct RsslMsgKey Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslRequestMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section TheMsgkeyHelpers RsslMsgKey Helper and Utility functions
 *  @defgroup MsgKeyHelpers RsslMsgKey Helper and Utility functions
 *  @brief Detailed information about the RsslRequestMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 *
 */


/**  
 *  @}
 * 
 */

 
/* Start of RsslRequestMsg */

/**
 *  @defgroup RsslRequestMsgGroup RsslRequestMsg Reference Group
 *	@brief The Request Message allows a user to open a stream for the indicated content, or to change requested attributes/properties on an already open stream.  
 *  @{
 *
 *	To use RsslRequestMsg functionality include @header "rtr/rsslRequestMsg.h"
 *
 *  The RsslRequestMsg allows a user to open a stream for the indicated content or to change requested attributes/properties on an already open stream.  
 *	The \ref RsslRequestMsg::msgBase::msgKey members, in conjunction with the \ref RsslRequestMsg::msgBase::domainType help to identify the stream,
 *	while priority information can be used to indicate the streams importance to the OMM Consumer.
 *	Quality of service information can be used to express either a specific desired quality
 *	of service or a range of acceptable qualities of service that can satisfy the request.
 *
 *  @note An RsslRequestMsg can be used to specify batch or view information on a stream or to pause/resume content flow on a stream. 
 *
 *
 *	See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section RequestMsgUseExample RsslRequestMsg Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslRequestMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslRequestMsg.  When decoding, the RsslRequestMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslRequestMsg and initialize
 *	RsslRequestMsg reqMsg;
 *
 *	// Use the clear function since it sets the msgClass and tend to perform better
 *	rsslClearRequestMsg(&requestMsg);
 *  
 *	// Populate RsslRequestMsg with relevant information
 *	// Set flags to indicate streaming request with concrete qos and priority
 *	reqMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY | RSSL_RQMF_HAS_QOS;
 *
 *	// Populate this with standard header information
 *	reqMsg.msgBase.streamId = 10;
 *	reqMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	reqMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
 *
 *	// Populate message key information
 *	reqMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
 *	reqMsg.msgBase.msgKey.serviceId = 1;
 *	reqMsg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
 *	reqMsg.msgBase.msgKey.name.data = "TRI";
 *	reqMsg.msgBase.msgKey.name.length = 3;
 *
 *	// Populate priority. 
 *	reqMsg.priorityClass = 5;
 *	reqMsg.priorityCount = 10;
 *
 *	// Populate quality of service
 *	reqMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
 *	reqMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
 *
 *	// If content is pre-encoded, can set on reqMsg.msgBase.encDataBody buffer and call rsslEncodeMsg().
 *	// If content is not pre-encoded, user can call rsslEncodeMsgInit(), encode content, and rsslEncodeMsgComplete().
 *
 *
 *	@endcode
 *
 *  @section TheReqMsgStruct RsslRequestMsg Structure, Initializers and Helpers
 *  @defgroup ReqMsgStruct RsslRequestMsg Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslRequestMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section TheReqMsgHelpers RsslRequestMsg Helper and Utility functions
 *  @defgroup ReqMsgHelpers RsslRequestMsg Helper and Utility functions
 *  @brief Detailed information about the RsslRequestMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslRequstMsg */


/* Start of RsslRefreshMsg */

/**
 *  @defgroup RsslRefreshMsgGroup RsslRefreshMsg Reference Group
 *	@brief The Refresh Message is used to provide an initial or subsequent data synchronization point, including quality of service and payload content.  When used to respond to an RsslRequestMsg, a solicited RsslRefreshMsg can be used.  If pushing out a content change to downstream applications, an unsolicited RsslRefreshMsg can be used.  When the stream is instantiated because of an RsslRequestMsg, the streamId from the request should be used; when a provider instantiates the stream with an RsslRefreshMsg (e.g. Non-Interactive provider), a negative streamId should be used.
 *
 *  @{
 * 
 *  To use RsslRefreshMsg functionality include @header "rtr/rsslRefreshMsg.h"
 *
 *	The Refresh Message is used to provide an initial or subsequent data synchronization point.  
 *	When used to respond to an RsslRequestMsg, a solicited RsslRefreshMsg can be used.  
 *	If pushing out a content change to downstream applications, an unsolicited RsslRefreshMsg can be used.  
 *	When the stream is instantiated because of an RsslRequestMsg, the streamId from the request should be used; 
 *	when a provider instantiates the stream with an RsslRefreshMsg (e.g. Non-Interactive provider), a negative streamId should be used.
 *	It provides an image of all existing data related to that item, and any additional information
 *	about such as the item's group and quality of service. In addition to providing information, this
 *	action can be used to establish a standard or private stream.
 *
 *	@note When responding to a batch request, each item in the batch receives its own RsslRefreshMsg.  
 *
 *
 *	See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *	See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *	See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *	<BR><BR>
 * 
 *  @section RefreshMsgUseExample RsslRefreshMsg Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslRefreshMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslRefreshMsg.  When decoding, the RsslRefreshMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslRefreshMsg and initialize
 *	RsslRefreshMsg refreshMsg;
 *  
 *	// Use the clear function since it sets the msgClass and tend to perform better
 *	rsslClearRefreshMsg(&refreshMsg);
 *
 *	// Populate RsslRefreshMsg with relevant information
 *	// Set flags to indicate complete, solicited refresh with qos and key.  
 *	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
 *
 *	// Populate this with standard header information
 *	// Since this is solicited, provider should use the streamId from the request being responded to
 *	refreshMsg.msgBase.streamId = 10; 
 *	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
 *
 *	// Populate message key information, typically matches info from request
 *	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
 *	refreshMsg.msgBase.msgKey.serviceId = 1;
 *	refreshMsg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
 *	refreshMsg.msgBase.msgKey.name.data = "TRI";
 *	refreshMsg.msgBase.msgKey.name.length = 3;
 *
 *	// Populate quality of service, indicates the QoS being provided for this stream
 *	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
 *	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
 *
 *	// Use default group Id in this example
 *	// Populate state
 *	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
 *	refreshMsg.state.dataState = RSSL_DATA_OK;
 *	refreshMsg.state.code = RSSL_DATA_NONE;
 *
 *	// If content is pre-encoded, can set on refreshMsg.msgBase.encDataBody buffer and call rsslEncodeMsg().
 *	// If content is not pre-encoded, user can call rsslEncodeMsgInit(), encode content, and rsslEncodeMsgComplete().
 *
 *
 *	@endcode
 *
 *  @section TheRefreshMsgStruct RsslRefreshMsg Structure, Initializers and Helpers
 *  @defgroup RefreshMsgStruct RsslRefreshMsg Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslRefreshMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section TheRefreshMsgHelpers RsslRefreshMsg Helper and Utility functions
 *  @defgroup RefreshMsgHelpers RsslRefreshMsg Helper and Utility functions
 *  @brief Detailed information about the RsslRefreshMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslRefreshMsg */


/* Start of RsslStatusMsg */

  
 
/**
 *  @defgroup RsslStatusMsgGroup RsslStatusMsg Reference Group
 *	@brief The Status Message is used to convey state information, permission change, or group Id change information on a stream. An interactive or non-interactive providing application can also close a stream with a status message. 
 *  @{
 *
 *  To use RsslStatusMsg functionality include @header "rtr/rsslStatusMsg.h"
 *
 *  The Status Message is used to convey or modify state information, a streams permission information, or a streams group Id information.  
 *	An interactive or non-interactive providing application can also close a stream with a status message. 
 *  Interactive and Non-Interactive Providers use the RsslStatusMsg to close streams to downstream devices, 
 *  both in conjunction with an initial request or at some point after the stream has been established. 
 *	An RsslStatusMsg can also be used to indicate successful establishment of a stream, 
 *	even though the message may not contain any data - this can be useful when establishing a stream 
 *	solely to exchange bi-directional RsslGenericMsg or to convey information about the success or failure of the 
 *	establishment of a private stream.
 *
 *  @note When responding to a batch request, an RsslStatusMsg is used to respond to the batch stream.  When sent with a OK/Closed state, this indicates the batch was received and is being processed; a Suspect/Closed stream indicates there was an issue with the batch request itself.
 *
 *  See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section StatusMsgUseExample RsslStatusMsg Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslStatusMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslStatusMsg.  When decoding, the RsslStatusMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslStatusMsg and initialize
 *	RsslStatusMsg statusMsg;
 *  
 *	// Use the clear function since it sets the msgClass and tend to perform better
 *	rsslClearStatusMsg(&statusMsg);
 *
 *	// Populate RsslStatusMsg with relevant information
 *	// Set flags to indicate message has state
 *	statusMsg.flags = RSSL_STMF_HAS_STATE;
 *
 *	// Populate this with standard header information
 *	statusMsg.msgBase.streamId = 10;
 *	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
 *
 *	// Populate state, indicate that item was already open on another stream
 *	statusMsg.state.streamState = RSSL_STREAM_OPEN;
 *	statusMsg.state.dataState = RSSL_DATA_SUSPECT;
 *	statusMsg.state.code = RSSL_DATA_ALREADY_OPEN;
 *	statusMsg.state.text.data = "Item already open on streamId 9";
 *	statusMsg.state.text.length = 31; 
 *
 *	// Because content is RSSL_DT_NO_DATA in this example, application can just call rsslEncodeMsg().
 *
 *
 *	@endcode
 *
 *  @section TheStatusMsgStruct RsslStatusMsg Structure, Initializers and Helpers
 *  @defgroup StatusMsgStruct RsslStatusMsg Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslStatusMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section TheStatusMsgHelpers RsslStatusMsg Helper and Utility functions
 *  @defgroup StatusMsgHelpers RsslStatusMsg Helper and Utility functions
 *  @brief Detailed information about the RsslStatusMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslStatusMsg */


/* Start of RsslUpdateMsg */


/**
 *  @defgroup RsslUpdateMsgGroup RsslUpdateMsg Reference Group
 *	@brief The Update Message is used to provide content change on an open stream.  
 *  @{
 *
 *  To use RsslUpdateMsg functionality include @header "rtr/rsslUpdateMsg.h"
 *
 *  The Update Message is used by Interactive and Non-Interactive Providers to 
 *	convey changes to data associated with an item stream. When streaming, update messages
 *	typically flow after an initial refresh has been delivered. Some providers can aggregate
 *	the information from multiple update messages into a single update message. This is known
 *	as conflation, and typically occurs if a conflated quality of service is requested, a
 *	stream is paused, or a consuming application is unable to keep up with the data rates
 *	associated with the stream.
 *  
 *	@note Typically updates are delivered when a stream is open as streaming (\ref RsslRequestMsg::RSSL_RQMF_STREAMING), however they can also be seen between the initial and final part of a multi-part RsslRefreshMsg.  Applications should be prepared to handle updates, even when requesting content as non-streaming.
 *
 *	See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section UpdateMsgUseExample RsslUpdateMsg Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslUpdateMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslUpdateMsg.  When decoding, the RsslUpdateMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslUpdateMsg and initialize
 *	RsslUpdateMsg updateMsg;
 *  
 *	// Use the clear function since it sets the msgClass and tend to perform better
 *	rsslClearUpdateMsg(&updateMsg);
 *
 *	// Populate RsslUpdateMsg with relevant information
 *	// Set flags to indicate update should not be conflated
 *	updateMsg.flags = RSSL_UPMF_DO_NOT_CONFLATE;
 *
 *	// Populate this with standard header information
 *	updateMsg.msgBase.streamId = 10;
 *	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
 *
 *	// Populate the update type - values for update types are typically provided at a domain model level, see rsslRDM.h.
 *	updateMsg.updateType = RDM_UPD_EVENT_TYPE_TRADE;
 *
 *	// If content is pre-encoded, can set on refreshMsg.msgBase.encDataBody buffer and call rsslEncodeMsg().
 *	// If content is not pre-encoded, user can call rsslEncodeMsgInit(), encode content, and rsslEncodeMsgComplete().
 *
 *
 *	@endcode
 *
 *  @section TheUpdateMsgStruct RsslUpdateMsg Structure, Initializers and Helpers
 *  @defgroup UpdateMsgStruct RsslUpdateMsg Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslUpdateMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section TheUpdateMsgHelpers RsslUpdateMsg Helper and Utility functions
 *  @defgroup UpdateMsgHelpers RsslUpdateMsg Helper and Utility functions
 *  @brief Detailed information about the RsslUpdateMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslUpdateMsg */


/* Start of RsslCloseMsg */


/**
 *  @defgroup RsslCloseMsgGroup RsslCloseMsg Reference Group
 *	@brief The Close Message is used to indicate that a consuming application is no longer interested in content on the stream being closed.
 *  @{
 *
 *  To use RsslCloseMsg functionality include @header "rtr/rsslCloseMsg.h"
 *
 *  The Close Message is used to indicate that an OMM Consumer application is no longer interested in content on the stream.
 *  The stream should be closed as a result and no further messages should be received once the close operation is fully complete throughout the system.
 *  
 *	@note If the application wishes to get an acknowledgment when the upstream component has closed the stream (e.g. a 'completion event'), the close allows the user to ask for this by specifying RsslCloseMsg::flags with \ref RsslCloseFlags::RSSL_CLMF_ACK.
 *
 *  See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section CloseMsgUseExample RsslCloseMsg Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslCloseMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslCloseMsg.  When decoding, the RsslCloseMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslCloseMsg and initialize
 *	RsslCloseMsg closeMsg;
 *  
 *	// Use the clear function since it sets the msgClass and tend to perform better
 *	rsslClearCloseMsg(&closeMsg);
 *
 *	// Populate RsslCloseMsg with relevant information
 *	// Set flags to indicate application would like ack when closed
 *	closeMsg.flags = RSSL_CLMF_ACK;
 *
 *	// Populate this with standard header information
 *	closeMsg.msgBase.streamId = 10;
 *	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;
 *
 *
 *	// Because content is RSSL_DT_NO_DATA in this example, application can just call rsslEncodeMsg(). 
 *
 *	@endcode
 *
 *  @section TheCloseMsgStruct RsslCloseMsg Structure, Initializers and Helpers
 *  @defgroup CloseMsgStruct RsslCloseMsg Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslCloseMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section TheCloseMsgHelpers RsslCloseMsg Helper and Utility functions
 *  @defgroup CloseMsgHelpers RsslCloseMsg Helper and Utility functions
 *  @brief Detailed information about the RsslCloseMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslCloseMsg */


/* Start of RsslPostMsg */


/**
 *  @defgroup RsslPostMsgGroup RsslPostMsg Reference Group
 *	@brief The Post Message is used to contribute content from a consumer into the platform.  The post message payload can contain another OMM message, OMM container, or other opaque content.  
 *  @{
 *
 *  To use RsslPostMsg functionality include @header "rtr/rsslPostMsg.h"
 *
 *	The Post Message allows OMM Consumer applications to push and contribute content to upstream components.
 *	The post message payload can contain another OMM message, OMM container, or other opaque content.  
 *  This information can be applied to an Enterprise Platform
 *	cache or routed further upstream to the source of data. Once received, the upstream
 *	components can republish data to downstream consumers. Post messages can be routed
 *	along a specific item stream, referred to as on-stream posting, or along a user’s
 *	Login stream, referred to as off-stream posting. An RsslPostMsg can contain any Transport API
 *	container type, including other messages. User identification information can be
 *	associated with a post message and can be provided along with the content that was posted.
 *  
 *	@note If the application wishes to get an acknowledgment for its posted content, it can specify the RsslPostMsg::flags with \ref RsslPostFlags::RSSL_PSMF_ACK.
 *
 *  See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 *
 *	@section ThePostUserInfo RsslPostUserInfo structure
 *	The RsslPostUserInfo is used to identify the user that has posted a message. This is used in \ref RsslUpdateMsg, \ref RsslPostMsg, \ref RsslStatusMsg, and \ref RsslRefreshMsg.
 * 
 *  @section PostMsgUseExample RsslPostMsg Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslPostMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslPostMsg.  When decoding, the RsslPostMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslPostMsg and initialize
 *	RsslPostMsg postMsg;
 *  
 *	// Use the clear function since it sets the msgClass and tend to perform better
 *	rsslClearPostMsg(&postMsg);
 *
 *	// Populate RsslPostMsg with relevant information
 *	// Set flags to indicate application would like ack when posted content succeeds
 *	// Indicate that it is complete and has a post Id
 *	postMsg.flags = RSSL_PSMF_ACK | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_POST_COMPLETE;
 *
 *	// Populate this with standard header information
 *	postMsg.msgBase.streamId = 10;
 *	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	postMsg.msgBase.containerType = RSSL_DT_MSG;  // house a message in the post message
 *
 *	// Populate other post message members
 *	postMsg.postId = 520;
 *	// Populate post user info, used to identify user that posted content.  
 *	// Should include IP address of machine running app and user id or pid
 *	postMsg.postUserInfo.postUserId = getpid(); 
 *	rsslIPAddrStringToUInt("192.0.1.10", &postMsg.postUserInfo.postUserAddr);
 *
 *
 *	// If we are doing on-stream posting, we do not need the key; when absent it is inferred from the stream being used to post.
 *	// If off-stream posting, RsslMsgKey is required to identify the item.
 *
 *	// If content is pre-encoded, can set on refreshMsg.msgBase.encDataBody buffer and call rsslEncodeMsg().
 *	// If content is not pre-encoded, user can call rsslEncodeMsgInit(), encode content, and rsslEncodeMsgComplete().
 *
 *	@endcode
 *
 *
 *  @section ThePostMsgStruct RsslPostMsg Structure, Initializers and Helpers
 *  @defgroup PostMsgStruct RsslPostMsg Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslPostMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section ThePostMsgHelpers RsslPostMsg Helper and Utility functions
 *  @defgroup PostMsgHelpers RsslPostMsg Helper and Utility functions
 *  @brief Detailed information about the RsslPostMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 *	@defgroup PostUserInfoStruct RsslPostUserInfo Structure and Initializers
 *  @brief Detailed information about the RsslPostUserInfo Structure, Initializers, and flag values
 *	@{
 *
 *	@}
 *
 *  @defgroup PostUserInfoHelpers RsslPostUserInfo Helper and Utility functions
 *  @brief Detailed information about the RsslPostUserInfo Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslPostMsg */

/* Start of RsslAckMsg */

/**
 *  @defgroup RsslAckMsgGroup RsslAckMsg Reference Group
 *	@brief The Ack Message is used to acknowledge success/failure or completion of an RsslPostMsg or RsslCloseMsg.  
 *  @{
 *
 *  To use RsslAckMsg functionality include @header "rtr/rsslAckMsg.h"
 *
 *  The Ack Message is used to acknowledge success/failure or completion of an RsslPostMsg or RsslCloseMsg. 
 *	Sent from OMM Providers to OMM Consumers, the acknowledgment carries success or failure
 *  (negative acknowledgment or nak) information to the consumer. The consumer must
 *  request acknowledgment for an RsslPostMsg or an RsslCloseMsg by specifying the appropriate flag.
 *
 *  See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section AckMsgUseExample RsslAckMsg Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslAckMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslAckMsg.  When decoding, the RsslAckMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslAckMsg and initialize
 *	RsslAckMsg ackMsg;
 *  
 *	// Use the clear function since it sets the msgClass and tend to perform better
 *	rsslClearAckMsg(&ackMsg);
 *
 *	// Populate RsslAckMsg with relevant information
 *	// Set flags to indicate that this is a NAK and there is text
 *	ackMsg.flags = RSSL_AKMF_HAS_NAK_CODE | RSSL_AKMF_HAS_TEXT;
 *
 *	// Populate this with standard header information
 *	ackMsg.msgBase.streamId = 10;
 *	ackMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
 *
 *	// Populate ack message members
 *	// When acknowledging a post message, the RsslAckMsg::ackId should match the RsslPostMsg::postId
 *	ackMsg.ackId = 520; 
 *	ackMsg.nakCode = RSSL_NAKC_GATEWAY_DOWN;
 *	ackMsg.text.data = "Cannot apply post, gateway is down.";
 *	ackMsg.text.length = 35;
 *
 *	// Because content is RSSL_DT_NO_DATA in this example, application can just call rsslEncodeMsg(). 
 *
 *	@endcode
 *
 *  @section TheAckMsgStruct RsslAckMsg Structure, Initializers and Helpers
 *  @defgroup AckMsgStruct RsslAckMsg Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslAckMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section TheAckMsgHelpers RsslAckMsg Helper and Utility functions
 *  @defgroup AckMsgHelpers RsslAckMsg Helper and Utility functions
 *  @brief Detailed information about the RsslAckMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslAckMsg */


/* Start of RsslGenericMsg */


/**
 *  @defgroup RsslGenericMsgGroup RsslGenericMsg Reference Group
 *	@brief The Generic Message allows applications to bidirectionally message without any implied interaction semantics (e.g. no Request/Response paradigm).  
 *  @{
 *
 *  To use RsslGenericMsg functionality include @header "rtr/rsslGenericMsg.h"
 *
 *	The Generic Message allows applications to bidirectionally message without any implied interaction 
 *  semantics (e.g. no Request/Response paradigm). Once a
 *	stream is established via an RsslRequestMsg-RsslRefreshMsg/RsslStatusMsg interaction, this message can
 *	be sent from consumer to provider as well as from provider to consumer, and can
 *	also be leveraged by non-interactive provider applications. Generic messages are
 *	transient and are typically not cached by any Enterprise Platform components.
 *	The \ref RsslGenericMsg::msgBase::msgKey does not need to match the msgKey information
 *	associated with the stream the RsslGenericMsg is flowing on. This allows for
 *	the key information to be used independently of the stream. Any specific message
 *	usage, msgKey usage, expected interactions, and handling instructions are typically
 *	defined by a domain message model specification.
 *
 *  See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section GenericMsgUseExample RsslGenericMsg Creation Examples
 *  The following code example demonstrates stack creation and initialization of an RsslGenericMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslGenericMsg.  When decoding, the RsslGenericMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslGenericMsg and initialize
 *	RsslGenericMsg genericMsg;
 *  
 *	// Use the clear function since it sets the msgClass and tend to perform better
 *	rsslClearGenericMsg(&genericMsg);
 *
 *	// Populate RsslGenericMsg with relevant information
 *	// Indicate that it is complete and has a sequence number
 *	genericMsg.flags = RSSL_GNMF_HAS_SEQ_NUM | RSSL_GNMF_MESSAGE_COMPLETE;
 *
 *	// Populate this with standard header information
 *	genericMsg.msgBase.streamId = 10;
 *	genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	genericMsg.msgBase.containerType = RSSL_DT_OPAQUE; 
 *
 *	// If content is pre-encoded, can set on refreshMsg.msgBase.encDataBody buffer and call rsslEncodeMsg().
 *	// If content is not pre-encoded, user can call rsslEncodeMsgInit(), encode content, and rsslEncodeMsgComplete().
 *
 *	@endcode
 *
 *  @section TheGenericMsgStruct RsslGenericMsg Structure, Initializers and Helpers
 *  @defgroup GenericMsgStruct RsslGenericMsg Structure, Initializers, and Flag values
 *  @brief Detailed information about the RsslGenericMsg Structure, Initializers, and flag values
 *  @{
 *
 *  @}
 *
 *
 *  @section TheGenericMsgHelpers RsslGenericMsg Helper and Utility functions
 *  @defgroup GenericMsgHelpers RsslGenericMsg Helper and Utility functions
 *  @brief Detailed information about the RsslGenericMsg Helper and Utility functions
 *  @{
 * 
 *  @}
 *
 */


/**  
 *  @}
 * 
 */

/* End of RsslGenericMsg */



/**
 * @defgroup RsslMsgUnion RsslMsg Union Reference Group
 * @brief OMM Message Union contains all messages.  This includes constructs, functions, and values.
 * @{
 *	@}
 */


 /**
 *	@}
 */
/* End of messages */

/* Start of Message Encoding */


/**
 *  @defgroup MsgEncoders Transport API Message Encoding Reference Group
 *	@brief Transport API Message Encoding functionality, used for all aspects of message header encoding.  
 *  @{
 *
 *  To use RsslMsg encoding functionality include @header "rtr/rsslMsgEncoders.h"
 *
 *  See \ref MsgDecoders for general Transport API Message Decoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section FullMsgEncExample rsslEncodeMsg() Usage Example
 *  The following code example demonstrates stack creation and initialization of an RsslGenericMsg.  This also demonstrates the use of the clear function to reuse the structures.
 *  This is typical usage prior to encoding an RsslGenericMsg.  When decoding, the RsslGenericMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslMsg and clear
 *	RsslMsg encMsg;
 *  
 *	rsslClearMsg(&encMsg);
 *	// Populate the message members.  
 *	encMsg.msgBase.msgClass = RSSL_MC_UPDATE;
 *	encMsg.msgBase.streamId = 10;
 *	encMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
 *	encMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	
 *	encMsg.updateMsg.flags = RSSL_UPMF_DO_NOT_CONFLATE;
 *	encMsg.updateMsg.updateType = RDM_UPD_EVENT_TYPE_TRADE;
 *
 *	//Set pre-encoded payload into message prior to encoding. 
 *	//This example assumes the encPayloadBuffer contains a properly encoded field list.
 *	encMsg.msgBase.encDataBody.data = encPayloadBuffer.data;
 *	encMsg.msgBase.encDataBody.length = encPayloadBuffer.length;
 *
 *	//Encode message
 *	if ((retVal = rsslEncodeMsg(&encIter, &encMsg)) < RSSL_RET_SUCCESS)
 *	{
 *		// an error occurred, if we arent in here, our message is successfully encoded
 *		printf("Error %s (%d) encountered with rsslEncodeMsg().  Error Text: %s\n", 
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *
 *	@endcode
 *
 *
 *  @section InitComplMsgExample rsslEncodeMsgInit()...rsslEncodeMsgComplete() Usage Example
 *  The following code example demonstrates message encoding using the rsslEncodeMsgInit()/rsslEncodeMsgComplete() functions.
 *  
 *	@code
 *	//Create RsslRequestMsg 
 *	RsslRequestMsg reqMsg; 
 *  
 *	//Clear message and populate with information
 *	rsslClearRequestMsg(&reqMsg);
 *	reqMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY | RSSL_RQMF_HAS_QOS | RSSL_RQMF_CONF_INFO_IN_UPDATES;
 *
 *	//Populate base message information, clear function already set msgClass 
 *	reqMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
 *	reqMsg.msgBase.containerType = RSSL_DT_NO_DATA;
 *	reqMsg.msgBase.streamId = 10;
 *	
 *	//Set Priority QoS information
 *	reqMsg.priorityClass = 5;
 *	reqMsg.priorityCount = 2;
 *	reqMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
 *	reqMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
 *
 *	// Message key is required, populate it 
 *	reqMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_ATTRIB;
 *	reqMsg.msgBase.msgKey.serviceId = 1;
 *	reqMsg.msgBase.msgKey.name.data = "TRI";
 *	reqMsg.msgBase.msgKey.name.length = 3;
 *	reqMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;
 *	//Attrib will be encoded after call to rsslEncodeMsgInit()
 *
 *	//Begin encoding message, assumes iterator is associated with large enough buffer and version information
 *  
 *	if ((retVal = rsslEncodeMsgInit(&encIter, (RsslMsg*)&reqMsg, 0)) < RSSL_RET_SUCCESS)
 *	{
 *		// Since we said we would encode message key attributes, but did not provide them
 *		// we should be told to encode them at this point
 *		if (retVal == RSSL_RET_ENCODE_MSG_KEY_OPAQUE)
 *		{
 *			//Encode attributes using same iterator and  specified attribContainerType
 *			//Call element list encoder, since we said type was element list
 *			....
 *					
 *			// When element list encoding completes successfully, finish attrib encoding
 *			retVal = rsslEncodeMsgKeyAttribComplete(&encIter, RSSL_TRUE);
 *		}
 *		
 *		// If we werent encoding attributes or we were, but called rsslEncodeMsgKeyAttribComplete
 *		// we should see the retVal telling us things are successful.  Since there is RSSL_DT_NO_DATA
 *		// as payload, there is nothing left to do but complete the message.
 *		if (retVal == RSSL_RET_SUCCESS)
 *		{
 *			rsslEncodeMsgComplete(&encIter, RSSL_TRUE);
 *		}
 *		else
 *		{
 *			printf("Error %s (%d) encountered with encoding process.  Error Text: %s\n", 
 *					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *		}
 *	}
 *	else
 *	{
 *		printf("Error %s (%d) encountered with encoding process().  Error Text: %s\n", 
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *
 *	@endcode
 *
 * @defgroup MsgEncodeUtilsHelpers Message Encode Utility Helper Functions
 * @brief These functions are general helpers to assist with the encoding of RsslMsg structures
 * @{
 *
 * @}
 *
 */

/**  
 *  @}
 * 
 */

/* End of Message Encoding */


/* Start of Message Decoding */


/**
 *  @defgroup MsgDecoders Transport API Message Decoding Reference Group
 *	@brief Transport API Message Decoding functionality, used for all aspects of message header decoding.  
 *  @{
 *
 *  To use RsslMsg decoding functionality include @header "rtr/rsslMsgDecoders.h"
 *
 *  See \ref MsgEncoders for general Transport API Message Encoding functionality. <BR>
 *  See \ref MsgUtils for general Transport API Message utility and helper functions.  <BR>
 *  <BR><BR>
 * 
 *  @section MsgDecExample rsslDecodeMsg() Usage Example
 *  The following code example demonstrates a decoding scenario using rsslDecodeMsg() and rsslDecodeMsgKeyAttrib()
 *  When decoding, the RsslMsg will be populated via the decode process so there is no need to previously clear.
 *	@code
 *	// Create RsslMsg
 *	RsslMsg decMsg;
 *	// This example assumes the decode iterator was created, cleared, and populated with an RsslBuffer containing
 *	// the encoded content (likely obtained from rsslRead()).
 *  
 *	if ((retVal = rsslDecodeMsg(&decIter, &decMsg)) >= RSSL_RET_SUCCESS)
 *	{
 *		//The message header information is now decoded.
 *		//Check if any message key attributes are present, first get the message key.
 *		const RsslMsgKey* msgKey = rsslGetMessageKey(&decMsg);
 *		//Check if the key contains attributes
 *		if (msgKey->flags & RSSL_MKF_HAS_ATTRIB)
 *		{
 *			//Use the same iterator to decode the message key attrib
 *			if ((retVal = rsslDecodeMsgKeyAttrib(&decIter, msgKey)) >= RSSL_RET_SUCCESS)
 *			{
 *				//Decode attributes using the proper decode function  
 *				switch(msgKey->attribContainerType)
 *				{
 *					//handle appropriate container types and do something with data
 *					...
 *				}
 *			}
 *			else
 *			{
 *				//handle error as needed
 *				printf("Error %s (%d) encountered while calling rsslDecodeMsgKeyAttrib().  Error text: %s\n",
 *							rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *			}
 *		}
 *		
 *		//Now decode any payload information.  
 *		switch(decMsg.msgBase.containerType)
 *		{
 *			case RSSL_DT_NO_DATA:
 *				//Do nothing for no data
 *			break;
 *			// Handle other container types here and do something with content
 *
 *		}
 *	}
 *	else
 *	{
 *		//initial RsslMsg decode failed
 *		printf("Error %s (%d) encountered while decoding RsslMsg.  Error text: %s\n",
 *				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
 *	}
 *	@endcode
 *
 *
 * @defgroup MsgDecodeUtilsHelpers Message Decode Utility Helper Functions
 * @brief These functions are general helpers to assist with the decoding of RsslMsg structures
 * @{
 *
 * @}
 *
 */

/**  
 *  @}
 * 
 */

/* End of Message Decoding */

 /**
 * @defgroup MsgUtils General Message Utility and Helper Functions
 * @brief These functions are intended to assist users with using the Transport API Message package structures. 
 * @{
 *
 *
 * @defgroup MsgPkgVersion Message Package Library Version
 * @brief This allows the user to programmatically extract version information about the library.
 * @{
 * 
 * @}
 *
 * @defgroup MsgUtilsHelpers Message Utility Helper Functions
 * @brief These functions are general helpers to assist with usage of RsslMsg structures
 * @{
 *
 * @}
 *
 * @defgroup MsgCopyUtils Copy Message Utility Helper Functions
 * @brief These functions provide deep copy and size of functionality for RsslMsg structures.
 * @{
 *
 * @section CopyMsgExample RsslCopyMsg Usage Example
 * The following code example demonstrates the usage of \ref rsslCopyMsg() with a user-allocated buffer. 
 * In this case, the user is expected to pre-allocate the memory that \ref rsslCopyMsg will copy to.
 * @code
 * // This RsslMsg should already have been populated by the user.
 * RsslMsg SourceMsg;
 * // RsslMsg pointer for final copy location
 * RsslMsg *CopyMsg;
 * // User allocated buffer
 * RsslBuffer copyBuf;
 * // Doing a full, deep copy of the message. Message does not have a filter list.
 * RsslUInt32 copyFlags = RSSL_CMF_ALL_FLAGS;
 * RsslUInt32 filter = 0;
 *
 * // Find size of the source message and allocate memory.
 * copyBuf.length = rsslSizeOfMsg(&SourceMsg, copyFlags);
 * copyBuf.data = (char*)malloc(copyBuf.length);
 *
 * // Copy message into copyBuf.
 * CopyMsg = rsslCopyMsg(&SourceMsg, copyFlags, filter, &copyBuf);
 * 
 * // ... //
 * 
 * // When done with the Copied Message, free the data buffer.
 * free(copyBuf.data);
 * @endcode
 * <BR><BR>
 * This code example demonstrates the usage of \ref rsslCopyMsg() without a user-allocated buffer. 
 * Here, the user is expected to use \ref rsslReleaseCopiedMsg() to free the allocated memory.
 * @code
 * // This RsslMsg should already have been populated by the user.
 * RsslMsg SourceMsg;
 * // RsslMsg pointer for final copy location
 * RsslMsg *CopyMsg;
 * // Doing a full, deep copy of the message. Message does not have a filter list.
 * RsslUInt32 copyFlags = RSSL_CMF_ALL_FLAGS;
 * RsslUInt32 filter = 0;
 *
 * // Copy message into copyBuf.
 * CopyMsg = rsslCopyMsg(&SourceMsg, copyFlags, filter, NULL);
 * 
 * // ... //
 * 
 * // When done with the Copied Message, free the data buffer.
 * rsslReleaseCopiedMsg(CopyMsg);
 * @endcode
 * <BR><BR>
 * 
 * @}
 *
 */
 
/**
 * @}
 */
 

/**
 * @}
 */
/* End of Message Package */
 

/**
 *	@defgroup RSSLWFDomain Reuters Domain Models
 *	@brief The Reuters Domain Model Package contains the enumerations and data structures used with Refinitiv provided domain message models. 
 *	This content is available through the Refinitiv Real-Time Distribution System, EleKtron, Reuters Data Feed Direct, and various other sources. 
 *	@{
 *	
 *  @section TheRSSLWFDomainHelpers Reuters Domain Model Helper and Utility functions
 *  @defgroup RSSLWFDomainHelpers Reuters Domain Model Helper and Utility functions
 *  @brief Detailed information about the Reuters Domain Model Helper and Utility functions
 *  @{
 * 
 *  @}
 
/**
 *	@}
 */

/**
 *	@}
 */
 

/* Start of Util Group */ 

/**
 *	@defgroup RSSLUtils Transport API Utility Functions
 *	@brief These functions are additional utility functions included in the Transport API
 *	@{
 *
 *	@defgroup RSSLGetTime Transport API GetTime Utility Functions
 *	@brief These functions are platform independent get time functions.  These functions provide Nano-second, Mirco-Second and Milli-Second access to timers. 
 *  These timers are based on a high-resolution non-decreasing time, not the system clock, and are not affected by changes to the clock.
 *	@{
 *	@}
 *
 *	@}
 */
 
/* End of Util Group */



