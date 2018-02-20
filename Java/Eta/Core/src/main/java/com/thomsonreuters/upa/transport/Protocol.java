package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.AcceptOptions;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.Server;

public interface Protocol 
{
	Channel channel(ConnectOptions opts, Error error);
	Server server(BindOptions opts, Error error);
	Channel channel(AcceptOptions options, Server server, Object object);
	void uninitialize();
}
