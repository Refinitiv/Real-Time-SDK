package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.UInt;

/* service data information */
public class ServiceDataInfo
{
	public UInt		type;
	public Buffer	data;
	
	public ServiceDataInfo()
	{
		type = CodecFactory.createUInt();
		data = CodecFactory.createBuffer();	
	}
	
	public String toString()
	{
		return "\tServiceDataInfo" + "\n" +
				"\t\ttype: " + type.toLong() + "\n" +
				"\t\tdata: " + data.toString();
	}
}
