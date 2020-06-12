package com.thomsonreuters.upa.examples.common;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.shared.ConsumerLoginState;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.*;

import java.util.Objects;

import static java.util.concurrent.TimeUnit.NANOSECONDS;

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

    public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_RTT = ChannelSession.MAX_MSG_SIZE;
    
    private ConsumerLoginState loginState = ConsumerLoginState.PENDING_LOGIN;

    // For requests
    private String userName;
    private String applicationName;
	private String authenticationToken;
	private String authenticationExtended;
	private String applicationId;
	private boolean enableRtt;

    private int role = Login.RoleTypes.CONS;

    private LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
    private LoginClose loginClose = (LoginClose)LoginMsgFactory.createMsg();

    private LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    private LoginStatus loginStatus = (LoginStatus)LoginMsgFactory.createMsg();

    private LoginRTT loginRtt = (LoginRTT) LoginMsgFactory.createMsg();
 
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    /**
     * Instantiates a new login handler.
     */
    public LoginHandler()
    {
        loginClose.rdmMsgType(LoginMsgType.CLOSE);
        loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        loginStatus.rdmMsgType(LoginMsgType.STATUS);
        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
        loginRtt.rdmMsgType(LoginMsgType.RTT);
    }
    
    /**
     * Get the cached login refresh.
     * @return  Cached login refresh.
     * @see LoginRefresh
     */
    public LoginRefresh refreshInfo()
    {
        return loginRefresh;
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
        this.userName = userName;
    }

    /**
     * Sets the authentication token requested by the application.
     *
     * @param authenticationToken the authentication token
     */
    public void authenticationToken(String authenticationToken)
    {
        this.authenticationToken = authenticationToken;
    }

    /**
     * Sets the authentication extended information requested by the application.
     *
     * @param authenticationExtended the authentication extended
     */
    public void authenticationExtended(String authenticationExtended)
    {
        this.authenticationExtended = authenticationExtended;
    }

    /**
     * Sets the application id for the request.
     *
     * @param applicationId the application id
     */
    public void applicationId(String applicationId)
    {
        this.applicationId = applicationId;
    }

    /**
     * Sets the application name for the request.
     *
     * @param applicationName the application name
     */
    public void applicationName(String applicationName)
    {
        this.applicationName = applicationName;
    }

    /**
     * Login Role. Constant from {@link com.thomsonreuters.upa.rdm.Login.RoleTypes}.
     * Default login role is {@link com.thomsonreuters.upa.rdm.Login.RoleTypes#CONS}
     *
     * @param role the role
     */
    public void role(int role)
    {
        this.role = role;
    }

    /**
     * Flag which defines supporting of the RTT feature by this application.
     * @param enableRtt
     */
    public void enableRtt(boolean enableRtt) {
        this.enableRtt = enableRtt;
    }

    /**
     * Sends a login request to a channel. This consists of getting a message
     * buffer, setting the login request information, encoding the login
     * request, and sending the login request to the server.
     *
     * @param chnl The channel to send a login request to
     * @param error the error
     * @return Returns success if send login request succeeds or failure if it
     *         fails.
     */
    public int sendRequest(ChannelSession chnl, Error error)
    {
        /* get a buffer for the login request */
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        loginRequest.clear();

        loginRequest.initDefaultRequest(LOGIN_STREAM_ID);

        if (userName != null && !userName.isEmpty())
        {
            loginRequest.userName().data(userName);
        }
        
        if (authenticationToken != null && !authenticationToken.isEmpty())
        {
        	loginRequest.applyHasUserNameType();
        	loginRequest.userNameType(Login.UserIdTypes.AUTHN_TOKEN);
        	loginRequest.userName().data(authenticationToken);
        	
	        if (authenticationExtended != null && !authenticationExtended.isEmpty())
	        {
	        	loginRequest.applyHasAuthenticationExtended();
	        	loginRequest.authenticationExtended().data(authenticationExtended);
	        }
        }

        if (applicationId != null && !applicationId.isEmpty())
        {
            loginRequest.applyHasAttrib();
            loginRequest.attrib().applyHasApplicationId();
            loginRequest.attrib().applicationId().data(applicationId);
        }

        if (applicationName != null && !applicationName.isEmpty())
        {
            loginRequest.applyHasAttrib();
            loginRequest.attrib().applyHasApplicationName();
            loginRequest.attrib().applicationName().data(applicationName);
        }

        loginRequest.applyHasRole();
        loginRequest.role(role);

        if (role == Login.RoleTypes.PROV)
        {
            loginRequest.attrib().applyHasSingleOpen();
            loginRequest.attrib().singleOpen(0);
        }

        if (enableRtt && role == Login.RoleTypes.CONS) {
            loginRequest.attrib().applyHasSupportRoundTripLatencyMonitoring();
        }

        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = loginRequest.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Encoding of login request failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        System.out.println(loginRequest.toString());
        return chnl.write(msgBuf, error);
    }

    /**
     * Sends RTT message to a channel if RTT feature had been enabled.
     * @param channelSession The channel to send a login request to
     * @param error the error
     * @return Returns success if send login RTT succeeds or failure if it
     * fails.
     */
    public int sendRttMessage(ChannelSession channelSession, Error error) {
        if (!enableRtt) { //ignore sending the RTT message.
            return TransportReturnCodes.SUCCESS;
        }

        Channel channel = channelSession.channel();
        TransportBuffer transportBuffer = channelSession
                .getTransportBuffer(TRANSPORT_BUFFER_SIZE_RTT, false, error);
        if (Objects.isNull(transportBuffer)) {
            return CodecReturnCodes.FAILURE;
        }

        encIter.clear();
        encIter.setBufferAndRWFVersion(transportBuffer, channel.majorVersion(), channel.minorVersion());

        int ret = loginRtt.encode(encIter);
        if (!Objects.equals(CodecReturnCodes.SUCCESS, ret)) {
            error.text("Encoding of login RTT failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        ret = channelSession.write(transportBuffer, error);
        if (Objects.equals(TransportReturnCodes.SUCCESS, ret)) {
            logRttMessageSending(channelSession.socketFdValue());
        }
        return ret;
    }

    /**
     * Close the login stream. Note that closing login stream will automatically
     * close all other streams at the provider.
     *
     * @param chnl The channel to send a login close to
     * @param error the error
     * @return Returns success if close login stream succeeds or failure if it
     *         fails.
     */
    public int closeStream(ChannelSession chnl, Error error)
    {
        // get a buffer for the login close
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, error);

        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        loginClose.clear();
        loginClose.streamId(LOGIN_STREAM_ID);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = loginClose.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Encoding of login close failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        return chnl.write(msgBuf, error);
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
            case MsgClasses.GENERIC:
                return handleLoginRtt(msg, dIter, error);
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
        loginStatus.clear();
        int ret = loginStatus.decode(dIter, msg);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Decoding of login status failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        System.out.println("Received Login StatusMsg");
        if (!loginStatus.checkHasState())
            return CodecReturnCodes.SUCCESS;

        State state = loginStatus.state();
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
        loginRefresh.clear();
        int ret = loginRefresh.decode(dIter, msg);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("Decoding of login refresh failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        System.out.println("Received Login Response for Username: " + loginRefresh.userName());
        System.out.println(loginRefresh.toString());
        
        State state = loginRefresh.state();
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

    private int handleLoginRtt(Msg msg, DecodeIterator decodeIterator, Error error) {
        int ret = CodecReturnCodes.SUCCESS;
        if (enableRtt) {
            loginRtt.clear();
            ret = loginRtt.decode(decodeIterator, msg);
            if (ret != CodecReturnCodes.SUCCESS) {
                error.text("Decoding of login RTT failed: <" + CodecReturnCodes.toString(ret) + ">");
            }
        }
        return ret;
    }

    private void logRttMessageSending(int socketFdValue) {
        System.out.printf("\nReceived login RTT message from Provider %d.\n", socketFdValue);
        System.out.printf("\tTicks: %du\n", NANOSECONDS.toMicros(loginRtt.ticks()));
        if (loginRtt.checkHasRTLatency()) {
            System.out.printf("\tLast Latency: %du\n", NANOSECONDS.toMicros(loginRtt.rtLatency()));
        }
        if (loginRtt.checkHasTCPRetrans()) {
            System.out.printf("\tProvider side TCP Retransmissions: %du\n", loginRtt.tcpRetrans());
        }
    }
}
