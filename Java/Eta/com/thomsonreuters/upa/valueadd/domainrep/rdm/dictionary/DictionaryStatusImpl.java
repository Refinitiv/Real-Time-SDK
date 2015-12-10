package com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl;

class DictionaryStatusImpl extends MsgBaseImpl
{
    private State state;
    private int flags;

    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();

    public int copy(DictionaryStatus destStatusMsg)
    {
        assert (destStatusMsg != null) : "destStatusMsg must be non-null";
        destStatusMsg.streamId(streamId());

        if (checkHasState())
        {
            destStatusMsg.state().streamState(this.state.streamState());
            destStatusMsg.state().dataState(this.state.dataState());
            destStatusMsg.state().code(this.state.code());

            if (this.state.text().length() > 0)
            {
                Buffer stateText = CodecFactory.createBuffer();
                ByteBuffer byteBuffer = ByteBuffer.allocate(this.state.text().length());
                this.state.text().copy(byteBuffer);
                stateText.data(byteBuffer);
                destStatusMsg.state().text(stateText);
            }
            int flags = destStatusMsg.flags();
            flags |= DictionaryStatusFlags.HAS_STATE;
            destStatusMsg.flags(flags);
        }

        return CodecReturnCodes.SUCCESS;
    }

    DictionaryStatusImpl()
    {
        state = CodecFactory.createState();
        streamId(1);
    }

    public void flags(int flags)
    {
        this.flags = flags;
    }

    public int flags()
    {
        return flags;
    }

    public boolean checkHasState()
    {
        return (flags & DictionaryStatusFlags.HAS_STATE) != 0;
    }
    
    public void applyHasState()
    {
        flags |= DictionaryStatusFlags.HAS_STATE;
    }
    
    public void clear()
    {
        super.clear();
        flags = 0;
        state.clear();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);
    }

    public int encode(EncodeIterator encodeIter)
    {
        statusMsg.clear();
        statusMsg.streamId(streamId());
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.domainType(DomainTypes.DICTIONARY);
        if (checkHasState())
        {
            statusMsg.applyHasState();
            statusMsg.state().dataState(state().dataState());
            statusMsg.state().streamState(state().streamState());
            statusMsg.state().code(state().code());
            statusMsg.state().text(state().text());
        }

        return statusMsg.encode(encodeIter);
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.STATUS)
            return CodecReturnCodes.FAILURE;

        StatusMsg statusMsg = (StatusMsg)msg;
        streamId(msg.streamId());
        if(statusMsg.checkHasState())
        {
            state.code(statusMsg.state().code());
            state.streamState(statusMsg.state().streamState());
            state.dataState(statusMsg.state().dataState());
            
            if (statusMsg.state().text().length() > 0)
            {
                Buffer buf = statusMsg.state().text();
                this.state.text().data(buf.data(), buf.position(), buf.length());
            }
            
            applyHasState();
        }

        return CodecReturnCodes.SUCCESS;
    }

    public State state()
    {
        return state;
    }
    
    public void state(State state)
    {
        state().streamState(state.streamState());
        state().dataState(state.dataState());
        state().code(state.code());
        state().text(state.text());
    }

    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "DictionaryStatus: \n");

        if (checkHasState())
        {
            stringBuf.append(tab);
            stringBuf.append("state: ");
            stringBuf.append(state());
            stringBuf.append(eol);
        }

        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.DICTIONARY;
    }
}