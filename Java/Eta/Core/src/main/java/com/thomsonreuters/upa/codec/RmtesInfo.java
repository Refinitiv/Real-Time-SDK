package com.thomsonreuters.upa.codec;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.RmtesBuffer;
import com.thomsonreuters.upa.codec.CharSet.RmtesWorkingSet;

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