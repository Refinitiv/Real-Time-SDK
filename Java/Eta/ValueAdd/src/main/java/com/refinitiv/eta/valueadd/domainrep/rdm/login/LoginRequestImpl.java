package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class LoginRequestImpl extends MsgBaseImpl
{
    private Buffer userName;
    private int userNameType;
    private int flags;
    
    private LoginAttrib attrib;
    private long downloadConnectionConfig;
    private Buffer instanceId;
    private Buffer password;
    private long role;
    private Buffer authenticationToken;
    private Buffer authenticationExtended;

    private static final String blankStringConst = new String(new byte[] { 0x0 });
    private static String defaultUsername;
    
    private ElementEntry elementEntry = CodecFactory.createElementEntry();
    private ElementList elementList = CodecFactory.createElementList();
    private UInt tmpUInt = CodecFactory.createUInt();
    private RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";

    public void flags(int flags)
    {
        this.flags = flags;
    }

    public int flags()
    {
        return flags;
    }

    public int copy(LoginRequest destRequestMsg)
    {
        assert (destRequestMsg != null) : "destRequestMsg must be non-null";

        destRequestMsg.streamId(streamId());
        destRequestMsg.flags(flags());
        // username
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.userName.length());
            this.userName.copy(byteBuffer);
            destRequestMsg.userName().data(byteBuffer);
        }
        
        if (checkHasUserNameType())
        {
            destRequestMsg.applyHasUserNameType();
            destRequestMsg.userNameType(userNameType);
        }
        if (checkHasAttrib())
        {
            destRequestMsg.applyHasAttrib();
            attrib().copy(destRequestMsg.attrib());
        }
        if (checkHasDownloadConnectionConfig())
        {
            destRequestMsg.applyHasDownloadConnectionConfig();
            destRequestMsg.downloadConnectionConfig(downloadConnectionConfig);
        }
        if (checkHasInstanceId())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.instanceId.length());
            this.instanceId.copy(byteBuffer);
            destRequestMsg.applyHasInstanceId();
            destRequestMsg.instanceId().data(byteBuffer);
        }
        if (checkHasPassword())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.password.length());
            this.password.copy(byteBuffer);
            destRequestMsg.applyHasPassword();
            destRequestMsg.password().data(byteBuffer);
        }
        if (checkHasRole())
        {
            destRequestMsg.applyHasRole();
            destRequestMsg.role(role);
        }
        
        if (checkHasAuthenticationExtended())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.authenticationExtended.length());
            this.authenticationExtended.copy(byteBuffer);
            destRequestMsg.applyHasAuthenticationExtended();
            destRequestMsg.authenticationExtended().data(byteBuffer);
        }
        
        return CodecReturnCodes.SUCCESS;
    }

    LoginRequestImpl()
    {
        password = CodecFactory.createBuffer();
        instanceId = CodecFactory.createBuffer();
        authenticationToken = CodecFactory.createBuffer();
        authenticationExtended = CodecFactory.createBuffer();
        attrib = new LoginAttribImpl();
        try
        {
            defaultUsername = System.getProperty("user.name");
        }
        catch (Exception e)
        {
            defaultUsername = "upa";
        }
        userName = CodecFactory.createBuffer();
      
        initDefaultRequest(1);
    }
    
   
    
    public void initDefaultRequest(int streamId)
    {
        clear();

        streamId(streamId);
        userName().data(defaultUsername);
        applyHasUserNameType(); 
        userNameType(Login.UserIdTypes.NAME);
        applyHasAttrib();
        ((LoginAttribImpl)attrib).initDefaultAttrib();
    }

    public void clear()
    {
        super.clear();
        userName.clear();
        userNameType = Login.UserIdTypes.NAME;
        flags = 0;
        password.clear();
        instanceId.clear();
        role = Login.RoleTypes.CONS;
        downloadConnectionConfig = 0;
        attrib.clear();
        authenticationToken.clear();
        authenticationExtended.clear();
    }
    
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.REQUEST)
            return CodecReturnCodes.FAILURE;
        
        RequestMsg requestMsg = (RequestMsg)msg;
        
        //All login requests should be streaming
        if((requestMsg.flags() & RequestMsgFlags.STREAMING) == 0)
            return CodecReturnCodes.FAILURE;
        
        if((requestMsg.flags() & RequestMsgFlags.NO_REFRESH) != 0)
            applyNoRefresh();
        
        if((requestMsg.flags() & RequestMsgFlags.PAUSE) != 0)
            applyPause();
        streamId(msg.streamId());
        
        MsgKey msgKey = msg.msgKey();
        if (msgKey == null || !msgKey.checkHasName() || (msgKey.checkHasAttrib() && msgKey.attribContainerType() != DataTypes.ELEMENT_LIST))
            return CodecReturnCodes.FAILURE;

        Buffer userName = msgKey.name();
        userName().data(userName.data(), userName.position(), userName.length());
        if (msgKey.checkHasNameType())
        {
            applyHasUserNameType();
            userNameType(msgKey.nameType());
        }
  
        if (msgKey.checkHasAttrib())
        {
            int ret = msg.decodeKeyAttrib(dIter, msgKey);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            return decodeAttrib(dIter);
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int decodeAttrib(DecodeIterator dIter)
    {
        elementList.clear();
        int ret = elementList.decode(dIter, null);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        elementEntry.clear();
        while ((ret = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (elementEntry.name().equals(ElementNames.ALLOW_SUSPECT_DATA))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;

                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAttrib();
                attrib.applyHasAllowSuspectData();
                attrib.allowSuspectData(tmpUInt.toLong());
            }
            else if (elementEntry.name().equals(ElementNames.APPID))
            {
                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
                    return CodecReturnCodes.FAILURE;

                applyHasAttrib();
                Buffer applicationId = elementEntry.encodedData();
                attrib.applyHasApplicationId();
                attrib.applicationId().data(applicationId.data(), applicationId.position(), applicationId.length());

            }
            else if (elementEntry.name().equals(ElementNames.APPNAME))
            {
                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
                    return CodecReturnCodes.FAILURE;

                applyHasAttrib();
                Buffer applicationName = elementEntry.encodedData();
                attrib.applyHasApplicationName();
                attrib.applicationName().data(applicationName.data(), applicationName.position(), applicationName.length());

            }
            else if (elementEntry.name().equals(ElementNames.POSITION))
            {
                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
                    return CodecReturnCodes.FAILURE;

                applyHasAttrib();
                Buffer position = elementEntry.encodedData();
                attrib.applyHasPosition();
                attrib.position().data(position.data(), position.position(), position.length());

            }
            else if (elementEntry.name().equals(ElementNames.PASSWORD))
            {
                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
                    return CodecReturnCodes.FAILURE;
                

                Buffer password = elementEntry.encodedData();
                applyHasAttrib();
                applyHasPassword();
                password().data(password.data(), password.position(), password.length());

            }
            else if (elementEntry.name().equals(ElementNames.AUTHN_TOKEN))
            {
                if (elementEntry.dataType() != DataTypes.ASCII_STRING
                        && elementEntry.dataType() != DataTypes.BUFFER)
                    return CodecReturnCodes.FAILURE;
                
                Buffer authenticationToken = elementEntry.encodedData();
                userName().data(authenticationToken.data(),
                                           authenticationToken.position(),
                                           authenticationToken.length());

            }
            else if (elementEntry.name().equals(ElementNames.AUTHN_EXTENDED))
            {
                if (elementEntry.dataType() != DataTypes.ASCII_STRING
                        && elementEntry.dataType() != DataTypes.BUFFER)
                    return CodecReturnCodes.FAILURE;

                Buffer authenticationExtended = elementEntry.encodedData();
                applyHasAuthenticationExtended();
                authenticationExtended().data(authenticationExtended.data(),
                                              authenticationExtended.position(),
                                              authenticationExtended.length());
            }
            else if (elementEntry.name().equals(ElementNames.INST_ID))
            {
                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
                    return CodecReturnCodes.FAILURE;

                Buffer instanceId = elementEntry.encodedData();
                applyHasInstanceId();
                instanceId().data(instanceId.data(), instanceId.position(), instanceId.length());

            }
            else if (elementEntry.name().equals(ElementNames.DOWNLOAD_CON_CONFIG))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasDownloadConnectionConfig();
                downloadConnectionConfig(tmpUInt.toLong());
            }
            else if (elementEntry.name().equals(ElementNames.PROV_PERM_EXP))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAttrib();
                attrib.applyHasProvidePermissionExpressions();
                attrib.providePermissionExpressions(tmpUInt.toLong());
            }
            else if (elementEntry.name().equals(ElementNames.PROV_PERM_PROF))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAttrib();
                attrib.applyHasProvidePermissionProfile();
                attrib.providePermissionProfile(tmpUInt.toLong());
            }
            else if (elementEntry.name().equals(ElementNames.SINGLE_OPEN))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                applyHasAttrib();
                attrib.applyHasSingleOpen();
                attrib.singleOpen(tmpUInt.toLong());
            }
            else if (elementEntry.name().equals(ElementNames.ROLE))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasRole();
                role(tmpUInt.toLong());
            }
            else if (elementEntry.name().equals(ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
            {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                
                applyHasAttrib();
                attrib.applyHasProviderSupportDictionaryDownload();
                attrib.supportProviderDictionaryDownload(tmpUInt.toLong());
            } else if (elementEntry.name().equals(ElementNames.ROUND_TRIP_LATENCY)) {
                if (elementEntry.dataType() != DataTypes.UINT) {
                    return CodecReturnCodes.FAILURE;
                }
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS) {
                    return ret;
                }
                applyHasAttrib();
                attrib.applyHasSupportRoundTripLatencyMonitoring();
                attrib.supportRTTMonitoring(tmpUInt.toLong());
            }
        }
        return CodecReturnCodes.SUCCESS;
    }
    public int encode(EncodeIterator encodeIter)
    {
        requestMsg.clear();

        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(streamId());
        requestMsg.domainType(DomainTypes.LOGIN);
        requestMsg.containerType(DataTypes.NO_DATA);

        requestMsg.applyStreaming();

        if (checkNoRefresh())
            requestMsg.applyNoRefresh();
        if (checkPause())
            requestMsg.applyPause();

        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name(userName());
        if (checkHasUserNameType())
        {
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(userNameType());
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                requestMsg.msgKey().name().data(blankStringConst);
        }

        requestMsg.msgKey().applyHasAttrib();
        requestMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        int ret = requestMsg.encodeInit(encodeIter, 0);
        if (ret != CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB)
            return ret;
        ret = encodeAttrib(encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        if ((ret = requestMsg.encodeKeyAttribComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
       
        if ((ret = requestMsg.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            return ret;

        return CodecReturnCodes.SUCCESS;
    }
    
    private int encodeAttrib(EncodeIterator encodeIter)
    {
        elementEntry.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        int ret;
        if ((ret = elementList.encodeInit(encodeIter, null, 0)) != CodecReturnCodes.SUCCESS)
            return ret;

        if (checkHasAttrib() && attrib.checkHasApplicationId() && attrib.applicationId().length() != 0)
        {
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.APPID);
            if ((ret = elementEntry.encode(encodeIter, attrib.applicationId())) != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasAttrib() && attrib.checkHasApplicationName() && attrib.applicationName().length() != 0)
        {
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.APPNAME);
            if ((ret = elementEntry.encode(encodeIter, attrib.applicationName())) != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasAttrib() && attrib.checkHasPosition() && attrib.position().length() != 0)
        {
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.POSITION);
            if ((ret = elementEntry.encode(encodeIter, attrib.position())) != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasPassword() && password().length() != 0)
        {
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.PASSWORD);
            if ((ret = elementEntry.encode(encodeIter, password())) != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasAttrib() && attrib.checkHasProvidePermissionProfile())
        {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.PROV_PERM_PROF);
            tmpUInt.value(attrib.providePermissionProfile());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasAttrib() && attrib.checkHasProvidePermissionExpressions())
        {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.PROV_PERM_EXP);
            tmpUInt.value(attrib.providePermissionExpressions());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasAttrib() && attrib.checkHasSingleOpen())
        {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.SINGLE_OPEN);
            tmpUInt.value(attrib.singleOpen());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasAttrib() && attrib.checkHasAllowSuspectData())
        {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.ALLOW_SUSPECT_DATA);
            tmpUInt.value(attrib.allowSuspectData());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }
        
        if (checkHasAttrib() && attrib.checkHasProviderSupportDictionaryDownload())
        {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD);
            tmpUInt.value(attrib.supportProviderDictionaryDownload());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }
        
        if (checkHasUserNameType() && userNameType == Login.UserIdTypes.AUTHN_TOKEN && userName().length() != 0)
        {
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.AUTHN_TOKEN);
            if ((ret = elementEntry.encode(encodeIter, userName())) != CodecReturnCodes.SUCCESS)
                return ret;

            if (checkHasAuthenticationExtended() && authenticationExtended().length() != 0)
            {
                elementEntry.dataType(DataTypes.BUFFER);
                elementEntry.name(ElementNames.AUTHN_EXTENDED);
                if ((ret = elementEntry.encode(encodeIter, authenticationExtended())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        
        if (checkHasInstanceId() && instanceId().length() != 0)
        {
            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.INST_ID);
            ret = elementEntry.encode(encodeIter, instanceId());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasRole())
        {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.ROLE);
            tmpUInt.value(role());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasDownloadConnectionConfig())
        {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.DOWNLOAD_CON_CONFIG);
            tmpUInt.value(downloadConnectionConfig());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
  
        }

        if (checkHasAttrib() && attrib.checkHasSupportRoundTripLatencyMonitoring()) {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.ROUND_TRIP_LATENCY);
            tmpUInt.value(attrib.supportRTTMonitoring());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS) {
                return ret;
            }
        }

        if ((ret = elementList.encodeComplete(encodeIter, true)) != CodecReturnCodes.SUCCESS)
            return ret;
        
        return CodecReturnCodes.SUCCESS;
    }
    
    public void applyPause()
    {
        flags |= LoginRequestFlags.PAUSE_ALL;
    }
    
    public boolean checkPause()
    {
        return (flags & LoginRequestFlags.PAUSE_ALL) != 0;
    }

    public void applyNoRefresh()
    {
        flags |= LoginRequestFlags.NO_REFRESH;
    }
    
    public boolean checkNoRefresh()
    {
        return (flags & LoginRequestFlags.NO_REFRESH) != 0;
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
    
    public void userNameType(int userNameType)
    {
        this.userNameType = userNameType;
    }

    public boolean checkHasUserNameType()
    {
        return (flags & LoginRequestFlags.HAS_USERNAME_TYPE) != 0;
    }
    
    public void applyHasUserNameType()
    {
        flags |= LoginRequestFlags.HAS_USERNAME_TYPE;
    }
    
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();

        stringBuf.insert(0, "LoginRequest: \n");
        stringBuf.append(tab);
        stringBuf.append("userName: ");
        stringBuf.append(userName().toString());
        stringBuf.append(eol);
        stringBuf.append(tab);
        stringBuf.append("streaming: ");
        stringBuf.append("true");
        stringBuf.append(eol);

        if (checkHasUserNameType())
        {
            stringBuf.append(tab);
            stringBuf.append("nameType: ");
            stringBuf.append(userNameType());
            stringBuf.append(eol);
        }
        
        if (checkPause())
        {
            stringBuf.append(tab);
            stringBuf.append("pauseAll:");
            stringBuf.append("true");
            stringBuf.append(eol);
        }
        if (checkNoRefresh())
        {
            stringBuf.append(tab);
            stringBuf.append("noRefresh:");
            stringBuf.append("true");
            stringBuf.append(eol);
        }
        
        if(checkHasAttrib())
        {
            stringBuf.append(attrib().toString());
        }
        
        if (checkHasDownloadConnectionConfig())
        {
            stringBuf.append(tab);
            stringBuf.append("downloadConnectionConfig: ");
            stringBuf.append(downloadConnectionConfig());
            stringBuf.append(eol);
        }
        if (checkHasInstanceId())
        {
            stringBuf.append(tab);
            stringBuf.append("instanceId: ");
            stringBuf.append(instanceId());
            stringBuf.append(eol);
        }
        
        if (checkHasRole())
        {
            stringBuf.append(tab);
            stringBuf.append("role: ");
            stringBuf.append(role());
            stringBuf.append(eol);
        }
        
        if (checkHasAuthenticationExtended())
        {
            stringBuf.append(tab);
            stringBuf.append("authenticationExtended: ");
            stringBuf.append(authenticationExtended());
            stringBuf.append(eol);
        }
        
        return stringBuf.toString();
    }
    
    public long downloadConnectionConfig()
    {
        return downloadConnectionConfig;
    }

    public void downloadConnectionConfig(long downloadConnectionConfig)
    {
        assert(checkHasDownloadConnectionConfig());
        this.downloadConnectionConfig = downloadConnectionConfig;
    }
    
    public void applyHasDownloadConnectionConfig()
    {
         flags |= LoginRequestFlags.HAS_DOWNLOAD_CONN_CONFIG;
    }
    
    public boolean checkHasDownloadConnectionConfig()
    {
         return (flags & LoginRequestFlags.HAS_DOWNLOAD_CONN_CONFIG) != 0;
    }

    public Buffer instanceId()
    {
        return instanceId;
    }
    
    public void instanceId(Buffer instanceId)
    {
        assert(checkHasInstanceId()) : "instanceId flag should be set first";
        assert (instanceId != null) : "instanceId can not be null";

        instanceId().data(instanceId.data(), instanceId.position(), instanceId.length());
    }

    public void applyHasInstanceId()
    {
        flags |= LoginRequestFlags.HAS_INSTANCE_ID;
    }

    public boolean checkHasInstanceId()
    {
        return (flags & LoginRequestFlags.HAS_INSTANCE_ID) != 0;
    }
    
    public Buffer password()
    {
        return password;
    }

    public void applyHasPassword()
    {
        flags |= LoginRequestFlags.HAS_PASSWORD;
    }

    public boolean checkHasPassword()
    {
        return (flags & LoginRequestFlags.HAS_PASSWORD) != 0;
    }
    
    public long role()
    {
        return role;
    }

    public void role(long role)
    {
        assert(checkHasRole());
        this.role = role;
    }
    
    public void applyHasRole()
    {
        flags |= LoginRequestFlags.HAS_ROLE;
    }

    public boolean checkHasRole()
    {
        return (flags & LoginRequestFlags.HAS_ROLE) != 0;
    }
    
    public boolean checkHasAttrib()
    {
        return (flags & LoginRequestFlags.HAS_ATTRIB) != 0;
    }
    
    public void applyHasAttrib()
    {
        flags |= LoginRequestFlags.HAS_ATTRIB;
    }
    
    public LoginAttrib attrib()
    {
        return attrib;
    }
    
    public void attrib(LoginAttrib attrib)
    {
        assert(attrib != null) : "attrib can not be null";
        assert(checkHasAttrib());
        LoginAttribImpl loginAttribImpl = (LoginAttribImpl)attrib;
        loginAttribImpl.copyReferences(loginAttribImpl);
    }
    
    
    public Buffer authenticationExtended()
    {
        return authenticationExtended;
    }

    public void authenticationExtended(Buffer authenticationExtended)
    {
        assert (authenticationExtended != null) : "authenticationExtended can not be null";
        assert (checkHasAuthenticationExtended()) : "authenticationExtended flag should be set first";
        authenticationExtended().data(authenticationExtended.data(),
                                      authenticationExtended.position(),
                                      authenticationExtended.length());
    }

    public boolean checkHasAuthenticationExtended()
    {
        return (flags & LoginRequestFlags.HAS_AUTHENTICATION_EXTENDED) != 0;
    }

    public void applyHasAuthenticationExtended()
    {
        flags |= LoginRequestFlags.HAS_AUTHENTICATION_EXTENDED;
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.LOGIN;
    }
}