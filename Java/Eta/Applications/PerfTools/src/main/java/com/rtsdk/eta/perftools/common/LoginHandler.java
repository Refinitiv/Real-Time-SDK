package com.rtsdk.eta.perftools.common;

import com.rtsdk.eta.codec.*;
import com.rtsdk.eta.shared.ConsumerLoginState;
import com.rtsdk.eta.rdm.Login;
import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportBuffer;
import com.rtsdk.eta.valueadd.domainrep.rdm.login.*;

/**
 * This is the Login handler for the UPA Consumer and NIProvider application. It
 * provides methods for encoding and sending of login request, as well as
 * processing of responses (refresh, status, update, close). Methods are also
 * provided to allow setting of application name, user name and role, to be used
 * in the login request.
 */
public class LoginHandler
{
    public static final int LOGIN_STREAM_ID = 1;

    public static final int MAX_MSG_SIZE = 1024;
    public static int TRANSPORT_BUFFER_SIZE_REQUEST = MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;
    
    private ConsumerLoginState loginState = ConsumerLoginState.PENDING_LOGIN;

    // For requests
    private String _userName;
    private String _applicationName;

    private int _role = Login.RoleTypes.CONS;

    private LoginRequest _loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
    private LoginClose _loginClose = (LoginClose)LoginMsgFactory.createMsg();

    private LoginRefresh _loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    private LoginStatus _loginStatus = (LoginStatus)LoginMsgFactory.createMsg();
 

    /**
     * Instantiates a new login handler.
     */
    public LoginHandler()
    {
        _loginClose.rdmMsgType(LoginMsgType.CLOSE);
        _loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        _loginStatus.rdmMsgType(LoginMsgType.STATUS);
        _loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
    }
    
    /**
     * Get the cached login refresh.
     * @return  Cached login refresh.
     * @see LoginRefresh
     */
    public LoginRefresh refreshInfo()
    {
        return _loginRefresh;
    }

    /**
     * Get the Consumer login state.
     * @return Consumer login state.
     * @see ConsumerLoginState
     */
    public ConsumerLoginState loginState()
    {
        return loginState;
    }
    
    /**
     * Sets the user name requested by the application.
     *
     * @param userName the user name
     */
    public void userName(String userName)
    {
        _userName = userName;
    }

    /**
     * Sets the application name for the request.
     *
     * @param applicationName the application name
     */
    public void applicationName(String applicationName)
    {
        _applicationName = applicationName;
    }

    /**
     * Login Role. Constant from {@link com.rtsdk.eta.rdm.Login.RoleTypes}.
     * Default login role is {@link com.rtsdk.eta.rdm.Login.RoleTypes#CONS}
     *
     * @param role the role
     */
    public void role(int role)
    {
        _role = role;
    }

    /**
     * Sends a login request to a channel. This consists of getting a message
     * buffer, setting the login request information, encoding the login
     * request, and sending the login request to the server.
     *
     * @param channel The channel to send a login request to
     * @param error the error
     * @param eIter the e iter
     * @return Returns success if send login request succeeds or failure if it
     *         fails.
     */
    public TransportBuffer getRequestMsg(Channel channel, Error error, EncodeIterator eIter)
    {
        /* get a buffer for the login request */
        TransportBuffer msgBuf = channel.getBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);
        if (msgBuf == null)
            return null;

        _loginRequest.clear();

        _loginRequest.initDefaultRequest(LOGIN_STREAM_ID);

        if (_userName != null && !_userName.isEmpty())
        {
            _loginRequest.userName().data(_userName);
        }

        if (_applicationName != null && !_applicationName.isEmpty())
        {
            _loginRequest.applyHasAttrib();
            _loginRequest.attrib().applyHasApplicationName();
            _loginRequest.attrib().applicationName().data(_applicationName);
        }

        _loginRequest.applyHasRole();
        _loginRequest.role(_role);

        if (_role == Login.RoleTypes.PROV)
        {
            _loginRequest.attrib().applyHasSingleOpen();
            _loginRequest.attrib().singleOpen(0);
        }

        eIter.clear();
        eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        int ret = _loginRequest.encode(eIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Encoding of login request failed: <" + CodecReturnCodes.toString(ret) + ">");
            return null;
        }
        System.out.println(_loginRequest.toString());
        return msgBuf;
    }

    /**
     * Close the login stream. Note that closing login stream will automatically
     * close all other streams at the provider.
     *
     * @param channel the channel
     * @param error the error
     * @param eIter the e iter
     * @return Returns success if close login stream succeeds or failure if it
     *         fails.
     */
    public TransportBuffer getCloseMsg(Channel channel, Error error, EncodeIterator eIter)
    {
        // get a buffer for the login close
        TransportBuffer msgBuf = channel.getBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, error);

        if (msgBuf == null)
        {
            return null;
        }

        _loginClose.clear();
        _loginClose.streamId(LOGIN_STREAM_ID);
        eIter.clear();
        eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        int ret = _loginClose.encode(eIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Encoding of login close failed: <" + CodecReturnCodes.toString(ret) + ">");
            return null;
        }

        return msgBuf;
    }

    /**
     * Processes login response. This consists of looking at the msg class and
     * decoding message into corresponding RDM login message. For every
     * login status and login refresh, it updates login states (closed, closed
     * recoverable, suspect, success). Query methods are provided to query these
     * login states.
     *
     * @param msg The partially decoded message
     * @param dIter The decode iterator
     * @param error the error
     * @return returns success if decoding of message succeeds or failure if it
     *         fails.
     */
    public int processResponse(Msg msg, DecodeIterator dIter, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                return handleLoginRefresh(msg, dIter, error);
            case MsgClasses.STATUS:
                return handleLoginStatus(msg, dIter, error);
            case MsgClasses.UPDATE:
                System.out.println("Received Login Update");
                return CodecReturnCodes.SUCCESS;
            case MsgClasses.CLOSE:
                System.out.println("Received Login Close");
                loginState = ConsumerLoginState.CLOSED;
                return CodecReturnCodes.FAILURE;
            default:
                error.text("Received Unhandled Login Msg Class: " + msg.msgClass());
                return CodecReturnCodes.FAILURE;
        }
    }

    private int handleLoginStatus(Msg msg, DecodeIterator dIter, Error error)
    {
        _loginStatus.clear();
        int ret = _loginStatus.decode(dIter, msg);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Decoding of login status failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        System.out.println("Received Login StatusMsg");
        if (!_loginStatus.checkHasState())
            return CodecReturnCodes.SUCCESS;

        State state = _loginStatus.state();
        System.out.println("	" + state);
    
        if (state.streamState() == StreamStates.CLOSED_RECOVER)
        {
            error.text("Login stream is closed recover");
            this.loginState = ConsumerLoginState.CLOSED_RECOVERABLE;
        }
        else if (state.streamState() == StreamStates.CLOSED)
        {
            error.text("Login stream closed");
            this.loginState = ConsumerLoginState.CLOSED;
        }
        else if (state.streamState() == StreamStates.OPEN
                && state.dataState() == DataStates.SUSPECT)
        {
            error.text("Login stream is suspect");
            this.loginState = ConsumerLoginState.SUSPECT;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int handleLoginRefresh(Msg msg, DecodeIterator dIter, Error error)
    {
        _loginRefresh.clear();
        int ret = _loginRefresh.decode(dIter, msg);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Decoding of login refresh failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        System.out.println("Received Login Response for Username: " + _loginRefresh.userName());
        System.out.println(_loginRefresh.toString());
        
        State state = _loginRefresh.state();
        if (state.streamState() == StreamStates.OPEN)
        {
            if(state.dataState() == DataStates.OK)
                this.loginState = ConsumerLoginState.OK_SOLICITED; 
            else if (state.dataState() == DataStates.SUSPECT)
                this.loginState = ConsumerLoginState.SUSPECT;
        }
        else if (state.streamState() == StreamStates.CLOSED_RECOVER)
        {
            this.loginState = ConsumerLoginState.CLOSED_RECOVERABLE;
        }
        else if (state.streamState() == StreamStates.CLOSED)
        {
            this.loginState = ConsumerLoginState.CLOSED;
        }
        else
        {
            this.loginState = ConsumerLoginState.SUSPECT;
        }
        return CodecReturnCodes.SUCCESS;
    }
}
