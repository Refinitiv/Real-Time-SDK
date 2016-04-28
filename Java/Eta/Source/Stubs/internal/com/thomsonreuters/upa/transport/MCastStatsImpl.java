package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.MCastStats;

public class MCastStatsImpl implements MCastStats
{
	
	@Override
	public String toString()
	{
		return null;
	}
   
	@Override
    public long mcastSent()
    {
	   return TransportReturnCodes.FAILURE;
    }
   
   @Override
    public long mcastRcvd()
    {
    	return TransportReturnCodes.FAILURE;
    }
  
    @Override
    public long unicastSent()
    {
    	return TransportReturnCodes.FAILURE;
    }
  
    @Override
    public long unicastRcvd()
    {
    	return TransportReturnCodes.FAILURE;
    }
  
    @Override
    public long gapsDetected()
    {
    	return TransportReturnCodes.FAILURE;
    }
  
    @Override
    public long retransReqSent()
    {
    	return TransportReturnCodes.FAILURE;
    }

    @Override
    public long retransReqRcvd()
    {
    	return TransportReturnCodes.FAILURE;
    }

      @Override
    public long retransPktsSent()
    {
    	  return TransportReturnCodes.FAILURE;
    }

     @Override
    public long retransPktsRcvd()
    {
    	 return TransportReturnCodes.FAILURE;
    }
}
