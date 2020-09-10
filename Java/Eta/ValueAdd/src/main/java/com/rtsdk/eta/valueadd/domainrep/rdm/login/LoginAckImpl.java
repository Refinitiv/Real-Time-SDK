package com.rtsdk.eta.valueadd.domainrep.rdm.login;

import com.rtsdk.eta.codec.AckMsg;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.valueadd.domainrep.rdm.MsgBaseImpl;

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