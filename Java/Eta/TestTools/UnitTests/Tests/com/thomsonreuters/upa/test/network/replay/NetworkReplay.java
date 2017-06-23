///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.test.network.replay;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * An interface for simulating network data 
 */
public interface NetworkReplay
{
    /**
     * Reads the next sequence of bytes from the simulated network.
     * A {@code null} array is returned if there is no data to be read.
     * 
     * @return A sequence of bytes from the simulated network.
     *         A {@code null} array is returned if there is no data to be read.
     */
    byte[] read();
    
    /**
     * Enqueues data to be replayed
     * 
     * @param data The data to enqueue
     */
    void enqueue(byte[] data);
    
    /**
     * Parses a file containing network replay data, and returns the total
     * number of non-comment bytes read from the file.
     * 
     * @param name The name of the file to parse
     * 
     * @return The total number of non-comment bytes read from the file
     * 
     * @throws IOException Thrown if an exception occurs while parsing the file
     */
    int parseFile(String name) throws IOException;
    
    /**
     * Returns the number of records remaining in the queue
     * @return the number of records remaining in the queue
     */
    int recordsInQueue();

    /**
     * Reads a sequence of bytes from this channel into the given buffer.
     * <p>
     * An attempt is made to read up to r bytes from the channel, where r is
     * the number of bytes remaining in the buffer, that is, dst.remaining(),
     * at the moment this method is invoked.
     * <p>
     * Suppose that a byte sequence of length n is read, where 0 &lt;= n &lt;= r.
     * This byte sequence will be transferred into the buffer so that the first byte in the sequence
     * is at index p and the last byte is at index p + n - 1,
     * where p is the buffer's position at the moment this method is invoked.
     * Upon return the buffer's position will be equal to p + n; its limit will not have changed.
     * <p>
     * A read operation might not fill the buffer, and in fact it might not read any bytes at all.
     * Whether or not it does so depends upon the nature and state of the channel.
     * A socket channel in non-blocking mode, for example, cannot read any more bytes
     * than are immediately available from the socket's input buffer;
     * similarly, a file channel cannot read any more bytes than remain in the file.
     * It is guaranteed, however, that if a channel is in blocking mode and there is at least one byte remaining
     * in the buffer then this method will block until at least one byte is read.
     * <p>
     * This method may be invoked at any time.
     * If another thread has already initiated a read operation upon this channel, however,
     * then an invocation of this method will block until the first operation is complete.
     * <p>
     * Specified by: read(...) in ReadableByteChannel
     * 
     * @param dst The buffer into which bytes are to be transferred
     * 
     * @return The number of bytes read, possibly zero, or -1 if the channel has reached end-of-stream
     * 
     * @throws IOException If some other I/O error occurs
     */
    int read(ByteBuffer dst) throws IOException;
    
    /**
     * Forces the next call to {@link NetworkReplay#read(ByteBuffer)} to throw an {@link IOException}
     *  
     * @param error The {@link IOException} thrown the next time {@link NetworkReplay#read(ByteBuffer)}
     *              is invoked will contain this error text
     */
    void forceNextReadToFail(String error);

    /**
     * Forces the next call to {@link NetworkReplay#read(ByteBuffer)} to return -1
     */
    void forceEndOfStream();
    
    /**
     * Starts listening for, and accepting connections on the specified port.
     * If we already started accepting connection, no action is taken
     * 
     * @param portNumber The port to listen on
     * 
     * @throws IOException Thrown if a listener could not be started on the specified port
     */
    void startListener(int portNumber) throws IOException;
    
    /**
     * Stops listening for connections. If we were not already listening, no action is taken.
     */
    void stopListener();

}


