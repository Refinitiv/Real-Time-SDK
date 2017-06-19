///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.test.network.replay;

import java.io.IOException;
import java.util.Arrays;

/**
 * Contains {@code static} utility methods for working with {@link NetworkReplay}
 */
public final class NetworkReplayUtil
{
    /**
     * This class is not instantiated
     */
    private NetworkReplayUtil()
    {
        throw new AssertionError();
    }
    
    /**
     * Parses the specified replay file, and WITH THE ASSUMPTION THAT EACH
     * RECORD IN THE FILE IS A COMPLETE RIPC message, returns an array of messages,
     * such that the RIPC header has been stripped off each message.
     * Empty (null) messages will be returned as zero-length byte arrays.
     * 
     * @param replayFileName The full path to the file containing the network replay data
     * 
     * @return An array of messages, such that the RIPC header has been stripped off each message.
     *         Empty (null) messages will be returned as zero-length byte arrays.
     *         
     * @throws IOException Thrown if an error occurs while reading the replay file.
     */
    public static byte[][] parseAndStripRipcHeaders(String replayFileName) throws IOException
    {
        final int ripcHeaderLen = 3; // bytes

        final NetworkReplay replay = NetworkReplayFactory.create();
        replay.parseFile(replayFileName);
        final int recordCount = replay.recordsInQueue();

        final byte[][] messages = new byte[recordCount][]; // assumption: each record is a complete RIPC message
        for (int i = 0; i < recordCount; i++)
        {
            final byte[] withHeader = replay.read();
            if (withHeader != null)
            {
                if (ripcHeaderLen <= withHeader.length)
                {
                    messages[i] = Arrays.copyOfRange(withHeader, ripcHeaderLen, withHeader.length);
                }
                else
                {
                    messages[i] = Arrays.copyOfRange(withHeader, 0, withHeader.length);
                }
            }
            else
            {
                messages[i] = new byte[0];
            }
        }

        return messages;
    }

    /**
     * Parses the specified replay file, and WITH THE ASSUMPTION THAT EACH
     * RECORD IN THE FILE IS A COMPLETE HTTP RIPC message, returns an array of messages,
     * such that the RIPC header has been stripped off each message.
     * Empty (null) messages will be returned as zero-length byte arrays.
     * 
     * @param replayFileName - The full path to the file containing the network replay data
     * 
     * @return An array of messages, such that the HTTP header, RIPC header, and chunkend have been
     *         stripped off each message. Empty (null) messages will be returned as zero-length byte arrays.
     * 
     * @throws IOException Thrown if an error occurs while reading the replay file.
     */
    public static byte[][] parseAndStripHttpAndRipcHeaders(String replayFileName) throws IOException
    {
        final int HTTP_HEADER6 = 6;
        final int ripcHeaderLen = 3; // bytes
        final int CHUNKEND_SIZE = 2;

        final NetworkReplay replay = NetworkReplayFactory.create();
        replay.parseFile(replayFileName);
        final int recordCount = replay.recordsInQueue();

        final byte[][] messages = new byte[recordCount][]; // assumption: each record is a complete RIPC message
        for (int i = 0; i < recordCount; i++)
        {
            final byte[] withHeader = replay.read();
            if (withHeader != null)
            {
                if (HTTP_HEADER6 + ripcHeaderLen <= withHeader.length - CHUNKEND_SIZE)
                {
                    messages[i] = Arrays.copyOfRange(withHeader, HTTP_HEADER6 + ripcHeaderLen, withHeader.length - CHUNKEND_SIZE);
                }
                else
                {
                    messages[i] = Arrays.copyOfRange(withHeader, 0, withHeader.length);
                }
            }
            else
            {
                messages[i] = new byte[0];
            }
        }

        return messages;
    }
    
}
