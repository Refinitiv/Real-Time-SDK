package com.rtsdk.eta.transport;

import com.rtsdk.eta.transport.AcceptOptions;
import com.rtsdk.eta.transport.BindOptions;
import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.transport.ConnectOptions;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.Server;

public interface Protocol 
{
	Channel channel(ConnectOptions opts, Error error);
	Server server(BindOptions opts, Error error);
	Channel channel(AcceptOptions options, Server server, Object object);
	void uninitialize();
}
