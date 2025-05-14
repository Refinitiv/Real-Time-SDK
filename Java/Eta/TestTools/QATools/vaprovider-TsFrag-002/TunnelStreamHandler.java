/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.provider;

// APIQA
import com.refinitiv.eta.codec.Buffer;
// END APIQA:
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
// APIQA
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.RefreshMsg;
// END APIQA:
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.ClassesOfService;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
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
import com.refinitiv.eta.valueadd.reactor.TunnelStreamInfo;

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

    void processNewStream(TunnelStreamRequestEvent event)
    {
        int ret;

        if (isFilterValid(event.classOfServiceFilter()) && isClassOfServiceValid(event.classOfService()))
        {
            _tunnelStreamAcceptOptions.clear();

            // set class of service to what this provider supports
            _tunnelStreamAcceptOptions.classOfService().dataIntegrity().type(ClassesOfService.DataIntegrityTypes.RELIABLE);
            _tunnelStreamAcceptOptions.classOfService().flowControl().type((ClassesOfService.FlowControlTypes.BIDIRECTIONAL));

            // Set Authentication to match consumer. This provider will perform
            // OMM Login authentication if requested.
            _tunnelStreamAcceptOptions.classOfService().authentication().type(event.classOfService().authentication().type());

            _tunnelStreamAcceptOptions.statusEventCallback(this);
            _tunnelStreamAcceptOptions.defaultMsgCallback(this);

            if ((ret = event.reactorChannel().acceptTunnelStream(event, _tunnelStreamAcceptOptions, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("acceptTunnelStream() failed with return code: " + ret + " <" + event.errorInfo().error().text() + ">");
            }
        }
        else
        // invalid tunnel stream request
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
        return (classOfService.common().maxMsgSize() == _expectedClassOfService.common().maxMsgSize()
                && classOfService.common().protocolType() == _expectedClassOfService.common().protocolType()
                && classOfService.common().protocolMajorVersion() == _expectedClassOfService.common().protocolMajorVersion()
                && classOfService.common().protocolMinorVersion() == _expectedClassOfService.common().protocolMinorVersion()
                && (classOfService.authentication().type() == _expectedClassOfService.authentication().type() || classOfService.authentication().type() == ClassesOfService.AuthenticationTypes.NOT_REQUIRED)
                && classOfService.flowControl().type() == _expectedClassOfService.flowControl().type() && classOfService.dataIntegrity().type() == _expectedClassOfService.dataIntegrity().type() && classOfService
                .guarantee().type() == _expectedClassOfService.guarantee().type());
    }

    private boolean isFilterValid(long filter)
    {
        // this provider must have authentication, flow control and data
        // integrity turned on and persistence turned off
        return ((filter & ClassesOfService.FilterFlags.FLOW_CONTROL) > 0 && (filter & ClassesOfService.FilterFlags.DATA_INTEGRITY) > 0 && (filter & ClassesOfService.FilterFlags.GUARANTEE) == 0);
    }

    @Override
    public int defaultMsgCallback(TunnelStreamMsgEvent event)
    {
        int ret;
        _decodeIter.clear();
        _decodeIter.setBufferAndRWFVersion(event.transportBuffer(), event.tunnelStream().classOfService().common().protocolMajorVersion(), event.tunnelStream().classOfService().common()
                .protocolMinorVersion());
        if (event.containerType() == DataTypes.MSG)
        {
            _msg.decode(_decodeIter);
        }

        // check for login request
        if (event.containerType() == DataTypes.MSG && event.tunnelStream().classOfService().authentication().type() == ClassesOfService.AuthenticationTypes.OMM_LOGIN
                && _msg.domainType() == DomainTypes.LOGIN && _msg.msgClass() == MsgClasses.REQUEST)
        {
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

            if (_loginRequest.checkHasAttrib() && _loginRequest.attrib().checkHasPosition())
            {
                _loginRefresh.attrib().applyHasPosition();
                _loginRefresh.attrib().position().data(_loginRequest.attrib().position().data(), _loginRequest.attrib().position().position(), _loginRequest.attrib().position().length());
            }

            // this provider does not support
            // singleOpen behavior
            _loginRefresh.attrib().applyHasSingleOpen();
            _loginRefresh.attrib().singleOpen(0);
			
            // APIQA:
            System.out.println("Get tunnel stream buffer size = 1024 by calling TunnelStream.getBuffer()");
            // END APIQA:

            TransportBuffer buffer = _tunnelStream.getBuffer(1024, event.errorInfo());
            if (buffer == null)
            {
                System.out.println("defaultMsgCallback failed: Unable to get a buffer from TunnelStream <" + event.errorInfo().error().text() + ">");
                return ReactorCallbackReturnCodes.SUCCESS;
            }
			
            // APIQA:
            TunnelStreamInfo tunnelStreamInfo = ReactorFactory.createTunnelStreamInfo();
            if(_tunnelStream.info(tunnelStreamInfo, event.errorInfo()) != ReactorReturnCodes.SUCCESS)
            {
            	System.out.println("TunnelStream.info() failed: " + CodecReturnCodes.toString(event.errorInfo().error().errorId())
                + "(" + event.errorInfo().error().text() + ")");
            }
            else
            {
            	System.out.println("tunnelStreamInfo.buffersUsed() = " + tunnelStreamInfo.buffersUsed() + " after calling TunnelStream.getBuffer()");
            }
            // END APIQA:

            _encodeIter.clear();
            ret = _encodeIter.setBufferAndRWFVersion(buffer, event.tunnelStream().classOfService().common().protocolMajorVersion(), event.tunnelStream().classOfService().common()
                    .protocolMinorVersion());
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
                System.out.println("TunnelStream.submit() failed : " + CodecReturnCodes.toString(ret) + "(" + event.errorInfo().error().text() + ")");
                _tunnelStream.releaseBuffer(buffer, event.errorInfo());
                return ReactorCallbackReturnCodes.SUCCESS;
            }
			
            // APIQA:
            tunnelStreamInfo.clear();
            if(_tunnelStream.info(tunnelStreamInfo, event.errorInfo()) != ReactorReturnCodes.SUCCESS)
            {
            	System.out.println("TunnelStream.info() failed: " + CodecReturnCodes.toString(event.errorInfo().error().errorId())
                + "(" + event.errorInfo().error().text() + ")");
            }
            else
            {
            	System.out.println("tunnelStreamInfo.buffersUsed() = " + tunnelStreamInfo.buffersUsed() + " after calling TunnelStream.submit()");
            }
            // END APIQA:

            System.out.println("Login Refresh sent by Provider TunnelStreamHandler\n");
        }
// APIQA
        else if (event.containerType() == DataTypes.MSG && _msg.domainType() == DomainTypes.SYSTEM && (_msg.msgClass() == MsgClasses.GENERIC || _msg.msgClass() == MsgClasses.REQUEST))
        {
            // put buffer into generic message if received message is generic
            if (_msg.msgClass() == MsgClasses.GENERIC)
            {
                boolean testPassed = true;
                byte b = 0;
                for (int i = event.msg().encodedDataBody().position(); i < event.msg().encodedDataBody().length(); i++, b++)
                {
                    if (b == 256)
                    {
                        b = 0;
                    }
                    if (event.msg().encodedDataBody().data().get(i) != b)
                    {
                        testPassed = false;
                        break;
                    }
                }

                String resultString = testPassed ? "TEST PASSED" : "TEST FAILED";
                System.out.println("Provider TunnelStreamHandler received MSG data: " + resultString + "\n");

                // get buffer to encode message into
                Buffer buffer = CodecFactory.createBuffer();
                buffer.data(resultString);

                GenericMsg genericMsg = (GenericMsg)CodecFactory.createMsg();
                genericMsg.clear();
                genericMsg.msgClass(MsgClasses.GENERIC);
                genericMsg.streamId(_msg.streamId());
                genericMsg.domainType(_msg.domainType());
                genericMsg.containerType(DataTypes.OPAQUE);
                genericMsg.encodedDataBody(buffer);

                // submit the encoded data buffer to the tunnel stream
                _tunnelStreamSubmitOptions.clear();
                _tunnelStreamSubmitOptions.containerType(DataTypes.OPAQUE);
                if ((ret = event.tunnelStream().submit(genericMsg, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("TunnelStream.submit() failed: " + CodecReturnCodes.toString(ret) + "(" + event.errorInfo().error().text() + ")");
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
            }
            else if (_msg.msgClass() == MsgClasses.REQUEST) // just send refresh
                                                            // message if
                                                            // request
            {
                RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
                refreshMsg.clear();
                refreshMsg.msgClass(MsgClasses.REFRESH);
                refreshMsg.streamId(_msg.streamId());
                refreshMsg.domainType(_msg.domainType());
                refreshMsg.containerType(DataTypes.NO_DATA);
                refreshMsg.state().streamState(StreamStates.OPEN);
                refreshMsg.state().dataState(DataStates.OK);
                refreshMsg.state().code(StateCodes.NONE);
                refreshMsg.state().text().data("TunnelStream Refresh Complete");
                refreshMsg.applyHasMsgKey();
                refreshMsg.msgKey().applyHasServiceId();
                refreshMsg.msgKey().serviceId(_msg.msgKey().serviceId());
                refreshMsg.msgKey().applyHasName();
                refreshMsg.msgKey().name(_msg.msgKey().name());

                // submit the encoded data buffer to the tunnel stream
                _tunnelStreamSubmitOptions.clear();
                if ((ret = event.tunnelStream().submit(refreshMsg, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("TunnelStream.submit() failed: " + CodecReturnCodes.toString(ret) + "(" + event.errorInfo().error().text() + ")");
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
            }
        }
// END APIQA:
        else if (event.containerType() == DataTypes.OPAQUE)
        {
// APIQA
            boolean testPassed = true;
            byte b = 0;
            for (int i = 0; i < event.transportBuffer().length(); i++, b++)
            {
                if (b == 256)
                {
                    b = 0;
                }
                if (event.transportBuffer().data().get(i) != b)
                {
                    testPassed = false;
                    break;
                }
            }

            String resultString = testPassed ? "TEST PASSED" : "TEST FAILED";
            System.out.println("Provider TunnelStreamHandler received OPAQUE data: " + resultString + "\n");
            // byte[] msgBytes = new byte[event.transportBuffer().length()];
            // event.transportBuffer().data().get(msgBytes);
            // String msgString = new String(msgBytes);
            // System.out.println("Provider TunnelStreamHandler received OPAQUE data: "
            // + msgString + "\n");
// END APIQA:

            // APIQA:
            System.out.println("Get tunnel stream buffer size = " + resultString.length() + " by calling TunnelStream.getBuffer()");
            // END APIQA:

            // get buffer to encode response message into
// APIQA
            TransportBuffer buffer = _tunnelStream.getBuffer(resultString.length(), event.errorInfo());
            // TransportBuffer buffer =
            // _tunnelStream.getBuffer(msgString.length() +
            // " Response".length(), event.errorInfo());
// END APIQA:
            if (buffer == null)
            {
                System.out.println("defaultMsgCallback failed: Unable to get a buffer from TunnelStream <" + event.errorInfo().error().text() + ">");
                return ReactorCallbackReturnCodes.SUCCESS;
            }
			
            // APIQA:
            TunnelStreamInfo tunnelStreamInfo = ReactorFactory.createTunnelStreamInfo();
            if(_tunnelStream.info(tunnelStreamInfo, event.errorInfo()) != ReactorReturnCodes.SUCCESS)
            {
            	System.out.println("TunnelStream.info() failed: " + CodecReturnCodes.toString(event.errorInfo().error().errorId())
                + "(" + event.errorInfo().error().text() + ")");
            }
            else
            {
            	System.out.println("tunnelStreamInfo.buffersUsed() = " + tunnelStreamInfo.buffersUsed() + " after calling TunnelStream.getBuffer()");
            }
            // END APIQA:

            // put basic text message in buffer
// APIQA
            buffer.data().put(resultString.getBytes());
//            buffer.data().put(new String(msgString + " Response").getBytes());
// END APIQA:

            // submit the encoded data buffer to the tunnel stream
            _tunnelStreamSubmitOptions.clear();
            _tunnelStreamSubmitOptions.containerType(DataTypes.OPAQUE);
            if ((ret = _tunnelStream.submit(buffer, _tunnelStreamSubmitOptions, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("TunnelStream.submit() failed : " + CodecReturnCodes.toString(ret) + "(" + event.errorInfo().error().text() + ")");
                _tunnelStream.releaseBuffer(buffer, event.errorInfo());
                return ReactorCallbackReturnCodes.SUCCESS;
            }
			
            // APIQA:
            tunnelStreamInfo.clear();
            if(_tunnelStream.info(tunnelStreamInfo, event.errorInfo()) != ReactorReturnCodes.SUCCESS)
            {
            	System.out.println("TunnelStream.info() failed: " + CodecReturnCodes.toString(event.errorInfo().error().errorId())
                + "(" + event.errorInfo().error().text() + ")");
            }
            else
            {
            	System.out.println("tunnelStreamInfo.buffersUsed() = " + tunnelStreamInfo.buffersUsed() + " after calling TunnelStream.submit()");
            }
            // END APIQA:
        }
        else
        // not a login or opaque
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

        switch (state.streamState())
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
                // For other stream states such as Closed & ClosedRecover, close
                // the tunnel stream.
                if ((ret = event.tunnelStream().close(_finalStatusEvent, event.errorInfo())) < ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("Failed to close TunnelStream:" + ReactorReturnCodes.toString(ret) + "(" + event.errorInfo().error().text() + ")");
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
}
