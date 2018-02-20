///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * 
 * Utilities that can be used by the EMA applications.
 *
 */
public class EmaUtility
{
	/**
	 * Represent as a String all the bytes (in hex format) in a ByteBuffer.
	 * 
	 * @param buffer ByteBuffer used
	 * @return String representation of the ByteBuffer
	 */
	public static String asHexString(ByteBuffer buffer)
	{
		final int charsPerLine = 16;
        StringBuilder asString = new StringBuilder();
        StringBuilder currentLine = new StringBuilder();
        StringBuilder all = new StringBuilder();

        boolean processedFirst = false;
        int currentChar = 0;
        int length = buffer.limit();
        int	eobyte= 0;

        for (int i = buffer.position(); i < length; i++)
        {
            if (!(currentChar < charsPerLine))
            {
                all.append(currentLine.toString());
                all.append("  ");
                all.append(asString.toString());

                currentLine.setLength(0);
                asString.setLength(0);
                currentChar = 0;
            	eobyte = 0;
            }

            byte b = buffer.get(i);
            currentLine.append(eobyte%2 != 0  ? String.format("%02X ", b) : String.format("%02X", b)); 
            eobyte ^= 1;
            
            if (b > 31 && b < 127)
                asString.append((char)b);
            else
                asString.append('.');

            if (currentChar == 7)
            {
                currentLine.append(" ");
            }
            ++currentChar;
        }

        if (currentLine.length() > 0)
        {
            if (processedFirst)
            {
                all.append("\n");
                eobyte = 0;
            }
                                                         
            int fill = currentChar;
            while (fill < charsPerLine)
            {
                currentLine.append("   ");

                if (fill == 7)
                {
                    currentLine.append(" "); 
                }
                ++fill;
            }

            all.append(currentLine.toString()); 
            all.append("  ");
            all.append(asString.toString());
        }

        return all.toString();
	}
}