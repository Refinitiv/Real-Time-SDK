/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;

namespace LSEG.Ema.Access;

/// <summary>
/// Utilities that can be used by the EMA applications.
/// </summary>
public class EmaUtility
{

    /// <summary>
    /// Represent as a String all the bytes (in hex format) in a ByteBuffer.
    /// The lines of hex are appended with respective string representations.
    /// </summary>
    /// <param name="buffer">EmaBuffer used</param>
    /// <returns>string representation of the ByteBuffer</returns>
    public static string AsHexString(EmaBuffer buffer)
    {
        int charsPerLine = 16;
        StringBuilder asString = new StringBuilder();
        StringBuilder currentLine = new StringBuilder();
        StringBuilder all = new StringBuilder();

        bool processedFirst = false;
        int currentChar = 0;
        int length = buffer.Length;
        int eobyte = 0;

        for (int i = 0; i < length; i++)
        {
            if (!(currentChar < charsPerLine))
            {
                all.Append(currentLine);
                all.Append("  ");
                all.Append(asString);

                currentLine.Clear();
                asString.Clear();
                currentChar = 0;
                eobyte = 0;
            }

            byte b = buffer[i];
            currentLine.Append(eobyte % 2 != 0
                ? String.Format("{0:X2} ", b)
                : String.Format("{0:X2}", b));
            eobyte ^= 1;

            if (b > 31 && b < 127)
                asString.Append((char)b);
            else
                asString.Append('.');

            if (currentChar == 7)
            {
                currentLine.Append(' ');
            }
            ++currentChar;
        }

        if (currentLine.Length > 0)
        {
            if (processedFirst)
            {
                all.AppendLine();
                eobyte = 0;
            }

            int fill = currentChar;
            while (fill < charsPerLine)
            {
                currentLine.Append("   ");

                if (fill == 7)
                {
                    currentLine.Append(' ');
                }
                ++fill;
            }

            all.Append(currentLine);
            all.Append("  ");
            all.Append(asString);
        }

        return all.ToString();
    }
}
