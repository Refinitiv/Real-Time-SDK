/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.Login;

class LoginWarmStandbyInfoImpl implements LoginWarmStandbyInfo
{
    private long warmStandbyMode;
    private int action;

    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry elementEntry = CodecFactory.createElementEntry();
    private UInt tmpUInt = CodecFactory.createUInt();
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private StringBuilder stringBuffer = new StringBuilder();

    public LoginWarmStandbyInfoImpl()
    {
        clear();
    }

    public void clear()
    {
        action = MapEntryActions.ADD;
        warmStandbyMode = Login.ServerTypes.ACTIVE;
    }

    public int encode(EncodeIterator encodeIter)
    {
        elementList.clear();
        elementList.applyHasStandardData();
        int ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;
        elementEntry.clear();
        elementEntry.name(ElementNames.WARMSTANDBY_MODE);
        elementEntry.dataType(DataTypes.UINT);
        tmpUInt.value(warmStandbyMode);
        ret = elementEntry.encode(encodeIter, tmpUInt);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;
        ret = elementList.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        return ret;
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();

        elementList.clear();
        int ret = elementList.decode(dIter, null);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        boolean warmStandbyModeFound = false;
        while ((ret = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (elementEntry.name().equals(ElementNames.WARMSTANDBY_MODE))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return ret;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
                    return ret;
                warmStandbyMode(tmpUInt.toLong());
                warmStandbyModeFound = true;
            }
        }

        if (!warmStandbyModeFound)
            return CodecReturnCodes.FAILURE;

        return CodecReturnCodes.SUCCESS;
    }

    public long warmStandbyMode()
    {
        return warmStandbyMode;
    }

    public void warmStandbyMode(long warmStandbyMode)
    {
        this.warmStandbyMode = warmStandbyMode;
    }
    
    public void action(int action)
    {
        this.action = action;
    }
    
    public int action()
    {
        return action;
    }
 
    public String toString()
    {
        stringBuffer.setLength(0);
        stringBuffer.insert(0, "LoginWarmStandbyInfo: \n");
        stringBuffer.append(tab);

        stringBuffer.append("warmStandbyMode: ");
        stringBuffer.append(warmStandbyMode());
        stringBuffer.append(eol);
        stringBuffer.append(tab);
        stringBuffer.append("action: ");
        stringBuffer.append(action());
        stringBuffer.append(eol);
        return stringBuffer.toString();
    }
    
    public int copy(LoginWarmStandbyInfo destWarmStandbyInfo)
    {
        assert (destWarmStandbyInfo != null) : "destWarmStandbyInfo must be non-null";
        
        destWarmStandbyInfo.action(action());
        destWarmStandbyInfo.warmStandbyMode(warmStandbyMode());
        
        return CodecReturnCodes.SUCCESS;
    }

    //copy warm standy by info from source object into this object.
    public void copyReferences(LoginWarmStandbyInfo srcWarmStandByInfo)
    {
        assert (srcWarmStandByInfo != null) : "srcWarmStandByInfo must be non-null";

        action(srcWarmStandByInfo.action());
        warmStandbyMode(srcWarmStandByInfo.warmStandbyMode());
    }
}