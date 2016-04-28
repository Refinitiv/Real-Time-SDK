package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.TcpOpts;

// This impl class contains the "replay" property, which may be used during
// debugging to specify a NetworkReplay input file. It must be temporarily
// made public (but not checked-in) to be accessible in "Consumer or
// Consumer Perf" code
class TcpOptsImpl implements TcpOpts
{
    
	@Override
	public String toString()
	{
		return null;
	}

    @Override
    public void tcpNoDelay(boolean tcpNoDelay)
    {
        
    }

    @Override
    public boolean tcpNoDelay()
    {
        return false;
    }

 
}
