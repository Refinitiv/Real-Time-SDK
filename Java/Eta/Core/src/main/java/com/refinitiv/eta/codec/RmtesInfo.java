/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.RmtesBuffer;
import com.refinitiv.eta.codec.CharSet.RmtesWorkingSet;

class RmtesInfo
{
    RmtesBuffer stringBuffer;
    int codecReturnCode;
    RmtesWorkingSet set;
    private int retCode;
    int value;
    private ByteBuffer iter;
    char charValue;
    String output = new String();

    RmtesBuffer getStringBuffer()
    {
        return stringBuffer;
    }

    int getCodecReturnCode()
    {
        return codecReturnCode;
    }

    int getValue()
    {
        return value;
    }

    RmtesWorkingSet getSet()
    {
        return set;
    }

    int getRetCode()
    {
        return retCode;
    }

    char getCharValue()
    {
        return charValue;
    }
     
    RmtesInfo returnUCS2ToUTF8(ByteBuffer data, int _value)
    {
        setIter(data);
        value = _value;
        return this;
    }

    RmtesInfo returnUTF8ToUCS2(int _value, char c)
    {
        value = _value;
        charValue = c;
        return this;
    }

    RmtesInfo returnControlParse(int escSuccess, RmtesWorkingSet currentSet, int i, ByteBuffer iIter)
    {
        setRetCode(escSuccess);
        set = currentSet;
        value = i;
        setIter(iIter);
        return this;
    }

    void setRetCode(int retCode)
    {
        this.retCode = retCode;
    }

    ByteBuffer getIter()
    {
        return iter;
    }

    void setIter(ByteBuffer iter)
    {
        this.iter = iter;
    }
 }