package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.State;

/* service group information */
public class ServiceGroupInfo
{
	public Buffer	group;
	public Buffer	mergedToGroup;
	public State	status;
	
	public ServiceGroupInfo()
	{
		group = CodecFactory.createBuffer();	
		mergedToGroup = CodecFactory.createBuffer();	
		status = CodecFactory.createState();
	}
	
	public String toString()
	{
		return "\tServiceGroupInfo" + "\n" +
				"\t\tgroup: " + group.toString() + "\n" +
				"\t\tmergedToGroup: " + mergedToGroup.toString() + "\n" +
				"\t\tstatus: " + status;
	}
}
