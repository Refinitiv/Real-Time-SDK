/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.provider;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.shared.*;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;
import com.refinitiv.eta.valueadd.reactor.*;

import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.TimeUnit;

import static java.util.concurrent.TimeUnit.SECONDS;

/*
 * This is the implementation of processing of login requests and login status
 * messages.
 * <p>
 * Only one login stream per channel is allowed by this simple provider.
 */
class LoginHandler
{
    private static final int REJECT_MSG_SIZE = 512;
    private static final int REFRESH_MSG_SIZE = 512;
    private static final int STATUS_MSG_SIZE = 512;
    private static final int RTT_MSG_SIZE = 1024;

    private static final int RTT_NOTIFICATION_INTERVAL = 5;

    private LoginStatus _loginStatus = (LoginStatus)LoginMsgFactory.createMsg();
    private LoginRefresh _loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    private LoginRequest _loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
    private LoginRTT loginRTT = (LoginRTT)LoginMsgFactory.createMsg();
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    private boolean enableRtt;

    // application id 
    private static String applicationId = "256";

    // application name
    private static String applicationName = "ETA Provider";

    private LoginRequestInfoList _loginRequestInfoList;

    private LoginRttInfoList loginRttInfoList;

    private ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();

    LoginHandler()
    {
        _loginStatus.rdmMsgType(LoginMsgType.STATUS);
        _loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
        _loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        loginRTT.rdmMsgType(LoginMsgType.RTT);
        _loginRequestInfoList = new LoginRequestInfoList();
        loginRttInfoList = new LoginRttInfoList();
    }

    /*
     * Initializes login information fields.
     */
    void init()
    {
        _loginRequestInfoList.init();
        loginRttInfoList.init();
    }

    /*
     * Closes a dictionary stream.
     */
    void closeStream(int streamId)
    {
        /* find original request information associated with streamId */
        for (LoginRequestInfo loginReqInfo : _loginRequestInfoList) {
            if (loginReqInfo.loginRequest().streamId() == streamId && loginReqInfo.isInUse())
            {
                /* clear original request information */
                System.out.println("Closing login stream id '" + loginReqInfo.loginRequest().streamId() + "' with user name: " + loginReqInfo.loginRequest().userName());
                loginReqInfo.clear();
                break;
            }
        }
        loginRttInfoList.clearForStream(streamId);
    }

    /*
     * Closes all open dictionary streams for a channel.
     */
    void closeStream(ReactorChannel channel)
    {
        //find original request information associated with channel
        LoginRequestInfo loginReqInfo = findLoginRequestInfo(channel.channel());
        if(loginReqInfo != null)
        {
            System.out.println("Closing login stream id '" + loginReqInfo.loginRequest().streamId() + "' with user name: " + loginReqInfo.loginRequest().userName());
            loginReqInfo.clear();
        }
        clearRttInfo(channel);
    }

    private void clearRttInfo(ReactorChannel reactorChannel) {
        if (enableRtt) {
            LoginRttInfo loginRttInfo = loginRttInfoList.get(reactorChannel.channel());
            if (!Objects.isNull(loginRttInfo)) {
                loginRttInfo.clear();
            }
        }
    }
    
    /*
     * Sends the login request reject status message for a channel. 
     */
    int sendRequestReject(ReactorChannel chnl, int streamId, LoginRejectReason reason, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the login request reject status */
        TransportBuffer msgBuf = chnl.getBuffer(REJECT_MSG_SIZE, false, errorInfo);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }
        int ret = encodeRequestReject(chnl, streamId, reason, msgBuf, errorInfo);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        return chnl.submit(msgBuf, _submitOptions, errorInfo);
    }

    /**
     * Send RTT message to a consumer connected via {@param reactorChannel}
     *
     * @param reactorChannel - channel instance onto which consumer has been connected
     * @param errorInfo      - error buffer for returning Reactor errors and warnings to user.
     * @return {@link ReactorReturnCodes#SUCCESS} when rtt message was sent successfully or rtt feature is not supported.
     * @see ReactorChannel#submit(TransportBuffer, ReactorSubmitOptions, ReactorErrorInfo)
     */
    int sendRTT(ReactorChannel reactorChannel, ReactorErrorInfo errorInfo) {
        if (enableRtt) {
            LoginRttInfo loginRttInfo = createOrGetRttInfo(reactorChannel);
            if (Objects.nonNull(loginRttInfo) && isRttReadyToSend(loginRttInfo)) {
                loginRTT = loginRttInfo.loginRtt();
                _encodeIter.clear();
                TransportBuffer msgBuf = reactorChannel.getBuffer(RTT_MSG_SIZE, false, errorInfo);
                if (msgBuf == null) {
                    return CodecReturnCodes.FAILURE;
                }
                _encodeIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());

                loginRTT.updateRTTActualTicks();
                int ret = loginRTT.encode(_encodeIter);
                if (ret != CodecReturnCodes.SUCCESS) {
                    return ret;
                }
                _submitOptions.clear();
                return reactorChannel.submit(msgBuf, _submitOptions, errorInfo);
            }
        }
        return ReactorReturnCodes.SUCCESS;
    }

    private boolean isRttReadyToSend(LoginRttInfo loginRttInfo) {
        final long rttSendTime = System.nanoTime();
        if (SECONDS.convert(rttSendTime - loginRttInfo.rttLastSendNanoTime(), TimeUnit.NANOSECONDS) > RTT_NOTIFICATION_INTERVAL) {
            loginRttInfo.rttLastSendNanoTime(rttSendTime);
            return true;
        }
        return false;
    }

    /*
     * Encodes the login request reject status. Returns success if encoding
     * succeeds or failure if encoding fails. 
     */
    private int encodeRequestReject(ReactorChannel chnl, int streamId, LoginRejectReason reason, TransportBuffer msgBuf, ReactorErrorInfo errorInfo)
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
                _loginStatus.state().text().data("Login request rejected for stream id " + streamId + " - max request count reached");
                break;
            case NO_USER_NAME_IN_REQUEST:
                _loginStatus.state().code(StateCodes.USAGE_ERROR);
                _loginStatus.state().text().data("Login request rejected for stream id  " + streamId + " - request does not contain user name");
                break;
            case LOGIN_RDM_DECODER_FAILED:
                _loginStatus.state().code(StateCodes.USAGE_ERROR);
                _loginStatus.state().text().data("Login request rejected for stream id  " + streamId + " - decoding failure: " + errorInfo.error().text());
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
            errorInfo.error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _loginStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("LoginStatus.encode() failed");
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Sends the login close status message for a channel.
     */
    int sendCloseStatus(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        LoginRequestInfo loginReqInfo = _loginRequestInfoList.get(chnl.channel());
        if (loginReqInfo == null)
        {
            errorInfo.error().text("Could not find login request information for the channel");
            return TransportReturnCodes.FAILURE;
        }

        // get a buffer for the login close
        TransportBuffer msgBuf = chnl.getBuffer(STATUS_MSG_SIZE, false, errorInfo);
        if (msgBuf == null)
            return TransportReturnCodes.FAILURE;
        
        _loginStatus.clear();
        _loginStatus.streamId(loginReqInfo.loginRequest().streamId());
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
            errorInfo.error().text("LoginStatus.encode() failed");
            return ret;
        }
        
        return chnl.submit(msgBuf, _submitOptions, errorInfo);
    }

    /*
     * Sends a login refresh.
     */
    int sendRefresh(ReactorChannel chnl, LoginRequest loginRequest, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the login response 
        TransportBuffer msgBuf = chnl.getBuffer(REFRESH_MSG_SIZE, false, errorInfo);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        _loginRefresh.clear();

        // provide login response information 

        // streamId 
        _loginRefresh.streamId(loginRequest.streamId());

        // username 
        _loginRefresh.applyHasUserName();
        _loginRefresh.userName().data(loginRequest.userName().data(), loginRequest.userName().position(), loginRequest.userName().length());

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

        if(loginRequest.checkHasAttrib() && loginRequest.attrib().checkHasPosition())
        {
            _loginRefresh.attrib().applyHasPosition();
            _loginRefresh.attrib().position().data(loginRequest.attrib().position().data(), loginRequest.attrib().position().position(), loginRequest.attrib().position().length());
        }

        if (enableRtt && loginRequest.checkHasAttrib() && loginRequest.attrib().checkHasSupportRoundTripLatencyMonitoring()) {
            _loginRefresh.attrib().applyHasSupportRoundTripLatencyMonitoring();
        }

        // this provider does not support
        // singleOpen behavior
        _loginRefresh.attrib().applyHasSingleOpen();
        _loginRefresh.attrib().singleOpen(0); 
        

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
            errorInfo.error().text("LoginRefresh.encode() failed");
            return ret;
        }

        return chnl.submit(msgBuf, _submitOptions, errorInfo);
    }

    /*
     * Finds login request information for a channel.
     */
    LoginRequestInfo findLoginRequestInfo(Channel chnl)
    {
        return _loginRequestInfoList.get(chnl);
    }

    /*
     * Gets login request information for the channel.
     */
	LoginRequestInfo getLoginRequestInfo(ReactorChannel reactorChannel, LoginRequest loginRequest)
	{
		return _loginRequestInfoList.get(reactorChannel.channel(), loginRequest);
	}

    public LoginRttInfo createOrGetRttInfo(ReactorChannel reactorChannel) {
        return Optional
                .ofNullable(getLoginRtt(reactorChannel))
                .orElseGet(() -> createLoginRtt(reactorChannel));
    }

    public LoginRttInfo createLoginRtt(ReactorChannel reactorChannel) {
        LoginRequestInfo loginRequestInfo = findLoginRequestInfo(reactorChannel.channel());
        if (Objects.nonNull(loginRequestInfo) && loginRequestInfo.isInUse()) {
            LoginRttInfo loginRttInfo = loginRttInfoList
                    .createFromRequest(reactorChannel.channel(), loginRequestInfo.loginRequest());

            //could be added or deleted also another flags for demonstration different behaviour
            loginRttInfo.loginRtt().applyHasRTLatency();
            return loginRttInfo;
        }
        return null;
    }

    public LoginRttInfo getLoginRtt(ReactorChannel reactorChannel) {
	    return loginRttInfoList.get(reactorChannel.channel());
    }

	void enableRtt(boolean enableRtt) {
	    this.enableRtt = enableRtt;
    }
}
