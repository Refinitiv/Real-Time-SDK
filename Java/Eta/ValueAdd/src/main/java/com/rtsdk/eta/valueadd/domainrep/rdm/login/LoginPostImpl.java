package com.rtsdk.eta.valueadd.domainrep.rdm.login;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.PostMsg;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.valueadd.domainrep.rdm.MsgBaseImpl;

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