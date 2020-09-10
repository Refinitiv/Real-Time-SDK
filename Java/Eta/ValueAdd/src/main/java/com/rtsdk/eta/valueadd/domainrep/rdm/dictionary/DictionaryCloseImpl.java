package com.rtsdk.eta.valueadd.domainrep.rdm.dictionary;

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

class DictionaryCloseImpl extends MsgBaseImpl
{
    private CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
    
    DictionaryCloseImpl()
    {
        super();
    }

    public void clear()
    {
        super.clear();
    }

    public int copy(DictionaryClose destCloseMsg)
    {
        assert (destCloseMsg != null) : "destCloseMsg must be non-null";
        destCloseMsg.streamId(streamId());
        return CodecReturnCodes.SUCCESS;
    }
    
    public int encode(EncodeIterator encodeIter)
    {
        closeMsg.clear();
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(streamId());
        closeMsg.domainType(DomainTypes.DICTIONARY);
        closeMsg.containerType(DataTypes.NO_DATA);

        return closeMsg.encode(encodeIter);
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.CLOSE)
            return CodecReturnCodes.FAILURE;

        streamId(msg.streamId());

        return CodecReturnCodes.SUCCESS;
    }

    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "DictionaryClose: \n");
        return stringBuf.toString();
    }

    @Override
    public int domainType()
    {
        return DomainTypes.DICTIONARY;
    }
}