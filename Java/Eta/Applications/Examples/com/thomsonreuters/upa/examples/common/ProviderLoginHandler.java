package com.thomsonreuters.upa.examples.common;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.shared.LoginRejectReason;
import com.thomsonreuters.upa.shared.LoginRequestInfo;
import com.thomsonreuters.upa.shared.LoginRequestInfoList;
import com.thomsonreuters.upa.shared.ProviderSession;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;

/**
 * This is the implementation of processing of login requests and login status
 * messages.
 * 
 * <p>
 * Only one login stream per channel is allowed by this simple provider.
 */
public class ProviderLoginHandler
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
    private ProviderSession _providerSession;

    public ProviderLoginHandler(ProviderSession providerSession)
    {
        _loginStatus.rdmMsgType(LoginMsgType.STATUS);
        _loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
        _loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        _providerSession = providerSession;
        _loginRequestInfoList = new LoginRequestInfoList();
    }

    /**
     * Initializes login information fields.
     */
    public void init()
    {
        _loginRequestInfoList.init();
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

    public void closeRequest(Channel channel)
    {
        //find original request information associated with channel
        LoginRequestInfo loginReqInfo = findLoginRequestInfo(channel);
        if(loginReqInfo != null)
        {
            System.out.println("Closing login stream id '" + loginReqInfo.loginRequest.streamId() + "' with user name: " + loginReqInfo.loginRequest.userName());
            loginReqInfo.clear();
        }
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
}
