///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


/*
 * TunnelStreamRequest contains options used for the creation of a tunnel stream.
 */
public interface TunnelStreamRequest
{
		
	/*
	 * Clears TunnelStreamRequest object
	 */
	public TunnelStreamRequest clear();
	
	/*
	 * Specifies DomainType member
	 */
	public TunnelStreamRequest domainType(int domainType);
	
	/*
	 * Specifies serviceId member
	 */
	public TunnelStreamRequest serviceId(int serviceId);
	
	/*
	 * Specifies serviceName member
	 */
	public TunnelStreamRequest serviceName(String serviceName);
	
	/*
	 * Specifies the tunnel stream name, which is provided to the remote application.
	 */
	public TunnelStreamRequest name(String name);
	
	/*
	 * Sets the duration to wait for a provider to respond to a tunnel stream open request.
	 */
	public TunnelStreamRequest responseTimeout(int timeout);
	
	/*
	 * Specifies the number of guaranteed output buffers available for the tunnel stream.
	 */
	@Deprecated
	public TunnelStreamRequest guaranteedOuputBuffers(int outputBuffers);
	
	/*
	 * Specifies the number of guaranteed output buffers available for the tunnel stream.
	 */
	public TunnelStreamRequest guaranteedOutputBuffers(int outputBuffers);
	
	/*
	 * Specifies the ClassOfService member
	 */
	public TunnelStreamRequest classOfService(ClassOfService cos);
	
	/*
	 * Specifies login request message to be used if authentication type is set to OmmLoginEnum.
	 */
	public TunnelStreamRequest loginReqMsg(ReqMsg loginReq);
	
	/*
	 * Indicates the presence of service id.
	 */
	public boolean hasServiceId();
	
	/*
	 * Indicates the presence of service name.
	 */
	public boolean hasServiceName();
	
	/*
	 * Indicates the presence of name.
	 */
	public boolean hasName();
	
	/*
	 * Indicates the presence of login request message.
	 */
	public boolean hasLoginReqMsg();
	
	/*
	 * Returns the DomainType member.
	 */
	public int domainType();
	
	/*
	 * Returns the ServiceId member.
	 */
	public int serviceId();
	
	/*
	 * Returns the serviceName member.
	 */
	public String serviceName();
	
	/*
	 * Returns tunnel stream name.
	 */
	public String name();
	
	/*
	 * Returns ResponseTimeOut member.
	 */
	public int responseTimeOut();
	
	/*
	 * Returns number of GuaranteedOutpoutBuffers.
	 */
	public int guaranteedOutputBuffers();
	
	/*
	 * Returns ClassOfService member.
	 */
	public ClassOfService classOfService();
	
	/*
	 * Returns login request message
	 */
	public ReqMsg loginReqMsg(); 
		
}