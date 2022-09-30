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
	/// Communicates data between system components: to exchange information,
	/// indicate status, permission users and access, and for a variety of other
	/// purposes. Each ETA message, while having their own unique members, shares a
	/// common set of members. This common set of members form the base portion of
	/// each message structure.
	/// </summary>
	/// <seealso cref="MsgClasses"/>
	public interface IMsg : IXMLDecoder
	{
		/// <summary>
		/// Clears the current contents of the message and prepares it for re-use.
		/// Useful for object reuse during encoding. While decoding, <see cref="IMsg"/>
		/// object can be reused without using <see cref="Clear()"/>.
		/// <para>
		/// (Messages may be pooled in a single collection via their common
		/// <see cref="IMsg"/> interface and re-used as a different <see cref="MsgClasses"/>).
		/// </para>
		/// </summary>
		void Clear();

        /// <summary>
        /// Encode a ETA Message. Encodes the key into buffer, all the data is
        /// passed in.<para />
        /// Typical use:<para />
        /// 1. Set Msg structure members.<para />
        /// 2. Encode key(s) in separate buffer(s) and set Msg key members appropriately.<para />
        /// 3. Encode message body in separate buffer and set Msg encodedDataBody member appropriately.<para />
        /// 4. Call Msg.encode().<para />
        /// </summary>
        /// <param name="iter"> Encoding iterator
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="IMsgKey"/>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode Encode(EncodeIterator iter);

        /// <summary>
        /// Initiate encoding of a ETA Message. Initiates encoding of a message.<para />
        /// Typical use:<para />
        /// 1. Call Msg.encodeInit().<para />
        /// 2. Encode the key contents.<para />
        /// 3. Call Msg.encodeKeyAttribComplete().<para />
        /// 4. Encode the message data body as needed.<para />
        /// 5. Call Msg.encodeComplete().<para />
        /// </summary>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="dataMaxSize"> Max encoding size of the data, set to 0 if unknown
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="IMsgKey"/>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode EncodeInit(EncodeIterator iter, int dataMaxSize);

        /// <summary>
        /// Complete encoding of a ETA Message. Complete encoding of a message.<para />
        /// Typical use:<para />
        /// 1. Call Msg.EncodeInit().<para />
        /// 2. Encode the key contents.<para />
        /// 3. Call Msg.EncodeKeyAttribComplete().<para />
        /// 4. Encode the message data body as needed.<para />
        /// 5. Call Msg.EncodeComplete().<para />
        /// Note, step 2 and 3 are optional, instead the application can set the
        /// respective key members of the <see cref="IMsg"/> structure using pre-encoded buffers.
        /// </summary>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> If true - successfully complete the key,
        ///                if false - remove the key from the buffer.
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="IMsgKey"/>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode EncodeComplete(EncodeIterator iter, bool success);

        /// <summary>
        /// Complete Encoding of msgKey.Opaque data.
        /// 
        /// Typically used when user calls Msg.encodeInit using a msgKey indicating that <para />
        /// the msgKey.encodedAttrib should be encoded and the encodedAttrib.length and <para />
        /// encodedAttrib.data are not populated with pre-encoded
        /// msgKey.encodedAttrib data. <para />
        /// Msg.encodeInit will return <see cref="CodecReturnCode.ENCODE_MSG_KEY_ATTRIB"/>, the user will <para />
        /// invoke the container encoders for their msgKey.encodedAttrib, and after it is complete call <para />
        /// encodeKeyAttribComplete.<para />
        /// </summary>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> If true, msgKey.encodedAttrib was encoded successfully so finish.
        ///                If not, rollback to last successful part.
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="IMsgKey"/>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode EncodeKeyAttribComplete(EncodeIterator iter, bool success);

        /// <summary>
        /// Complete encoding of an Extended Header. Encodes the extended header, the data is passed in.<para />
        /// Typical use:<para />
        /// 1. Call Msg.EncodeInit().<para />
        /// 2. Call extended header encoding methods.<para />
        /// 3. Call Msg.EncodeExtendedHeaderComplete().<para />
        /// </summary>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> If true, finish encoding - else rollback
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        CodecReturnCode EncodeExtendedHeaderComplete(EncodeIterator iter, bool success);

        /// <summary>
        /// Decode a message.
        /// </summary>
        /// <param name="iter"> Decode iterator
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        CodecReturnCode Decode(DecodeIterator iter);

        /// <summary>
        /// Set iterator to decode a message's key attribute buffer
        /// 
        /// After calling Decode(), this method can be used on the iterator to set
        /// it for decoding the key's encodedAttrib buffer instead of the
        /// encodedDataBody. When the decoding of the encodedAttrib buffer is
        /// complete, it will be ready to decode the encodedDataBody as normal.
        /// </summary>
        /// <param name="iter"> Iterator to set </param>
        /// <param name="key"> Key from the decoded message
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        /// <seealso cref="IMsgKey"/>
        CodecReturnCode DecodeKeyAttrib(DecodeIterator iter, IMsgKey key);

        /// <summary>
        /// Copy <seealso cref="IMsg"/>.
        /// 
        /// Performs a deep copy of a <see cref="IMsg"/> structure. Expects all memory to be
        /// owned and managed by user. If the memory for the buffers (i.e. name, attrib, ect.)
        /// are not provided, they will be created.
        /// </summary>
        /// <param name="destMsg"> Msg to copy Msg structure into. It cannot be null. </param>
        /// <param name="copyMsgFlags"> controls which parameters of message are copied to destination message
        /// </param>
        /// <returns> <see cref="CodecReturnCode.SUCCESS"/>, if the message is copied successfully,
        ///         <see cref="CodecReturnCode.FAILURE"/> if the source message is invalid
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the buffer provided is too small
        /// </returns>
        /// <seealso cref="CopyMsgFlags"/>
        CodecReturnCode Copy(IMsg destMsg, int copyMsgFlags);

        /// <summary>
        /// Is <seealso cref="IMsg"/> in final state.
        /// </summary>
        /// <returns> true - if this is a final message for the request, false otherwise </returns>
        bool FinalMsg();

		/// <summary>
		/// Validates <seealso cref="IMsg"/>.
		/// 
		/// Validates fully populated <seealso cref="IMsg"/> structure to ensure validity of its data members.
		/// </summary>
		/// <returns> true - if valid; false if not valid </returns>
		bool ValidateMsg();

		/// <summary>
		/// Gets or sets class of this message (Update, Refresh, Status, etc).
		/// Populated from <seealso cref="MsgClasses"/> enumeration.
		/// Must be in the range of 1 - 31.
		/// </summary>
		int MsgClass { get; set; }

		/// <summary>
		/// Gets or sets domain Type of this message, corresponds to a domain model definition
		/// (values less than 128 are Thomson Reuters defined domain models, values
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
		/// <returns> the msgKey </returns>
		IMsgKey MsgKey { get; }

		/// <summary>
		/// Gets or sets extended header information.
		/// </summary>
		/// <returns> the extendedHeader </returns>
		Buffer ExtendedHeader { get; set; }

		/// <summary>
		/// Gets or sets encoded payload contents of the message.
		/// </summary>
		/// <returns> the encodedDataBody </returns>
		Buffer EncodedDataBody { get; set; }

		/// <summary>
		/// Gets buffer that contains the entire encoded message, typically only populated during decode.
		/// </summary>
		/// <returns> the encodedMsgBuffer </returns>
		Buffer EncodedMsgBuffer { get; }

		/// <summary>
		/// Gets or sets all the flags applicable to this message.
		/// Must be in the range of 0 - 32767.
		/// </summary>
		int Flags { get; set; }
	}
}