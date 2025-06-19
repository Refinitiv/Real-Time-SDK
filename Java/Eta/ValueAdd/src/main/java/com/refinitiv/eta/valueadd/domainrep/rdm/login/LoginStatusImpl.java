/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class LoginStatusImpl extends MsgBaseImpl
{
    private State state;
    private Buffer userName;
    private int userNameType;
    private long authenticationErrorCode;
    private Buffer authenticationErrorText;
    private static final String blankStringConst = new String(new byte[] { 0x0 });
    private int flags;
    
    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry element = CodecFactory.createElementEntry();
    private UInt tmpUInt = CodecFactory.createUInt();

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
        
        if(checkClearCache())
        {
        	destStatusMsg.applyClearCache();
        }
       
        if (checkHasAuthenticationErrorCode())
        {
            destStatusMsg.applyHasAuthenticationErrorCode();
            destStatusMsg.authenticationErrorCode(authenticationErrorCode);
        }
        if (checkHasAuthenticationErrorText())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.authenticationErrorText.length());
            this.authenticationErrorText.copy(byteBuffer);
            destStatusMsg.applyHasAuthenticationErrorText();
            destStatusMsg.authenticationErrorText().data(byteBuffer);
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    LoginStatusImpl()
    {
        state = CodecFactory.createState();
        userName = CodecFactory.createBuffer();
        authenticationErrorText = CodecFactory.createBuffer();
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
        authenticationErrorCode = 0;
        authenticationErrorText.clear();
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
                if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                {
                    statusMsg.msgKey().applyHasName();
                    statusMsg.msgKey().name().data(blankStringConst);
                }
            }
        }
        
        if(checkClearCache())
        {
        	statusMsg.applyClearCache();
        }
        
        if(checkHasState())
        {
            statusMsg.applyHasState();
            statusMsg.state().dataState(state().dataState());
            statusMsg.state().streamState(state().streamState());
            statusMsg.state().code(state().code());
            statusMsg.state().text(state().text());
        }
       
        if (checkHasAuthenticationErrorCode() || checkHasAuthenticationErrorText())
        {
            statusMsg.applyHasMsgKey();
            statusMsg.msgKey().applyHasAttrib();            
            statusMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

            int ret = statusMsg.encodeInit(encodeIter, 0);
            if (ret != CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB)
                return ret;
            ret = encodeAttrib(encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = statusMsg.encodeKeyAttribComplete(encodeIter, true);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;

            ret = statusMsg.encodeComplete(encodeIter, true);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }
        else
        {
        int ret = statusMsg.encode(encodeIter);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        return CodecReturnCodes.SUCCESS;
    }
    

        return CodecReturnCodes.SUCCESS;
    }
   
    private int encodeAttrib(EncodeIterator encodeIter)
    {
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        int ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        if (this.checkHasAuthenticationErrorCode())
        {
            element.dataType(DataTypes.UINT);
            element.name(ElementNames.AUTHN_ERROR_CODE);
            tmpUInt.value(authenticationErrorCode());
            if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (checkHasAuthenticationErrorText() && authenticationErrorText().length() != 0)
        {
            element.dataType(DataTypes.ASCII_STRING);
            element.name(ElementNames.AUTHN_ERROR_TEXT);
            ret = element.encode(encodeIter, authenticationErrorText());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }        
        
        return elementList.encodeComplete(encodeIter, true);
        
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
        
        if(statusMsg.checkClearCache())
        {
        	applyClearCache();
        }
        
        MsgKey msgKey = msg.msgKey();
        if (msgKey != null)
        {
            if (msgKey.checkHasName())
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
            if (msgKey.checkHasAttrib())
            {
                int ret = msg.decodeKeyAttrib(dIter, msgKey);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return decodeAttrib(dIter);
            }
        }
        return CodecReturnCodes.SUCCESS;
    }
        
    private int decodeAttrib(DecodeIterator dIter)
    {
        elementList.clear();
        int ret = elementList.decode(dIter, null);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        element.clear();
        while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            else if (element.name().equals(ElementNames.AUTHN_ERROR_CODE))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAuthenticationErrorCode();
                authenticationErrorCode(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.AUTHN_ERROR_TEXT))
            {
                if (element.dataType() != DataTypes.ASCII_STRING
                        && element.dataType() != DataTypes.BUFFER)
                    return CodecReturnCodes.FAILURE;

                Buffer authenticationErrorText = element.encodedData();
                applyHasAuthenticationErrorText();
                authenticationErrorText().data(authenticationErrorText.data(),
                                               authenticationErrorText.position(),
                                               authenticationErrorText.length());
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
    
    public void applyClearCache()
    {
        flags |= LoginStatusFlags.CLEAR_CACHE;
    }

    public boolean checkClearCache()
    {
        return (flags & LoginStatusFlags.CLEAR_CACHE) != 0;
    }

    public void authenticationErrorCode(long authenticationErrorCode)
    {
        assert (checkHasAuthenticationErrorCode());
        this.authenticationErrorCode = authenticationErrorCode;
    }

    public long authenticationErrorCode()
    {
        return authenticationErrorCode;
    }

    public boolean checkHasAuthenticationErrorCode()
    {
        return (flags() & LoginStatusFlags.HAS_AUTHENTICATION_ERROR_CODE) != 0;
    }

    public void applyHasAuthenticationErrorCode()
    {
        flags |= LoginStatusFlags.HAS_AUTHENTICATION_ERROR_CODE;
    }

    public void authenticationErrorText(Buffer authenticationErrorText)
    {
        assert (checkHasAuthenticationErrorText());
        authenticationErrorText.data(authenticationErrorText.data(),
                                     authenticationErrorText.position(),
                                     authenticationErrorText.length());
    }

    public Buffer authenticationErrorText()
    {
        return authenticationErrorText;
    }

    public boolean checkHasAuthenticationErrorText()
    {
        return (flags() & LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT) != 0;
    }

    public void applyHasAuthenticationErrorText()
    {
        flags |= LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT;
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

        if (checkHasAuthenticationErrorCode())
        {
            stringBuf.append(tab);
            stringBuf.append("authenticationErrorCode: ");
            stringBuf.append(authenticationErrorCode());
            stringBuf.append(eol);
        }
        if (checkHasAuthenticationErrorText())
        {
            stringBuf.append(tab);
            stringBuf.append("authenticationErrorText: ");
            stringBuf.append(authenticationErrorText());
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