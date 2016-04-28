package com.thomsonreuters.upa.transport;

public class JNIProtocol implements Protocol 
{

	@Override
	public Channel channel(ConnectOptions opts, Error error) {
		
		return null;
	}

	@Override
	public Server server(BindOptions opts, Error error) {
		
		return null;
	}

	@Override
	public Channel channel(AcceptOptions options, Server server, Object object) {
		
		return null;
	}

	@Override
	public void uninitialize() {
		
		
	}
}
