/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.common;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.shared.ConsumerLoginState;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
// APIQA:
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequestFlags;
// END APIQA
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginStatus;

/**
 * This is the Login handler for the ETA Consumer and NIProvider application. It
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

    private ConsumerLoginState loginState = ConsumerLoginState.PENDING_LOGIN;

    // For requests
    private String userName;
    private String applicationName;
    private String authenticationToken;
    private String authenticationExtended;
    private String applicationId;

    private int role = Login.RoleTypes.CONS;

    private LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
    private LoginClose loginClose = (LoginClose)LoginMsgFactory.createMsg();

    private LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    private LoginStatus loginStatus = (LoginStatus)LoginMsgFactory.createMsg();

    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    public LoginHandler()
    {
        loginClose.rdmMsgType(LoginMsgType.CLOSE);
        loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        loginStatus.rdmMsgType(LoginMsgType.STATUS);
        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
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
     * @param userName
     */
    public void userName(String userName)
    {
        this.userName = userName;
    }

    /**
     * Sets the authentication token requested by the application.
     * 
     * @param authenticationToken
     */
    public void authenticationToken(String authenticationToken)
    {
        this.authenticationToken = authenticationToken;
    }

    /**
     * Sets the authentication extended information requested by the application.
     * 
     * @param authenticationExtended
     */
    public void authenticationExtended(String authenticationExtended)
    {
        this.authenticationExtended = authenticationExtended;
    }

    /**
     * Sets the application id for the request.
     * 
     * @param applicationId
     */
    public void applicationId(String applicationId)
    {
        this.applicationId = applicationId;
    }

    /**
     * Sets the application name for the request.
     * 
     * @param applicationName
     */
    public void applicationName(String applicationName)
    {
        this.applicationName = applicationName;
    }

    /**
     * Login Role. Constant from {@link com.refinitiv.eta.rdm.Login.RoleTypes}.
     * Default login role is {@link com.refinitiv.eta.rdm.Login.RoleTypes#CONS}
     * 
     * @param role
     */
    public void role(int role)
    {
        this.role = role;
    }

    /**
     * Sends a login request to a channel. This consists of getting a message
     * buffer, setting the login request information, encoding the login
     * request, and sending the login request to the server.
     * 
     * @return Returns success if send login request succeeds or failure if it
     *         fails.
     * 
     * @param chnl The channel to send a login request to
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

    // APIQA: Method for sending a PAUSE ALL and RESUME ALL
    public int sendPauseResumeRequest(ChannelSession chnl, Error error, boolean pauseAll, boolean resumeAll)
    {
        /* get a buffer for the login request */
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        loginRequest.clear();

        loginRequest.initDefaultRequest(LOGIN_STREAM_ID);

        if (pauseAll)
        {
            loginRequest.applyPause();
            loginRequest.applyNoRefresh();
        }
        if (resumeAll)
        {
            loginRequest.flags(loginRequest.flags() & ~LoginRequestFlags.PAUSE_ALL);
            loginRequest.flags(loginRequest.flags() & ~LoginRequestFlags.NO_REFRESH);
        }

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

    // END APIQA: Method for sending a PAUSE ALL and RESUME ALL

    /**
     * Close the login stream. Note that closing login stream will automatically
     * close all other streams at the provider.
     * 
     * @return Returns success if close login stream succeeds or failure if it
     *         fails.
     * 
     * @param chnl The channel to send a login close to
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
     * @param dIter The decode iterator
     * @param msg The partially decoded message
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
        else if (state.streamState() == StreamStates.OPEN && state.dataState() == DataStates.SUSPECT)
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
            if (state.dataState() == DataStates.OK)
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
