/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.CharSet.RmtesCharSet;
import com.refinitiv.eta.codec.CharSet.RmtesWorkingSet;

class RmtesDecoderImpl implements RmtesDecoder
{
    static char ESC_CHAR = 0x1B;
    static char LBRKT_CHAR = 0x5B;
    static char RHPA_CHAR = 0x60; // Used for partial updates
    static char RREP_CHAR = 0x62; // Repeat command
    static char CSI_CHAR = 0x9B;
    static char LS0_CHAR = 0x0F;
    static char LS1_CHAR = 0x0E;
    static char SS2_CHAR = 0x8E;
    static char SS3_CHAR = 0x8F;

    private CharSet _characterSet = new CharSet();
    private RmtesWorkingSet _curWorkingSet = new RmtesWorkingSet();
    private RmtesCharSet _shiftGL = null;
    private RmtesCharSet _tmpGL = null;
    private byte _gL;
    private byte _gR;

    private RmtesInfo _tempInfo = new RmtesInfo();
    private RmtesInfo _returnInfo = new RmtesInfo();

    int GLLowest(RmtesCharSet set)
    {
        return ((set.get_shape() == CharSet.SHAPE_96) ? 0x20 : 0x21);
    }

    int GLHighest(RmtesCharSet set)
    {
        return ((set.get_shape() == CharSet.SHAPE_96) ? 0x7F : 0x7E);
    }

    int GRLowest(RmtesCharSet set)
    {
        return ((set.get_shape() == CharSet.SHAPE_96) ? 0xA0 : 0xA1);
    }

    int GRHighest(RmtesCharSet set)
    {
        return ((set.get_shape() == CharSet.SHAPE_96) ? 0xFF : 0xFE);
    }

    boolean isGLChar(char c, RmtesCharSet set)
    {
        return ((GLLowest(set) <= (c & 0xFF)) && ((c & 0xFF) <= GLHighest(set)));
    }

    boolean isGRChar(char c, RmtesCharSet set)
    {
        return ((GRLowest(set) <= (c & 0xFF)) && ((c & 0xFF) <= GRHighest(set)));
    }

    /* Used to get the UCS2 encoded character from the first table.
     * Precondition: set is a singleton character table(not a _stride of 2)
     * inChar is a valid character(checked with isGRChar/isGLChar
     */
    char GLConvertSingle1(char inChar, RmtesCharSet set)
    {
        if (set.get_table1() != null)
            return (char)set.get_table1()[inChar - GLLowest(set)];
        else
            return inChar;
    }

    /* Used to get the UCS2 encoded character from the second table.
     * Precondition: set is a singleton character table(not a _stride of 2)
     * inChar is a valid character(checked with isGRChar/isGLChar _table2 exists in the set
     */
    char GLConvertSingle2(char inChar, RmtesCharSet set)
    {
        return (char)set.get_table2()[inChar - GLLowest(set)];
    }

    /* Used to get the UCS2 encoded character from the first table.
     * Precondition: set is a singleton character table(not a _stride of 2)
     * inChar is a valid character(checked with isGRChar/isGLChar
     */
    char GRConvertSingle1(char inChar, RmtesCharSet set)
    {
        if (set.get_table1() != null && (inChar - GRLowest(set) < set.get_table1_length()))
            return (char)set.get_table1()[inChar - GRLowest(set)];
        else
            return inChar;
    }

    /* Used to get the UCS2 encoded character from the second table.
     * Precondition: set is a singleton character table(not a _stride of 2)
     * inChar is a valid character(checked with isGRChar/isGLChar _table2 exists in the set
     */
    char GRConvertSingle2(char inChar, RmtesCharSet set)
    {
        if (set.get_table2() != null)
            return (char)set.get_table2()[inChar - GRLowest(set)];
        else
            return inChar;
    }

    char ConvertStride2GL(char inChar1, char inChar2, RmtesCharSet set)
    {
        int mapIndex;

        mapIndex = (inChar1 - GLLowest(set)) * (94) + (inChar2 - GLLowest(set));

        if (mapIndex < set.get_table1_length())
        {
            return (char)set.get_table1()[mapIndex];
        }
        else if (set.get_table2() != null && mapIndex >= set.get_table2_start()
                 && mapIndex < (set.get_table2_start() + set.get_table2_length()))
        {
            return (char)set.get_table2()[mapIndex - set.get_table2_start()];
        }
        else
            return (char)0xFFFD;
    }

    char ConvertStride2GR(char inChar1, char inChar2, RmtesCharSet set)
    {
        int mapIndex;

        mapIndex = (inChar1 - GRLowest(set)) * (94) + (inChar2 - GRLowest(set));

        if (mapIndex < set.get_table1_length())
        {
            return (char)set.get_table1()[mapIndex];
        }
        else if (set.get_table2() != null && mapIndex >= set.get_table2_start()
                 && mapIndex < (set.get_table2_start() + set.get_table2_length()))
        {
            return (char)set.get_table2()[mapIndex - set.get_table2_start()];
        }
        else
            return (char)0xFFFD;
    }

    int byteToUnsigned(byte input)
    {
        int output = 0xFFFFFFFF;
        output = input & 0x000000FF;
        if (output >= 255)
            return output;
        return output;
    }

    RmtesInfo UTF8ToUCS2(ByteBuffer data, int i, char c, int endInput)
    {
        if (data == null)
            return _returnInfo.returnUTF8ToUCS2(0, c);

        int unsignedByte1 = byteToUnsigned(data.get(i));

        if (unsignedByte1 < 0x80)
        {
            c = (char)unsignedByte1;
            return _returnInfo.returnUTF8ToUCS2(1, c);
        }
        else if (((unsignedByte1 & 0xE0) == 0xE0))
        {
            int unsignedByte2 = byteToUnsigned(data.get(i + 1));
            int unsignedByte3 = byteToUnsigned(data.get(i + 2));
            if (i + 2 <= endInput)
            {
                if (unsignedByte2 == 0)
                {
                    return _returnInfo.returnUTF8ToUCS2(-1, c);
                }

                c = (char)((unsignedByte1 & 0x0F) << 12 | (unsignedByte2 & 0x3F) << 6 | (unsignedByte3 & 0x3F));

                return _returnInfo.returnUTF8ToUCS2(3, c);
            }
        }
        else if (((data.get(i) & 0xC0) == 0xC0))
        {
            int unsignedByte2 = byteToUnsigned(data.get(i + 1));
            if (i + 1 <= endInput)
            {
                if (unsignedByte2 == 0)
                {
                    return _returnInfo.returnUTF8ToUCS2(0, c);
                }

                c = (char)((unsignedByte1 & 0x1F) << 6 | (unsignedByte2 & 0x3F));

                return _returnInfo.returnUTF8ToUCS2(2, c);
            }
        }

        return _returnInfo.returnUTF8ToUCS2(0, c);
    }

    /**
     * Control parse.
     *
     * @param currPtr the curr ptr
     * @param i the i
     * @param endPtr the end ptr
     * @param currentSet the current set
     * @return the rmtes info
     */
    /* Parses the characters for a control group sequence for conversion
     * (first character is between 0x00 and 0x1F(CL) or between 0x70 and 0x8F(CR) )
     * If error returned, this means that the sequence is either invalid,
     * or contains a partial update/repeat character sequence
     * (these should be removed from the buffer with the applyToCache function).
     * Otherwise, returns codes for success if a working set change is correctly applied,
     * or that the next character is a shift value.
     */
    public RmtesInfo controlParse(ByteBuffer currPtr, int i, int endPtr, RmtesWorkingSet currentSet)
    {
        int state = RMTESParseState.NORMAL;
        int newState = RMTESParseState.NORMAL;
        int length = 0;

        _gL = 0;
        _gR = 0;

        if (currPtr.get(i) == 0x00) /* ignore NULL characters for this parse */
        {
            return _returnInfo.returnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, 1, currPtr);
        }
        else if (currPtr.get(i) == 0x0F)
        {
            currentSet.GL = currentSet.G0;
            return _returnInfo.returnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, 1, currPtr);
        }
        else if (currPtr.get(i) == 0x0E)
        {
            currentSet.GL = currentSet.G1;
            return _returnInfo.returnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, 1, currPtr);
        }
        else if (currPtr.get(i) == 0x1B)
        {
            state = RMTESParseState.ESC;
            length++;
        }
        else if (currPtr.get(i) == 0x1C)
        {
            return _returnInfo.returnControlParse(ESCReturnCode.END_CHAR, currentSet, 0, currPtr);
        }
        else
            return _returnInfo
                    .returnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, 0, currPtr);

        do
        {
            i++;
            length++;

            newState = RMTESParseState.NORMAL;
            switch (state)
            {
                case RMTESParseState.ESC:
                    if (currPtr.get(i) == 0x21)
                        newState = RMTESParseState.ESC_21;
                    else if (currPtr.get(i) == 0x22)
                        newState = RMTESParseState.ESC_22;
                    else if (currPtr.get(i) == 0x24)
                        newState = RMTESParseState.ESC_24;
                    else if (currPtr.get(i) == 0x25)
                        newState = RMTESParseState.ESC_25;
                    else if (currPtr.get(i) == 0x26)
                        newState = RMTESParseState.ESC_26;
                    else if (currPtr.get(i) == 0x28)
                        newState = RMTESParseState.ESC_28;
                    else if (currPtr.get(i) == 0x29)
                        newState = RMTESParseState.ESC_29;
                    else if (currPtr.get(i) == 0x2A)
                        newState = RMTESParseState.ESC_2A;
                    else if (currPtr.get(i) == 0x2B)
                        newState = RMTESParseState.ESC_2B;
                    else if (currPtr.get(i) == LBRKT_CHAR)
                        newState = RMTESParseState.LBRKT;
                    else if (currPtr.get(i) == 0x6E)
                    {
                        currentSet.GL = currentSet.G2;
                        _gL = 2;
                    }
                    else if (currPtr.get(i) == 0x6F)
                    {
                        currentSet.GL = currentSet.G3;
                        _gL = 3;
                    }
                    else if (currPtr.get(i) == 0x7E)
                    {
                        currentSet.GR = currentSet.G1;
                        _gR = 1;
                    }
                    else if (currPtr.get(i) == 0x7D)
                    {
                        currentSet.GR = currentSet.G2;
                        _gR = 2;
                    }
                    else if (currPtr.get(i) == 0x7C)
                    {
                        currentSet.GR = currentSet.G3;
                        _gR = 3;
                    }
                    else
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);

                    break;
                case RMTESParseState.ESC_21:
                    if (currPtr.get(i) != 0x40) /* Refinitiv Ctrl 1 to CL */
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_22:
                    if (currPtr.get(i) != 0x30) /* Refinitiv Ctrl 2 to CR */
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_24:
                    if (currPtr.get(i) == 0x28)
                        newState = RMTESParseState.ESC_24_28;
                    else if (currPtr.get(i) == 0x29)
                        newState = RMTESParseState.ESC_24_29;
                    else if (currPtr.get(i) == 0x2A)
                        newState = RMTESParseState.ESC_24_2A;
                    else if (currPtr.get(i) == 0x2B)
                        newState = RMTESParseState.ESC_24_2B;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_24_28:
                    if (currPtr.get(i) == 0x47)
                        currentSet.G0 = _characterSet._rsslChinese1;
                    else if (currPtr.get(i) == 0x48)
                        currentSet.G0 = _characterSet._rsslChinese2;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_24_29:
                    if (currPtr.get(i) == 0x47)
                        currentSet.G1 = _characterSet._rsslChinese1;
                    else if (currPtr.get(i) == 0x48)
                        currentSet.G1 = _characterSet._rsslChinese2;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_24_2A:
                    if (currPtr.get(i) == 0x47)
                        currentSet.G2 = _characterSet._rsslChinese1;
                    else if (currPtr.get(i) == 0x35)
                        currentSet.G2 = _characterSet._rsslChinese1;
                    else if (currPtr.get(i) == 0x48)
                        currentSet.G2 = _characterSet._rsslChinese2;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_24_2B:
                    if (currPtr.get(i) == 0x47)
                        currentSet.G3 = _characterSet._rsslChinese1;
                    else if (currPtr.get(i) == 0x48)
                        currentSet.G3 = _characterSet._rsslChinese2;
                    else if (currPtr.get(i) == 0x36)
                        currentSet.G3 = _characterSet._rsslChinese2;
                    else if (currPtr.get(i) == 0x34)
                        currentSet.G3 = _characterSet._rsslJapaneseKanji;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_25:
                    if (currPtr.get(i) == 0x30)
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.UTF_ENC, currentSet, length, currPtr);
                    }
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                case RMTESParseState.ESC_26:
                    if (currPtr.get(i) == 0x40)
                        newState = RMTESParseState.ESC_26_40;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_26_40:
                    if (currPtr.get(i) == ESC_CHAR)
                        newState = RMTESParseState.ESC_26_40_ESC;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_26_40_ESC:
                    if (currPtr.get(i) == 0x24)
                        newState = RMTESParseState.ESC_26_40_ESC_24;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_26_40_ESC_24:
                    if (currPtr.get(i) == 0x42)
                        currentSet.G0 = _characterSet._rsslJapaneseKanji;
                    else if (currPtr.get(i) == 0x29)
                        newState = RMTESParseState.ESC_26_40_ESC_24_29;
                    else if (currPtr.get(i) == 0x2A)
                        newState = RMTESParseState.ESC_26_40_ESC_24_2A;
                    else if (currPtr.get(i) == 0x2B)
                        newState = RMTESParseState.ESC_26_40_ESC_24_2B;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_26_40_ESC_24_29:
                    if (currPtr.get(i) == 0x42)
                        currentSet.G1 = _characterSet._rsslJapaneseKanji;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_26_40_ESC_24_2A:
                    if (currPtr.get(i) == 0x42)
                        currentSet.G2 = _characterSet._rsslJapaneseKanji;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_26_40_ESC_24_2B:
                    if (currPtr.get(i) == 0x42)
                        currentSet.G3 = _characterSet._rsslJapaneseKanji;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_28:
                    if (currPtr.get(i) == 0x42)
                        currentSet.G0 = _characterSet._rsslReuterBasic1;
                    else if (currPtr.get(i) == 0x49)
                        currentSet.G0 = _characterSet._rsslJapaneseKatakana;
                    else if (currPtr.get(i) == 0x4A)
                        currentSet.G0 = _characterSet._rsslJapaneseLatin;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_29:
                    if (currPtr.get(i) == 0x31)
                        currentSet.G1 = _characterSet._rsslReuterBasic2;
                    else if (currPtr.get(i) == 0x42)
                        currentSet.G1 = _characterSet._rsslReuterBasic1;
                    else if (currPtr.get(i) == 0x49)
                        currentSet.G1 = _characterSet._rsslJapaneseKatakana;
                    else if (currPtr.get(i) == 0x4A)
                        currentSet.G1 = _characterSet._rsslJapaneseLatin;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_2A:
                    if (currPtr.get(i) == 0x32)
                        currentSet.G2 = _characterSet._rsslJapaneseKatakana;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                case RMTESParseState.ESC_2B:
                    if (currPtr.get(i) == 0x33)
                        currentSet.G3 = _characterSet._rsslJapaneseLatin;
                    else
                    {
                        return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
                    }
                    break;
                default:
                    return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
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
            return _returnInfo.returnControlParse(ESCReturnCode.ESC_SUCCESS, currentSet, length, currPtr);
        }
        else
        {
            return _returnInfo.returnControlParse(ESCReturnCode.ESC_ERROR, currentSet, 0, currPtr);
        }
    }

    public int RMTESToUCS2(RmtesBuffer rmtesBuffer, RmtesCacheBuffer cacheBuffer)
    {
        int state = RMTESParseState.NORMAL;
        int inIterCount = 0;
        int outIterCount = 0;
        int encType = EncodeType.TYPE_RMTES;

        char tempChar;

        _shiftGL = null;
        _tmpGL = null;

        if (rmtesBuffer.allocatedLength() == 0)
            return CodecReturnCodes.INVALID_ARGUMENT;
        if (cacheBuffer.length() == 0)
            return CodecReturnCodes.NO_DATA;

        _gL = 0;
        _gR = 0;

        _characterSet.initWorkingSet(_curWorkingSet);

        _tempInfo.setRetCode(ESCReturnCode.ESC_SUCCESS);

        while (inIterCount < cacheBuffer.length() && state != RMTESParseState.ERROR)
        {
            if (encType == EncodeType.TYPE_RMTES)
            {
                if (cacheBuffer.byteData() == null)
                    return CodecReturnCodes.FAILURE;

                if (((char)cacheBuffer.byteData().get(inIterCount) & 0xFF) < 0x20) // CL Character
                {
                    if (_shiftGL != null)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                    else if ((_tempInfo = controlParse(cacheBuffer.byteData(), inIterCount,
                            cacheBuffer.length(), _curWorkingSet)).getValue() == 0)
                    {
                        _curWorkingSet = _tempInfo.getSet();
                        if (_tempInfo.getRetCode() == ESCReturnCode.ESC_ERROR)
                        {
                            return CodecReturnCodes.FAILURE;
                        }
                        else if (_tempInfo.getRetCode() == ESCReturnCode.END_CHAR)
                        {
                            rmtesBuffer.length(outIterCount);
                            return CodecReturnCodes.SUCCESS;
                        }
                        else if (_tempInfo.getRetCode() == ESCReturnCode.ESC_SUCCESS)
                        {
                            if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                            {
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                            }
                            rmtesBuffer.byteData().put(outIterCount++, (byte)0);
                            if (cacheBuffer.byteData() != null)
                                rmtesBuffer.byteData().put(outIterCount++, cacheBuffer.byteData().get(inIterCount));
                            else
                                rmtesBuffer.byteData().put(outIterCount++, (byte)0);
                            inIterCount++;
                        }
                    }
                    else
                    {
                        _curWorkingSet = _tempInfo.getSet();
                        inIterCount += _tempInfo.getValue();
                        if (_tempInfo.getRetCode() == ESCReturnCode.UTF_ENC)
                            encType = EncodeType.TYPE_UTF8;
                    }
                }
                else if (_curWorkingSet.GL.get_shape() == CharSet.SHAPE_94 &&
                         cacheBuffer.byteData().get(inIterCount) == 0x20) /* Space character, if 94 character set */
                {
                    if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                    {
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    rmtesBuffer.byteData().put(outIterCount++, (byte)0);
                    rmtesBuffer.byteData().put(outIterCount++, (byte)0x20);
                    inIterCount++;
                }
                else if (_curWorkingSet.GL.get_shape() == CharSet.SHAPE_94 &&
                         cacheBuffer.byteData().get(inIterCount) == 0x7F) /* Delete character, if 94 character set */
                {
                    if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                    {
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    rmtesBuffer.byteData().put(outIterCount++, (byte)0xFF);
                    rmtesBuffer.byteData().put(outIterCount++, (byte)0xFD);
                    inIterCount++;
                }
                else if (((char)cacheBuffer.byteData().get(inIterCount) & 0xFF) < 0x80) // GL Character
                {
                    if (_shiftGL != null)
                        _tmpGL = _shiftGL;
                    else
                        _tmpGL = _curWorkingSet.GL;

                    if (isGLChar((char)cacheBuffer.byteData().get(inIterCount), _tmpGL))
                    {
                        if (_tmpGL.get_stride() == 2)
                        {
                            tempChar = (char)((char)cacheBuffer.byteData().get(inIterCount) & 0xFF);
                            if (inIterCount < cacheBuffer.length())
                            {
                                inIterCount++;
                                if (isGLChar((char)cacheBuffer.byteData().get(inIterCount), _tmpGL))
                                {
                                    char toPut = ConvertStride2GL(tempChar, (char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _tmpGL);
                                    if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                                    {
                                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                                    }
                                    rmtesBuffer.byteData().put(outIterCount++, (byte)((toPut & 0xFF00) >> 8));
                                    rmtesBuffer.byteData().put(outIterCount++, (byte)(toPut & 0x00FF));
                                }
                                else
                                {
                                    return CodecReturnCodes.FAILURE;
                                }
                            }
                            else
                            {
                                return CodecReturnCodes.FAILURE;
                            }
                        }
                        else
                        {
                            char toPut = GLConvertSingle1((char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _tmpGL);
                            if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                            {
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                            }
                            rmtesBuffer.byteData().put(outIterCount++, (byte)((toPut & 0xFF00) >> 8));
                            rmtesBuffer.byteData().put(outIterCount++, (byte)(toPut & 0x00FF));

                            if (_tmpGL.get_table2() != null)
                            {
                                if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                                {
                                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                                }

                                tempChar = GLConvertSingle2((char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _tmpGL);

                                if (tempChar != 0)
                                {
                                    rmtesBuffer.byteData().put(outIterCount++, (byte)((tempChar & 0xFF00) >> 8));
                                    rmtesBuffer.byteData().put(outIterCount++, (byte)(tempChar & 0x00FF));
                                }
                            }
                        }
                    }
                    else
                    {
                        return CodecReturnCodes.FAILURE;
                    }

                    inIterCount++;
                    _shiftGL = null;
                }
                else if (((char)cacheBuffer.byteData().get(inIterCount) & 0xFF) < 0xA0) // CR Character Set
                {
                    if (_shiftGL != null)
                    {
                        return CodecReturnCodes.FAILURE;
                    }

                    if (cacheBuffer.byteData().get(inIterCount) == 0x8E)
                        _shiftGL = _curWorkingSet.G2;
                    else if (cacheBuffer.byteData().get(inIterCount) == 0x8F)
                        _shiftGL = _curWorkingSet.G3;
                    else
                    {
                        if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                        {
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                        rmtesBuffer.byteData().put(outIterCount++, (byte)0);
                        rmtesBuffer.byteData().put(outIterCount++, (byte)0xFFFD);
                    }

                    inIterCount++;
                }
                else
                {
                    if (_shiftGL != null)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                    else if (isGRChar((char)cacheBuffer.byteData().get(inIterCount), _curWorkingSet.GR))
                    {
                        if (_curWorkingSet.GR.get_stride() == 2)
                        {
                            tempChar = (char)((char)cacheBuffer.byteData().get(inIterCount) & 0xFF);
                            if (inIterCount < cacheBuffer.length())
                            {
                                inIterCount++;
                                if (isGRChar((char)cacheBuffer.byteData().get(inIterCount), _curWorkingSet.GR))
                                {
                                    if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                                    {
                                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                                    }
                                    char toPut = ConvertStride2GR(tempChar, (char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _curWorkingSet.GR);
                                    rmtesBuffer.byteData().put(outIterCount++, (byte)((toPut & 0xFF00) >> 8));
                                    rmtesBuffer.byteData().put(outIterCount++, (byte)(toPut & 0x00FF));
                                }
                            }
                            else
                            {
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                            }
                        }
                        else
                        {
                            if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                            {
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                            }
                            char toPut = GRConvertSingle1((char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _curWorkingSet.GR);
                            rmtesBuffer.byteData().put(outIterCount++, (byte)((toPut & 0xFF00) >> 8));
                            rmtesBuffer.byteData().put(outIterCount++, (byte)(toPut & 0x00FF));

                            if (_curWorkingSet.GR.get_table2() != null)
                            {
                                tempChar = GRConvertSingle2((char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _curWorkingSet.GR);
                                if (tempChar != 0)
                                {
                                    if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                                    {
                                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                                    }
                                    rmtesBuffer.byteData().put(outIterCount++, (byte)((tempChar & 0xFF00) >> 8));
                                    rmtesBuffer.byteData().put(outIterCount++, (byte)(tempChar & 0x00FF));
                                }
                            }
                        }
                    }
                    else
                    {
                        return CodecReturnCodes.FAILURE;
                    }

                    inIterCount++;
                }
            }
            else
                /* UTF8 Encode */
            {
                if (cacheBuffer.byteData() != null && cacheBuffer.byteData().get(inIterCount) == 0x1B) /* Escape control character */
                {
                    _tempInfo = controlParse(cacheBuffer.byteData(), inIterCount, cacheBuffer.length(), _curWorkingSet);
                    cacheBuffer.data(_tempInfo.getIter());
                    _curWorkingSet = _tempInfo.getSet();
                    if (_tempInfo.getValue() < 0)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                    else
                    {
                        if (_tempInfo.getValue() == 0)
                            inIterCount++;
                        else
                            inIterCount += _tempInfo.getValue();
                    }
                }
                else if (cacheBuffer.byteData() != null && cacheBuffer.byteData().get(inIterCount) == 0x00)
                {
                    inIterCount++;
                }
                else
                    /* Just copy the data, since it's already encoded in UTF8 */
                {
                    if (outIterCount + 2 > rmtesBuffer.allocatedLength())
                    {
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    char toConvert = (char)(rmtesBuffer.byteData().get(outIterCount) & 0xFF00 | rmtesBuffer.byteData().get(outIterCount + 1) & 0xFF);
                    _tempInfo = UTF8ToUCS2(cacheBuffer.byteData(), inIterCount, toConvert, cacheBuffer.length());
                    if (_tempInfo.getValue() >= 3)
                    {
                        rmtesBuffer.byteData().put(outIterCount++, (byte)((_tempInfo.getCharValue() & 0xFF00) >> 8));
                        rmtesBuffer.byteData().put(outIterCount++, (byte)(_tempInfo.getCharValue() & 0x00FF));
                    }
                    else if (_tempInfo.getValue() >= 2)
                    {
                        rmtesBuffer.byteData().put(outIterCount++, (byte)((_tempInfo.getCharValue() & 0xFF00) >> 8));
                        rmtesBuffer.byteData().put(outIterCount++, (byte)(_tempInfo.getCharValue() & 0x00FF));
                    }
                    else
                    {
                        rmtesBuffer.byteData().put(outIterCount++, (byte)0);
                        rmtesBuffer.byteData().put(outIterCount++, (byte)_tempInfo.getCharValue());
                    }

                    if (_tempInfo.getValue() == 0)
                        inIterCount++;
                    else
                        inIterCount += _tempInfo.getValue();
                }
            }
        }

        /* Trim outer buffer length */
        rmtesBuffer.length(outIterCount);

        return CodecReturnCodes.SUCCESS;
    }

    private int UCS2ToUTF8(RmtesBuffer buffer, char ch, int pos) {

        if (ch < 0x0080) {
            if (pos + 1 <= buffer.allocatedLength()) {
                buffer.byteData().put(pos, (byte)ch);
                return 1;
            }
        } else if (ch < 0x0800) {
            if (pos + 2 <= buffer.allocatedLength()) {
                buffer.byteData().put(pos++, (byte)(0x0C0 | (ch >> 6)));
                buffer.byteData().put(pos, (byte)(0x080 | (ch & 0x3F)));
                return 2;
            }
        } else {
            if (pos + 3 < buffer.allocatedLength()) {
                buffer.byteData().put(pos++, (byte)(0x0E0 | (ch >> 12)));
                buffer.byteData().put(pos++, (byte)(0x080 | ((ch >> 6) & 0x3F)));
                buffer.byteData().put(pos++, (byte)(0x080 | (ch & 0x3F)));
                return 3;
            }
        }

        return -1;
    }

    public int RMTESToUTF8(RmtesBuffer rmtesBuffer, RmtesCacheBuffer cacheBuffer)
    {
        int state = RMTESParseState.NORMAL;
        int inIterCount = 0;
        int outIterCount = 0;
        int encType = EncodeType.TYPE_RMTES;
        int ret;

        char tempChar;

        _shiftGL = null;
        _tmpGL = null;

        if (rmtesBuffer.allocatedLength() == 0)
            return CodecReturnCodes.INVALID_ARGUMENT;
        if (cacheBuffer.length() == 0)
            return CodecReturnCodes.NO_DATA;

        _gL = 0;
        _gR = 0;

        _characterSet.initWorkingSet(_curWorkingSet);

        _tempInfo.setRetCode(ESCReturnCode.ESC_SUCCESS);

        while (inIterCount < cacheBuffer.length() && state != RMTESParseState.ERROR)
        {
            if (encType == EncodeType.TYPE_RMTES)
            {
                if (cacheBuffer.byteData() == null)
                    return CodecReturnCodes.FAILURE;

                if (((char)cacheBuffer.byteData().get(inIterCount) & 0xFF) < 0x20) // CL Character
                {
                    if (_shiftGL != null)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                    else if ((_tempInfo = controlParse(cacheBuffer.byteData(), inIterCount,
                            cacheBuffer.length(), _curWorkingSet)).getValue() == 0)
                    {
                        _curWorkingSet = _tempInfo.getSet();
                        if (_tempInfo.getRetCode() == ESCReturnCode.ESC_ERROR)
                        {
                            return CodecReturnCodes.FAILURE;
                        }
                        else if (_tempInfo.getRetCode() == ESCReturnCode.END_CHAR)
                        {
                            rmtesBuffer.length(outIterCount);
                            return CodecReturnCodes.SUCCESS;
                        }
                        else if (_tempInfo.getRetCode() == ESCReturnCode.ESC_SUCCESS)
                        {
                            if (outIterCount + 1 > rmtesBuffer.allocatedLength())
                            {
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                            }
                            if (cacheBuffer.byteData() != null)
                                rmtesBuffer.byteData().put(outIterCount++, cacheBuffer.byteData().get(inIterCount));
                            else
                                rmtesBuffer.byteData().put(outIterCount++, (byte)0);
                            inIterCount++;
                        }
                    }
                    else
                    {
                        _curWorkingSet = _tempInfo.getSet();
                        inIterCount += _tempInfo.getValue();
                        if (_tempInfo.getRetCode() == ESCReturnCode.UTF_ENC)
                            encType = EncodeType.TYPE_UTF8;
                    }
                }
                else if (_curWorkingSet.GL.get_shape() == CharSet.SHAPE_94 &&
                         cacheBuffer.byteData().get(inIterCount) == 0x20) /* Space character, if 94 character set */
                {
                    if (outIterCount + 1 > rmtesBuffer.allocatedLength())
                    {
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    rmtesBuffer.byteData().put(outIterCount++, (byte)0x20); // < 0x0080 , it is UTF8 already
                    inIterCount++;
                }
                else if (_curWorkingSet.GL.get_shape() == CharSet.SHAPE_94 &&
                         cacheBuffer.byteData().get(inIterCount) == 0x7F) /* Delete character, if 94 character set */
                {
                    if (outIterCount + 3 > rmtesBuffer.allocatedLength()) // 0xFFFD needs 3 bytes in UTF8
                    {
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    rmtesBuffer.byteData().put(outIterCount++, (byte)(0x0E0 | (0xFFFD >> 12)));
                    rmtesBuffer.byteData().put(outIterCount++, (byte)(0x080 | ((0xFFFD >> 6) & 0x3F)));
                    rmtesBuffer.byteData().put(outIterCount++, (byte)(0x080 | (0xFFFD & 0x3F)));
                    inIterCount++;
                }
                else if (((char)cacheBuffer.byteData().get(inIterCount) & 0xFF) < 0x80) // GL Character
                {
                    if (_shiftGL != null)
                        _tmpGL = _shiftGL;
                    else
                        _tmpGL = _curWorkingSet.GL;

                    if (isGLChar((char)cacheBuffer.byteData().get(inIterCount), _tmpGL))
                    {
                        if (_tmpGL.get_stride() == 2)
                        {
                            tempChar = (char)((char)cacheBuffer.byteData().get(inIterCount) & 0xFF);
                            if (inIterCount < cacheBuffer.length())
                            {
                                inIterCount++;
                                if (isGLChar((char)cacheBuffer.byteData().get(inIterCount), _tmpGL))
                                {
                                    char toPut = ConvertStride2GL(tempChar, (char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _tmpGL);
                                    ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                                    if (ret < 0)
                                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                                    else
                                        outIterCount += ret;
                                }
                                else
                                {
                                    return CodecReturnCodes.FAILURE;
                                }
                            }
                            else
                            {
                                return CodecReturnCodes.FAILURE;
                            }
                        }
                        else
                        {
                            char toPut = GLConvertSingle1((char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _tmpGL);
                            ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                            if (ret < 0)
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                            else
                                outIterCount += ret;

                            if (_tmpGL.get_table2() != null)
                            {
                                tempChar = GLConvertSingle2((char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _tmpGL);

                                if (tempChar != 0)
                                {
                                    ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                                    if (ret < 0)
                                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                                    else
                                        outIterCount += ret;
                                }
                            }
                        }
                    }
                    else
                    {
                        return CodecReturnCodes.FAILURE;
                    }

                    inIterCount++;
                    _shiftGL = null;
                }
                else if (((char)cacheBuffer.byteData().get(inIterCount) & 0xFF) < 0xA0) // CR Character Set
                {
                    if (_shiftGL != null)
                    {
                        return CodecReturnCodes.FAILURE;
                    }

                    if (cacheBuffer.byteData().get(inIterCount) == 0x8E)
                        _shiftGL = _curWorkingSet.G2;
                    else if (cacheBuffer.byteData().get(inIterCount) == 0x8F)
                        _shiftGL = _curWorkingSet.G3;
                    else
                    {
                        if (outIterCount + 1 > rmtesBuffer.allocatedLength())
                        {
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                        rmtesBuffer.byteData().put(outIterCount++, (byte)0xFFFD);
                    }

                    inIterCount++;
                }
                else
                {
                    if (_shiftGL != null)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                    else if (isGRChar((char)cacheBuffer.byteData().get(inIterCount), _curWorkingSet.GR))
                    {
                        if (_curWorkingSet.GR.get_stride() == 2)
                        {
                            tempChar = (char)((char)cacheBuffer.byteData().get(inIterCount) & 0xFF);
                            if (inIterCount < cacheBuffer.length())
                            {
                                inIterCount++;
                                if (isGRChar((char)cacheBuffer.byteData().get(inIterCount), _curWorkingSet.GR))
                                {
                                    char toPut = ConvertStride2GR(tempChar, (char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _curWorkingSet.GR);
                                    ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                                    if (ret < 0)
                                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                                    else
                                        outIterCount += ret;
                                }
                            }
                            else
                            {
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                            }
                        }
                        else
                        {
                            char toPut = GRConvertSingle1((char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _curWorkingSet.GR);
                            ret = UCS2ToUTF8(rmtesBuffer, toPut, outIterCount);

                            if (ret < 0)
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                            else
                                outIterCount += ret;

                            if (_curWorkingSet.GR.get_table2() != null)
                            {
                                tempChar = GRConvertSingle2((char)(cacheBuffer.byteData().get(inIterCount) & 0xFF), _curWorkingSet.GR);
                                if (tempChar != 0)
                                {
                                    ret = UCS2ToUTF8(rmtesBuffer, tempChar, outIterCount);

                                    if (ret < 0)
                                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                                    else
                                        outIterCount += ret;
                                }
                            }
                        }
                    }
                    else
                    {
                        return CodecReturnCodes.FAILURE;
                    }

                    inIterCount++;
                }
            }
            else
                /* UTF8 Encode */
            {
                if (cacheBuffer.byteData() != null && cacheBuffer.byteData().get(inIterCount) == 0x1B) /* Escape control character */
                {
                    _tempInfo = controlParse(cacheBuffer.byteData(), inIterCount, cacheBuffer.length(), _curWorkingSet);
                    cacheBuffer.data(_tempInfo.getIter());
                    _curWorkingSet = _tempInfo.getSet();
                    if (_tempInfo.getValue() < 0)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                    else
                    {
                        if (_tempInfo.getValue() == 0)
                            inIterCount++;
                        else
                            inIterCount += _tempInfo.getValue();
                    }
                }
                else if (cacheBuffer.byteData() != null && cacheBuffer.byteData().get(inIterCount) == 0x00)
                {
                    inIterCount++;
                }
                else
                    /* Just copy the data, since it's already encoded in UTF8 */
                {
                    if (outIterCount + 1 > rmtesBuffer.allocatedLength())
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    rmtesBuffer.byteData().put(outIterCount++, rmtesBuffer.byteData().get(outIterCount));
                    inIterCount++;
                }
            }
        }

        /* Trim outer buffer length */
        rmtesBuffer.length(outIterCount);

        return CodecReturnCodes.SUCCESS;
    }

    /* Handles the following cases ( Same as ETAC ): 1) ESC LBRKT
     * NumericNumber(base 10) RHPA - Cursor command 2) ESC LBRKT
     * NumericNumber(base 10) RREP - Repeat command for previous character
     */
    public boolean hasPartialRMTESUpdate(Buffer buffer)
    {
        int i = 0;
        int numCount = 0;
        int state = RMTESParseState.NORMAL;

        for (i = buffer.position(); i < buffer.length()+buffer.position(); ++i)
        {
            switch (state)
            {
                case RMTESParseState.NORMAL:
                    if (buffer.data().get(i) == ESC_CHAR)
                        state = RMTESParseState.ESC;
                    break;
                case RMTESParseState.ESC:
                    if (buffer.data().get(i) == LBRKT_CHAR)
                        state = RMTESParseState.LBRKT;
                    else if (buffer.data().get(i) == 0x30) // ESC_25
                        return false; // Error. UTF8 sequence should not be before command
                    else
                        state = RMTESParseState.NORMAL;
                    break;
                case RMTESParseState.LBRKT:
                    if (buffer.data().get(i) >= '0' && buffer.data().get(i) <= '9')
                        numCount += numCount * 10 + buffer.data().get(i) - '0';
                    else if (buffer.data().get(i) == RHPA_CHAR || buffer.data().get(i) == RREP_CHAR)
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

    /* Applies the _inBuffer's partial update data to the outBuffer.
     * Preconditions: outBuffer is large enough to handle the additional data
     * outBuffer has already been populated with data Result:
     * _inBuffer's partial update(s) are applied to outBuffer
     */
    public int RMTESApplyToCache(Buffer fEntry, RmtesCacheBuffer cacheBuffer)
    {
        int inBufPos = fEntry.position();
        int cacheBufferPos = 0;
        int numCount = 0;
        int i = 0;
        byte prevByte = 0;
        boolean escPresent = false;
        int state = RMTESParseState.NORMAL;
        int maxLen = 0;


        if (fEntry.length() == 0)
            return CodecReturnCodes.INVALID_ARGUMENT;
        if (cacheBuffer.allocatedLength() == 0)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        i = 1;

        while (inBufPos < (fEntry.length() + fEntry.position()))
        {
            switch (state)
            {
                case RMTESParseState.NORMAL:
                    if (fEntry.data().get(inBufPos) == ESC_CHAR)
                    {
                        state = RMTESParseState.ESC;
                        numCount = 0;
                    }
                    else
                    {
                        if (cacheBufferPos >= cacheBuffer.allocatedLength())
                        {
                            return CodecReturnCodes.FAILURE;
                        }
                        prevByte = fEntry.data().get(inBufPos);
                        cacheBuffer.byteData().put(cacheBufferPos++, prevByte);
                    }
                    break;
                case RMTESParseState.ESC:
                    if (fEntry.data().get(inBufPos) == LBRKT_CHAR)
                        state = RMTESParseState.LBRKT;
                    else if (fEntry.data().get(inBufPos) == 0x25)
                    {
                        inBufPos++;
                        if (inBufPos < (fEntry.length() + fEntry.position()) && fEntry.data().get(inBufPos) == 0x30)
                        {
                            if (cacheBufferPos + 3 > cacheBuffer.allocatedLength())
                            {
                                /* Error: Out of space */
                                return CodecReturnCodes.FAILURE;
                            }
                            cacheBuffer.byteData().put(cacheBufferPos++, (byte)ESC_CHAR);
                            cacheBuffer.byteData().put(cacheBufferPos++, (byte)0x25);
                            cacheBuffer.byteData().put(cacheBufferPos++, (byte)0x30);
                            state = RMTESParseState.NORMAL;
                        }
                        else
                        {
                            /* Error */
                            return CodecReturnCodes.FAILURE;
                        }
                    }
                    else
                    {
                        /* normal escape code, print ESC character to buffer and continue */
                        state = RMTESParseState.NORMAL;
                        if (cacheBufferPos + 2 > cacheBuffer.allocatedLength())
                        {
                            return CodecReturnCodes.FAILURE;
                        }
                        cacheBuffer.byteData().put(cacheBufferPos++, (byte)ESC_CHAR);
                        cacheBuffer.byteData().put(cacheBufferPos++, fEntry.data().get(inBufPos));
                    }
                    break;
                case RMTESParseState.LBRKT:
                    if (fEntry.data().get(inBufPos) >= '0' && fEntry.data().get(inBufPos) <= '9')
                    {
                        numCount = numCount * 10 + fEntry.data().get(inBufPos) - '0';
                    }
                    else if (fEntry.data().get(inBufPos) == RHPA_CHAR)
                    {
                        /* Move cursor command */
                        /* Escape command is completed, flag as true */
                        escPresent = true;

                        if (numCount >= 0)
                        {
                            cacheBufferPos = numCount;
                            numCount = 0;
                            state = RMTESParseState.NORMAL;

                            if (cacheBufferPos > cacheBuffer.allocatedLength())
                                return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                        else
                        {
                            return CodecReturnCodes.FAILURE;
                        }
                    }
                    else if (fEntry.data().get(inBufPos) == RREP_CHAR)
                    {
                        /* Repeat character command. This is always 1 char */
                        /* Check for overrun first */
                        /* Escape command completed */
                        escPresent = true;

                        if (cacheBuffer.allocatedLength() < cacheBufferPos + numCount)
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        for (i = 0; i < numCount; i++)
                        {
                            cacheBuffer.data(cacheBuffer.byteData().put(cacheBufferPos++, prevByte));
                        }
                        state = RMTESParseState.NORMAL;
                    }
                    else
                        return CodecReturnCodes.FAILURE;
                    break;
                default:
                    return CodecReturnCodes.FAILURE;
            }
            inBufPos++;
            if (cacheBufferPos > maxLen)
                maxLen = cacheBufferPos;
        }

        /* Check that we're not in a weird state */
        if (state != RMTESParseState.NORMAL)
            return CodecReturnCodes.FAILURE;

        if (escPresent == true)
        {
            if (maxLen > cacheBuffer.length())
            {
                cacheBuffer.length(maxLen);
            }
            else
                return CodecReturnCodes.SUCCESS;
        }
        else
        {
            cacheBuffer.length(maxLen);
        }
        return CodecReturnCodes.SUCCESS;
    }
}
