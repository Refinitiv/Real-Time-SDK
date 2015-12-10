package com.thomsonreuters.upa.examples.consumerperf;

import java.util.ArrayList;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.UInt;

/* entire source directory response information */
public class SourceDirectoryResponseInfo
{
	public int							streamId;
	public UInt						serviceId;
	public ServiceGeneralInfo			serviceGeneralInfo;
	public ServiceStateInfo				serviceStateInfo;
	public ServiceGroupInfo				serviceGroupInfo;
	public ServiceLoadInfo				serviceLoadInfo;
	public ServiceDataInfo				serviceDataInfo;
	public ArrayList<ServiceLinkInfo>	serviceLinkInfo;
	
	public SourceDirectoryResponseInfo()
	{
		serviceId = CodecFactory.createUInt();
		serviceGeneralInfo = new ServiceGeneralInfo();
		serviceStateInfo = new ServiceStateInfo();
		serviceGroupInfo = new ServiceGroupInfo();
		serviceLoadInfo = new ServiceLoadInfo();
		serviceDataInfo = new ServiceDataInfo();
		serviceLinkInfo = new ArrayList<ServiceLinkInfo>();
	}
	
	public String toString()
	{		
		StringBuffer strBuf = new StringBuffer("\nSourceDirectoryResponseInfo" + "\n" +
						"\tstreamId: " + streamId + "\n" +
						"\tserviceId: " + serviceId.toLong() + "\n" +
						serviceGeneralInfo + "\n" +
						serviceStateInfo + "\n" +
						serviceGroupInfo + "\n" +
						serviceLoadInfo + "\n" +
						serviceDataInfo + "\n");
		
		for (int i = 0; i < serviceLinkInfo.size() - 1; i++)
		{
			strBuf.append(serviceLinkInfo.get(i) + "\n");
		}
		
		return strBuf.toString();
	}
	
	/*
     * Returns whether or not the requested domain is supported by the provider
     * domainId - the desired domain 
     */
    public boolean hasCapability(int domainId)
    {
        /*check to see if the provider is able to support the requested domain*/
        for(int i = 0; i < serviceGeneralInfo.capabilities.size(); i++)
        {
            if(serviceGeneralInfo.capabilities.get(i).toLong() == domainId)
            {
                return true;    
            }
        }

        return false;
    }
}
