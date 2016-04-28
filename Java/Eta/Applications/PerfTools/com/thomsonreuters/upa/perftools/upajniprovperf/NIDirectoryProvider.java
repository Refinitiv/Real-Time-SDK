package com.thomsonreuters.upa.perftools.upajniprovperf;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.perftools.common.DirectoryProvider;
import com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;

/**
 * The directory handler for the upajNIProvPerf. Configures a
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