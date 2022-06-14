/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.provider;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.ClassesOfService;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.shared.LoginRejectReason;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;
import com.refinitiv.eta.valueadd.reactor.ClassOfService;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.TunnelStream;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamAcceptOptions;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamDefaultMsgCallback;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamMsgEvent;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamRejectOptions;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamRequestEvent;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamStatusEvent;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamStatusEventCallback;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamSubmitOptions;

/* Handles TunnelStream connections for the VA Provider. */
class TunnelStreamHandler implements TunnelStreamStatusEventCallback, TunnelStreamDefaultMsgCallback
{
    TunnelStream _tunnelStream;
    private TunnelStreamAcceptOptions _tunnelStreamAcceptOptions = ReactorFactory.createTunnelStreamAcceptOptions();
    private TunnelStreamRejectOptions _tunnelStreamRejectOptions = ReactorFactory.createTunnelStreamRejectOptions();
    private ClassOfService _expectedClassOfService = ReactorFactory.createClassOfService();
    private DecodeIterator _decodeIter = CodecFactory.createDecodeIterator();
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    private Msg _msg = CodecFactory.createMsg();
    private LoginRequest _loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
    private LoginRefresh _loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    private TunnelStreamSubmitOptions _tunnelStreamSubmitOptions = ReactorFactory.createTunnelStreamSubmitOptions();
    private boolean _finalStatusEvent;
    private boolean _sendNack;
    private boolean _rejectTsLogin;
    
    public TunnelStreamHandler()
    {
        // set the expected class of service for this provider (use defaults for common properties)
        _expectedClassOfService.authentication().type(ClassesOfService.AuthenticationTypes.OMM_LOGIN);
        _expectedClassOfService.flowControl().type(ClassesOfService.FlowControlTypes.BIDIRECTIONAL);
        _expectedClassOfService.dataIntegrity().type(ClassesOfService.DataIntegrityTypes.RELIABLE);
    }

    // application id 
    private static String applicationId = "256";

    // application name
    private static String applicationName = "ETA TunnelStream Provider";

    void processNewStream(TunnelStreamRequestEvent event, boolean sendNack, boolean rejectTsLogin)
    {
    	int ret;
    	
        if (isFilterValid(event.classOfServiceFilter()) &&
                isClassOfServiceValid(event.classOfService()))
        {
            _tunnelStreamAcceptOptions.clear();
            
            // set class of service to what this provider supports
            _tunnelStreamAcceptOptions.classOfService().dataIntegrity().type(ClassesOfService.DataIntegrityTypes.RELIABLE);
            _tunnelStreamAcceptOptions.classOfService().flowControl().type((ClassesOfService.FlowControlTypes.BIDIRECTIONAL));

			// Set Authentication to match consumer. This provider will perform OMM Login authentication if requested.
            _tunnelStreamAcceptOptions.classOfService().authentication().type(event.classOfService().authentication().type());
            
            _tunnelStreamAcceptOptions.statusEventCallback(this);
            _tunnelStreamAcceptOptions.defaultMsgCallback(this);

            _sendNack = sendNack;
            _rejectTsLogin = rejectTsLogin;

            if ((ret = event.reactorChannel().acceptTunnelStream(event, _tunnelStreamAcceptOptions, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("acceptTunnelStream() failed with return code: " + ret + " <" + event.errorInfo().error().text() + ">");
            }           
        }
        else // invalid tunnel stream request
        {
            _tunnelStreamRejectOptions.clear();
            
			// Since we're rejecting due to a Class-of-Service mismatch,
			// send a redirect to the consumer.
			_tunnelStreamRejectOptions.expectedClassOfService(_expectedClassOfService);
			_tunnelStreamRejectOptions.state().streamState(StreamStates.REDIRECTED);
			_tunnelStreamRejectOptions.state().dataState(DataStates.SUSPECT);
			_tunnelStreamRejectOptions.state().code(StateCodes.NONE);
            _tunnelStreamRejectOptions.state().text().data("Unsupported TunnelStream class of service");

            if ((ret = event.reactorChannel().rejectTunnelStream(event, _tunnelStreamRejectOptions, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("rejectTunnelStream() failed with return code: " + ret + " <" + event.errorInfo().error().text() + ">");
            }            
        }
    }

    private boolean isClassOfServiceValid(ClassOfService classOfService)
    {
        return (classOfService.common().maxMsgSize() == _expectedClassOfService.common().maxMsgSize() &&
                classOfService.common().protocolType() == _expectedClassOfService.common().protocolType() &&
                classOfService.common().protocolMajorVersion() == _expectedClassOfService.common().protocolMajorVersion() &&
                classOfService.common().protocolMinorVersion() == _expectedClassOfService.common().protocolMinorVersion() &&
                (classOfService.authentication().type() == _expectedClassOfService.authentication().type() ||
				 classOfService.authentication().type() == ClassesOfService.AuthenticationTypes.NOT_REQUIRED) &&
                classOfService.flowControl().type() == _expectedClassOfService.flowControl().type() &&
                classOfService.dataIntegrity().type() == _expectedClassOfService.dataIntegrity().type() &&
                classOfService.guarantee().type() == _expectedClassOfService.guarantee().type());
    }

    private boolean isFilterValid(long filter)
    {
        // this provider must have authentication, flow control and data integrity turned on and persistence turned off
        return ((filter & ClassesOfService.FilterFlags.FLOW_CONTROL) > 0 &&
                (filter & ClassesOfService.FilterFlags.DATA_INTEGRITY) > 0 &&
                (filter & ClassesOfService.FilterFlags.GUARANTEE) == 0);
    }

    @Override
    public int defaultMsgCallback(TunnelStreamMsgEvent event)
    {
        int ret;
        _decodeIter.clear();
		_decodeIter.setBufferAndRWFVersion(event.transportBuffer(), event.tunnelStream().classOfService().common().protocolMajorVersion(),
				event.tunnelStream().classOfService().common().protocolMinorVersion());
        if (event.containerType() == DataTypes.MSG)
        {
            _msg.decode(_decodeIter);
        }
        
        // check for login request
        if (event.containerType() == DataTypes.MSG &&
            event.tunnelStream().classOfService().authentication().type() == ClassesOfService.AuthenticationTypes.OMM_LOGIN &&
            _msg.domainType() == DomainTypes.LOGIN && _msg.msgClass() == MsgClasses.REQUEST)
        {
            if(_rejectTsLogin)
            {
                if(sendRequestReject(event, event.msg().streamId(), LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED) != CodecReturnCodes.SUCCESS)
                    closeStream(true, event.errorInfo());

                return ReactorCallbackReturnCodes.SUCCESS;
            }

            _loginRequest.clear();
            _loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            _loginRequest.decode(_decodeIter, _msg);
            
            // send login refresh
            _loginRefresh.clear();
            _loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
            _loginRefresh.streamId(_loginRequest.streamId());
            _loginRefresh.applyHasUserName();
            _loginRefresh.userName().data(_loginRequest.userName().data(), _loginRequest.userName().position(), _loginRequest.userName().length());

            _loginRefresh.applyHasUserNameType();
            _loginRefresh.userNameType(Login.UserIdTypes.NAME);

            _loginRefresh.state().code(StateCodes.NONE);
            _loginRefresh.state().dataState(DataStates.OK);
            _loginRefresh.state().streamState(StreamStates.OPEN);
            _loginRefresh.state().text().data("Login accepted by TunnelStream " + _tunnelStream.name());

            _loginRefresh.applySolicited();

            _loginRefresh.applyHasAttrib();
            _loginRefresh.attrib().applyHasApplicationId();
            _loginRefresh.attrib().applicationId().data(applicationId);

            _loginRefresh.applyHasAttrib();
            _loginRefresh.attrib().applyHasApplicationName();
            _loginRefresh.attrib().applicationName().data(applicationName);

            if(_loginRequest.checkHasAttrib() && _loginRequest.attrib().checkHasPosition())
            {
                _loginRefresh.attrib().applyHasPosition();
                _loginRefresh.attrib().position().data(_loginRequest.attrib().position().data(), _loginRequest.attrib().position().position(), _loginRequest.attrib().position().length());
            }

            // this provider does not support
            // singleOpen behavior
            _loginRefresh.attrib().applyHasSingleOpen();
            _loginRefresh.attrib().singleOpen(0); 
            
            TransportBuffer buffer = _tunnelStream.getBuffer(1024, event.errorInfo());
            if (buffer == null)
            {
                System.out.println("defaultMsgCallback failed: Unable to get a buffer from TunnelStream <" + event.errorInfo().error().text() + ">");
                return ReactorCallbackReturnCodes.SUCCESS;
            }

            _encodeIter.clear();
            ret = _encodeIter.setBufferAndRWFVersion(buffer, event.tunnelStream().classOfService().common().protocolMajorVersion(),
					event.tunnelStream().classOfService().common().protocolMinorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ReactorCallbackReturnCodes.SUCCESS;
            }

            ret = _loginRefresh.encode(_encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("LoginRefresh.encode() failed with return code: " + CodecReturnCodes.toString(ret));
                return ReactorCallbackReturnCodes.SUCCESS;
            }

            _tunnelStreamSubmitOptions.clear();
            _tunnelStreamSubmitOptions.containerType(DataTypes.MSG);
            if ((ret = _tunnelStream.submit(buffer, _tunnelStreamSubmitOptions, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("TunnelStream.submit() failed : " + CodecReturnCodes.toString(ret)
                        + "(" + event.errorInfo().error().text() + ")");
                _tunnelStream.releaseBuffer(buffer, event.errorInfo());
                return ReactorCallbackReturnCodes.SUCCESS;
            }

            System.out.println("Login Refresh sent by Provider TunnelStreamHandler\n");
        }
        else if(event.containerType() == DataTypes.MSG && _msg.msgClass() == MsgClasses.POST)
        {
            PostMsg postMsg  = (PostMsg) _msg;

            if(_sendNack)
            {
                postMsg.applyAck();
                return sendAck(event, postMsg, NakCodes.NO_RESPONSE, "Intentional NACK");
            }
            else
            {
                if (postMsg.checkHasMsgKey()) {
                    return sendAck(event, postMsg, NakCodes.NONE, null);
                }
            }
        }
        else if (event.containerType() == DataTypes.OPAQUE)
        {
            byte[] msgBytes = new byte[event.transportBuffer().length()];
            event.transportBuffer().data().get(msgBytes);
            String msgString = new String(msgBytes);
            System.out.println("Provider TunnelStreamHandler received OPAQUE data: " + msgString + "\n");
            
            // get buffer to encode response message into
            TransportBuffer buffer = _tunnelStream.getBuffer(msgString.length() + " Response".length(), event.errorInfo());
            if (buffer == null)
            {
                System.out.println("defaultMsgCallback failed: Unable to get a buffer from TunnelStream <" + event.errorInfo().error().text() + ">");
                return ReactorCallbackReturnCodes.SUCCESS;
            }
            
            // put basic text message in buffer
            buffer.data().put((msgString + " Response").getBytes());

            // submit the encoded data buffer to the tunnel stream
            _tunnelStreamSubmitOptions.clear();
            _tunnelStreamSubmitOptions.containerType(DataTypes.OPAQUE);
            if ((ret = _tunnelStream.submit(buffer, _tunnelStreamSubmitOptions, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("TunnelStream.submit() failed : " + CodecReturnCodes.toString(ret)
                        + "(" + event.errorInfo().error().text() + ")");
                _tunnelStream.releaseBuffer(buffer, event.errorInfo());
                return ReactorCallbackReturnCodes.SUCCESS;
            }
        }
        else // not a login or opaque
        {
            System.out.println("TunnelStreamHandler received unsupported container type");
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int statusEventCallback(TunnelStreamStatusEvent event)
    {
        int ret;
        State state = event.state();
        
        System.out.println("Received TunnelStreamStatusEvent for Stream ID " + event.tunnelStream().streamId() + " with " + state + "\n");
        
        switch(state.streamState())
        {
            case StreamStates.OPEN:

                if (state.dataState() == DataStates.OK && _tunnelStream == null)
                {
                    // Stream is open and ready for use.
                    _tunnelStream = event.tunnelStream();                    
                }
                break;

            case StreamStates.CLOSED_RECOVER:
            case StreamStates.CLOSED:
            default:
                // For other stream states such as Closed & ClosedRecover, close the tunnel stream. 
                if ((ret = event.tunnelStream().close(_finalStatusEvent, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("Failed to close TunnelStream:"
                            + ReactorReturnCodes.toString(ret) + "(" + event.errorInfo().error().text() + ")");
                }
                
                // Remove our tunnel information if the tunnel was open.
                _tunnelStream = null;
                
                break;
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    int closeStream(boolean finalStatusEvent, ReactorErrorInfo errorInfo)
    {
    	int ret = ReactorReturnCodes.SUCCESS;
    	_finalStatusEvent = finalStatusEvent; 
    	
        if (_tunnelStream != null)
        {
            ret = _tunnelStream.close(finalStatusEvent, errorInfo);
            _tunnelStream = null;
        }
        
        return ret;
    }
    
    boolean isStreamClosed()
    {
    	return _tunnelStream == null;
    }

    private int sendAck(TunnelStreamMsgEvent event, PostMsg postMsg, int nakCode, String errText) {
        if (postMsg.checkAck())
        {
            TransportBuffer msgBuf = event.tunnelStream().getBuffer(1024, event.errorInfo());
            if (msgBuf == null)
            {
                System.out.println("tunnelStream().getBuffer() Failed (errorId = " + event.errorInfo().error().errorId() + ")");
                return CodecReturnCodes.FAILURE;
            }

            int ret = encodeAck(msgBuf, event, postMsg, nakCode, errText);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                event.tunnelStream().releaseBuffer(msgBuf, event.errorInfo());
                System.out.println("encodeAck() Failed (ret = " + ret + ")");
                return ret;
            }

            _tunnelStreamSubmitOptions.containerType(DataTypes.MSG);
            return event.tunnelStream().submit(msgBuf, _tunnelStreamSubmitOptions, event.errorInfo());
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Encodes an Ack message.
     */
    private int encodeAck(TransportBuffer msgBuf, TunnelStreamMsgEvent event, PostMsg postMsg, int nakCode, String text)
    {
        AckMsg ackMsg = (AckMsg)CodecFactory.createMsg();
        //set-up message
        ackMsg.clear();
        ackMsg.msgClass(MsgClasses.ACK);
        ackMsg.streamId(postMsg.streamId());
        ackMsg.domainType(postMsg.domainType());
        ackMsg.containerType(DataTypes.NO_DATA);
        ackMsg.flags(AckMsgFlags.NONE);
        ackMsg.nakCode(nakCode);
        ackMsg.ackId(postMsg.postId());
        ackMsg.seqNum(postMsg.seqNum());

        if (nakCode != NakCodes.NONE)
            ackMsg.applyHasNakCode();

        if (postMsg.checkHasSeqNum())
            ackMsg.applyHasSeqNum();

        if (text != null)
        {
            ackMsg.applyHasText();
            ackMsg.text().data(text);
        }

        //encode message
        _encodeIter.clear();

        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, event.tunnelStream().classOfService().common().protocolMajorVersion(),
                event.tunnelStream().classOfService().common().protocolMinorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            event.errorInfo().error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " +
                    CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = ackMsg.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            event.errorInfo().error().text("AckMsg.encode() failed");
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Sends the login request reject status message for a channel.
     */
    private int sendRequestReject(TunnelStreamMsgEvent event, int streamId, LoginRejectReason reason)
    {
        // get a buffer for the login request reject status
        TransportBuffer msgBuf = event.tunnelStream().getBuffer(512, event.errorInfo());
        if(msgBuf == null)
            return CodecReturnCodes.FAILURE;

        int ret = encodeRequestReject(event, streamId, reason, msgBuf);
        if(ret != CodecReturnCodes.SUCCESS)
            return ret;

        return event.tunnelStream().submit(msgBuf, _tunnelStreamSubmitOptions, event.errorInfo());
    }

    /*
     * Encodes the login request reject status. Returns success if encoding
     * succeeds or failure if encoding fails.
     */
    private int encodeRequestReject(TunnelStreamMsgEvent event, int streamId, LoginRejectReason reason, TransportBuffer msgBuf)
    {
        LoginStatus loginStatus = (LoginStatus)LoginMsgFactory.createMsg();

        loginStatus.clear();
        loginStatus.streamId(streamId);
        loginStatus.applyHasState();
        loginStatus.state().streamState(StreamStates.CLOSED_RECOVER);
        loginStatus.state().dataState(DataStates.SUSPECT);
        switch (reason)
        {
            case MAX_LOGIN_REQUESTS_REACHED:
                loginStatus.state().code(StateCodes.TOO_MANY_ITEMS);
                loginStatus.state().text().data("Login request rejected for stream id " + streamId + " - max request count reached");
                break;
            case NO_USER_NAME_IN_REQUEST:
                loginStatus.state().code(StateCodes.USAGE_ERROR);
                loginStatus.state().text().data("Login request rejected for stream id  " + streamId + " - request does not contain user name");
                break;
            case LOGIN_RDM_DECODER_FAILED:
                loginStatus.state().code(StateCodes.USAGE_ERROR);
                loginStatus.state().text().data("Login request rejected for stream id  " + streamId + " - decoding failure: " + event.errorInfo().error().text());
                break;
            default:
                break;
        }

        // clear encode iterator
        _encodeIter.clear();

        // encode message
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, event.tunnelStream().classOfService().common().protocolMajorVersion(),
                event.tunnelStream().classOfService().common().protocolMinorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            event.errorInfo().error().text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = loginStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            event.errorInfo().error().text("LoginStatus.encode() failed");
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }
}
