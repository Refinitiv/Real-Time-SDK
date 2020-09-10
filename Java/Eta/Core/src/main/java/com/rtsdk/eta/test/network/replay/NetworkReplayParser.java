///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.test.network.replay;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.rtsdk.eta.transport.ByteRoutines;

/**
 * Parses network replay data records 
 */
final class NetworkReplayParser
{
    /**
     * This class is not instantiated
     */
    private NetworkReplayParser()
    {
        throw new AssertionError();
    }
    
    /**
     * The pattern for a single line of hex consists of:
     * <ul>
     *     <li>The start of the line</li>
     *     <li>Followed by optional whitespace</li>
     *     <li>Followed by a "line number prefix", which consists of a sequence
     *         of four alphanumeric characters, directly followed by a colon and a trailing space</li>
     *     <li>Followed by a sequence of up to 48 characters containing hex
     *         numbers separated by spaces</li>
     *     <li>Additional whitespace and text (containing a string translation
     *         of the hex, but we ignore it, so it is not included in the regex</li>
     * </ul>
     */
    private static final Pattern HEX_FORMAT = Pattern.compile("^\\s*\\w{4,6}:\\s(.{1,48})"); //note we only allow 4-6 chars before the colon    
    
    /**
     * Parses a single line of a hex dump (in a specific format)
     * 
     * @param singleLine a single line from the hex dump
     * 
     * @return an array containing the parsed bytes, or an empty array if the input could not be parsed
     */
    public static byte[] parse(String singleLine)
    {
        byte[] converted = null;

        try
        {
            Matcher matcher = HEX_FORMAT.matcher(singleLine);

            if (matcher.find() && matcher.groupCount() >= 1)
            {
                String justHex = matcher.group(1);
                converted = ByteRoutines.fromHexString(justHex.getBytes(), 0, justHex.length());
            }
        }
        catch (Exception e)
        {
            // ignored
        }
        finally
        {
            if (converted == null)
            {
                converted = new byte[0];
            }
        }

        return converted;
    }
}
