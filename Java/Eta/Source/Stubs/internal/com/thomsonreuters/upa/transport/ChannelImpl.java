package com.thomsonreuters.upa.transport;

import java.nio.channels.SelectableChannel;
import java.nio.channels.SocketChannel;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;

public class ChannelImpl implements Channel
{

	@Override
	public int info(ChannelInfo info, Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int ioctl(int code, Object value, Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int ioctl(int code, int value, Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int bufferUsage(Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int init(InProgInfo inProg, Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int close(Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public TransportBuffer read(ReadArgs readArgs, Error error) {
		
		return null;
	}

	@Override
	public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error) {
		
		return null;
	}

	@Override
	public int releaseBuffer(TransportBuffer buffer, Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int packBuffer(TransportBuffer buffer, Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int write(TransportBuffer buffer, WriteArgs writeArgs, Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int flush(Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int ping(Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int majorVersion() {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int minorVersion() {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int protocolType() {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int state() {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public SocketChannel scktChannel() {
		
		return null;
	}

	@Override
	public SocketChannel oldScktChannel() {
		
		return null;
	}

	@Override
	public SelectableChannel selectableChannel() {
		
		return null;
	}

	@Override
	public SelectableChannel oldSelectableChannel() {
		
		return null;
	}

	@Override
	public int pingTimeout() {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public Object userSpecObject() {
		
		return null;
	}

	@Override
	public boolean blocking() {
		
		return false;
	}

	@Override
	public int reconnectClient(Error error) {
		
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int connectionType() {
		
		return TransportReturnCodes.FAILURE;
	}

}
