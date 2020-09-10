package com.rtsdk.eta.valueadd.domainrep.rdm.directory;

import com.rtsdk.eta.codec.CloseMsg;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class DirectoryCloseImpl extends MsgBaseImpl
{
    public DirectoryCloseImpl()
    {
        super();
    }

    public void clear()
    {
        super.clear();
    }

    public int copy(DirectoryClose destCloseMsg)
    {
        assert (destCloseMsg != null) : "destCloseMsg must be non-null";
        destCloseMsg.streamId(streamId());
        return CodecReturnCodes.SUCCESS;
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.CLOSE)
            return CodecReturnCodes.FAILURE;
        
        streamId(msg.streamId());
        
        return CodecReturnCodes.SUCCESS;
    }
    
    public int encode(EncodeIterator encodeIter)
    {
        closeMsg.clear();

        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(streamId());
        closeMsg.domainType(DomainTypes.SOURCE);
        closeMsg.containerType(DataTypes.NO_DATA);
        return closeMsg.encode(encodeIter);
    }
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "DirectoryClose: \n");
        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.SOURCE;
    }

    private CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
}