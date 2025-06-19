/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class LoginAckImpl extends MsgBaseImpl
{
    private AckMsg ackMsg = (AckMsg)CodecFactory.createMsg();

    LoginAckImpl()
    {
        super();
    }
    
    public void clear()
    {
        super.clear();
    }
    
    @SuppressWarnings("deprecation")
    public int copy(LoginAck destAckMsg)
    {
        assert (destAckMsg != null) : "destAckMsg must be non-null";
        destAckMsg.streamId(streamId());
        return CodecReturnCodes.SUCCESS;
    }
    
    public int encode(EncodeIterator encodeIter)
    {
        ackMsg.clear();
        ackMsg.msgClass(MsgClasses.ACK);
        ackMsg.streamId(streamId());
        ackMsg.domainType(DomainTypes.LOGIN);

        return ackMsg.encode(encodeIter);
    }
    
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.ACK || msg.domainType() != DomainTypes.LOGIN)
            return CodecReturnCodes.FAILURE;

        streamId(msg.streamId());
        
        return CodecReturnCodes.SUCCESS;
    }
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "LoginAck: \n");
        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.LOGIN;
    }
 }