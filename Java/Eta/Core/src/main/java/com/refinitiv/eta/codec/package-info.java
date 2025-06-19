/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/**
  Provides classes and interfaces for encoding and decoding various messages and other data content.
  Besides iterators, it contains various data types that can be combined in a variety of ways to assist with 
  modeling user's data. It also provides interfaces for messages that communicate data between system components 
  to exchange information indicate status, permission users and access, and for a variety of other purposes.
  These three groups of classes and interfaces are described in the following sections:
  <p>
  
  <b>ETA Data Types</b>
  <p>
  These types manages binary representation of ETA data payload, ranging from simple primitive types
  through comprehensive hierarchical container types. These types are split across two categories:
  <ul>
  <li>
  <p>
  A Primitive Type represents simple, atomically updating information. 
  Primitive types represent values like integers, dates, and ASCII string buffers. 
  <p>
  A primitive type represents some type of base, system information (such as integers, dates, or array values).
  If contained in a set of updating information, primitive types update atomically (incoming data replaces any
  previously held values). Primitive types support ranges from simple primitive types (e.g., an integer) to
  more complex primitive types (e.g., an array).
  
  The {@link com.refinitiv.eta.codec.DataTypes} enumeration includes values that define the type of a 
  primitive:
  <ul>
  <li>
      <p>
      Values between {@value com.refinitiv.eta.codec.DataTypes#UNKNOWN} 
      and {@value com.refinitiv.eta.codec.DataTypes#BASE_PRIMITIVE_MAX} 
      are base primitive types.Base primitive types support the full range of values
       allowed by the primitive type.
       
<dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
<dt><b>Blank Data:</b></dt>
<dd>
    When contained in a {@link com.refinitiv.eta.codec.FieldEntry} or 
    {@link com.refinitiv.eta.codec.ElementEntry}, base primitive types can also represent 
    a blank value. A blank value indicates that no value is currently present and any previously stored or 
    displayed primitive value should 
    be cleared. When decoding any base primitive value, the <code>decode</code> interface method
    returns {@link com.refinitiv.eta.codec.CodecReturnCodes#BLANK_DATA}. To encode blank data into a 
    {@link com.refinitiv.eta.codec.FieldEntry} or {@link com.refinitiv.eta.codec.ElementEntry}, 
    use <code>encodeBlank()</code> method.
</dd>
</dl>
  </li>
  <li>
    Values between {@value com.refinitiv.eta.codec.DataTypes#SET_PRIMITIVE_MIN} 
    and {@value com.refinitiv.eta.codec.DataTypes#SET_PRIMITIVE_MAX} are set-defined primitive types, 
    which define fixed-length encodings for many of the base primitive types
    (e.g. INT_1 is a one byte fixed-length encoding of INT). These types can be leveraged only
    within a Set Definition and encoded or decoded as part of a {@link com.refinitiv.eta.codec.FieldList} 
    or {@link com.refinitiv.eta.codec.ElementList}.
    Only certain set-defined primitive types can represent blank values.
  </li>
  </ul>
  <li>
  A Container Type models more intricate data representations than ETA Primitive Types and 
  can manage dynamic content at a more granular level. Container types represent complex types 
  like field identifier-value, name-value, or key-value pairs. 
  ETA offers several uniform (i.e., homogeneous) container types whose entries house the
  same type of data. Additionally, there are several non-uniform (i.e., heterogeneous)
  container types in which different entries can hold different types of data.
  </li>
</ul>
<p>
<b>ETA Encode and Decode Iterators</b>
<p>
The following sections describe high level usage of the ETA Encode and Decode Iterators. 
Specific iterator structure and method definitions can be found in ETA Iterator Reference Group.
<ul>
<li>
<b>ETA Iterator Overview</b>
When encoding or decoding RWF content with ETA, the user leverages an iterator to manage the encoding or
decoding process. ETA defines a single encode iterator type ({@link com.refinitiv.eta.codec.EncodeIterator})
and a single decode iterator type ({@link com.refinitiv.eta.codec.DecodeIterator}). 
A single instance of this iterator can manage the full depth and breadth of the encoding or decoding process. 
Alternately, multiple iterator instances can be used to individually manage separate portions of the encode
or decode process. 
See {@link com.refinitiv.eta.codec.DecodeIterator} for more information and code examples.

The ETA encoder/decoder does not provide any inherent threading or locking capability. 
Separate iterator and type instances do not cause contention and do not share resources between instances.
Any needed threading, locking, or thread-model implementation is at the discretion of the application. 
Different application threads can encode or decode different messages without requiring a lock; 
thus each thread must use its own iterator instance and each message should be encoded or decoded using 
unique and independent buffers.
<p>
See {@link com.refinitiv.eta.codec.DecodeIterator} for more information and code examples for using decode iterator.<br>
See {@link com.refinitiv.eta.codec.EncodeIterator} for more information and code examples for using encode iterator.

<dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
<dt><b>Note:</b></dt>
<dd>
Though possible, LSEG recommends that you do not encode or decode related messages 
(messages flowing on the same stream) on different threads as this can impact the delivery order.
</dd>
</dl>
</li>
<li>
<b>Iterator RWF Protocol Versioning</b>
<p>
The ETA iterators help the user to manage version information associated with the RWF content being exchanged.
When using the ETA Transport the protocol type and version information can be exchanged and negotiated on the
connection (via the {@link com.refinitiv.eta.transport.ConnectOptions} or 
{@link com.refinitiv.eta.transport.BindOptions}).
The ETA Transport will reject any connection establishment when the protocol type does not match across the
connection. If the protocol type does match, an appropriate major and minor version will be determined and 
this should be the version of RWF encoded or decoded when using this connection. After the connection becomes
active, this negotiated version information is available and can then be provided to the iterator to ensure 
that the proper version is encoded or decoded. If not using the ETA Transport, the user can determine the
desired version of RWF to encode and specify this information on their iterator. ETA provides RWF protocol 
type and protocol version values in  {@link com.refinitiv.eta.codec.Codec} for this purpose.

<dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
<dt><b>Note:</b></dt>
<dd>
Specifying appropriate version information on {@link com.refinitiv.eta.codec.DecodeIterator}
and {@link com.refinitiv.eta.codec.EncodeIterator} is important to future-proof applications 
and avoid incompatibility if new RWF functionality is added.
</dd>
</dl>
</li>
</ul>
<p>
<b>ETA Messages</b>
<p>
The interfaces for ETA Message group manages the binary representation of ETA message headers, ETA messages and messaging constructs.
There are constructs to allow for communication stream identification and to determine uniqueness of streams within a connection
<p>
See {@link com.refinitiv.eta.codec.Msg} for more information and code examples.
 */
package com.refinitiv.eta.codec;
