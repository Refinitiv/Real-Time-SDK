package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.AcceptOptions;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.Server;

public interface Protocol 
{
	Channel channel(ConnectOptions opts, Error error);
	Server server(BindOptions opts, Error error);
	Channel channel(AcceptOptions options, Server server, Object object, Error error);
	void uninitialize();
}
