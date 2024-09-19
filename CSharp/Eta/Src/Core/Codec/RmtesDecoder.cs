/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using System;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// RmtesDecoder class provides functionality for converting RMTES byte sequences 
    /// to UCS2 of UTF8 byte sequences
    /// </summary>
    sealed public class RmtesDecoder
    {
        const byte ESC_CHAR = 0x1B;
        const byte LBRKT_CHAR = 0x5B;
        const byte RHPA_CHAR = 0x60; // Used for partial updates
        const byte RREP_CHAR = 0x62; // Repeat command
        const byte LS0_CHAR = 0x0F;
        const byte LS1_CHAR = 0x0E;
        const byte SS2_CHAR = 0x8E;
        const byte SS3_CHAR = 0x8F;

        private CharSet _characterSet = new CharSet();
        private RmtesWorkingSet _curWorkingSet = new RmtesWorkingSet();
        private RmtesCharSet _shiftGL = null;
        private RmtesCharSet _tmpGL = null;
        private byte _gL;
        private byte _gR;

        private RmtesInfo _tempInfo = new RmtesInfo();
        private RmtesInfo _returnInfo = new RmtesInfo();

        /// <summary>
        /// Determines the smallest byte that belongs to a character set mapped to GL area
        /// </summary>
        /// <param name="set">Character set input parameter</param>
        /// <returns>The smallest byte that is contained in the given character set
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        int GLLowest(RmtesCharSet set)
        {
            return (set.Shape == CharSet.SHAPE_96) ? 0x20 : 0x21;
        }

        /// <summary>
        /// Determines the largest byte that belongs to a character set of 94 or 96 repertoire mapped to GL area
        /// </summary>
        /// <param name="set">Character set input parameter</param>
        /// <returns>The smallest byte that is contained in the given character set
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        int GLHighest(RmtesCharSet set)
        {
            return (set.Shape == CharSet.SHAPE_96) ? 0x7F : 0x7E;
        }

        /// <summary>
        /// Determines the smallest byte that belongs to a character set mapped to GR area
        /// </summary>
        /// <param name="set">Character set input parameter</param>
        /// <returns>The smallest byte that is contained in the given character set
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        int GRLowest(RmtesCharSet set)
        {
            return (set.Shape == CharSet.SHAPE_96) ? 0xA0 : 0xA1;
        }

        /// <summary>
        /// Determines the largest byte that belongs to a character set of 94 or 96 repertoire mapped to GR area
        /// </summary>
        /// <param name="set">Character set input parameter</param>
        /// <returns>The smallest byte that is contained in the given character set
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        int GRHighest(RmtesCharSet set)
        {
            return (set.Shape == CharSet.SHAPE_96) ? 0xFF : 0xFE;
        }

        /// <summary>
        /// Detemines whether a character belongs to GL area
        /// </summary>
        /// <param name="c">input character</param>
        /// <param name="set">current working set</param>
        /// <returns>True if the input character is a GL character, false otherwise
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        Boolean IsGLChar(char c, RmtesCharSet set)
        {
            return (GLLowest(set) <= (c & 0xFF)) && ((c & 0xFF) <= GLHighest(set));
        }

        /// <summary>
        /// Detemines whether a character belongs to GR area
        /// </summary>
        /// <param name="c">input character</param>
        /// <param name="set">current working set</param>
        /// <returns>True if the input character is a GR character, false otherwise
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        Boolean IsGRChar(char c, RmtesCharSet set)
        {
            return (GRLowest(set) <= (c & 0xFF)) && ((c & 0xFF) <= GRHighest(set));
        }

        /// <summary>
        /// Gets the UCS2 encoded character from the first table.
        /// Precondition: set is a singleton character table(not a _stride of 2),
        /// inChar is a valid character (checked with isGRChar/isGLChar)
        /// </summary>
        /// <param name="inChar">The character to be transformed</param>
        /// <param name="set">The source <see cref="RmtesCharSet"/></param>
        /// <returns>The transformed character
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        char GLConvertSingle1(char inChar, RmtesCharSet set)
        {
            if (set.Table1 != null)
                return (char)set.Table1[inChar - GLLowest(set)];
            else
                return inChar;
        }

        /// <summary>
        /// Gets the UCS2 encoded character from the second table.
        /// Precondition: set is a singleton character table(not a _stride of 2),
        /// inChar is a valid character(checked with isGRChar/isGLChar), _table2 exists in the set
        /// </summary>
        /// <param name="inChar">The character to be transformed</param>
        /// <param name="set">The source <see cref="RmtesCharSet"/></param>
        /// <returns>The transformed character
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        char GLConvertSingle2(char inChar, RmtesCharSet set)
        {
            return (char)set.Table2[inChar - GLLowest(set)];
        }

        /// <summary>
        /// Gets the UCS2 encoded character from the first table of the RmtesCharSet.
        /// Preconditions: set is a singleton character table (not a _stride of 2),
        /// inChar is a valid character (checked with isGRChar/isGLChar)
        /// </summary>
        /// <param name="inChar">The character to be transformed</param>
        /// <param name="set">The source <see cref="RmtesCharSet"/></param>
        /// <returns>The transformed character
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        char GRConvertSingle1(char inChar, RmtesCharSet set)
        {
            if (set.Table1 != null && (inChar - GRLowest(set) < set.Table1_Length))
                return (char)set.Table1[inChar - GRLowest(set)];
            else
                return inChar;
        }

        /// <summary>
        /// Gets the UCS2 encoded character from the second table.
        /// Precondition: set is a singleton character table (not a _stride of 2),
        /// inChar is a valid character(checked with isGRChar/isGLChar), _table2 exists in the set
        /// </summary>
        /// <param name="inChar">The character to be transformed</param>
        /// <param name="set">The source <see cref="RmtesCharSet"/></param>
        /// <returns>The transformed character
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        char GRConvertSingle2(char inChar, RmtesCharSet set)
        {
            if (set.Table2 != null)
                return (char)set.Table2[inChar - GRLowest(set)];
            else
                return inChar;
        }

        /// <summary>
        /// Gets the UCS2 encoded character from RMTES characters in the GL area 
        /// that belong to a working set with stride 2.
        /// Precondition: set has the _stride of 2,
        /// inChar1 abd inChar2 are valid characters (checked with isGRChar/isGLChar)
        /// </summary>
        /// <param name="inChar1">the first input character</param>
        /// <param name="inChar2">the second input character</param>
        /// <param name="set">the current working set</param>
        /// <returns>The transformed character
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        char ConvertStride2GL(char inChar1, char inChar2, RmtesCharSet set)
        {
            int mapIndex;

            mapIndex = (inChar1 - GLLowest(set)) * (94) + (inChar2 - GLLowest(set));

            if (mapIndex < set.Table1_Length)
            {
                return (char)set.Table1[mapIndex];
            }
            else if (set.Table2 != null && mapIndex >= set.Table2_Start
                     && mapIndex < (set.Table2_Start + set.Table2_Length))
            {
                return (char)set.Table2[mapIndex - set.Table2_Start];
            }
            else
                return (char)0xFFFD;
        }

        /// <summary>
        /// Gets the UCS2 encoded character from RMTES characters in the GR area 
        /// that belong to a working set with stride 2.
        /// Precondition: set has the _stride of 2,
        /// inChar1 abd inChar2 are valid characters (checked with isGRChar/isGLChar)
        /// </summary>
        /// <param name="inChar1">the first input character</param>
        /// <param name="inChar2">the second input character</param>
        /// <param name="set">the current working set</param>
        /// <returns>The transformed character
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        char ConvertStride2GR(char inChar1, char inChar2, RmtesCharSet set)
        {
            int mapIndex;

            mapIndex = (inChar1 - GRLowest(set)) * (94) + (inChar2 - GRLowest(set));

            if (mapIndex < set.Table1_Length)
            {
                return (char)set.Table1[mapIndex];
            }
            else if (set.Table2 != null && mapIndex >= set.Table2_Start
                     && mapIndex < (set.Table2_Start + set.Table2_Length))
            {
                return (char)set.Table2[mapIndex - set.Table2_Start];
            }
            else
                return (char)0xFFFD;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        int ByteToUnsigned(byte input)
        {
            return input & 0xFF;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        RmtesInfo UTF8ToUCS2(ByteBuffer data, int i, char c, int endInput)
        {
            if (data == null)
                return _returnInfo.ReturnUTF8ToUCS2(0, c);

            int unsignedByte1 = ByteToUnsigned(data.Contents[i]);

            if (unsignedByte1 < 0x80)
            {
                c = (char)unsignedByte1;
                return _returnInfo.ReturnUTF8ToUCS2(1, c);
            }
            else if (((unsignedByte1 & 0xE0) == 0xE0))
            {
                int unsignedByte2 = ByteToUnsigned(data.Contents[i + 1]);
                int unsignedByte3 = ByteToUnsigned(data.Contents[i + 2]);
                if (i + 2 <= endInput)
                {
                    if (unsignedByte2 == 0)
                    {
                        return _returnInfo.ReturnUTF8ToUCS2(-1, c);
                    }

                    c = (char)((unsignedByte1 & LS0_CHAR) << 12 | (unsignedByte2 & 0x3F) << 6 | (unsignedByte3 & 0x3F));

                    return _returnInfo.ReturnUTF8ToUCS2(3, c);
                }
            }
            else if ((data.Contents[i] & 0xC0) == 0xC0)
            {
                int unsignedByte2 = ByteToUnsigned(data.Contents[i + 1]);
                if (i + 1 <= endInput)
                {
                    if (unsignedByte2 == 0)
                    {
                        return _returnInfo.ReturnUTF8ToUCS2(0, c);
                    }

                    c = (char)((unsignedByte1 & 0x1F) << 6 | (unsignedByte2 & 0x3F));

                    return _returnInfo.ReturnUTF8ToUCS2(2, c);
                }
            }

            return _returnInfo.ReturnUTF8ToUCS2(0, c);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        int UCS2ToUTF8(RmtesBuffer buffer, char ch, int pos)
        {

            if (ch < 0x0080)
            {
                if (pos + 1 <= buffer.AllocatedLength)
                {
                    buffer.Data.WriteAt(pos, (byte)ch);
                    return 1;
                }
            }
            else if (ch < 0x0800)
            {
                if (pos + 2 <= buffer.AllocatedLength)
                {
                    buffer.Data.WriteAt(pos++, (byte)(0x0C0 | (ch >> 6)));
                    buffer.Data.WriteAt(pos, (byte)(0x080 | (ch & 0x3F)));
                    return 2;
                }
            }
            else
            {
                if (pos + 3 < buffer.AllocatedLength)
                {
                    buffer.Data.WriteAt(pos++, (byte)(0x0E0 | (ch >> 12)));
                    buffer.Data.WriteAt(pos++, (byte)(0x080 | ((ch >> 6) & 0x3F)));
                    buffer.Data.WriteAt(pos++, (byte)(0x080 | (ch & 0x3F)));
                    return 3;
                }
            }

            return -1;
        }

        /// <summary>
        /// Parses the characters for a control group sequence for conversion.
        /// (first character is between 0x00 and 0x1F(CL) or between 0x70 and 0x8F(CR) )
        /// </summary>
        /// <remarks>
        /// <para>
        /// If error returned, this means that the sequence is either invalid,
        /// or contains a partial update/repeat character sequence
        /// (these should be removed from the buffer with the applyToCache function).</para>
        ///
        /// <para>
        /// Otherwise, returns codes for success if a working set change is correctly applied,
        /// or that the next character is a shift value.</para>
        /// </remarks>
        /// <param name="currPtr">The ByteBuffer that contains data</param>
        /// <param name="i">the position of the current character in the ByteBuffer</param>
        /// <param name="endPtr">The end of the data in the ByteBuffer</param>
        /// <param name="currentSet">Current RMTES decoding context</param>
        /// <returns>RMTES decoding state represented by <see cref="RmtesInfo"/>
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal RmtesInfo ControlParse(ByteBuffer currPtr, int i, int endPtr, RmtesWorkingSet currentSet)
        {
            RMTESParseState state = RMTESParseState.NORMAL;
            RMTESParseState newState = RMTESParseState.NORMAL;
            int length = 0;

            _gL = 0;
            _gR = 0;

            if (currPtr.Contents[i] == 0x00) /* ignore NULL characters for this parse */
            {
                return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, 1, currPtr);
            }
            else if (currPtr.Contents[i] == LS0_CHAR)
            {
                currentSet.GL = currentSet.G0;
                return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, 1, currPtr);
            }
            else if (currPtr.Contents[i] == LS1_CHAR)
            {
                currentSet.GL = currentSet.G1;
                return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, 1, currPtr);
            }
            else if (currPtr.Contents[i] == 0x1B)
            {
                state = RMTESParseState.ESC;
                length++;
            }
            else if (currPtr.Contents[i] == 0x1C)
            {
                return _returnInfo.ReturnControlParse(ESCReturnCode.END_CHAR, currentSet, 0, currPtr);
            }
            else
                return _returnInfo
                        .ReturnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, 0, currPtr);

            do
            {
                i++;
                length++;

                newState = RMTESParseState.NORMAL;
                switch (state)
                {
                    case RMTESParseState.ESC:
                        if (currPtr.Contents[i] == 0x21)
                            newState = RMTESParseState.ESC_21;
                        else if (currPtr.Contents[i] == 0x22)
                            newState = RMTESParseState.ESC_22;
                        else if (currPtr.Contents[i] == 0x24)
                            newState = RMTESParseState.ESC_24;
                        else if (currPtr.Contents[i] == 0x25)
                            newState = RMTESParseState.ESC_25;
                        else if (currPtr.Contents[i] == 0x26)
                            newState = RMTESParseState.ESC_26;
                        else if (currPtr.Contents[i] == 0x28)
                            newState = RMTESParseState.ESC_28;
                        else if (currPtr.Contents[i] == 0x29)
                            newState = RMTESParseState.ESC_29;
                        else if (currPtr.Contents[i] == 0x2A)
                            newState = RMTESParseState.ESC_2A;
                        else if (currPtr.Contents[i] == 0x2B)
                            newState = RMTESParseState.ESC_2B;
                        else if (currPtr.Contents[i] == LBRKT_CHAR)
                            newState = RMTESParseState.LBRKT;
                        else if (currPtr.Contents[i] == 0x6E)
                        {
                            currentSet.GL = currentSet.G2;
                            _gL = 2;
                        }
                        else if (currPtr.Contents[i] == 0x6F)
                        {
                            currentSet.GL = currentSet.G3;
                            _gL = 3;
                        }
                        else if (currPtr.Contents[i] == 0x7E)
                        {
                            currentSet.GR = currentSet.G1;
                            _gR = 1;
                        }
                        else if (currPtr.Contents[i] == 0x7D)
                        {
                            currentSet.GR = currentSet.G2;
                            _gR = 2;
                        }
                        else if (currPtr.Contents[i] == 0x7C)
                        {
                            currentSet.GR = currentSet.G3;
                            _gR = 3;
                        }
                        else
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);

                        break;
                    case RMTESParseState.ESC_21:
                        if (currPtr.Contents[i] != 0x40) /* Refinitiv Ctrl 1 to CL */
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_22:
                        if (currPtr.Contents[i] != 0x30) /* Refinitiv Ctrl 2 to CR */
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_24:
                        if (currPtr.Contents[i] == 0x28)
                            newState = RMTESParseState.ESC_24_28;
                        else if (currPtr.Contents[i] == 0x29)
                            newState = RMTESParseState.ESC_24_29;
                        else if (currPtr.Contents[i] == 0x2A)
                            newState = RMTESParseState.ESC_24_2A;
                        else if (currPtr.Contents[i] == 0x2B)
                            newState = RMTESParseState.ESC_24_2B;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_24_28:
                        if (currPtr.Contents[i] == 0x47)
                            currentSet.G0 = _characterSet._rsslChinese1;
                        else if (currPtr.Contents[i] == 0x48)
                            currentSet.G0 = _characterSet._rsslChinese2;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_24_29:
                        if (currPtr.Contents[i] == 0x47)
                            currentSet.G1 = _characterSet._rsslChinese1;
                        else if (currPtr.Contents[i] == 0x48)
                            currentSet.G1 = _characterSet._rsslChinese2;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_24_2A:
                        if (currPtr.Contents[i] == 0x47)
                            currentSet.G2 = _characterSet._rsslChinese1;
                        else if (currPtr.Contents[i] == 0x35)
                            currentSet.G2 = _characterSet._rsslChinese1;
                        else if (currPtr.Contents[i] == 0x48)
                            currentSet.G2 = _characterSet._rsslChinese2;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_24_2B:
                        if (currPtr.Contents[i] == 0x47)
                            currentSet.G3 = _characterSet._rsslChinese1;
                        else if (currPtr.Contents[i] == 0x48)
                            currentSet.G3 = _characterSet._rsslChinese2;
                        else if (currPtr.Contents[i] == 0x36)
                            currentSet.G3 = _characterSet._rsslChinese2;
                        else if (currPtr.Contents[i] == 0x34)
                            currentSet.G3 = _characterSet._rsslJapaneseKanji;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_25:
                        if (currPtr.Contents[i] == 0x30)
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.UTF_ENC, currentSet, length, currPtr);
                        }
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                    case RMTESParseState.ESC_26:
                        if (currPtr.Contents[i] == 0x40)
                            newState = RMTESParseState.ESC_26_40;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_26_40:
                        if (currPtr.Contents[i] == ESC_CHAR)
                            newState = RMTESParseState.ESC_26_40_ESC;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_26_40_ESC:
                        if (currPtr.Contents[i] == 0x24)
                            newState = RMTESParseState.ESC_26_40_ESC_24;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_26_40_ESC_24:
                        if (currPtr.Contents[i] == 0x42)
                            currentSet.G0 = _characterSet._rsslJapaneseKanji;
                        else if (currPtr.Contents[i] == 0x29)
                            newState = RMTESParseState.ESC_26_40_ESC_24_29;
                        else if (currPtr.Contents[i] == 0x2A)
                            newState = RMTESParseState.ESC_26_40_ESC_24_2A;
                        else if (currPtr.Contents[i] == 0x2B)
                            newState = RMTESParseState.ESC_26_40_ESC_24_2B;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_26_40_ESC_24_29:
                        if (currPtr.Contents[i] == 0x42)
                            currentSet.G1 = _characterSet._rsslJapaneseKanji;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_26_40_ESC_24_2A:
                        if (currPtr.Contents[i] == 0x42)
                            currentSet.G2 = _characterSet._rsslJapaneseKanji;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_26_40_ESC_24_2B:
                        if (currPtr.Contents[i] == 0x42)
                            currentSet.G3 = _characterSet._rsslJapaneseKanji;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_28:
                        if (currPtr.Contents[i] == 0x42)
                            currentSet.G0 = _characterSet._rsslReuterBasic1;
                        else if (currPtr.Contents[i] == 0x49)
                            currentSet.G0 = _characterSet._rsslJapaneseKatakana;
                        else if (currPtr.Contents[i] == 0x4A)
                            currentSet.G0 = _characterSet._rsslJapaneseLatin;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_29:
                        if (currPtr.Contents[i] == 0x31)
                            currentSet.G1 = _characterSet._rsslReuterBasic2;
                        else if (currPtr.Contents[i] == 0x42)
                            currentSet.G1 = _characterSet._rsslReuterBasic1;
                        else if (currPtr.Contents[i] == 0x49)
                            currentSet.G1 = _characterSet._rsslJapaneseKatakana;
                        else if (currPtr.Contents[i] == 0x4A)
                            currentSet.G1 = _characterSet._rsslJapaneseLatin;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_2A:
                        if (currPtr.Contents[i] == 0x32)
                            currentSet.G2 = _characterSet._rsslJapaneseKatakana;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.ESC_2B:
                        if (currPtr.Contents[i] == 0x33)
                            currentSet.G3 = _characterSet._rsslJapaneseLatin;
                        else
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                        }
                        break;
                    case RMTESParseState.LBRKT:
                        while (currPtr.Contents[i] >= '0' && currPtr.Contents[i] <= '9')
                        {
                            ++i;
                            ++length;
                        }
                        if (currPtr.Contents[i] == RHPA_CHAR)
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.RHPA_CMD, currentSet, length, currPtr);
                        }
                        if (currPtr.Contents[i] == RREP_CHAR)
                        {
                            return _returnInfo.ReturnControlParse(ESCReturnCode.RREP_CMD, currentSet, length, currPtr);
                        }
                        break;

                    default:
                        return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                }
                state = newState;
            }
            while (i <= endPtr && newState != RMTESParseState.NORMAL);

            if (state == RMTESParseState.NORMAL)
            {
                switch (_gL)
                {
                    case 2:
                        currentSet.GL = currentSet.G2;
                        break;
                    case 3:
                        currentSet.GL = currentSet.G3;
                        break;
                    default:
                        break;
                }

                switch (_gR)
                {
                    case 1:
                        currentSet.GR = currentSet.G1;
                        break;
                    case 2:
                        currentSet.GR = currentSet.G2;
                        break;
                    case 3:
                        currentSet.GR = currentSet.G3;
                        break;
                    default:
                        break;
                }
                return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, length, currPtr);
            }
            else
            {
                return _returnInfo.ReturnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
            }
        }

        /// <summary>
        ///Converts the given cache to UCS2 Unicode. 
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Allocate memory for the cache buffer.</item>
        /// <item> After decoding the payload buffer, call <see cref="RMTESApplyToCache(Buffer, RmtesCacheBuffer)"/>
        ///    to copy the data to the RmtesCacheBuffer.</item>
        /// <item> Allocate memory for the unicode string.</item>
        /// <item> Call <see cref="RMTESToUCS2(RmtesBuffer, RmtesCacheBuffer)"/> to convert the RMTES data 
        ///    for display or parsing.</item>
        /// </list>
        /// </remarks>
        /// <param name="rmtesBuffer">Buffer used to store decoded RMTES data</param>
        /// <param name="cacheBuffer">Buffer containing encoded RMTES data</param>
        /// <returns><see cref="CodecReturnCode"/>
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode RMTESToUCS2(RmtesBuffer rmtesBuffer, RmtesCacheBuffer cacheBuffer)
        {
            RMTESParseState state = RMTESParseState.NORMAL;
            int inIterCount = 0;
            int outIterCount = 0;
            EncodeType encType = EncodeType.TYPE_RMTES;

            char tempChar;

            _shiftGL = null;
            _tmpGL = null;

            if (rmtesBuffer.AllocatedLength == 0)
                return CodecReturnCode.INVALID_ARGUMENT;
            if (cacheBuffer.Length == 0)
                return CodecReturnCode.NO_DATA;

            _gL = 0;
            _gR = 0;

            _characterSet.initWorkingSet(_curWorkingSet);

            _tempInfo.ESCRetCode = ESCReturnCode.ESC_SUCCESS;

            while (inIterCount < cacheBuffer.Length && state != RMTESParseState.ERROR)
            {
                if (encType == EncodeType.TYPE_RMTES)
                {
                    if (cacheBuffer.Data == null)
                        return CodecReturnCode.FAILURE;

                    if (((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF) < 0x20) // CL Character
                    {
                        if (_shiftGL != null)
                        {
                            return CodecReturnCode.FAILURE;
                        }
                        else if ((_tempInfo = ControlParse(cacheBuffer.Data, inIterCount,
                                cacheBuffer.Length, _curWorkingSet)).Value == 0)
                        {
                            _curWorkingSet = _tempInfo.Set;
                            if (_tempInfo.ESCRetCode == ESCReturnCode.ESC_ERROR)
                            {
                                return CodecReturnCode.FAILURE;
                            }
                            else if (_tempInfo.ESCRetCode == ESCReturnCode.END_CHAR)
                            {
                                rmtesBuffer.Length = outIterCount;
                                return CodecReturnCode.SUCCESS;
                            }
                            else if (_tempInfo.ESCRetCode == ESCReturnCode.ESC_SUCCESS)
                            {
                                if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                {
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                                }
                                rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0);
                                if (cacheBuffer.Data != null)
                                    rmtesBuffer.Data.WriteAt(outIterCount++, cacheBuffer.Data.Contents[inIterCount]);
                                else
                                    rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0);
                                inIterCount++;
                            }
                        }
                        else
                        {
                            _curWorkingSet = _tempInfo.Set;
                            if (_tempInfo.ESCRetCode == ESCReturnCode.RHPA_CMD || _tempInfo.ESCRetCode == ESCReturnCode.RREP_CMD)
                            {
                                for (int i = 0; i < _tempInfo.Value; i++)
                                {
                                    if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                    {
                                        return CodecReturnCode.BUFFER_TOO_SMALL;
                                    }
                                    rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0);
                                    if (cacheBuffer.Data != null)
                                        rmtesBuffer.Data.WriteAt(outIterCount++, cacheBuffer.Data.Contents[inIterCount]);
                                    else
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0);
                                    inIterCount++;
                                }
                            }
                            else
                            {
                                inIterCount += _tempInfo.Value;
                                if (_tempInfo.ESCRetCode == ESCReturnCode.UTF_ENC)
                                    encType = EncodeType.TYPE_UTF8;
                            }
                        }
                    }
                    else if (_curWorkingSet.GL.Shape == CharSet.SHAPE_94 &&
                             cacheBuffer.Data.Contents[inIterCount] == 0x20) /* Space character, if 94 character set */
                    {
                        if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0);
                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0x20);
                        inIterCount++;
                    }
                    else if (_curWorkingSet.GL.Shape == CharSet.SHAPE_94 &&
                             cacheBuffer.Data.Contents[inIterCount] == 0x7F) /* Delete character, if 94 character set */
                    {
                        if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0xFF);
                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0xFD);
                        inIterCount++;
                    }
                    else if (((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF) < 0x80) // GL Character
                    {
                        if (_shiftGL != null)
                            _tmpGL = _shiftGL;
                        else
                            _tmpGL = _curWorkingSet.GL;

                        if (IsGLChar((char)cacheBuffer.Data.Contents[inIterCount], _tmpGL))
                        {
                            if (_tmpGL.Stride == 2)
                            {
                                tempChar = (char)((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF);
                                if (inIterCount < cacheBuffer.Length)
                                {
                                    inIterCount++;
                                    if (IsGLChar((char)cacheBuffer.Data.Contents[inIterCount], _tmpGL))
                                    {
                                        char toPut = ConvertStride2GL(tempChar, (char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _tmpGL);
                                        if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                        {
                                            return CodecReturnCode.BUFFER_TOO_SMALL;
                                        }
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)((toPut & 0xFF00) >> 8));
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(toPut & 0x00FF));
                                    }
                                    else
                                    {
                                        return CodecReturnCode.FAILURE;
                                    }
                                }
                                else
                                {
                                    return CodecReturnCode.FAILURE;
                                }
                            }
                            else
                            {
                                char toPut = GLConvertSingle1((char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _tmpGL);
                                if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                {
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                                }
                                rmtesBuffer.Data.WriteAt(outIterCount++, (byte)((toPut & 0xFF00) >> 8));
                                rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(toPut & 0x00FF));

                                if (_tmpGL.Table2 != null)
                                {
                                    if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                    {
                                        return CodecReturnCode.BUFFER_TOO_SMALL;
                                    }

                                    tempChar = GLConvertSingle2((char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _tmpGL);

                                    if (tempChar != 0)
                                    {
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)((tempChar & 0xFF00) >> 8));
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(tempChar & 0x00FF));
                                    }
                                }
                            }
                        }
                        else
                        {
                            return CodecReturnCode.FAILURE;
                        }

                        inIterCount++;
                        _shiftGL = null;
                    }
                    else if (((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF) < 0xA0) // CR Character Set
                    {
                        if (_shiftGL != null)
                        {
                            return CodecReturnCode.FAILURE;
                        }

                        if (cacheBuffer.Data.Contents[inIterCount] == SS2_CHAR)
                            _shiftGL = _curWorkingSet.G2;
                        else if (cacheBuffer.Data.Contents[inIterCount] == SS3_CHAR)
                            _shiftGL = _curWorkingSet.G3;
                        else
                        {
                            if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0);
                            rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0xFD);
                        }

                        inIterCount++;
                    }
                    else
                    {
                        if (_shiftGL != null)
                        {
                            return CodecReturnCode.FAILURE;
                        }
                        else if (IsGRChar((char)cacheBuffer.Data.Contents[inIterCount], _curWorkingSet.GR))
                        {
                            if (_curWorkingSet.GR.Stride == 2)
                            {
                                tempChar = (char)((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF);
                                if (inIterCount < cacheBuffer.Length)
                                {
                                    inIterCount++;
                                    if (IsGRChar((char)cacheBuffer.Data.Contents[inIterCount], _curWorkingSet.GR))
                                    {
                                        if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                        {
                                            return CodecReturnCode.BUFFER_TOO_SMALL;
                                        }
                                        char toPut = ConvertStride2GR(tempChar, (char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _curWorkingSet.GR);
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)((toPut & 0xFF00) >> 8));
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(toPut & 0x00FF));
                                    }
                                }
                                else
                                {
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                                }
                            }
                            else
                            {
                                if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                {
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                                }
                                char toPut = GRConvertSingle1((char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _curWorkingSet.GR);
                                rmtesBuffer.Data.WriteAt(outIterCount++, (byte)((toPut & 0xFF00) >> 8));
                                rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(toPut & 0x00FF));

                                if (_curWorkingSet.GR.Table2 != null)
                                {
                                    tempChar = GRConvertSingle2((char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _curWorkingSet.GR);
                                    if (tempChar != 0)
                                    {
                                        if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                        {
                                            return CodecReturnCode.BUFFER_TOO_SMALL;
                                        }
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)((tempChar & 0xFF00) >> 8));
                                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(tempChar & 0x00FF));
                                    }
                                }
                            }
                        }
                        else
                        {
                            return CodecReturnCode.FAILURE;
                        }

                        inIterCount++;
                    }
                }
                else
                /* UTF8 Encode */
                {
                    if (cacheBuffer.Data != null && cacheBuffer.Data.Contents[inIterCount] == 0x1B) /* Escape control character */
                    {
                        _tempInfo = ControlParse(cacheBuffer.Data, inIterCount, cacheBuffer.Length, _curWorkingSet);
                        cacheBuffer.Data = _tempInfo.Iter;
                        _curWorkingSet = _tempInfo.Set;
                        if (_tempInfo.Value == 0)
                        {
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            if (_tempInfo.ESCRetCode == ESCReturnCode.RHPA_CMD || _tempInfo.ESCRetCode == ESCReturnCode.RREP_CMD)
                            {
                                for (int i = 0; i < _tempInfo.Value; i++)
                                {
                                    if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                                    {
                                        return CodecReturnCode.BUFFER_TOO_SMALL;
                                    }
                                    rmtesBuffer.Data.WriteAt(outIterCount++, cacheBuffer.Data.Contents[inIterCount++]);
                                }
                            }
                            else
                            {
                                if (_tempInfo.Value == 0)
                                    inIterCount++;
                                else
                                    inIterCount += _tempInfo.Value;
                            }
                        }
                    }
                    else if (cacheBuffer.Data != null && cacheBuffer.Data.Contents[inIterCount] == 0x00)
                    {
                        inIterCount++;
                    }
                    else
                    /* Just copy the data, since it's already encoded in UTF8 */
                    {
                        if (outIterCount + 2 > rmtesBuffer.AllocatedLength)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        char toConvert = (char)((((int)rmtesBuffer.Data.Contents[outIterCount]) << 8) | ((int)rmtesBuffer.Data.Contents[outIterCount + 1]));
                        _tempInfo = UTF8ToUCS2(cacheBuffer.Data, inIterCount, toConvert, cacheBuffer.Length);
                        if (_tempInfo.Value >= 3)
                        {
                            rmtesBuffer.Data.WriteAt(outIterCount++, (byte)((_tempInfo.CharValue & 0xFF00) >> 8));
                            rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(_tempInfo.CharValue & 0x00FF));
                        }
                        else if (_tempInfo.Value >= 2)
                        {
                            rmtesBuffer.Data.WriteAt(outIterCount++, (byte)((_tempInfo.CharValue & 0xFF00) >> 8));
                            rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(_tempInfo.CharValue & 0x00FF));
                        }
                        else
                        {
                            rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0);
                            rmtesBuffer.Data.WriteAt(outIterCount++, (byte)_tempInfo.CharValue);
                        }

                        if (_tempInfo.Value == 0)
                            inIterCount++;
                        else
                            inIterCount += _tempInfo.Value;
                    }
                }
            }

            /* Trim outer buffer length */
            rmtesBuffer.Length = outIterCount;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Converts the given cache to UTF8.
        /// </summary>
        /// <remarks>
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Allocate memory for the cache buffer.</item>
        /// <item> After decoding the payload buffer, call <see cref="RMTESApplyToCache(Buffer, RmtesCacheBuffer)"/>
        ///     to copy the data to the RmtesCacheBuffer.</item>
        /// <item> Allocate memory for the unicode string.</item>
        /// <item> Call <see cref="RMTESToUTF8(RmtesBuffer, RmtesCacheBuffer)"/> to convert the RMTES data for display or parsing.</item>
        /// </list>
        /// </remarks>
        /// <param name="rmtesBuffer">Buffer used to store decoded RMTES data</param>
        /// <param name="cacheBuffer">Buffer containing encoded RMTES data
        /// </param>
        /// <returns><c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode RMTESToUTF8(RmtesBuffer rmtesBuffer, RmtesCacheBuffer cacheBuffer)
        {
            RMTESParseState state = RMTESParseState.NORMAL;
            int inIterCount = 0;
            int outIterCount = 0;
            EncodeType encType = EncodeType.TYPE_RMTES;
            int ret;

            char tempChar;

            _shiftGL = null;
            _tmpGL = null;

            if (rmtesBuffer.AllocatedLength == 0)
                return CodecReturnCode.INVALID_ARGUMENT;
            if (cacheBuffer.Length == 0)
                return CodecReturnCode.NO_DATA;

            _gL = 0;
            _gR = 0;

            _characterSet.initWorkingSet(_curWorkingSet);

            _tempInfo.ESCRetCode = ESCReturnCode.ESC_SUCCESS;

            while (inIterCount < cacheBuffer.Length && state != RMTESParseState.ERROR)
            {
                if (encType == EncodeType.TYPE_RMTES)
                {
                    if (cacheBuffer.Data == null)
                        return CodecReturnCode.FAILURE;

                    if (((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF) < 0x20) // CL Character
                    {
                        if (_shiftGL != null)
                        {
                            return CodecReturnCode.FAILURE;
                        }
                        else if ((_tempInfo = ControlParse(cacheBuffer.Data, inIterCount,
                                cacheBuffer.Length, _curWorkingSet)).Value == 0)
                        {
                            _curWorkingSet = _tempInfo.Set;
                            if (_tempInfo.ESCRetCode == ESCReturnCode.ESC_ERROR)
                            {
                                return CodecReturnCode.FAILURE;
                            }
                            else if (_tempInfo.ESCRetCode == ESCReturnCode.END_CHAR)
                            {
                                rmtesBuffer.Length = outIterCount;
                                return CodecReturnCode.SUCCESS;
                            }
                            else if (_tempInfo.ESCRetCode == ESCReturnCode.ESC_SUCCESS)
                            {
                                if (outIterCount + 1 > rmtesBuffer.AllocatedLength)
                                {
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                                }
                                if (cacheBuffer.Data != null)
                                    rmtesBuffer.Data.WriteAt(outIterCount++, cacheBuffer.Data.Contents[inIterCount]);
                                else
                                    rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0);
                                inIterCount++;
                            }
                        }
                        else
                        {
                            _curWorkingSet = _tempInfo.Set;
                            if (_tempInfo.ESCRetCode == ESCReturnCode.RHPA_CMD || _tempInfo.ESCRetCode == ESCReturnCode.RREP_CMD)
                            {
                                for (int i = 0; i < _tempInfo.Value; i++)
                                {
                                    if (outIterCount + 1 > rmtesBuffer.AllocatedLength)
                                    {
                                        return CodecReturnCode.BUFFER_TOO_SMALL;
                                    }
                                    rmtesBuffer.Data.WriteAt(outIterCount++, cacheBuffer.Data.Contents[inIterCount++]);
                                }
                            }
                            else
                            {
                                inIterCount += _tempInfo.Value;
                                if (_tempInfo.ESCRetCode == ESCReturnCode.UTF_ENC)
                                    encType = EncodeType.TYPE_UTF8;
                            }                              
                        }
                    }
                    else if (_curWorkingSet.GL.Shape == CharSet.SHAPE_94 &&
                             cacheBuffer.Data.Contents[inIterCount] == 0x20) /* Space character, if 94 character set */
                    {
                        if (outIterCount + 1 > rmtesBuffer.AllocatedLength)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)0x20); // < 0x0080 , it is UTF8 already
                        inIterCount++;
                    }
                    else if (_curWorkingSet.GL.Shape == CharSet.SHAPE_94 &&
                             cacheBuffer.Data.Contents[inIterCount] == 0x7F) /* Delete character, if 94 character set */
                    {
                        if (outIterCount + 3 > rmtesBuffer.AllocatedLength) // 0xFFFD needs 3 bytes in UTF8
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(0x0E0 | (0xFFFD >> 12)));
                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(0x080 | ((0xFFFD >> 6) & 0x3F)));
                        rmtesBuffer.Data.WriteAt(outIterCount++, (byte)(0x080 | (0xFFFD & 0x3F)));
                        inIterCount++;
                    }
                    else if (((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF) < 0x80) // GL Character
                    {
                        if (_shiftGL != null)
                            _tmpGL = _shiftGL;
                        else
                            _tmpGL = _curWorkingSet.GL;

                        if (IsGLChar((char)cacheBuffer.Data.Contents[inIterCount], _tmpGL))
                        {
                            if (_tmpGL.Stride == 2)
                            {
                                tempChar = (char)((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF);
                                if (inIterCount < cacheBuffer.Length)
                                {
                                    inIterCount++;
                                    if (IsGLChar((char)cacheBuffer.Data.Contents[inIterCount], _tmpGL))
                                    {
                                        char toPut = ConvertStride2GL(tempChar, (char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _tmpGL);
                                        ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                                        if (ret < 0)
                                            return CodecReturnCode.BUFFER_TOO_SMALL;
                                        else
                                            outIterCount += ret;
                                    }
                                    else
                                    {
                                        return CodecReturnCode.FAILURE;
                                    }
                                }
                                else
                                {
                                    return CodecReturnCode.FAILURE;
                                }
                            }
                            else
                            {
                                char toPut = GLConvertSingle1((char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _tmpGL);
                                ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                                if (ret < 0)
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                                else
                                    outIterCount += ret;

                                if (_tmpGL.Table2 != null)
                                {
                                    tempChar = GLConvertSingle2((char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _tmpGL);

                                    if (tempChar != 0)
                                    {
                                        ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                                        if (ret < 0)
                                            return CodecReturnCode.BUFFER_TOO_SMALL;
                                        else
                                            outIterCount += ret;
                                    }
                                }
                            }
                        }
                        else
                        {
                            return CodecReturnCode.FAILURE;
                        }

                        inIterCount++;
                        _shiftGL = null;
                    }
                    else if (((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF) < 0xA0) // CR Character Set
                    {
                        if (_shiftGL != null)
                        {
                            return CodecReturnCode.FAILURE;
                        }

                        if (cacheBuffer.Data.Contents[inIterCount] == SS2_CHAR)
                            _shiftGL = _curWorkingSet.G2;
                        else if (cacheBuffer.Data.Contents[inIterCount] == SS3_CHAR)
                            _shiftGL = _curWorkingSet.G3;
                        else
                        {
                            if (outIterCount + 1 > rmtesBuffer.AllocatedLength)
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            rmtesBuffer.Data.WriteAt(outIterCount++, 0xFD); //TODO: CHECK that this is the right way to rewrite the cast
                        }

                        inIterCount++;
                    }
                    else
                    {
                        if (_shiftGL != null)
                        {
                            return CodecReturnCode.FAILURE;
                        }
                        else if (IsGRChar((char)cacheBuffer.Data.Contents[inIterCount], _curWorkingSet.GR))
                        {
                            if (_curWorkingSet.GR.Stride == 2)
                            {
                                tempChar = (char)((char)cacheBuffer.Data.Contents[inIterCount] & 0xFF);
                                if (inIterCount < cacheBuffer.Length)
                                {
                                    inIterCount++;
                                    if (IsGRChar((char)cacheBuffer.Data.Contents[inIterCount], _curWorkingSet.GR))
                                    {
                                        char toPut = ConvertStride2GR(tempChar, (char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _curWorkingSet.GR);
                                        ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                                        if (ret < 0)
                                            return CodecReturnCode.BUFFER_TOO_SMALL;
                                        else
                                            outIterCount += ret;
                                    }
                                }
                                else
                                {
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                                }
                            }
                            else
                            {
                                char toPut = GRConvertSingle1((char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _curWorkingSet.GR);
                                ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                                if (ret < 0)
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                                else
                                    outIterCount += ret;

                                if (_curWorkingSet.GR.Table2 != null)
                                {
                                    tempChar = GRConvertSingle2((char)(cacheBuffer.Data.Contents[inIterCount] & 0xFF), _curWorkingSet.GR);
                                    if (tempChar != 0)
                                    {
                                        ret = UCS2ToUTF8(rmtesBuffer, tempChar, outIterCount);

                                        if (ret < 0)
                                            return CodecReturnCode.BUFFER_TOO_SMALL;
                                        else
                                            outIterCount += ret;
                                    }
                                }
                            }
                        }
                        else
                        {
                            return CodecReturnCode.FAILURE;
                        }

                        inIterCount++;
                    }
                }
                else

                /* UTF8 Encode */
                {
                    if (cacheBuffer.Data != null && cacheBuffer.Data.Contents[inIterCount] == 0x1B) /* Escape control character */
                    {
                        _tempInfo = ControlParse(cacheBuffer.Data, inIterCount, cacheBuffer.Length, _curWorkingSet);
                        cacheBuffer.Data = _tempInfo.Iter;
                        _curWorkingSet = _tempInfo.Set;
                        if (_tempInfo.Value < 0)
                        {
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            if (_tempInfo.ESCRetCode == ESCReturnCode.RREP_CMD || _tempInfo.ESCRetCode == ESCReturnCode.RHPA_CMD)
                            {
                                for (int i = 0; i < _tempInfo.Value; i++)
                                {
                                    if (outIterCount + 1 > rmtesBuffer.AllocatedLength)
                                    {
                                        return CodecReturnCode.BUFFER_TOO_SMALL;
                                    }
                                    rmtesBuffer.Data.WriteAt(outIterCount++, cacheBuffer.Data.Contents[inIterCount++]);
                                }
                            }
                            else
                            {
                                if (_tempInfo.Value == 0)
                                    inIterCount++;
                                else
                                    inIterCount += _tempInfo.Value;
                            }
                        }
                    }
                    else if (cacheBuffer.Data != null && cacheBuffer.Data.Contents[inIterCount] == 0x00)
                    {
                        inIterCount++;
                    }
                    else
                    /* Just copy the data, since it's already encoded in UTF8 */
                    {
                        if (outIterCount + 1 > rmtesBuffer.AllocatedLength)
                            return CodecReturnCode.BUFFER_TOO_SMALL;

                        rmtesBuffer.Data.WriteAt(outIterCount++, rmtesBuffer.Data.Contents[outIterCount]);
                        inIterCount++;
                    }
                }
            }

            /* Trim outer buffer length */
            rmtesBuffer.Length = outIterCount;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Handles the following cases ( Same as ETAC ):
        /// <list type="number">
        /// <item> ESC LBRKT
        ///   NumericNumber(base 10) RHPA - Cursor command</item>
        /// <item> ESC LBRKT
        ///   NumericNumber(base 10) RREP - Repeat command for previous character</item>
        /// </list>
        /// </summary>
        /// <param name="buffer">Buffer holding RMTES data</param>
        /// <returns><c>true</c> if the input buffer contains partial update, <c>false</c> otherwise
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Boolean HasPartialRMTESUpdate(Buffer buffer)
        {
            int numCount = 0;
            RMTESParseState state = RMTESParseState.NORMAL;

            for (int i = buffer.Position; i < buffer.GetLength() + buffer.Position; ++i)
            {
                switch (state)
                {
                    case RMTESParseState.NORMAL:
                        if (buffer.Data().Contents[i] == ESC_CHAR)
                            state = RMTESParseState.ESC;
                        break;
                    case RMTESParseState.ESC:
                        if (buffer.Data().Contents[i] == LBRKT_CHAR)
                            state = RMTESParseState.LBRKT;
                        else if (buffer.Data().Contents[i] == 0x30) // ESC_25
                            return false; // Error. UTF8 sequence should not be before command
                        else
                            state = RMTESParseState.NORMAL;
                        break;
                    case RMTESParseState.LBRKT:
                        if (buffer.Data().Contents[i] >= '0' && buffer.Data().Contents[i] <= '9')
                            numCount += numCount * 10 + buffer.Data().Contents[i] - '0';
                        else if (buffer.Data().Contents[i] == RHPA_CHAR || buffer.Data().Contents[i] == RREP_CHAR)
                            return true;
                        else
                            return false; // Error
                        break;
                    default:
                        return false;
                }
            }

            return false;
        }

        /// <summary>
        /// Applies the _inBuffer's partial update data to the outBuffer.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Preconditions: outBuffer is large enough to handle the additional data,
        /// outBuffer has already been populated with data</para>
        /// <para>
        /// Result: _inBuffer's partial update(s) are applied to outBuffer</para>
        /// </remarks>
        /// <param name="fEntry">inBuffer Buffer containing encoded RMTES data</param>
        /// <param name="cacheBuffer">cacheBuffer Buffer used to store encoded RMTES data and for 
        ///     finding any partial RMTES updates
        /// </param>
        /// <returns><c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode RMTESApplyToCache(Buffer fEntry, RmtesCacheBuffer cacheBuffer)
        {
            int inBufPos = fEntry.Position;
            int cacheBufferPos = 0;
            int numCount = 0;
            byte prevByte = 0;
            Boolean escPresent = false;
            RMTESParseState state = RMTESParseState.NORMAL;
            int maxLen = 0;


            if (fEntry.Length == 0)
                return CodecReturnCode.INVALID_ARGUMENT;
            if (cacheBuffer.AllocatedLength == 0)
                return CodecReturnCode.BUFFER_TOO_SMALL;

            while (inBufPos < (fEntry.GetLength() + fEntry.Position))
            {
                switch (state)
                {
                    case RMTESParseState.NORMAL:
                        if (fEntry.Data().Contents[inBufPos] == ESC_CHAR)
                        {
                            state = RMTESParseState.ESC;
                            numCount = 0;
                        }
                        else
                        {
                            if (cacheBufferPos >= cacheBuffer.AllocatedLength)
                            {
                                return CodecReturnCode.FAILURE;
                            }
                            prevByte = fEntry.Data().Contents[inBufPos];
                            cacheBuffer.Data.WriteAt(cacheBufferPos++, prevByte);
                        }
                        break;
                    case RMTESParseState.ESC:
                        if (fEntry.Data().Contents[inBufPos] == LBRKT_CHAR)
                            state = RMTESParseState.LBRKT;
                        else if (fEntry.Data().Contents[inBufPos] == 0x25)
                        {
                            inBufPos++;
                            if (inBufPos < (fEntry.Length + fEntry.Position) && fEntry.Data().Contents[inBufPos] == 0x30)
                            {
                                if (cacheBufferPos + 3 > cacheBuffer.AllocatedLength)
                                {
                                    /* Error: Out of space */
                                    return CodecReturnCode.FAILURE;
                                }
                                cacheBuffer.Data.WriteAt(cacheBufferPos++, (byte)ESC_CHAR);
                                cacheBuffer.Data.WriteAt(cacheBufferPos++, (byte)0x25);
                                cacheBuffer.Data.WriteAt(cacheBufferPos++, (byte)0x30);
                                state = RMTESParseState.NORMAL;
                            }
                            else
                            {
                                /* Error */
                                return CodecReturnCode.FAILURE;
                            }
                        }
                        else
                        {
                            /* normal escape code, print ESC character to buffer and continue */
                            state = RMTESParseState.NORMAL;
                            if (cacheBufferPos + 2 > cacheBuffer.AllocatedLength)
                            {
                                return CodecReturnCode.FAILURE;
                            }
                            cacheBuffer.Data.WriteAt(cacheBufferPos++, (byte)ESC_CHAR);
                            cacheBuffer.Data.WriteAt(cacheBufferPos++, fEntry.Data().Contents[inBufPos]);
                        }
                        break;
                    case RMTESParseState.LBRKT:
                        if (fEntry.Data().Contents[inBufPos] >= '0' && fEntry.Data().Contents[inBufPos] <= '9')
                        {
                            numCount = numCount * 10 + fEntry.Data().Contents[inBufPos] - '0';
                        }
                        else if (fEntry.Data().Contents[inBufPos] == RHPA_CHAR)
                        {
                            /* Move cursor command */
                            /* Escape command is completed, flag as true */
                            escPresent = true;

                            if (numCount >= 0)
                            {
                                cacheBufferPos = numCount;
                                numCount = 0;
                                state = RMTESParseState.NORMAL;

                                if (cacheBufferPos > cacheBuffer.AllocatedLength)
                                    return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            else
                            {
                                return CodecReturnCode.FAILURE;
                            }
                        }
                        else if (fEntry.Data().Contents[inBufPos] == RREP_CHAR)
                        {
                            /* Repeat character command. This is always 1 char */
                            /* Check for overrun first */
                            /* Escape command completed */
                            escPresent = true;

                            if (cacheBuffer.AllocatedLength < cacheBufferPos + numCount)
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            for (int i = 0; i < numCount; i++)
                            {
                                cacheBuffer.Data = cacheBuffer.Data.WriteAt(cacheBufferPos++, prevByte);
                            }
                            state = RMTESParseState.NORMAL;
                        }
                        else
                            return CodecReturnCode.FAILURE;
                        break;
                    default:
                        return CodecReturnCode.FAILURE;
                }
                inBufPos++;
                if (cacheBufferPos > maxLen)
                    maxLen = cacheBufferPos;
            }

            /* Check that we're not in a weird state */
            if (state != RMTESParseState.NORMAL)
                return CodecReturnCode.FAILURE;

            if (escPresent == true)
            {
                if (maxLen > cacheBuffer.Length)
                {
                    cacheBuffer.Length = maxLen;
                }
                else
                    return CodecReturnCode.SUCCESS;
            }
            else
            {
                cacheBuffer.Length = maxLen;
            }
            return CodecReturnCode.SUCCESS;
        }
    }
}
