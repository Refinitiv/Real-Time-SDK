package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.WriteArgs;

/**
 * ReactorSubmitOptions to be used in the
 * {@link ReactorChannel#submit(com.thomsonreuters.upa.transport.TransportBuffer,
 * ReactorSubmitOptions, ReactorErrorInfo)},
 * {@link ReactorChannel#submit(com.thomsonreuters.upa.codec.Msg, ReactorSubmitOptions,
 * ReactorErrorInfo)} and
 * {@link ReactorChannel#submit(com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase,
 * ReactorSubmitOptions, ReactorErrorInfo)} call.
 */
public class ReactorSubmitOptions
{
    WriteArgs _writeArgs = null;
    String _serviceName;
    ReactorRequestMsgOptions _requestMsgOptions = null;

    ReactorSubmitOptions()
    {
        _writeArgs = TransportFactory.createWriteArgs();
        _requestMsgOptions = new ReactorRequestMsgOptions();
    }

    /**
     * Returns the {@link WriteArgs}, which will be used with the
     * {@link Channel#write(com.thomsonreuters.upa.transport.TransportBuffer, WriteArgs, com.thomsonreuters.upa.transport.Error)
     * Channel.write} call.
     * 
     * @return the WriteArgs
     * 
     * @see ReactorChannel
     */
    public WriteArgs writeArgs()
    {
        return _writeArgs;
    }

    /**
     * Service name to be associated with the message, if specifying the service by name 
     * instead of by ID (watchlist enabled only)  
     * 
     *  @return serviceName
     */ 
    public String serviceName()
    {
        return _serviceName;
    }
    
    /**
     * Service name to be associated with the message, if specifying the service by name 
     * instead of by ID (watchlist enabled only)  
     * 
     *  @param  serviceName
     */
    public void serviceName(String serviceName)
    {
        _serviceName = serviceName;
    }
    
    /**
     * If the submitted message is a RequestMsg and a watchlist is enabled, 
     * these options may also be specified. Use to set request message options.
     *    
     * @return requestMsgOptions
     */    
    public ReactorRequestMsgOptions requestMsgOptions()
    {
		return _requestMsgOptions;
	}

	/**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _writeArgs.clear();
        _serviceName = null;
        _requestMsgOptions.clear();
    }
}
