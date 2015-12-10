package com.thomsonreuters.upa.examples.consumerperf;

import java.util.ArrayList;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.UInt;

/* service general information */
public class ServiceGeneralInfo
{
	public Buffer				serviceName;
	public Buffer				vendor;
	public UInt					isSource;
	public ArrayList<UInt>		capabilities;
	public ArrayList<Buffer>	dictionariesProvided;
	public ArrayList<Buffer>	dictionariesUsed;
	public ArrayList<Qos>		qos;
	public UInt					supportsQosRange;
	public Buffer				itemList;
	public UInt					supportsOutOfBandSnapshots;
	public UInt					acceptingConsumerStatus;
	
	public ServiceGeneralInfo()
	{
		serviceName = CodecFactory.createBuffer();
		vendor = CodecFactory.createBuffer();
		isSource = CodecFactory.createUInt();
		capabilities = new ArrayList<UInt>();
		dictionariesProvided = new ArrayList<Buffer>();
		dictionariesUsed = new ArrayList<Buffer>();
		qos = new ArrayList<Qos>();
		supportsQosRange = CodecFactory.createUInt();
		itemList = CodecFactory.createBuffer();
		supportsOutOfBandSnapshots = CodecFactory.createUInt();
		acceptingConsumerStatus = CodecFactory.createUInt();
	}
	
	public String toString()
	{
		StringBuffer strBuf = new StringBuffer("\tServiceGeneralInfo" + "\n" +
				"\t\tserviceName: " + serviceName.toString() + "\n" +
				"\t\tvendor: " + vendor.toString() + "\n" +
				"\t\tisSource: " + isSource.toLong() + "\n");
		
		strBuf.append("\t\tcapabilities: \n");
		for (int i = 0; i < capabilities.size(); i++)
		{
			strBuf.append("\t\t\tcapability[" + i + "]: " + capabilities.get(i).toLong() + "\n");
		}
		
		strBuf.append("\t\tdictionariesProvided: \n");
		for (int i = 0; i < dictionariesProvided.size(); i++)
		{
			strBuf.append("\t\t\tdictionariesProvided[" + i + "]: " + dictionariesProvided.get(i).toString() + "\n");
		}

		strBuf.append("\t\tdictionariesUsed: \n");
		for (int i = 0; i < dictionariesUsed.size(); i++)
		{
			strBuf.append("\t\t\tdictionariesUsed[" + i + "]: " + dictionariesUsed.get(i).toString() + "\n");
		}

		strBuf.append("\t\tqos: \n");
		for (int i = 0; i < qos.size(); i++)
		{
			strBuf.append("\t\t\tqos[" + i + "]: " + qos.get(i) + "\n");
		}
		
		strBuf.append("\t\tsupportsQosRange: " + supportsQosRange.toLong() + "\n" +
				"\t\titemList: " + itemList.toString() + "\n" +
				"\t\tsupportsOutOfBandSnapshots: " + supportsOutOfBandSnapshots.toLong() + "\n" +
				"\t\tacceptingConsumerStatus: " + acceptingConsumerStatus.toLong());

		return strBuf.toString();
	}
}
