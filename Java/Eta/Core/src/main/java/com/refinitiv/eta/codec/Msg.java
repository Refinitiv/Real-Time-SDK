/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.XMLDecoder;

/**
 * Communicates data between system components: to exchange information,
 * indicate status, permission users and access, and for a variety of other
 * purposes. Each ETA message, while having their own unique members, shares a
 * common set of members. This common set of members form the base portion of
 * each message structure.
 * 
 * @see MsgClasses
 */
public interface Msg extends XMLDecoder
{
    /**
     * Clears the current contents of the message and prepares it for re-use.
     * Useful for object reuse during encoding. While decoding, {@link Msg}
     * object can be reused without using {@link #clear()}.
     * <p>
     * (Messages may be pooled in a single collection via their common
     * {@link Msg} interface and re-used as a different {@link MsgClasses}).
     */
    public void clear();

    /**
     * Encode a ETA Message. Encodes the key into buffer, all the data is
     * passed in.<BR>
     * Typical use:<BR>
     * 1. Set Msg structure members.<BR>
     * 2. Encode key(s) in separate buffer(s) and set Msg key members appropriately.<BR>
     * 3. Encode message body in separate buffer and set Msg encodedDataBody member appropriately.<BR>
     * 4. Call Msg.encode().<BR>
     * 
     * @param iter Encoding iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see MsgKey
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Initiate encoding of a ETA Message. Initiates encoding of a message.<BR>
     * Typical use:<BR>
     * 1. Call Msg.encodeInit().<BR>
     * 2. Encode the key contents.<BR>
     * 3. Call Msg.encodeKeyAttribComplete().<BR>
     * 4. Encode the message data body as needed.<BR>
     * 5. Call Msg.encodeComplete().<BR>
     * 
     * @param iter Encoding iterator
     * @param dataMaxSize Max encoding size of the data, set to 0 if unknown
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see MsgKey
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter, int dataMaxSize);

    /**
     * Complete encoding of a ETA Message. Complete encoding of a message.<BR>
     * Typical use:<BR>
     * 1. Call Msg.encodeInit().<BR>
     * 2. Encode the key contents.<BR>
     * 3. Call Msg.encodeKeyAttribComplete().<BR>
     * 4. Encode the message data body as needed.<BR>
     * 5. Call Msg.encodeComplete().<BR>
     * Note, step 2 and 3 are optional, instead the application can set the
     * respective key members of the {@link Msg} structure using pre-encoded buffers.
     * 
     * @param iter Encoding iterator
     * @param success If true - successfully complete the key,
     *                if false - remove the key from the buffer.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see MsgKey
     * @see EncodeIterator
     */
    public int encodeComplete(EncodeIterator iter, boolean success);

    /**
     * Complete Encoding of msgKey.opaque data.
     * 
     * Typically used when user calls Msg.encodeInit using a msgKey indicating that <BR>
     * the msgKey.encodedAttrib should be encoded and the encodedAttrib.length and <BR>
     * encodedAttrib.data are not populated with pre-encoded
     * msgKey.encodedAttrib data. <BR>
     * Msg.encodeInit will return {@link CodecReturnCodes#ENCODE_MSG_KEY_ATTRIB}, the user will <BR>
     * invoke the container encoders for their msgKey.encodedAttrib, and after it is complete call <BR>
     * encodeKeyAttribComplete.<BR>
     * 
     * @param iter Encoding iterator
     * @param success If true, msgKey.encodedAttrib was encoded successfully so finish.
     *                If not, rollback to last successful part.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see MsgKey
     * @see EncodeIterator
     */
    public int encodeKeyAttribComplete(EncodeIterator iter, boolean success);

    /**
     * Complete encoding of an Extended Header. Encodes the extended header, the data is passed in.<BR>
     * Typical use:<BR>
     * 1. Call Msg.encodeInit().<BR>
     * 2. Call extended header encoding methods.<BR>
     * 3. Call Msg.encodeExtendedHeaderComplete().<BR>
     * 
     * @param iter Encoding iterator
     * @param success If true, finish encoding - else rollback
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeExtendedHeaderComplete(EncodeIterator iter, boolean success);

    /**
     * Decode a message.
     * 
     * @param iter Decode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Set iterator to decode a message's key attribute buffer
     * 
     * After calling decode(), this method can be used on the iterator to set
     * it for decoding the key's encodedAttrib buffer instead of the
     * encodedDataBody. When the decoding of the encodedAttrib buffer is
     * complete, it will be ready to decode the encodedDataBody as normal.
     * 
     * @param iter Iterator to set
     * @param key Key from the decoded message
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     * @see MsgKey
     */
    public int decodeKeyAttrib(DecodeIterator iter, MsgKey key);

    /**
     * Copy {@link Msg}.
     * 
     * Performs a deep copy of a {@link Msg} structure. Expects all memory to be
     * owned and managed by user. If the memory for the buffers (i.e. name, attrib, ect.)
     * are not provided, they will be created.
     * 
     * @param destMsg Msg to copy Msg structure into. It cannot be null.
     * @param copyMsgFlags controls which parameters of message are copied to destination message
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the message is copied successfully,
     *         {@link CodecReturnCodes#FAILURE} if the source message is invalid
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the buffer provided is too small
     *         
     * @see CopyMsgFlags
     */
    public int copy(Msg destMsg, int copyMsgFlags);

    /**
     * Is {@link Msg} in final state.
     * 
     * @return true - if this is a final message for the request, false otherwise
     */
    public boolean isFinalMsg();

    /**
     * Validates {@link Msg}.
     * 
     * Validates fully populated {@link Msg} structure to ensure validity of its data members.
     * 
     * @return true - if valid; false if not valid
     */
    public boolean validateMsg();

    /**
     * Class of this message (Update, Refresh, Status, etc).
     * Populated from {@link MsgClasses} enumeration.
     * Must be in the range of 1 - 31.
     * 
     * @param msgClass the msgClass to set
     */
    public void msgClass(int msgClass);

    /**
     * Class of this message (Update, Refresh, Status, etc).
     * Populated from {@link MsgClasses} enumeration.
     * 
     * @return the msgClass
     */
    public int msgClass();

    /**
     * Domain Type of this message, corresponds to a domain model definition
     * (values less than 128 are Refinitiv defined domain models, values
     * between 128 - 255 are user defined domain models).
     * Must be in the range of 0 - 255.
     * 
     * @param domainType the domainType to set
     */
    public void domainType(int domainType);

    /**
     * Domain Type of this message, corresponds to a domain model definition
     * (values less than 128 are Refinitiv defined domain models, values
     * between 128 - 255 are user defined domain models).
     * 
     * @return the domainType
     */
    public int domainType();

    /**
     * Container type that is held in the encodedDataBody. ContainerType must be
     * from the {@link DataTypes} enumeration in the range
     * {@link DataTypes#CONTAINER_TYPE_MIN} to 255.
     * 
     * @param containerType the container type
     * 
     * @see DataTypes
     */
    public void containerType(int containerType);

    /**
     * Container type that is held in the encodedDataBody.
     * 
     * @return the {@link DataTypes}
     */
    public int containerType();

    /**
     * Unique identifier associated with all messages flowing within a stream
     * (positive values indicate a consumer instantiated stream, negative values
     * indicate a provider instantiated stream often associated with non-interactive providers). 
     * Must be in the range of -2147483648 - 2147483647.
     * 
     * @param streamId the streamId to set
     */
    public void streamId(int streamId);

    /**
     * Unique identifier associated with all messages flowing within a stream
     * (positive values indicate a consumer instantiated stream, negative values
     * indicate a provider instantiated stream often associated with non-interactive providers).
     * 
     * @return the streamId
     */
    public int streamId();

    /**
     * Key providing unique identifier (msgKey, in conjunction with quality of
     * service and domainType, is used to uniquely identify a stream).
     * 
     * @return the msgKey
     */
    public MsgKey msgKey();

    /**
     * Extended header information.
     * 
     * @param extendedHeader the extendedHeader to set
     */
    public void extendedHeader(Buffer extendedHeader);

    /**
     * Extended header information.
     * 
     * @return the extendedHeader
     */
    public Buffer extendedHeader();

    /**
     * Encoded payload contents of the message.
     * 
     * @param encodedDataBody the encodedDataBody to set
     */
    public void encodedDataBody(Buffer encodedDataBody);

    /**
     * Encoded payload contents of the message.
     * 
     * @return the encodedDataBody
     */
    public Buffer encodedDataBody();

    /**
     * Buffer that contains the entire encoded message, typically only populated during decode.
     * 
     * @return the encodedMsgBuffer
     */
    public Buffer encodedMsgBuffer();

    /**
     * Sets all the flags applicable to this message.
     * Must be in the range of 0 - 32767.
     * 
     * @param flags An integer containing all the flags applicable to this message
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this message.
     * 
     * @return All the flags applicable to this message
     */
    public int flags();
}
