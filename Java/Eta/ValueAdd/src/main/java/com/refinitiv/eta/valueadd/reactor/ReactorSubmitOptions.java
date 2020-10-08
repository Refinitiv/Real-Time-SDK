package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.WriteArgs;

/**
 * ReactorSubmitOptions to be used in the
 * {@link ReactorChannel#submit(com.refinitiv.eta.transport.TransportBuffer,
 * ReactorSubmitOptions, ReactorErrorInfo)},
 * {@link ReactorChannel#submit(com.refinitiv.eta.codec.Msg, ReactorSubmitOptions,
 * ReactorErrorInfo)} and
 * {@link ReactorChannel#submit(com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase,
 * ReactorSubmitOptions, ReactorErrorInfo)} call.
 */
public class ReactorSubmitOptions
{
    WriteArgs _writeArgs = null;
    String _serviceName;
    ReactorRequestMsgOptions _requestMsgOptions = null;

    /**
     * Instantiates a new reactor submit options.
     */
    ReactorSubmitOptions()
    {
        _writeArgs = TransportFactory.createWriteArgs();
        _requestMsgOptions = new ReactorRequestMsgOptions();
    }

    /**
     * Returns the {@link WriteArgs}, which will be used with the
     * {@link Channel#write(com.refinitiv.eta.transport.TransportBuffer, WriteArgs, com.refinitiv.eta.transport.Error)
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
     * instead of by ID (watchlist enabled only)  .
     *
     * @return service name
     */ 
    public String serviceName()
    {
        return _serviceName;
    }
    
    /**
     * Service name to be associated with the message, if specifying the service by name 
     * instead of by ID (watchlist enabled only)  .
     *
     * @param serviceName the service name
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
