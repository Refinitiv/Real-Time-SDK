/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.shared.ConsumerLoginState;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;

import java.net.InetAddress;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.core.JsonGenerator;

import java.io.IOException;

/**
 * This is the Login handler for the ETA Consumer and NIProvider application. It
 * provides methods for encoding and sending of login request, as well as
 * processing of responses (refresh, status, update, close). Methods are also
 * provided to allow setting of application name, user name and role, to be used
 * in the login request.
 */
public class LoginHandlerJson
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

    private LoginRefresh _loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    private LoginStatus _loginStatus = (LoginStatus)LoginMsgFactory.createMsg();


    /**
     * Instantiates a new login handler.
     */
    public LoginHandlerJson()
    {
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
     * Login Role. Constant from {@link com.refinitiv.eta.rdm.Login.RoleTypes}.
     * Default login role is {@link com.refinitiv.eta.rdm.Login.RoleTypes#CONS}
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
    public TransportBuffer getRequestMsg(Channel channel, Error error, String username, ObjectMapper mapper, JsonGenerator generator, ByteBufferOutputStream byteStream)
    {
        /* get a buffer for the login request */
        TransportBuffer msgBuf = channel.getBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);
        if (msgBuf == null)
            return null;

        byteStream.setByteBuffer(msgBuf.data());

        if (username == null || username.isEmpty())
        {
            /* use default user name */
            try
            {
                username = System.getProperty("user.name");
            }
            catch (Exception e)
            {
                username = "eta";
            }
        }

        /* use default position */
        String defaultPosition = null;
        try
        {
            defaultPosition = InetAddress.getLocalHost().getHostAddress() + "/"
                    + InetAddress.getLocalHost().getHostName();
        }
        catch (Exception e)
        {
            defaultPosition = "1.1.1.1/net";
        }

        // {"ID":1,"Type":"Request","Domain":"Login","KeyInUpdates":false,"Key":{"Name":"","Elements":{"ApplicationId":"256","ApplicationName":"ConsPerf","Position":"10.46.188.76/net","Role":0}}}
        ObjectNode json = mapper.createObjectNode()
            .put("ID", Integer.valueOf(1))
            .put("Type", "Request")
            .put("Domain", "Login")
            .put("KeyInUpdates", Boolean.FALSE)
            .set("Key", mapper.createObjectNode()
                .put("Name", username)
                .set("Elements", mapper.createObjectNode()
                    .put("ApplicationId", "256")
                    .put("ApplicationName", "ConsPerf")
                    .put("Position", defaultPosition)
                    .put("Role", Integer.valueOf(0))
            ));

        try
        {
            mapper.writeTree(generator, json);
        }
        catch (IOException e)
        {
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
    public int processResponse(Msg msg, JsonNode jsonNode, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                return handleLoginRefresh(msg, jsonNode, error);
            case MsgClasses.STATUS:
                return handleLoginStatus(msg, jsonNode, error);
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

    private int handleLoginStatus(Msg msg, JsonNode jsonNode, Error error)
    {
        _loginStatus.clear();

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

    private int handleLoginRefresh(Msg msg, JsonNode jsonNode, Error error)
    {
        _loginRefresh.clear();

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
