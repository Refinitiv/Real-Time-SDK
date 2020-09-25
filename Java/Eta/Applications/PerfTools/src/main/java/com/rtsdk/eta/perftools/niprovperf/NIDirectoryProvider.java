package com.rtsdk.eta.perftools.niprovperf;

import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.StateCodes;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.perftools.common.DirectoryProvider;
import com.rtsdk.eta.rdm.Directory.ServiceFilterFlags;
import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportBuffer;

/**
 * The directory handler for the NIProvPerf. Configures a
 * single service and provides encoding and sending of a directory message.
 */
public class NIDirectoryProvider extends DirectoryProvider
{
    /**
     * Initializes the directory refresh.
     * 
     * @param streamId - stream id of refresh
     */
    void initRefresh(int streamId)
    {
        _directoryRefresh.clear();

        _directoryRefresh.applyClearCache();
        _directoryRefresh.filter(ServiceFilterFlags.INFO | ServiceFilterFlags.STATE | ServiceFilterFlags.GROUP);

        // streamId
        _directoryRefresh.streamId(streamId);

        _directoryRefresh.state().streamState(StreamStates.OPEN);
        _directoryRefresh.state().dataState(DataStates.OK);
        _directoryRefresh.state().code(StateCodes.NONE);

        _directoryRefresh.serviceList().add(_service);        
    }
    
    /**
     * Encodes the directory refresh.
     * 
     * @param channel 
     * @param streamId - stream id of refresh
     * @param error - detailed error information in case of failure 
     * @return - TransportBuffer with encoded message.
     */
	TransportBuffer encodeRefresh(Channel channel, int streamId, Error error)
	{
		// get a buffer for the source directory response
		TransportBuffer msgBuf = channel.getBuffer(REFRESH_MSG_SIZE, false, error);
        if (msgBuf == null)
        {
            return null;
        }
 		
        // set buffer on encode iterator
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return null;
        }

        // initialize source directory refresh
        initRefresh(streamId);

        // encode source directory refresh
        ret = _directoryRefresh.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return null;
        }

        // return encoded source directory refresh
        return msgBuf;
	}
}