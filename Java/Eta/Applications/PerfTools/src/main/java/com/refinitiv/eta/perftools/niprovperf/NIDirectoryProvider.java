/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.niprovperf;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.perftools.common.DirectoryProvider;
import com.refinitiv.eta.rdm.Directory.ServiceFilterFlags;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

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