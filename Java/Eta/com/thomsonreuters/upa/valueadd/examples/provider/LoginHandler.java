package com.thomsonreuters.upa.valueadd.examples.provider;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.examples.common.LoginRejectReason;
import com.thomsonreuters.upa.examples.common.LoginRequestInfo;
import com.thomsonreuters.upa.examples.common.LoginRequestInfoList;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

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

    private LoginStatus _loginStatus = (LoginStatus)LoginMsgFactory.createMsg();
    private LoginRefresh _loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    private LoginRequest _loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();

    // application id 
    private static String applicationId = "256";

    // application name
    private static String applicationName = "UPA Provider";

    private LoginRequestInfoList _loginRequestInfoList;

    private ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();

    LoginHandler()
    {
        _loginStatus.rdmMsgType(LoginMsgType.STATUS);
        _loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
        _loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        _loginRequestInfoList = new LoginRequestInfoList();
    }

    /*
     * Initializes login information fields.
     */
    void init()
    {
        _loginRequestInfoList.init();
    }

    /*
     * Closes a dictionary stream.
     */
    void closeStream(int streamId)
    {
        /* find original request information associated with streamId */
        for (LoginRequestInfo loginReqInfo : _loginRequestInfoList)
        {
            if (loginReqInfo.loginRequest().streamId() == streamId && loginReqInfo.isInUse())
            {
                /* clear original request information */
                System.out.println("Closing login stream id '" + loginReqInfo.loginRequest().streamId() + "' with user name: " + loginReqInfo.loginRequest().userName());
                loginReqInfo.clear();
                break;
            }
        }
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
}
