/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.provperf;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.perftools.common.ChannelHandler;
import com.refinitiv.eta.perftools.common.ClientChannelInfo;
import com.refinitiv.eta.perftools.common.DirectoryProvider;
import com.refinitiv.eta.perftools.common.PerfToolsReturnCodes;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.WritePriorities;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/**
 * The directory handler for the ProvPerf. Configures a
 * single service and provides encoding and sending of a directory message.
 */
public class IDirectoryProvider extends DirectoryProvider
{    
    // directory request received for interactive provider
    private DirectoryRequest            _directoryRequest; 
    private ReactorErrorInfo      _errorInfo; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorSubmitOptions  _reactorSubmitOptions; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    
    public IDirectoryProvider()
    {
    	super();
        _directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        _directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
        
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _reactorSubmitOptions = ReactorFactory.createReactorSubmitOptions();
        _reactorSubmitOptions.clear();
        _reactorSubmitOptions.writeArgs().priority(WritePriorities.HIGH);
    }

    /**
     * Processes a directory request. This consists of decoding directory
     * request message and encoding/sending directory refresh.
     * 
     */
    int processMsg(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Msg msg, DecodeIterator dIter, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                System.out.println("Received Source Directory Request");
                int ret = _directoryRequest.decode(dIter, msg);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    error.text("DirectoryRequest.decode?() failed with return code: " + CodecReturnCodes.toString(ret));
                    error.errorId(ret);
                    return PerfToolsReturnCodes.FAILURE;
                }
                // send source directory response
                return sendRefresh(channelHandler, clientChannelInfo, error);
            case MsgClasses.CLOSE:
                System.out.println("Received Directory Close for streamId " + msg.streamId());
                break;
            default:
                error.text("Received unhandled Source Directory msg type: " + msg.msgClass());
                error.errorId(PerfToolsReturnCodes.FAILURE);
                return PerfToolsReturnCodes.FAILURE;
        }

        return PerfToolsReturnCodes.SUCCESS;
    }

    /*
     * Send source directory refresh.
     */
    private int sendRefresh(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Error error)
    {
        Channel channel = clientChannelInfo.channel;
        
        // get a buffer for the source directory refresh 
        TransportBuffer msgBuf = channel.getBuffer(REFRESH_MSG_SIZE, false, error);
        if (msgBuf == null)
        {
            return PerfToolsReturnCodes.FAILURE;
        }

        // encode source directory refresh
        _directoryRefresh.clear();
        _directoryRefresh.streamId(_directoryRequest.streamId());

        // clear cache
        _directoryRefresh.applyClearCache();
        _directoryRefresh.applySolicited();

        // state information for refresh message
        _directoryRefresh.state().clear();
        _directoryRefresh.state().streamState(StreamStates.OPEN);
        _directoryRefresh.state().dataState(DataStates.OK);
        _directoryRefresh.state().code(StateCodes.NONE);
        _directoryRefresh.state().text().data("Source Directory Refresh Completed");

        // attribInfo information for response message
        _directoryRefresh.filter(_directoryRequest.filter());

        // ServiceId
        if (_directoryRequest.checkHasServiceId())
        {
            _directoryRefresh.applyHasServiceId();
            _directoryRefresh.serviceId(_directoryRequest.serviceId());
            if (_directoryRequest.serviceId() == _service.serviceId())
            {
                _directoryRefresh.serviceList().add(_service);
            }
        }
        else
        {
            _directoryRefresh.serviceList().add(_service);
        }

        // encode directory refresh
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIter.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }
        ret = _directoryRefresh.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("DirectoryRefresh.encode() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        // send source directory refresh
        return channelHandler.writeChannel(clientChannelInfo, msgBuf, 0, error);
    }
    
    /*
     * Send source directory refresh.
     */
    int sendRefreshReactor(ClientChannelInfo clientChannelInfo, Error error)
    {
        ReactorChannel reactorChannel = clientChannelInfo.reactorChannel;
        
        // get a buffer for the source directory refresh 
        TransportBuffer msgBuf = reactorChannel.getBuffer(REFRESH_MSG_SIZE, false, _errorInfo);
        if (msgBuf == null)
        {
            return PerfToolsReturnCodes.FAILURE;
        }

        // encode source directory refresh
        _directoryRefresh.clear();
        _directoryRefresh.streamId(_directoryRequest.streamId());

        // clear cache
        _directoryRefresh.applyClearCache();
        _directoryRefresh.applySolicited();

        // state information for refresh message
        _directoryRefresh.state().clear();
        _directoryRefresh.state().streamState(StreamStates.OPEN);
        _directoryRefresh.state().dataState(DataStates.OK);
        _directoryRefresh.state().code(StateCodes.NONE);
        _directoryRefresh.state().text().data("Source Directory Refresh Completed");

        // attribInfo information for response message
        _directoryRefresh.filter(_directoryRequest.filter());

        // ServiceId
        if (_directoryRequest.checkHasServiceId())
        {
            _directoryRefresh.applyHasServiceId();
            _directoryRefresh.serviceId(_directoryRequest.serviceId());
            if (_directoryRequest.serviceId() == _service.serviceId())
            {
                _directoryRefresh.serviceList().add(_service);
            }
        }
        else
        {
            _directoryRefresh.serviceList().add(_service);
        }

        // encode directory refresh
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIter.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }
        ret = _directoryRefresh.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("DirectoryRefresh.encode() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        // send source directory refresh
        return reactorChannel.submit(msgBuf, _reactorSubmitOptions, _errorInfo);
    }

    /* Returns directoryRequest. */
    public DirectoryRequest directoryRequest()
    {
        return _directoryRequest;
    }
}