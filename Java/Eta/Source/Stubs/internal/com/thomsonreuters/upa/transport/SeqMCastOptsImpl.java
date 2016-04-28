package com.thomsonreuters.upa.transport;

class SeqMCastOptsImpl implements SeqMCastOpts
{
  
    @Override
    public void maxMsgSize(int size)
    {
       
    }
    
    @Override
    public int maxMsgSize()
    {
    	return TransportReturnCodes.FAILURE;
    }
    
    @Override
    public void instanceId(int id)
    {
       
    }
    
    @Override
    public int instanceId()
    {
    	return TransportReturnCodes.FAILURE;
    }
   
    @Override
    public String toString()
    {
        return null;
    }
}
