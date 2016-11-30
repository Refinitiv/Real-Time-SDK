package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.TransportBuffer;

/**
 * An iterator for encoding the RWF content.
 * 
 * <p>
 * All RWF encoding requires the use of an <code>EncodeIterator</code>, where a
 * single iterator can manage the full encoding process. For instance, if the
 * application is encoding a message containing a {@link FieldList}, the
 * same <code>EncodeIterator</code> can be used to encode the message header
 * information, the {@link FieldList} information, each
 * {@link FieldEntry}, and the content of each entry (primitive types or
 * container types). Separate iterators can be used as well, depending on the
 * needs of the application. Following the same example, one
 * <code>EncodeIterator</code> can be used to pre-encode an entry's content.
 * This pre-encoded content can then be set on the {@link FieldEntry} and
 * encoded using the <code>EncodeIterator</code> that is encoding the
 * {@link FieldList}. This encoded field list content can then be set on
 * the {@link Msg} and yet another <code>EncodeIterator</code> can be used
 * to encode the message and its pre-encoded payload.
 * 
 * <p>
 * Before encoding begins, the iterator should be initialized to ready it for
 * the encoding process. Initialization consists of several steps. The
 * {@link #clear()} method can be used to initialize (or re-initialize for
 * reuse) the <code>EncodeIterator</code>. After clearing, a
 * {@link Buffer} with ample memory should be associated with the iterator;
 * this will be the buffer that content is encoded into (if using with the UPA
 * Transport, this is often a buffer obtained from the
 * {@link Channel#getBuffer(int, boolean, com.thomsonreuters.upa.transport.Error)}
 * method so it can be immediately written after encoding completes). 
 * In addition, RWF version information should be provided to the <code>EncodeIterator</code> 
 * so the desired version of RWF is encoded.
 * 
 * <b>Encode Iterator Example</b>
 * <p>
 * The following code example demonstrates creation of the UPA encode iterator
 * and associating with buffer to encode into:
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * //Create EncodeIterator using CodecFactory
 * EncodeIterator encIter = CodecFactory.createEncodeIterator()}; 
 * 
 * //Clear iterator 
 * clear();
 * 
 * //Associate buffer to decode with the iterator and set RWF version on the iterator 
 * if (encIter.setBufferAndRWFVersion(encBuffer, 
 *                            Codec.majorVersion(), 
 *                            Codec.minorVersion()) < CodecReturnCodes.SUCCESS) 
 * { 
 *      //Handle error appropriately 
 *      return 0; 
 * }
 * 
 * //Do encoding using iterator
 * </pre>
 * 
 * </li>
 * </ul>
 */
public interface EncodeIterator
{
    /**
     * Clears EncodeIterator, defaults to use current version of RWF.
     * After clearing an iterator, the buffer needs to be set using
     * {@link #setBufferAndRWFVersion(Buffer, int, int)}
     * 
     * <dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
     * <dt><b>Note:</b></dt>
     * <dd>
     * This should be used to achieve the best performance while clearing
     * the iterator
     * </dd>
     * </dl>
     */
    public void clear();

    /**
     * Sets the encode iterator's buffer and the desired RWF Version on the
     * iterator. When used for encoding, the iterator will then use that version
     * of the RWF to encode.
     * 
     * Use this method when you want to encode into RWF, but use your own
     * transport.
     * 
     * @param buffer Buffer to use for encoding
     * @param rwfMajorVersion - this is the major version of the wire format to encode
     * @param rwfMinorVersion - this is the minor version of the wire format to encode
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if {@code Buffer} is
     *         successfully associated with iterator and RWF version is set,
     *         {@link CodecReturnCodes#VERSION_NOT_SUPPORTED} if version is invalid,
     *         {@link CodecReturnCodes#FAILURE} if invalid buffer length or buffer data.
     *         
     * @see Buffer
     */
    public int setBufferAndRWFVersion(Buffer buffer, int rwfMajorVersion, int rwfMinorVersion);

    /**
     * Sets the encode iterator's buffer and the desired RWF Version on the
     * iterator. When used for encoding, the iterator will then use that version
     * of the RWF to encode.
     * 
     * Use this method when you want to encode into RWF and use UPAJ's transport.
     * 
     * @param buffer {@link TransportBuffer} to use for encoding
     * @param rwfMajorVersion - this is the major version of the wire format to encode
     * @param rwfMinorVersion - this is the minor version of the wire format to encode
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if {@link TransportBuffer}
     *         successfully associated with iterator and RWF version is set,
     *         {@link CodecReturnCodes#VERSION_NOT_SUPPORTED} if version is invalid,
     *         {@link CodecReturnCodes#FAILURE} if invalid buffer length or buffer data.
     * 
     * @see TransportBuffer
     */
    public int setBufferAndRWFVersion(TransportBuffer buffer, int rwfMajorVersion, int rwfMinorVersion);

    /**
     * Sets the Global Field Set Definition Database on the iterator.
     * When used for encoding, the iterator will use that database to encode any field lists that
     * contain set definitions.
     * 
     * @param setDefDb - this is the database that will be used for encode.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if {@code GlobalFieldSetDefDb} is successfully set on the iterator.
     * 
     * @see TransportBuffer
     */
    public int setGlobalFieldSetDefDb(GlobalFieldSetDefDb setDefDb);
    
    /**
     * Sets the Global Element Set Definition Database on the iterator. When used for 
     * encoding, the iterator will use that database to encode any field lists that contain set definitions.
     * 
     * @param setDefDb - this is the database that will be used for encode.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if {@code GlobalFieldSetDefDb} is successfully set on the iterator.
     * 
     * @see TransportBuffer
     */
    public int setGlobalElementSetDefDb(GlobalElementSetDefDb setDefDb);
    
    /**
     * Realigns Buffer's Iterator with buffer when new, larger buffer is needed to complete encoding.
     * 
     * Typical use:<BR>
     * 1. Call realignBuffer() with the current iterator, and the new larger
     * buffer to complete encoding into.<BR>
     * 2. Finish encoding using the new buffer and the same iterator you were
     * using before.<BR>
     * The user must pass in a newly allocated buffer, and the method does not
     * deallocate the previous buffer.
     * 
     * @param newEncodeBuffer The larger buffer to continue encoding into
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if new iterator is successfully
     *         aligned with new buffer, {@link CodecReturnCodes#FAILURE}
     *         otherwise, typically due to new buffer not being sufficiently
     *         populated with buffer length or buffer data.
     * 
     * @see Buffer
     */
    public int realignBuffer(Buffer newEncodeBuffer);

    /**
     * Realigns Transport Buffer's Iterator with buffer when new, larger buffer is needed to complete encoding.
     * 
     * Typical use:<BR>
     * 1. Call realignBuffer() with the current iterator, and the new larger
     * buffer to complete encoding into.<BR>
     * 2. Finish encoding using the new buffer and the same iterator you were
     * using before.<BR>
     * The user must pass in a newly allocated buffer, and the method does not
     * deallocate the previous buffer.
     * 
     * @param newEncodeBuffer The larger buffer to continue encoding into
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if new iterator is successfully
     *         aligned with new buffer, {@link CodecReturnCodes#FAILURE}
     *         otherwise, typically due to new buffer not being sufficiently
     *         populated with buffer length or buffer data.
     * 
     * @see Buffer
     */
    public int realignBuffer(TransportBuffer newEncodeBuffer);

    /**
     * Initialize encoding of non-RWF data into the encode iterator's buffer.
     * 
     * @param buffer Buffer to encode into the iterator buffer
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if new non-RWF Buffer is
     *         successfully encoded into iterator, non-success code from 
     *         {@link CodecReturnCodes} otherwise
     * 
     * @see Buffer
     */
    public int encodeNonRWFInit(Buffer buffer);

    /**
     * Complete encoding of non-RWF data into the encode iterator's buffer.
     * 
     * @param buffer {@link Buffer} to encode into the iterator buffer
     * @param success If true - successfully complete the aggregate,
     *                if false - remove the aggregate from the buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} for successful completion, 
     * non-success code from {@link CodecReturnCodes} if not.
     * 
     * @see Buffer
     */
    public int encodeNonRWFComplete(Buffer buffer, boolean success);
    
    /**
     * Major version number of the encode iterator. Valid only after
     * {@link #setBufferAndRWFVersion(Buffer, int, int)} call.
     * 
     * @return Encode iterator major version when valid or {@link CodecReturnCodes#FAILURE} when invalid.
     */
    public int majorVersion();
    
    /**
     * Minor version number of the encode iterator. Valid only after
     * {@link #setBufferAndRWFVersion(Buffer, int, int)} call.
     * 
     * @return Encode iterator minor version when valid or {@link CodecReturnCodes#FAILURE} when invalid.
     */
    public int minorVersion();
    
    /**
     * Convenience method that replaces the streamId on an encoded UPA
     * message.
     * 
     * @param streamId the new stream id
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the stream has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     */
    public int replaceStreamId(int streamId);
    
    /**
     * Convenience method that replaces the seqNum on an encoded UPA message.
     * This only works if there is a seqNum already encoded in the message.
     * 
     * @param seqNum the new sequence number
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the seqNum has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         {@link CodecReturnCodes#FAILURE} if the encoded message does not have seqNum
     */
    public int replaceSeqNum(long seqNum);
    
    /**
     * Convenience method that replaces the stream state on an encoded UPA
     * message. This only works if there is a state already encoded in the message.
     * 
     * @param streamState the new stream state
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the streamState has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         {@link CodecReturnCodes#FAILURE} if the encoded message does not have state
     * 
     * @see com.thomsonreuters.upa.codec.StreamStates
     */
    public int replaceStreamState(int streamState);
    
    /**
     * Convenience method that replaces the data state on an encoded UPA
     * message. This only works if there is a state already encoded in the message.
     * 
     * @param dataState the new data state
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the dataState has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         {@link CodecReturnCodes#FAILURE} if the encoded message does not have state
     * 
     * @see com.thomsonreuters.upa.codec.DataStates
     */
    public int replaceDataState(int dataState);

    /**
     * Convenience method that replaces the state code on an encoded UPA
     * message. This only works if there is a state already encoded in the message.
     * 
     * @param stateCode the state code
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the code has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         {@link CodecReturnCodes#FAILURE} if the encoded message does not have state
     * 
     * @see com.thomsonreuters.upa.codec.StateCodes
     */
    public int replaceStateCode(int stateCode);
    
    /**
     * Convenience method that replaces the group id on an encoded UPA
     * message. This only works if there is a group id already encoded in the
     * message and the length of the new groupId is the same as the old one.
     * 
     * @param groupId {@link Buffer} with groupID information
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the groupId has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         {@link CodecReturnCodes#FAILURE} if the encoded message does not have state
     *         or the length of the new groupId is different than the encoded.
     * 
     * @see Buffer
     */
    public int replaceGroupId(Buffer groupId);

    /**
     * Convenience method that replaces an encoded post messages postId. This
     * only works if there is a postId already encoded in the message.
     * 
     * @param postId the post id
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the postId has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         {@link CodecReturnCodes#FAILURE} if the encoded message does not have postId
     */
    public int replacePostId(int postId);

    /**
     * Set the RefreshMsgFlags.SOLICITED flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    public int setSolicitedFlag();
    
    /**
     * Unset the RefreshMsgFlags.SOLICITED flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    public int unsetSolicitedFlag();

    /**
     * Set the RefreshMsgFlags.REFRESH_COMPLETE flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    public int setRefreshCompleteFlag();

    /**
     * Unset the RefreshMsgFlags.REFRESH_COMPLETE flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int unsetRefreshCompleteFlag();

    /**
     * Set the RefreshMsgFlags.STREAMING flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int setStreamingFlag();

    /**
     * Unset the RefreshMsgFlags.STREAMING flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int unsetStreamingFlag();

    /**
     * Set the RefreshMsgFlags.NO_REFRESH flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int setNoRefreshFlag();

    /**
     * Unset the RefreshMsgFlags.NO_REFRESH flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int unsetNoRefreshFlag();

    /**
     * Set the RefreshMsgFlags.MSG_KEY_IN_UPDATES flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int setMsgKeyInUpdatesFlag();

    /**
     * Unset the RefreshMsgFlags.MSG_KEY_IN_UPDATES flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int unsetMsgKeyInUpdatesFlag();

    /**
     * Set the RefreshMsgFlags.CONF_INFO_IN_UPDATES flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int setConfInfoInUpdatesFlag();

    /**
     * Unset the RefreshMsgFlags.CONF_INFO_IN_UPDATES flag on an encoded buffer.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the flag has been replaced,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the iterator buffer is too small
     *         or new flags would be different length
     * 
     * @see RefreshMsgFlags
     */
    int unsetConfInfoInUpdatesFlag();
	
    /**
     * buffer {@link Buffer} to use for encoding
     * 
     * @return buffer to use for encoding
     */
    public Buffer buffer();
	
    /**
     * buffer {@link TransportBuffer} to use for encoding
     * 
     * @return transport buffer to use for encoding
     */
    public TransportBuffer transportBuffer();

}