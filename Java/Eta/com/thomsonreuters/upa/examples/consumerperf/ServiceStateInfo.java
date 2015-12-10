package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.UInt;

/* service state information */
public class ServiceStateInfo
{
	public UInt		serviceState;
	public UInt		acceptingRequests;
	public State	status;
	
	public ServiceStateInfo()
	{
		serviceState = CodecFactory.createUInt();
		acceptingRequests = CodecFactory.createUInt();
		status = CodecFactory.createState();
	}
	
	public String toString()
	{
		return "\tServiceStateInfo" + "\n" +
				"\t\tserviceState: " + serviceState.toLong() + "\n" +
				"\t\tacceptingRequests: " + acceptingRequests.toLong() + "\n" +
				"\t\tstatus: " + status;
	}
}

