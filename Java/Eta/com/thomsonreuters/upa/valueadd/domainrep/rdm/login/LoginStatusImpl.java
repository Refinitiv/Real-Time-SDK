package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

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
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl;

class LoginStatusImpl extends MsgBaseImpl
{
    private State state;
    private Buffer userName;
    private int userNameType;
    private int flags;
    
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();           
    
    public int copy(LoginStatus destStatusMsg)
    {
        assert (destStatusMsg != null) : "destStatusMsg must be non-null";
        destStatusMsg.streamId(streamId());
        if(checkHasUserName())
        {
            destStatusMsg.applyHasUserName();
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.userName.length());
            userName.copy(byteBuffer);
            destStatusMsg.userName().data(byteBuffer);
        }
        if(checkHasUserNameType())
        {
            destStatusMsg.applyHasUserNameType();
            destStatusMsg.userNameType(userNameType);
        }
        if(checkHasState())
        {
            destStatusMsg.state().streamState(this.state.streamState());
            destStatusMsg.state().dataState(this.state.dataState());
            destStatusMsg.state().code(this.state.code());
            if(this.state.text().length() >  0)
            {
                ByteBuffer byteBuffer = ByteBuffer.allocate(this.state.text().length());
                this.state.text().copy(byteBuffer);
                destStatusMsg.state().text().data(byteBuffer);
            }    
            destStatusMsg.applyHasState();
        }
        
       
        return CodecReturnCodes.SUCCESS;
    }
    
    LoginStatusImpl()
    {
        state = CodecFactory.createState();
        userName = CodecFactory.createBuffer();
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
    
    public void clear()
    {
        super.clear();
        userName.clear();
        userNameType = Login.UserIdTypes.NAME;
        flags = 0;
        state.clear();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);
        streamId(1);
    }

    public int encode(EncodeIterator encodeIter)
    {
        statusMsg.clear();
        statusMsg.streamId(streamId());
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.domainType(DomainTypes.LOGIN);
        if(checkHasUserName())
        {
            statusMsg.applyHasMsgKey();
            statusMsg.msgKey().applyHasName();
            statusMsg.msgKey().name(userName());
            if(checkHasUserNameType())
            {
                statusMsg.msgKey().applyHasNameType();
                statusMsg.msgKey().nameType(userNameType());
            }
        }
        
        if(checkHasState())
        {
            statusMsg.applyHasState();
            statusMsg.state().dataState(state().dataState());
            statusMsg.state().streamState(state().streamState());
            statusMsg.state().code(state().code());
            statusMsg.state().text(state().text());
        }
       
        int ret = statusMsg.encode(encodeIter);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        return CodecReturnCodes.SUCCESS;
    }
    
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.STATUS)
             return CodecReturnCodes.FAILURE;
        
       
        StatusMsg statusMsg = (StatusMsg) msg;
        streamId(msg.streamId());
        if(statusMsg.checkHasState())
        {
            state().streamState(statusMsg.state().streamState());
            state().dataState(statusMsg.state().dataState());
            state().code(statusMsg.state().code());
            if(statusMsg.state().text().length() >  0)
            {
                Buffer text = statusMsg.state().text();
                this.state.text().data(text.data(), text.position(), text.length());
            }    
            applyHasState();
        }
        
        MsgKey msgKey = msg.msgKey();
        if(msgKey != null && msgKey.checkHasName())
        {
            applyHasUserName();
            Buffer name = msgKey.name();
            userName().data(name.data(), name.position(), name.length());
            if(msgKey.checkHasNameType())
            {
                applyHasUserNameType();
                userNameType(msgKey.nameType());
            }
        }
        
        return CodecReturnCodes.SUCCESS;
    }

    public boolean checkHasUserName()
    {
        return (flags() & LoginStatusFlags.HAS_USERNAME) != 0;
    }
    
    public void applyHasUserName()
    {
        flags |= LoginStatusFlags.HAS_USERNAME;
    }
    
    public Buffer userName()
    {
        return userName;
    }

    public void userName(Buffer userName)
    {
        assert(userName != null) : "userName can not be null";
        userName().data(userName.data(), userName.position(), userName.length());
    }
    
    public int userNameType()
    {
        return userNameType;
    }

    public void userNameType(int nameType)
    {
        this.userNameType = nameType;
    }

    public boolean checkHasUserNameType()
    {
        return (flags() & LoginStatusFlags.HAS_USERNAME_TYPE) != 0;
    }
    
    public void applyHasUserNameType()
    {
        flags |= LoginStatusFlags.HAS_USERNAME_TYPE;
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
    
    public boolean checkHasState()
    {
        return (flags() & LoginStatusFlags.HAS_STATE) != 0;
    }
    
    public void applyHasState()
    {
        flags |= LoginStatusFlags.HAS_STATE;
    }
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "LoginStatus: \n");
     
        if (checkHasUserNameType())
        {
            stringBuf.append(tab);
            stringBuf.append("nameType: ");
            stringBuf.append(userNameType());
            stringBuf.append(eol);
        }
        
        if (checkHasUserName())
        {
            stringBuf.append(tab);
            stringBuf.append("name: ");
            stringBuf.append(userName());
            stringBuf.append(eol);
        }

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
        return DomainTypes.LOGIN;
    }
}