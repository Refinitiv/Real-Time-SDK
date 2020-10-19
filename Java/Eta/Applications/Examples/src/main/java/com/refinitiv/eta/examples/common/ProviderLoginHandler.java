package com.refinitiv.eta.examples.common;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.shared.*;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginStatus;

import java.util.Objects;
import java.util.Optional;
import com.refinitiv.eta.shared.LoginRejectReason;
import com.refinitiv.eta.shared.LoginRequestInfo;
import com.refinitiv.eta.shared.LoginRequestInfoList;
import com.refinitiv.eta.shared.ProviderSession;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;

import java.util.concurrent.TimeUnit;

import static java.util.concurrent.TimeUnit.SECONDS;

/**
 * This is the implementation of processing of login requests and login status
 * messages.
 * 
 * <p>
 * Only one login stream per channel is allowed by this simple provider.
 */
public class ProviderLoginHandler
{
    private static final int RTT_NOTIFICATION_INTERVAL = 5;

    private static final int REJECT_MSG_SIZE = 512;
    private static final int REFRESH_MSG_SIZE = 512;
    private static final int STATUS_MSG_SIZE = 512;
    private static final int RTT_MSG_SIZE = 1024;

    private LoginStatus _loginStatus = (LoginStatus)LoginMsgFactory.createMsg();
    private LoginRefresh _loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    private LoginRequest _loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
    private LoginRTT loginRtt = (LoginRTT) LoginMsgFactory.createMsg();
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();

    // application id 
    private static String applicationId = "256";

    // application name
    private static String applicationName = "ETA Provider";

    private boolean enableRtt;

    private LoginRequestInfoList _loginRequestInfoList;
    private LoginRttInfoList loginRttInfoList;
    private ProviderSession _providerSession;

    public ProviderLoginHandler(ProviderSession providerSession)
    {
        _loginStatus.rdmMsgType(LoginMsgType.STATUS);
        _loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
        _loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        loginRtt.rdmMsgType(LoginMsgType.RTT);
        _providerSession = providerSession;
        _loginRequestInfoList = new LoginRequestInfoList();
        loginRttInfoList = new LoginRttInfoList();
    }

    /**
     * Initializes login information fields.
     */
    public void init()
    {
        _loginRequestInfoList.init();
        loginRttInfoList.init();
    }

    /**
     * Set flag for defining supporting of the RTT feature by current provider.
     * @param enableRtt - value for setting.
     */
    public void enableRtt(boolean enableRtt) {
        this.enableRtt = enableRtt;
    }

    /**
     * Processes a login request.
     * 
     * @param chnl - The channel of the request
     * @param msg - The partially decoded message
     * @param dIter - The decode iterator
     * @param error - Error information in case of failure
     * @return returns success if decoding of request message and sending of
     *         response message succeeds or failure if it fails.
     */
    public int processRequest(Channel chnl, Msg msg, DecodeIterator dIter, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                _loginRequest.clear();
                int ret = _loginRequest.decode(dIter, msg);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    error.text("LoginRequest.decode() failed with return code: " + CodecReturnCodes.toString(ret));
                    return ret;
                }

                System.out.println("Received Login Request for Username: " + _loginRequest.userName());
                
                try
                {
                	System.out.println(_loginRequest.toString());
                }
                catch(Exception e)
                {
                    return sendRequestReject(chnl, msg.streamId(), LoginRejectReason.LOGIN_RDM_DECODER_FAILED, error);
                }

                LoginRequestInfo loginRequestInfo = _loginRequestInfoList.get(chnl, _loginRequest);
                if (loginRequestInfo == null)
                {
                    return sendRequestReject(chnl, msg.streamId(), LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED, error);
                }

                /* check if key has user name */
                /*
                 * user name is only login user type accepted by this
                 * application (user name is the default type)
                 */
                if (!msg.msgKey().checkHasName() || (msg.msgKey().checkHasNameType() && (msg.msgKey().nameType() != Login.UserIdTypes.NAME)))
                {
                    return sendRequestReject(chnl, msg.streamId(), LoginRejectReason.NO_USER_NAME_IN_REQUEST, error);
                }

                /* send login response */
                return sendRefresh(chnl, loginRequestInfo, error);
            case MsgClasses.GENERIC:
                if (Objects.equals(DataTypes.ELEMENT_LIST, msg.containerType())) {
                    loginRtt.clear();
                    ret = loginRtt.decode(dIter, msg);
                    if (ret != CodecReturnCodes.SUCCESS) {
                        error.text("LoginRTT.decode() failed with return code: " + CodecReturnCodes.toString(ret));
                        return ret;
                    }
                    System.out.printf("Received login RTT message from Consumer %d.\n",
                            _providerSession.getClientSessionForChannel(chnl).socketFdValue());
                    System.out.printf("\tRTT Tick value is %dus.\n", TimeUnit.NANOSECONDS.toMicros(loginRtt.ticks()));
                    if (loginRtt.checkHasTCPRetrans()) {
                        System.out.printf("\tConsumer side TCP retransmissions: %d\n", loginRtt.tcpRetrans());
                    }
                    long calculatedRtt = loginRtt.calculateRTTLatency(TimeUnit.MICROSECONDS);
                    LoginRTT storedLoginRtt = getLoginRttInfo(chnl).loginRtt();
                    loginRtt.copy(storedLoginRtt);
                    System.out.printf("\tLast RTT message latency is %dus.\n\n", calculatedRtt);
                }
                break;
            case MsgClasses.CLOSE:
                error.text("Received Login Close for StreamId " + msg.streamId());
                closeStream(msg.streamId());
                break;

            default:
                error.text("Received Unhandled Login Msg Class: " + MsgClasses.toString(msg.msgClass()));
                return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private void closeStream(int streamId)
    {
        /* find original request information associated with streamId */
        for (LoginRequestInfo loginReqInfo : _loginRequestInfoList)
        {
            if (loginReqInfo.loginRequest.streamId() == streamId)
            {
                /* clear original request information */
                System.out.println("Closing login stream id '" + loginReqInfo.loginRequest.streamId() + "' with user name: " + loginReqInfo.loginRequest.userName());
                loginReqInfo.clear();
                break;
            }
        }
    }

    public void closeRequestAndRtt(Channel channel)
    {
        //find original request information associated with channel
        LoginRequestInfo loginReqInfo = findLoginRequestInfo(channel);
        if(loginReqInfo != null)
        {
            System.out.println("Closing login stream id '" + loginReqInfo.loginRequest.streamId() + "' with user name: " + loginReqInfo.loginRequest.userName());
            loginReqInfo.clear();
        }

        //find RTT information associated with channel
        clearRttInfo(channel);
    }


    private void clearRttInfo(Channel channel) {
        if (enableRtt) {
            LoginRttInfo loginRttInfo = loginRttInfoList.get(channel);
            if (Objects.nonNull(loginRttInfo)) {
                loginRttInfo.clear();
            }
        }
    }

    /**
     * Sends RTT message if this feature has been enabled.
     *
     * @param channel - The channel for sending Login RTT message to.
     * @param error - error buffer for storing errors and warnings.
     *
     * @return {@link CodecReturnCodes#FAILURE} if encoding or transport buffer creation was failed.
     * In another cases returns any of {@link TransportReturnCodes}
     *
     * @see ProviderSession#write(Channel, TransportBuffer, Error)
     */
    public int proceedLoginRttMessage(Channel channel, Error error) {
        if (enableRtt) {
            LoginRttInfo loginRttInfo = createOrGetRttInfo(channel);
            if (Objects.nonNull(loginRttInfo) && isRttReadyToSend(loginRttInfo)) {
                TransportBuffer transportBuffer = channel.getBuffer(RTT_MSG_SIZE, false, error);
                if (Objects.isNull(transportBuffer)) {
                    return CodecReturnCodes.FAILURE;
                }

                _encodeIter.clear();
                _encodeIter.setBufferAndRWFVersion(transportBuffer, channel.majorVersion(), channel.minorVersion());

                loginRtt = loginRttInfo.loginRtt();
                loginRtt.updateRTTActualTicks();
                int ret = loginRtt.encode(_encodeIter);
                if (!Objects.equals(CodecReturnCodes.SUCCESS, ret)) {
                    error.text("Encoding of login RTT failed: <" + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }
                return _providerSession.write(channel, transportBuffer, error);
            }
        }
        return TransportReturnCodes.SUCCESS;
    }

    private boolean isRttReadyToSend(LoginRttInfo loginRttInfo) {
        final long rttLastSendTime = loginRttInfo.rttLastSendNanoTime();
        final long rttSendTime = System.nanoTime();
        if (SECONDS.convert(rttSendTime - rttLastSendTime, TimeUnit.NANOSECONDS) > RTT_NOTIFICATION_INTERVAL) {
            loginRttInfo.rttLastSendNanoTime(rttSendTime);
            return true;
        }
        return false;
    }

    private int sendRequestReject(Channel chnl, int streamId, LoginRejectReason reason, Error error)
    {
        // get a buffer for the login request reject status */
        TransportBuffer msgBuf = chnl.getBuffer(REJECT_MSG_SIZE, false, error);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }
        int ret = encodeRequestReject(chnl, streamId, reason, msgBuf, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        return _providerSession.write(chnl, msgBuf, error);
    }

    private int encodeRequestReject(Channel chnl, int streamId, LoginRejectReason reason, TransportBuffer msgBuf, Error error)
    {
        // set-up message 
        _loginStatus.streamId(streamId);
        _loginStatus.applyHasState();
        _loginStatus.state().streamState(StreamStates.CLOSED_RECOVER);
        _loginStatus.state().dataState(DataStates.SUSPECT);
        switch (reason)
        {
            case MAX_LOGIN_REQUESTS_REACHED:
                _loginStatus.state().code(StateCodes.TOO_MANY_ITEMS);
                _loginStatus.state().text().data("Login request rejected for stream id " + streamId + "- max request count reached");
                break;
            case NO_USER_NAME_IN_REQUEST:
                _loginStatus.state().code(StateCodes.USAGE_ERROR);
                _loginStatus.state().text().data("Login request rejected for stream id  " + streamId + "- request does not contain user name");
                _loginStatus.state().code(StateCodes.USAGE_ERROR);
                break;
            default:
                break;
        }

        // clear encode iterator
        _encodeIter.clear();

        // encode message
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _loginStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("LoginStatus.encode() failed");
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Sends the login close status message for a channel.
     * 
     * @param chnl - The channel to send close status message to
     * @param error - Error information in case of failure
     * @return {@link CodecReturnCodes#SUCCESS} if close status message sent
     *         successfully, &lt; {@link CodecReturnCodes#SUCCESS} when it fails.
     */
    public int sendCloseStatus(Channel chnl, Error error)
    {
        LoginRequestInfo loginReqInfo = _loginRequestInfoList.get(chnl);
        if (loginReqInfo == null)
        {
            error.text("Could not find login request information for the channel");
            return TransportReturnCodes.FAILURE;
        }

        // get a buffer for the login close
        TransportBuffer msgBuf = chnl.getBuffer(STATUS_MSG_SIZE, false, error);
        if (msgBuf == null)
            return TransportReturnCodes.FAILURE;

        _loginStatus.clear();
        _loginStatus.streamId(loginReqInfo.loginRequest.streamId());
        _loginStatus.applyHasState();
        _loginStatus.state().streamState(StreamStates.CLOSED);
        _loginStatus.state().dataState(DataStates.SUSPECT);
        _loginStatus.state().text().data("Login stream closed");
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _loginStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("LoginStatus.encode() failed");
            return ret;
        }

        return _providerSession.write(chnl, msgBuf, error);

    }

    private int sendRefresh(Channel chnl, LoginRequestInfo loginReqInfo, Error error)
    {
        // get a buffer for the login response 
        TransportBuffer msgBuf = chnl.getBuffer(REFRESH_MSG_SIZE, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        _loginRefresh.clear();

        // provide login response information 

        // streamId 
        _loginRefresh.streamId(loginReqInfo.loginRequest.streamId());

        // username 
        _loginRefresh.applyHasUserName();
        _loginRefresh.userName().data(loginReqInfo.loginRequest.userName().data(), loginReqInfo.loginRequest.userName().position(), loginReqInfo.loginRequest.userName().length());

        _loginRefresh.applyHasUserNameType();
        _loginRefresh.userNameType(Login.UserIdTypes.NAME);

        _loginRefresh.state().code(StateCodes.NONE);
        _loginRefresh.state().dataState(DataStates.OK);
        _loginRefresh.state().streamState(StreamStates.OPEN);
        _loginRefresh.state().text().data("Login accepted by host localhost");

        _loginRefresh.applySolicited();

        _loginRefresh.applyHasAttrib();
        _loginRefresh.attrib().applyHasApplicationId();
        _loginRefresh.attrib().applicationId().data(applicationId);

        _loginRefresh.applyHasAttrib();
        _loginRefresh.attrib().applyHasApplicationName();
        _loginRefresh.attrib().applicationName().data(applicationName);

        if(loginReqInfo.loginRequest.checkHasAttrib() && loginReqInfo.loginRequest.attrib().checkHasPosition())
        {
            _loginRefresh.attrib().applyHasPosition();
            _loginRefresh.attrib().position().data(loginReqInfo.loginRequest.attrib().position().data(), loginReqInfo.loginRequest.attrib().position().position(), loginReqInfo.loginRequest.attrib().position().length());
        }

        // this provider does not support
        // singleOpen behavior
        _loginRefresh.attrib().applyHasSingleOpen();
        _loginRefresh.attrib().singleOpen(0);

        if (enableRtt) {
            _loginRefresh.attrib().applyHasSupportRoundTripLatencyMonitoring();
        }

        // this provider supports batch requests
        _loginRefresh.applyHasFeatures();
        _loginRefresh.features().applyHasSupportBatchRequests();
        _loginRefresh.features().supportBatchRequests(1); 

        _loginRefresh.applyHasFeatures();
        _loginRefresh.features().applyHasSupportPost();
        _loginRefresh.features().supportOMMPost(1);

        // keep default values for all others
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _loginRefresh.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("LoginRefresh.encode() failed");
            return ret;
        }

        return _providerSession.write(chnl, msgBuf, error);
    }

    /**
     * Finds a login request information for the channel.
     * 
     * @param chnl - The channel to get the login request information for.
     * 
     * @return {@link LoginRequestInfo} for the channel
     */
    public LoginRequestInfo findLoginRequestInfo(Channel chnl)
    {
        return _loginRequestInfoList.get(chnl);
    }

    /**
     * Try to get a login rtt information for the specified channel.
     * @param channel - The channel to get the login RTT information for.
     * @return {@link LoginRttInfo} for the channel
     */
    public LoginRttInfo createOrGetRttInfo(Channel channel) {
        return Optional
                .ofNullable(loginRttInfoList.get(channel))
                .orElseGet(() -> createLoginRtt(channel));
    }

    public LoginRttInfo createLoginRtt(Channel channel) {
        LoginRequestInfo loginRequestInfo = findLoginRequestInfo(channel);
        if (Objects.nonNull(loginRequestInfo) && loginRequestInfo.isInUse()) {
            LoginRttInfo loginRttInfo = loginRttInfoList.createFromRequest(channel, loginRequestInfo.loginRequest());
            loginRttInfo.loginRtt().applyHasRTLatency();
            return loginRttInfo;
        }
        return null;
    }


    public LoginRttInfo getLoginRttInfo(Channel channel) {
        return loginRttInfoList.get(channel);
    }
}
