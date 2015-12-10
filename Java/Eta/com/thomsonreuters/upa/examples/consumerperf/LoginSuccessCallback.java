package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.transport.Channel;

public interface LoginSuccessCallback
{
	public void loginSucceeded(Channel chnl); 
}
