/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{

	/// <summary>
	/// <para>
	/// Communicates data between system components: to exchange information,
	/// indicate status, permission users and access, and for a variety of other
	/// purposes.</para>
	///
	/// <para>
	/// Each ETA message, while having their own unique members, shares a
	/// common set of members. This common set of members form the base portion of
	/// each message structure.</para>
	/// </summary>
	/// <seealso cref="MsgClasses"/>
	public interface IMsg : IXMLDecoder
	{
		/// <summary>
		/// Clears the current contents of the message and prepares it for re-use.
		/// Useful for object reuse during encoding. While decoding, <see cref="IMsg"/>
		/// object can be reused without using <see cref="Clear()"/>.
		/// </summary>
		/// <remarks>
		/// (Messages may be pooled in a single collection via their common
		/// <see cref="IMsg"/> interface and re-used as a different <see cref="MsgClasses"/>).
        /// </remarks>
		void Clear();

        /// <summary>
        /// Encode a ETA Message. Encodes the key into buffer, all the data is
        /// passed in.
        /// </summary>
        /// <remarks>
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Set Msg structure members.</item>
        /// <item> Encode key(s) in separate buffer(s) and set Msg key members appropriately.</item>
        /// <item> Encode message body in separate buffer and set Msg encodedDataBody member appropriately.</item>
        /// <item> Call <see cref="Encode(EncodeIterator)"/>.</item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> Encoding iterator
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="IMsgKey"/>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode Encode(EncodeIterator iter);

        /// <summary>
        /// Initiate encoding of a ETA Message. Initiates encoding of a message.
        /// </summary>
        /// <remarks>
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="EncodeInit(EncodeIterator, int)"/>.</item>
        /// <item> Encode the key contents.</item>
        /// <item> Call <see cref="EncodeKeyAttribComplete(EncodeIterator, bool)"/>.</item>
        /// <item> Encode the message data body as needed.</item>
        /// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/>.</item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="dataMaxSize"> Max encoding size of the data, set to 0 if unknown
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="IMsgKey"/>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode EncodeInit(EncodeIterator iter, int dataMaxSize);

        /// <summary>
        /// Complete encoding of a ETA Message. Complete encoding of a message.
        /// </summary>
        /// <remarks>
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="EncodeInit(EncodeIterator, int)"/>.</item>
        /// <item> Encode the key contents.</item>
        /// <item> Call <see cref="EncodeKeyAttribComplete(EncodeIterator, bool)"/>.</item>
        /// <item> Encode the message data body as needed.</item>
        /// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/>.</item>
        /// </list>
        /// <para>
        /// Note, step 2 and 3 are optional, instead the application can set the
        /// respective key members of the <see cref="IMsg"/> structure using pre-encoded buffers.
        /// </para>
        /// </remarks>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> If <c>true</c> - successfully complete the key,
        ///                if <c>false</c> - remove the key from the buffer.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="IMsgKey"/>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode EncodeComplete(EncodeIterator iter, bool success);

        /// <summary>
        /// Complete Encoding of msgKey.Opaque data.
        /// </summary>
        /// <remarks>
        ///
        /// <para>
        /// Typically used when user calls Msg.encodeInit using a msgKey indicating that
        /// the msgKey.encodedAttrib should be encoded and the encodedAttrib.length and
        /// encodedAttrib.data are not populated with pre-encoded
        /// msgKey.encodedAttrib data.</para>
        ///
        /// <para>
        /// Msg.encodeInit will return <see cref="CodecReturnCode.ENCODE_MSG_KEY_ATTRIB"/>, the user will
        /// invoke the container encoders for their msgKey.encodedAttrib, and after it is complete call
        /// encodeKeyAttribComplete.</para>
        /// </remarks>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> If <c>true</c>, msgKey.encodedAttrib was encoded successfully so finish.
        ///                If not, rollback to last successful part.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="IMsgKey"/>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode EncodeKeyAttribComplete(EncodeIterator iter, bool success);

        /// <summary>
        /// Complete encoding of an Extended Header. Encodes the extended header, the data is passed in.
        /// </summary>
        /// <remarks>
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="EncodeInit(EncodeIterator, int)"/>.</item>
        /// <item> Call extended header encoding methods.</item>
        /// <item> Call <see cref="EncodeExtendedHeaderComplete(EncodeIterator, bool)"/>.</item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> If <c>true</c>, finish encoding - else rollback
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode EncodeExtendedHeaderComplete(EncodeIterator iter, bool success);

        /// <summary>
        /// Decode a message.
        /// </summary>
        /// <param name="iter"> Decode iterator
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        CodecReturnCode Decode(DecodeIterator iter);

        /// <summary>
        /// Set iterator to decode a message's key attribute buffer
        /// </summary>
        /// <remarks>
        ///
        /// After calling <c>Decode()</c>, this method can be used on the iterator to set
        /// it for decoding the key's encodedAttrib buffer instead of the
        /// encodedDataBody. When the decoding of the encodedAttrib buffer is
        /// complete, it will be ready to decode the encodedDataBody as normal.
        /// </remarks>
        /// <param name="iter"> Iterator to set </param>
        /// <param name="key"> Key from the decoded message
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        /// <seealso cref="IMsgKey"/>
        CodecReturnCode DecodeKeyAttrib(DecodeIterator iter, IMsgKey key);

        /// <summary>
        /// Copy <see cref="IMsg"/>.
        /// </summary>
        /// <remarks>
        ///
        /// Performs a deep copy of a <see cref="IMsg"/> structure. Expects all memory to be
        /// owned and managed by user. If the memory for the buffers (i.e. name, attrib, ect.)
        /// are not provided, they will be created.
        /// </remarks>
        /// <param name="destMsg"> Msg to copy Msg structure into. It cannot be null. </param>
        /// <param name="copyMsgFlags"> controls which parameters of message are copied to destination message
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the message is copied successfully,
        ///         <c>CodecReturnCode.FAILURE</c> if the source message is invalid
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the buffer provided is too small
        /// </returns>
        /// <seealso cref="CopyMsgFlags"/>
        CodecReturnCode Copy(IMsg destMsg, int copyMsgFlags);

        /// <summary>
        /// Is <see cref="IMsg"/> in final state.
        /// </summary>
        /// <returns> <c>true</c> - if this is a final message for the request,
        ///   <c>false</c> otherwise
        /// </returns>
        bool FinalMsg();

		/// <summary>
		/// Validates <see cref="IMsg"/>.
		/// </summary>
		/// <remarks>
		///
		/// Validates fully populated <see cref="IMsg"/> structure to ensure validity of its data members.
        /// </remarks>
		/// <returns> <c>true</c> - if valid; <c>false</c> if not valid
		/// </returns>
		bool ValidateMsg();

		/// <summary>
		/// Gets or sets class of this message (Update, Refresh, Status, etc).
		/// Populated from <see cref="MsgClasses"/> enumeration.
		/// Must be in the range of 1 - 31.
		/// </summary>
		int MsgClass { get; set; }

		/// <summary>
		/// Gets or sets domain Type of this message, corresponds to a domain model definition
		/// (values less than 128 are LSEG defined domain models, values
		/// between 128 - 255 are user defined domain models).
		/// Must be in the range of 1 - 255.
		/// </summary>
		int DomainType { get; set; }

		/// <summary>
		/// Gets or sets container type that is held in the encodedDataBody. ContainerType must be
		/// from the <see cref="DataTypes"/> enumeration in the range
		/// <see cref="DataTypes.CONTAINER_TYPE_MIN"/> to 255.
		/// </summary>
		/// <seealso cref="DataTypes"/>
		int ContainerType { get; set; }

		/// <summary>
		/// Gets or sets unique identifier associated with all messages flowing within a stream
		/// (positive values indicate a consumer instantiated stream, negative values
		/// indicate a provider instantiated stream often associated with non-interactive providers).
		/// Must be in the range of -2147483648 - 2147483647.
		/// </summary>
		int  StreamId { get; set; }

		/// <summary>
		/// Gets key providing unique identifier (msgKey, in conjunction with quality of
		/// service and domainType, is used to uniquely identify a stream).
		/// </summary>
		/// <value> the msgKey </value>
		MsgKey MsgKey { get; }

		/// <summary>
		/// Gets or sets extended header information.
		/// </summary>
		/// <value> the extendedHeader </value>
		Buffer ExtendedHeader { get; set; }

		/// <summary>
		/// Gets or sets encoded payload contents of the message.
		/// </summary>
		/// <value> the encodedDataBody </value>
		Buffer EncodedDataBody { get; set; }

		/// <summary>
		/// Gets buffer that contains the entire encoded message, typically only populated during decode.
		/// </summary>
		/// <value> the encodedMsgBuffer </value>
		Buffer EncodedMsgBuffer { get; }

		/// <summary>
		/// Gets or sets all the flags applicable to this message.
		/// Must be in the range of 0 - 32767.
		/// </summary>
		int Flags { get; set; }
	}
}
