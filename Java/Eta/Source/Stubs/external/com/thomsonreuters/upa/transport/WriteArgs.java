package com.thomsonreuters.upa.transport;

/**
 * UPA Write Arguments used in the {@link Channel#write(TransportBuffer, WriteArgs, Error)} call.
 * 
 * @see Channel
 * @see WritePriorities
 * @see WriteFlags
 */
public interface WriteArgs
{
    /**
     * Priority to flush the message for {@link Channel#write(TransportBuffer, WriteArgs, Error)}.
     * Must be in the range of {@link WritePriorities#HIGH} - {@link WritePriorities#LOW}.
     * 
     * @param priority the priority to set
     * 
     * @see WritePriorities
     */
    public void priority(int priority);

    /**
     * Priority to flush the message for {@link Channel#write(TransportBuffer, WriteArgs, Error)}.
     * 
     * @return the priority
     * 
     * @see WritePriorities
     */
    public int priority();

    /**
     * Write flags used for {@link Channel#write(TransportBuffer, WriteArgs, Error)}.
     * Must be in the range of {@link WriteFlags#NO_FLAGS} - {@link WriteFlags#DIRECT_SOCKET_WRITE}.
     * 
     * @param flags the flags to set
     * 
     * @see WriteFlags
     */
    public void flags(int flags);

    /**
     * Write flags used for {@link Channel#write(TransportBuffer, WriteArgs, Error)}.
     * 
     * @return the flags
     * 
     * @see WriteFlags
     */
    public int flags();

    /**
     * The number of bytes to be written, including any transport header overhead
     * and taking into account any compression, for this write call.
     * 
     * @return the bytesWritten
     */
    public int bytesWritten();

    /**
     * The number of bytes to be written, including any transport header
     * overhead but not taking into account any compression, for this write call.
     * 
     * @return the uncompressedBytesWritten
     */
    public int uncompressedBytesWritten();

    /**
     * Specifies the sequence number of the message {@link Channel#write(TransportBuffer, WriteArgs, Error)}.
     * 
     * @param seqNum the sequence number to set
     */
    public void seqNum(long seqNum);

    /**
     * The sequence number of the message for {@link Channel#write(TransportBuffer, WriteArgs, Error)}.
     * 
     * @return the seqNum
     */
    public long seqNum();
    
    /**
     * Clears UPA Write Arguments.
     */
    public void clear();
}
