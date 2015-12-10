package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.UInt;

/* service load information */
public class ServiceLoadInfo
{
	public UInt	openLimit;
	public UInt	openWindow;
	public UInt	loadFactor;
	
	public ServiceLoadInfo()
	{
		openLimit = CodecFactory.createUInt();
		openWindow = CodecFactory.createUInt();
		loadFactor = CodecFactory.createUInt();
	}
	
	public String toString()
	{
		return "\tServiceLoadInfo" + "\n" +
				"\t\topenLimit: " + openLimit.toLong() + "\n" +
				"\t\topenWindow: " + openWindow.toLong() + "\n" +
				"\t\tloadFactor: " + loadFactor.toLong();
	}

}
