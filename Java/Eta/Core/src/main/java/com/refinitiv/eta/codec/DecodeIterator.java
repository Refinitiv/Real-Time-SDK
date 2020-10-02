package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * An iterator for decoding the RWF content.
 * 
 * <p>
 * All RWF decoding requires the use of a <code>DecodeIterator</code>, where a
 * single iterator can manage the full decoding process. For instance, if the
 * application is decoding a message containing a {@link FieldList}, the same
 * <code>DecodeIterator</code> can be used to decode the {@link FieldList},
 * all {@link FieldEntry}, and all types (primitive types or container types)
 * housed in the entries. Separate iterators can be used as well, depending on
 * the needs of the application. Following the same example, one
 * <code>DecodeIterator</code> can be used to decode the message information (up
 * to the beginning of the {@link FieldList} payload). Another
 * <code>DecodeIterator</code> reference can be used to decode the
 * {@link FieldList} and entries, and if desired, other iterators can be
 * used to decode the contents of each {@link FieldEntry}.
 * 
 * <p>
 * Before decoding begins, the iterator should be initialized to ready it for
 * decoding. Initialization consists of several steps. The {@link #clear()}
 * method can be used to initialize (or re-initialize for reuse) the
 * <code>DecodeIterator</code>. After clearing, a {@link Buffer} containing the
 * content to decode should be associated with the <code>DecodeIterator</code>.
 * In addition, RWF version information should be provided to the
 * <code>DecodeIterator</code> so the desired version of RWF is decoded.
 * <p>
 * 
 * <b>Decode Iterator Example</b>
 * <p>
 * The following code example demonstrates creation of the ETA decode iterator,
 * associating buffer to decode and setting RWF version to the decode iterator:
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * {@code
 * //Create DecodeIterator using CodecFactory
 * DecodeIterator decIter = CodecFactory.createDecodeIterator(); 
 * 
 * //Clear iterator 
 * clear();
 * 
 * //Associate buffer to decode with the iterator and set RWF version on the iterator 
 * if (decIter.setBufferAndRWFVersion(decBuffer, 
 *                            Codec.majorVersion(), 
 *                            Codec.minorVersion()) < CodecReturnCodes.SUCCESS) 
 * { 
 *      //Handle error appropriately 
 *      return 0; 
 * }
 * 
 * //do decoding using iterator
 *  
 * }
 * </pre>
 * 
 * </li>
 * </ul>
 */
public interface DecodeIterator
{
    /**
     * Clears DecodeIterator, defaults to use current version of RWF.
     * After clearing an iterator, the buffer needs to be set using
     * {@link #setBufferAndRWFVersion(Buffer, int, int)}
     * 
     * <dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
     * <dt><b>Note:</b></dt>
     * <dd>
     * This should be used to achieve the best performance while clearing the iterator
     * </dd>
     * </dl>
     */
    public void clear();

    /**
     * Sets the Decode iterator's buffer and the desired RWF Version on the
     * iterator. When used for decoding, the iterator will then use that version
     * of the RWF to Decode.
     * 
     * @param buffer {@link Buffer} to use for decoding
     * @param rwfMajorVersion - this is the major version of the wire format to Decode
     * @param rwfMinorVersion - this is the minor version of the wire format to Decode
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
     * Sets the Decode iterator's buffer and the desired RWF Version on the iterator.
     * When used for decoding, the iterator will then use that version of the RWF to Decode.
     * 
     * @param buffer Buffer to use for decoding
     * @param rwfMajorVersion - this is the major version of the wire format to Decode
     * @param rwfMinorVersion - this is the minor version of the wire format to Decode
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if {@code TransportBuffer} is
     *         successfully associated with iterator and RWF version is set,
     *         {@link CodecReturnCodes#VERSION_NOT_SUPPORTED} if version is invalid,
     *         {@link CodecReturnCodes#FAILURE} if invalid buffer length or buffer data.
     * 
     * @see TransportBuffer
     */
    public int setBufferAndRWFVersion(TransportBuffer buffer, int rwfMajorVersion, int rwfMinorVersion);
    
    
    /**
     * Sets the Global Field Set Definition Database on the iterator. When used for 
     * decoding, the iterator will use that database to decode any field lists that
     * contain set definitions.
     * 
     * @param setDefDb - this is the database that will be used for decoding.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if {@code GlobalFieldSetDefDb} is successfully set on the iterator.
     * 
     * @see TransportBuffer
     */
    public int setGlobalFieldSetDefDb(GlobalFieldSetDefDb setDefDb);
    
    /**
     * Sets the Global Element Set Definition Database on the iterator. When used for 
     * decoding, the iterator will use that database to decode any field lists that contain set definitions.
     * 
     * @param setDefDb - this is the database that will be used for decoding.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if {@code GlobalElementSetDefDb} is successfully set on the iterator.
     * 
     * @see TransportBuffer
     */
    public int setGlobalElementSetDefDb(GlobalElementSetDefDb setDefDb);
    
    /**
     * Finishes decoding of a container. Once a user begins decoding a container,
     * typically they must decode all entries in that container before
     * continuing.This method may be used instead to skip straight to the end.<BR>
     * 
     * Typical use:<BR>
     * 1. Call the appropriate container decoder(e.g. {@link FieldList#decode(DecodeIterator, LocalFieldSetDefDb)}).<BR>
     * 2. Call the entry decoder until the desired entry is found (e.g. a
     * {@link FieldEntry} with a particular ID).<BR>
     * 3. Call finishDecodeEntries() to complete decoding the container.<BR>
     * 
     * @return {@link CodecReturnCodes#END_OF_CONTAINER} when successful, other non-success code 
     * from {@link CodecReturnCodes} if not. 
     */
    public int finishDecodeEntries();
    
    /**
     * Major version number of the decode iterator. Valid only after
     * {@link #setBufferAndRWFVersion(Buffer, int, int)} call.
     * 
     * @return Decode iterator major version when valid or {@link CodecReturnCodes#FAILURE}
     * when invalid.
     */
    public int majorVersion();
    
    /**
     * Minor version number of the decode iterator. Valid only after
     * {@link #setBufferAndRWFVersion(Buffer, int, int)} call.
     * 
     * @return Decode iterator minor version when valid or {@link CodecReturnCodes#FAILURE}
     * when invalid.
     */
    public int minorVersion();
    
    /**
     * Extract msgClass from the ETA message encoded in the buffer.
     * 
     * @return msgClass or {@link CodecReturnCodes#INCOMPLETE_DATA} if the encoded buffer is too small
     * 
     * @see MsgClasses
     */
    public int extractMsgClass();

    /**
     * Extract domainType from the ETA message encoded in the buffer.
     * 
     * @return domainType or {@link CodecReturnCodes#INCOMPLETE_DATA} if the encoded buffer is too small
     * 
     * @see DomainTypes
     */
    public int extractDomainType();

    /**
     * Extract streamId from the ETA message encoded in the buffer.
     * 
     * @return streamId or {@link CodecReturnCodes#INCOMPLETE_DATA} if the encoded buffer is too small
     */
    public int extractStreamId();

    /**
     * Extract seqNum from the ETA message encoded in the buffer.
     * 
     * @return seqNum or {@link CodecReturnCodes#INCOMPLETE_DATA} if the encoded buffer is too small
     */
    public int extractSeqNum();

    /**
     * Extract groupId from the ETA message encoded in the buffer.
     * 
     * @param groupId the Buffer to extract groupId into
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if groupId is populated in the groupId,
     *         {@link CodecReturnCodes#FAILURE} if groupId not available,
     *         {@link CodecReturnCodes#INCOMPLETE_DATA} if the encoded buffer is too small;
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the groupId buffer is too small
     */
    public int extractGroupId(Buffer groupId);
    
    /**
     * Extract postId from the ETA message encoded in the buffer.
     * 
     * @return seqNum or {@link CodecReturnCodes#INCOMPLETE_DATA} if the encoded buffer is too small
     */
    public int extractPostId();
}
