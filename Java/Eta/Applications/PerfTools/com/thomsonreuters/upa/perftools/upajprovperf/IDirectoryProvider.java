package com.thomsonreuters.upa.perftools.upajprovperf;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.perftools.common.ChannelHandler;
import com.thomsonreuters.upa.perftools.common.ClientChannelInfo;
import com.thomsonreuters.upa.perftools.common.DirectoryProvider;
import com.thomsonreuters.upa.perftools.common.PerfToolsReturnCodes;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;

/**
 * The directory handler for the upajProvPerf. Configures a
 * single service and provides encoding and sending of a directory message.
 */
public class IDirectoryProvider extends DirectoryProvider
{    
    // directory request received for interactive provider
    private DirectoryRequest            _directoryRequest; 
    
    public IDirectoryProvider()
    {
    	super();
        _directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        _directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
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
}