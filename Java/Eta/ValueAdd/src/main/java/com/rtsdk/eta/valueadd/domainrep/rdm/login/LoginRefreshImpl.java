package com.rtsdk.eta.valueadd.domainrep.rdm.login;

import java.nio.ByteBuffer;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.ElementEntry;
import com.rtsdk.eta.codec.ElementList;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.MsgKey;
import com.rtsdk.eta.codec.RefreshMsg;
import com.rtsdk.eta.codec.State;
import com.rtsdk.eta.codec.StateCodes;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.codec.UInt;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.rdm.ElementNames;
import com.rtsdk.eta.rdm.Login;
import com.rtsdk.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class LoginRefreshImpl extends MsgBaseImpl
{
    private int flags;

    private long sequenceNumber;
    private State state;
    private Buffer userName;
    private int userNameType;
    private LoginAttrib attrib;
    private LoginSupportFeatures features;
    private LoginConnectionConfig connectionConfig;
    private long authenticationTTReissue;
    private Buffer authenticationExtendedResp;
    private long authenticationErrorCode;
    private Buffer authenticationErrorText;
    private static final String blankStringConst = new String(new byte[] { 0x0 });

    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry element = CodecFactory.createElementEntry();
    private UInt tmpUInt = CodecFactory.createUInt();
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

    LoginRefreshImpl()
    {
        state = CodecFactory.createState();
        userName = CodecFactory.createBuffer();
        attrib = new LoginAttribImpl();
        features = new LoginSupportFeaturesImpl();
        connectionConfig = new LoginConnectionConfigImpl();
        authenticationExtendedResp = CodecFactory.createBuffer();
        authenticationErrorText = CodecFactory.createBuffer();
    }

    public void clear()
    {
        super.clear();
        state.clear();
        state.streamState(StreamStates.OPEN);
        state.dataState(DataStates.OK);
        state.code(StateCodes.NONE);
        flags = 0;
        userNameType = Login.UserIdTypes.NAME;
        userName.clear();
        sequenceNumber = 0;
        authenticationTTReissue = 0;
        authenticationErrorCode = 0;
        authenticationExtendedResp.clear();
        authenticationErrorText.clear();
        clearAttrib();
        connectionConfig.clear();
    }

    private void clearAttrib()
    {
        attrib.clear();
        features.clear();
    }

    public int copy(LoginRefresh destRefreshMsg)
    {
        assert (destRefreshMsg != null) : "destRefreshMsg must be non-null";
        destRefreshMsg.streamId(streamId());
        if (checkHasUserName())
        {
            destRefreshMsg.applyHasUserName();
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.userName.length());
            this.userName.copy(byteBuffer);
            destRefreshMsg.applyHasUserName();
            destRefreshMsg.userName().data(byteBuffer);
        }
        if (checkHasUserNameType())
        {
            destRefreshMsg.applyHasUserNameType();
            destRefreshMsg.userNameType(userNameType);
        }
        if (checkHasSequenceNumber())
        {
            destRefreshMsg.applyHasSequenceNumber();
            destRefreshMsg.sequenceNumber(sequenceNumber);
        }

        if (checkHasAuthenticationTTReissue())
        {
            destRefreshMsg.applyHasAuthenticationTTReissue();
            destRefreshMsg.authenticationTTReissue(authenticationTTReissue);
        }
        if (checkHasAuthenticationExtendedResp())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.authenticationExtendedResp.length());
            authenticationExtendedResp.copy(byteBuffer);
            destRefreshMsg.applyHasAuthenticationExtendedResp();
            destRefreshMsg.authenticationExtendedResp().data(byteBuffer);
        }
        if (checkHasAuthenticationErrorCode())
        {
            destRefreshMsg.applyHasAuthenticationErrorCode();
            destRefreshMsg.authenticationErrorCode(authenticationErrorCode);
        }
        if (checkHasAuthenticationErrorText())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.authenticationErrorText.length());
            authenticationErrorText.copy(byteBuffer);
            destRefreshMsg.applyHasAuthenticationErrorText();
            destRefreshMsg.authenticationErrorText().data(byteBuffer);
        }

        if (checkClearCache())
        {
            destRefreshMsg.applyClearCache();
        }
        if (checkSolicited())
        {
            destRefreshMsg.applySolicited();
        }

        // state
        {
            destRefreshMsg.state().streamState(this.state.streamState());
            destRefreshMsg.state().dataState(this.state.dataState());
            destRefreshMsg.state().code(this.state.code());
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.state.text().length());
            this.state.text().copy(byteBuffer);
            destRefreshMsg.state().text().data(byteBuffer);
        }

        if (checkHasConnectionConfig())
        {
            destRefreshMsg.applyHasConnectionConfig();
            connectionConfig().copy(destRefreshMsg.connectionConfig());
        }

        if (checkHasAttrib())
        {
            destRefreshMsg.applyHasAttrib();
            attrib().copy(destRefreshMsg.attrib());
        }
        if (checkHasFeatures())
        {
            destRefreshMsg.applyHasFeatures();
            LoginSupportFeatures thisfeatures = features();
            LoginSupportFeatures destfeatures = destRefreshMsg.features();
            if (thisfeatures.checkHasSupportBatchRequests())
            {
                destfeatures.applyHasSupportBatchRequests();
                destfeatures.supportBatchRequests(thisfeatures.supportBatchRequests());
            }
            if (thisfeatures.checkHasSupportBatchReissues())
            {
                destfeatures.applyHasSupportBatchReissues();
                destfeatures.supportBatchReissues(thisfeatures.supportBatchReissues());
            }
            if (thisfeatures.checkHasSupportBatchCloses())
            {
                destfeatures.applyHasSupportBatchCloses();
                destfeatures.supportBatchCloses(thisfeatures.supportBatchCloses());
            }
            if (thisfeatures.checkHasSupportViewRequests())
            {
                destfeatures.applyHasSupportViewRequests();
                destfeatures.supportViewRequests(thisfeatures.supportViewRequests());
            }
            if (thisfeatures.checkHasSupportOptimizedPauseResume())
            {
                destfeatures.applyHasSupportOptimizedPauseResume();
                destfeatures.supportOptimizedPauseResume(thisfeatures.supportOptimizedPauseResume());
            }
            if (thisfeatures.checkHasSupportPost())
            {
                destfeatures.applyHasSupportPost();
                destfeatures.supportOMMPost(thisfeatures.supportOMMPost());
            }
            if (thisfeatures.checkHasSupportStandby())
            {
                destfeatures.applyHasSupportStandby();
                destfeatures.supportStandby(thisfeatures.supportStandby());
            }
            if (thisfeatures.checkHasSupportProviderDictionaryDownload())
            {
                destfeatures.applyHasSupportProviderDictionaryDownload();
                destfeatures.supportProviderDictionaryDownload(thisfeatures.supportProviderDictionaryDownload());
            }            
        }

        return CodecReturnCodes.SUCCESS;
    }

    public void flags(int flags)
    {
        this.flags = flags;
    }

    public int flags()
    {
        return flags;
    }

    public int encode(EncodeIterator encodeIter)
    {
        refreshMsg.clear();

        // message header
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.streamId(streamId());
        refreshMsg.domainType(DomainTypes.LOGIN);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.applyRefreshComplete();
        refreshMsg.state().dataState(state().dataState());
        refreshMsg.state().streamState(state().streamState());
        refreshMsg.state().code(state().code());
        refreshMsg.state().text(state().text());

        if (checkClearCache())
            refreshMsg.applyClearCache();
        if (checkSolicited())
            refreshMsg.applySolicited();

        if (checkHasSequenceNumber())
        {
            refreshMsg.applyHasSeqNum();
            refreshMsg.seqNum(sequenceNumber());
        }

        if (checkHasUserName())
        {
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name(userName());
            refreshMsg.msgKey().nameType(userNameType());
        }

        if (checkHasUserNameType())
        {
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                refreshMsg.msgKey().applyHasName();
                refreshMsg.msgKey().name().data(blankStringConst);
            }
            refreshMsg.msgKey().applyHasNameType();
            refreshMsg.msgKey().nameType(userNameType());
        }

        // key attrib
        refreshMsg.msgKey().applyHasAttrib();
        refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        if (checkHasConnectionConfig())
        {
            refreshMsg.containerType(DataTypes.ELEMENT_LIST);
        }

        int ret = refreshMsg.encodeInit(encodeIter, 0);
        if (ret != CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB)
            return ret;
        ret = encodeAttrib(encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        ret = refreshMsg.encodeKeyAttribComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        // encode conn config now, if specified
        if (checkHasConnectionConfig())
        {
            ret = ((LoginConnectionConfigImpl)connectionConfig).encode(encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        ret = refreshMsg.encodeComplete(encodeIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        return CodecReturnCodes.SUCCESS;
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.REFRESH)
            return CodecReturnCodes.FAILURE;
        streamId(msg.streamId());

        RefreshMsg refreshMsg = (RefreshMsg)msg;
        if (refreshMsg.checkSolicited())
            applySolicited();
        if (refreshMsg.checkClearCache())
            applyClearCache();

        state().streamState(refreshMsg.state().streamState());
        state().dataState(refreshMsg.state().dataState());
        state().code(refreshMsg.state().code());
        if (refreshMsg.state().text().length() > 0)
        {
            Buffer text = refreshMsg.state().text();
            this.state.text().data(text.data(), text.position(), text.length());
        }

        if (refreshMsg.checkHasSeqNum())
        {
            applyHasSequenceNumber();
            sequenceNumber(refreshMsg.seqNum());
        }

        MsgKey msgKey = msg.msgKey();
        if (msgKey == null || (msgKey.checkHasAttrib() && msgKey.attribContainerType() != DataTypes.ELEMENT_LIST))
            return CodecReturnCodes.FAILURE;

        if (msgKey.checkHasName() && msgKey.name() != null)
        {
            applyHasUserName();
            Buffer name = msgKey.name();
            userName().data(name.data(), name.position(), name.length());
        }

        if (msgKey.checkHasNameType())
        {
            applyHasUserNameType();
            userNameType(msgKey.nameType());
        }

        if (msg.containerType() == DataTypes.ELEMENT_LIST)
        {
            int ret = decodePayload(dIter, refreshMsg);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
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

    private int decodePayload(DecodeIterator dIter, Msg msg)
    {
        assert (msg.containerType() == DataTypes.ELEMENT_LIST) : "element list expected in login refresh payload";

        // decode payload containing connection config in login refresh
        int ret = elementList.decode(dIter, null);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        // decode each element entry in list
        while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            //connectionconfig
            if (element.name().equals(ElementNames.CONNECTION_CONFIG))
            {
                applyHasConnectionConfig();
                if (element.dataType() != DataTypes.VECTOR)
                {
                    return CodecReturnCodes.FAILURE;
                }
                ret = ((LoginConnectionConfigImpl)connectionConfig).decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
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

            if (element.name().equals(ElementNames.ALLOW_SUSPECT_DATA))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;

                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAttrib();
                attrib.applyHasAllowSuspectData();
                attrib.allowSuspectData(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.APPID))
            {
                if (element.dataType() != DataTypes.ASCII_STRING)
                    return CodecReturnCodes.FAILURE;

            	applyHasAttrib();
                Buffer applicationId = element.encodedData();
                attrib.applyHasApplicationId();
                attrib.applicationId().data(applicationId.data(), applicationId.position(), applicationId.length());

            }
            else if (element.name().equals(ElementNames.APPNAME))
            {
                if (element.dataType() != DataTypes.ASCII_STRING)
                    return CodecReturnCodes.FAILURE;

                applyHasAttrib();
                Buffer applicationName = element.encodedData();
                attrib.applyHasApplicationName();
                attrib.applicationName().data(applicationName.data(), applicationName.position(), applicationName.length());

            }
            else if (element.name().equals(ElementNames.POSITION))
            {
                if (element.dataType() != DataTypes.ASCII_STRING)
                    return CodecReturnCodes.FAILURE;

                applyHasAttrib();
                Buffer position = element.encodedData();
                attrib.applyHasPosition();
                attrib.position().data(position.data(), position.position(), position.length());

            }
            else if (element.name().equals(ElementNames.PROV_PERM_EXP))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAttrib();
                attrib.applyHasProvidePermissionExpressions();
                attrib.providePermissionExpressions(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.PROV_PERM_PROF))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAttrib();
                attrib.applyHasProvidePermissionProfile();
                attrib.providePermissionProfile(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.SINGLE_OPEN))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAttrib();
                attrib.applyHasSingleOpen();
                attrib.singleOpen(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.SUPPORT_POST))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasFeatures();
                features.applyHasSupportPost();
                features.supportOMMPost(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.SUPPORT_STANDBY))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasFeatures();
                features.applyHasSupportStandby();
                features.supportStandby(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.SUPPORT_BATCH))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasFeatures();
                if ((tmpUInt.toLong() & Login.BatchSupportFlags.SUPPORT_REQUESTS) > 0)
                {
                    features.applyHasSupportBatchRequests();
                    features.supportBatchRequests(1);
                }
                if ((tmpUInt.toLong() & Login.BatchSupportFlags.SUPPORT_REISSUES) > 0)
                {
                    features.applyHasSupportBatchReissues();
                    features.supportBatchReissues(1);
                }
                if ((tmpUInt.toLong() & Login.BatchSupportFlags.SUPPORT_CLOSES) > 0)
                {
                    features.applyHasSupportBatchCloses();
                    features.supportBatchCloses(1);
                }
            }
            else if (element.name().equals(ElementNames.SUPPORT_VIEW))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasFeatures();
                features.applyHasSupportViewRequests();
                features.supportViewRequests(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.SUPPORT_OPR))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasFeatures();
                features.applyHasSupportOptimizedPauseResume();
                features.supportOptimizedPauseResume(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasFeatures();
                features.applyHasSupportProviderDictionaryDownload();
                features.supportProviderDictionaryDownload(tmpUInt.toLong());
            }            
            else if (element.name().equals(ElementNames.AUTHN_TT_REISSUE))
            {
                if (element.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;
                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasAuthenticationTTReissue();
                authenticationTTReissue(tmpUInt.toLong());
            }
            else if (element.name().equals(ElementNames.AUTHN_EXTENDED_RESP))
            {
                if (element.dataType() != DataTypes.ASCII_STRING
                        && element.dataType() != DataTypes.BUFFER)
                    return CodecReturnCodes.FAILURE;

                Buffer authenticationExtendedResp = element.encodedData();
                applyHasAuthenticationExtendedResp();
                authenticationExtendedResp().data(authenticationExtendedResp.data(),
                                                  authenticationExtendedResp.position(),
                                                  authenticationExtendedResp.length());

            }
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

            } else if (element.name().equals(ElementNames.ROUND_TRIP_LATENCY)) {
                if (element.dataType() != DataTypes.UINT) {
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

    public void applySolicited()
    {
        flags |= LoginRefreshFlags.SOLICITED;
    }

    public boolean checkSolicited()
    {
        return (flags & LoginRefreshFlags.SOLICITED) != 0;
    }

    public void applyClearCache()
    {
        flags |= LoginRefreshFlags.CLEAR_CACHE;
    }

    public boolean checkClearCache()
    {
        return (flags & LoginRefreshFlags.CLEAR_CACHE) != 0;
    }

    public Buffer userName()
    {
        return userName;
    }

    public void userName(Buffer userName)
    {
        assert (userName != null) : "userName can not be null";
        userName().data(userName.data(), userName.position(), userName.length());
    }

    public boolean checkHasUserName()
    {
        return (flags() & LoginRefreshFlags.HAS_USERNAME) != 0;
    }

    public void applyHasUserName()
    {
        flags |= LoginRefreshFlags.HAS_USERNAME;
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
        return (flags() & LoginRefreshFlags.HAS_USERNAME_TYPE) != 0;
    }

    public void applyHasUserNameType()
    {
        flags |= LoginRefreshFlags.HAS_USERNAME_TYPE;
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

    public long sequenceNumber()
    {
        return sequenceNumber;
    }

    public void sequenceNumber(long sequenceNumber)
    {
        assert (checkHasSequenceNumber());
        this.sequenceNumber = sequenceNumber;
    }

    public boolean checkHasSequenceNumber()
    {
        return (flags() & LoginRefreshFlags.HAS_SEQ_NUM) != 0;
    }

    public void applyHasSequenceNumber()
    {
        flags |= LoginRefreshFlags.HAS_SEQ_NUM;
    }

    public void authenticationTTReissue(long authenticationTTReissue)
    {
        assert (checkHasAuthenticationTTReissue());
        this.authenticationTTReissue = authenticationTTReissue;
    }

    public long authenticationTTReissue()
    {
        return authenticationTTReissue;
    }

    public boolean checkHasAuthenticationTTReissue()
    {
        return (flags() & LoginRefreshFlags.HAS_AUTHENTICATION_TT_REISSUE) != 0;
    }

    public void applyHasAuthenticationTTReissue()
    {
        flags |= LoginRefreshFlags.HAS_AUTHENTICATION_TT_REISSUE;
    }

    public void authenticationExtendedResp(Buffer authenticationExtendedResp)
    {
        assert (checkHasAuthenticationExtendedResp());
        authenticationExtendedResp.data(authenticationExtendedResp.data(),
                                        authenticationExtendedResp.position(),
                                        authenticationExtendedResp.length());
    }

    public Buffer authenticationExtendedResp()
    {
        return authenticationExtendedResp;
    }

    public boolean checkHasAuthenticationExtendedResp()
    {
        return (flags() & LoginRefreshFlags.HAS_AUTHENTICATION_EXTENDED_RESP) != 0;
    }

    public void applyHasAuthenticationExtendedResp()
    {
        flags |= LoginRefreshFlags.HAS_AUTHENTICATION_EXTENDED_RESP;
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
        return (flags() & LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_CODE) != 0;
    }

    public void applyHasAuthenticationErrorCode()
    {
        flags |= LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_CODE;
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
        return (flags() & LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_TEXT) != 0;
    }

    public void applyHasAuthenticationErrorText()
    {
        flags |= LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_TEXT;
    }
    
    public LoginAttrib attrib()
    {
        return attrib;
    }

    public void attrib(LoginAttrib attrib)
    {
        assert (attrib != null) : "attrib can not be null";
        assert (checkHasAttrib());
        LoginAttribImpl loginAttribImpl = (LoginAttribImpl)attrib;
        loginAttribImpl.copyReferences(loginAttribImpl);
    }

    public boolean checkHasAttrib()
    {
        return (flags() & LoginRefreshFlags.HAS_ATTRIB) != 0;
    }

    public void applyHasAttrib()
    {
        flags |= LoginRefreshFlags.HAS_ATTRIB;
    }

    public LoginSupportFeatures features()
    {
        return features;
    }

    public void features(LoginSupportFeatures features)
    {
        assert (checkHasFeatures());
        this.features = features;
    }

    public boolean checkHasFeatures()
    {
        return (flags() & LoginRefreshFlags.HAS_FEATURES) != 0;
    }

    public void applyHasFeatures()
    {
        flags |= LoginRefreshFlags.HAS_FEATURES;
    }

    private int encodeAttrib(EncodeIterator encodeIter)
    {
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        int ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        if (checkHasAuthenticationTTReissue())
        {
            element.dataType(DataTypes.UINT);
            element.name(ElementNames.AUTHN_TT_REISSUE);
            tmpUInt.value(authenticationTTReissue());
            if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (checkHasAuthenticationExtendedResp() && authenticationExtendedResp().length() != 0)
        {
            element.dataType(DataTypes.ASCII_STRING);
            element.name(ElementNames.AUTHN_EXTENDED_RESP);
            ret = element.encode(encodeIter, authenticationExtendedResp());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (checkHasAuthenticationErrorCode())
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
        
        if (checkHasAttrib())
        {
            if (attrib().checkHasApplicationId() && attrib().applicationId().length() != 0)
            {
                element.dataType(DataTypes.ASCII_STRING);
                element.name(ElementNames.APPID);
                ret = element.encode(encodeIter, attrib.applicationId());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (attrib().checkHasApplicationName() && attrib().applicationName().length() != 0)
            {
                element.dataType(DataTypes.ASCII_STRING);
                element.name(ElementNames.APPNAME);
                ret = element.encode(encodeIter, attrib.applicationName());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (attrib().checkHasPosition() && attrib().position().length() != 0)
            {
                element.dataType(DataTypes.ASCII_STRING);
                element.name(ElementNames.POSITION);
                ret = element.encode(encodeIter, attrib.position());
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (attrib().checkHasProvidePermissionProfile())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.PROV_PERM_PROF);
                tmpUInt.value(attrib.providePermissionProfile());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (attrib().checkHasProvidePermissionExpressions())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.PROV_PERM_EXP);
                tmpUInt.value(attrib.providePermissionExpressions());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (attrib().checkHasSingleOpen())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.SINGLE_OPEN);
                tmpUInt.value(attrib.singleOpen());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (attrib().checkHasAllowSuspectData())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.ALLOW_SUSPECT_DATA);
                tmpUInt.value(attrib.allowSuspectData());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (attrib().checkHasSupportRoundTripLatencyMonitoring()) {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.ROUND_TRIP_LATENCY);
                tmpUInt.value(attrib.supportRTTMonitoring());
                ret = element.encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS) {
                    return ret;
                }
            }
        }

        if (checkHasFeatures())
        {
            if (features().checkHasSupportPost())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.SUPPORT_POST);
                tmpUInt.value(features().supportOMMPost());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (features().checkHasSupportBatchRequests() ||
                features().checkHasSupportBatchReissues() ||
                features().checkHasSupportBatchCloses())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.SUPPORT_BATCH);
                int temp = 0;
                if (features().checkHasSupportBatchRequests())
                    temp |= Login.BatchSupportFlags.SUPPORT_REQUESTS;
                if (features().checkHasSupportBatchReissues())
                    temp |= Login.BatchSupportFlags.SUPPORT_REISSUES;
                if (features().checkHasSupportBatchCloses())
                    temp |= Login.BatchSupportFlags.SUPPORT_CLOSES;
                tmpUInt.value(temp);
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (features().checkHasSupportViewRequests())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.SUPPORT_VIEW);
                tmpUInt.value(features().supportViewRequests());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (features().checkHasSupportStandby())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.SUPPORT_STANDBY);
                tmpUInt.value(features().supportStandby());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (features().checkHasSupportOptimizedPauseResume())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.SUPPORT_OPR);
                tmpUInt.value(features().supportOptimizedPauseResume());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            
            if (features().checkHasSupportProviderDictionaryDownload())
            {
                element.dataType(DataTypes.UINT);
                element.name(ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD);
                tmpUInt.value(features().supportProviderDictionaryDownload());
                if ((ret = element.encode(encodeIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }

        return elementList.encodeComplete(encodeIter, true);
    }

    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "LoginRefresh: \n");
        stringBuf.append(tab);
        stringBuf.append("name: ");
        stringBuf.append(userName());
        stringBuf.append(eol);
        stringBuf.append(tab);
        stringBuf.append("nameType: ");
        stringBuf.append(userNameType());
        stringBuf.append(eol);

        stringBuf.append(tab);
        stringBuf.append(state());
        stringBuf.append(eol);

        if (checkSolicited())
        {
            stringBuf.append(tab);
            stringBuf.append("isSolicited: ");
            stringBuf.append(true);
            stringBuf.append(eol);
        }

        if (checkHasAuthenticationTTReissue())
        {
            stringBuf.append(tab);
            stringBuf.append("authenticationTTReissue: ");
            stringBuf.append(authenticationTTReissue());
            stringBuf.append(eol);
        }
        if (checkHasAuthenticationExtendedResp())
        {
            stringBuf.append(tab);
            stringBuf.append("authenticationExtendedResp: ");
            stringBuf.append(authenticationExtendedResp());
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

        if (checkHasAttrib())
        {
            stringBuf.append(attrib.toString());
        }

        if (checkHasFeatures())
        {
            stringBuf.append(features.toString());
        }
        if(checkHasConnectionConfig())
        {
            stringBuf.append(connectionConfig.toString());
        }
        return stringBuf.toString();
    }

    public void connectionConfig(LoginConnectionConfig connectionConfig)
    {
        assert (connectionConfig != null) : "connectionConfig can not be null";
        assert (checkHasConnectionConfig());
        LoginConnectionConfigImpl connectionConfigImpl = (LoginConnectionConfigImpl)connectionConfig();
        connectionConfigImpl.copyReferences(connectionConfig);
    }

    public LoginConnectionConfig connectionConfig()
    {
        return connectionConfig;
    }

    public boolean checkHasConnectionConfig()
    {
        return (flags() & LoginRefreshFlags.HAS_CONN_CONFIG) != 0;
    }

    public void applyHasConnectionConfig()
    {
        flags |= LoginRefreshFlags.HAS_CONN_CONFIG;
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.LOGIN;
    }
}