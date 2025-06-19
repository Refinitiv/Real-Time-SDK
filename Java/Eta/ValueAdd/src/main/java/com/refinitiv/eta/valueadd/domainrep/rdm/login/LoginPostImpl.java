/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class LoginPostImpl extends MsgBaseImpl
{
    private PostMsg postMsg = (PostMsg)CodecFactory.createMsg();

    LoginPostImpl()
    {
    }
    
    public void clear()
    {
        super.clear();
    }
    
    @SuppressWarnings("deprecation")
    public int copy(LoginPost destPostMsg)
    {
        assert (destPostMsg != null) : "destPostMsg must be non-null";
        destPostMsg.streamId(streamId());
        return CodecReturnCodes.SUCCESS;
    }
    
    public int encode(EncodeIterator encodeIter)
    {
        postMsg.clear();

        postMsg.streamId(streamId());
        postMsg.msgClass(MsgClasses.POST);
        postMsg.domainType(DomainTypes.LOGIN);
        
        return postMsg.encode(encodeIter);
    }
    
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.POST || msg.domainType() != DomainTypes.LOGIN)
            return CodecReturnCodes.FAILURE;

        streamId(msg.streamId());
        
        return CodecReturnCodes.SUCCESS;
    }
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "LoginPost: \n");

        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.LOGIN;
    }
}