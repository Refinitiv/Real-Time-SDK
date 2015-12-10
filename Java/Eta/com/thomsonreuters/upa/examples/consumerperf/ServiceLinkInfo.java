package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.UInt;

/* service link information */
public class ServiceLinkInfo
{
	public Buffer	linkName;
	public UInt		type;
	public UInt		linkState;
	public UInt		linkCode;
	public Buffer	text;

	public ServiceLinkInfo()
	{
		linkName = CodecFactory.createBuffer();
		type = CodecFactory.createUInt();
		linkState = CodecFactory.createUInt();
		linkCode = CodecFactory.createUInt();
		text = CodecFactory.createBuffer();	
	}
	
	public String toString()
	{
		return "\tServiceLinkInfo" + "\n" +
				"\t\tlinkName: " + linkName.toString() + "\n" +
				"\t\ttype: " + type.toLong() + "\n" +
				"\t\tlinkState: " + linkState.toLong() + "\n" +
				"\t\tlinkCode: " + linkCode.toLong() + "\n" +
				"\t\ttext: " + text.toString();
	}
}
