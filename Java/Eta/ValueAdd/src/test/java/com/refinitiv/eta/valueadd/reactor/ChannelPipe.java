/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.channels.*;
import java.util.Iterator;
import java.util.Set;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.*;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;

class ChannelPipe
{
	/* Set to true to enable tracing */
	private static final boolean ENABLE_TRACE = false;

	private Selector _selector;
    private Error _error = TransportFactory.createError();
    private BindOptions _bindOpts = TransportFactory.createBindOptions();
    private static final int SELECT_TIMEOUT = 500;
	private Server _server;
	private Channel _pipeToClientChannel, _pipeToServerChannel;
	private static int _pipePort = 14010;
	private ChannelSession _pipeToClientChannelSession, _pipeToServerChannelSession;
	private DecodeIterator _dIter = CodecFactory.createDecodeIterator();
	private Msg _msg = CodecFactory.createMsg();
	
	public ChannelPipe()
	{
        try
        {
            _selector = Selector.open();
        }
        catch (IOException e)
        {
            e.printStackTrace();
            fail();
        }
	}

    /** Returns the port number of the channel pipe's server. */
    public int serverPort()
    {
        return _server.portNumber();
    }

    /** Returns the pipe's client-facing channel. */
    public Channel clientFacingChannel()
    {
        return _pipeToClientChannelSession.channel();
    }

    /** Returns the pipe's client-facing channel. */
    public Channel serverFacingChannel()
    {
        return _pipeToServerChannelSession.channel();
    }

    /** Binds the server used to connect a client to the pipe. */
    public void bind()
    {
        _bindOpts.clear(); 
        _bindOpts.serviceName(String.valueOf(_pipePort));
        _bindOpts.majorVersion(Codec.majorVersion());
        _bindOpts.minorVersion(Codec.minorVersion());
        _bindOpts.protocolType(Codec.protocolType());
        ++_pipePort;
        _server = Transport.bind(_bindOpts, _error);
        assertNotNull("Bind failed:" + _error.text(), _server);
    }

	/** Create a connection from the client, through the pipe, to the intended server.
     * This method assumes that a ReactorChannel is currently attempting to connect to this pipe. */
	void connect(String serverHost, String serverPort)
	{
        ConnectOptions connectOpts = TransportFactory.createConnectOptions();
		AcceptOptions acceptOpts = TransportFactory.createAcceptOptions();
		InProgInfo inProgInfo = TransportFactory.createInProgInfo();
		int selRet;
		int ret;

        try
		{
			++_pipePort;

			/* Accept client-to-pipe connection */
			_server.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT);

			// make sure the connect has triggered the bind socket before calling accept();
			selRet = _selector.select(SELECT_TIMEOUT);
			assertTrue("No accept notification", selRet > 0);
			Set<SelectionKey> keySet = _selector.selectedKeys();
			Iterator<SelectionKey> iter = keySet.iterator();
			while (iter.hasNext())
			{
				SelectionKey key = iter.next();
				iter.remove();
				if(!key.isValid())
					continue;
				if (key.isAcceptable())
				{
					acceptOpts.clear();

					if ((_pipeToClientChannel = _server.accept(acceptOpts, _error)) == null)
						fail("Pipe-to-client channel accept failed: " + TransportReturnCodes.toString(_error.errorId()) + "(" + _error.text() + ")");
				}
			}

			/* Initialize channel. */
			_pipeToClientChannel.selectableChannel().register(_selector, SelectionKey.OP_READ, _pipeToClientChannel);

			while (_pipeToClientChannel.state() != ChannelState.ACTIVE)
			{
				if (_pipeToClientChannel.state() != ChannelState.ACTIVE)
				{
					ret = _pipeToClientChannel.init(inProgInfo, _error);
					if (ret != TransportReturnCodes.SUCCESS && ret != TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
						fail("Pipe-to-client channel init failed: " + TransportReturnCodes.toString(_error.errorId()) + "(" + _error.text() + ")");
				}
			}

			_pipeToClientChannel.selectableChannel().register(_selector, SelectionKey.OP_READ, _pipeToClientChannel);

			/* Create pipe-to-server connection */
			connectOpts.clear();
			connectOpts.connectionType(ConnectionTypes.SOCKET);
			connectOpts.majorVersion(Codec.majorVersion());
			connectOpts.minorVersion(Codec.minorVersion());
			connectOpts.protocolType(Codec.protocolType());
			connectOpts.unifiedNetworkInfo().address(serverHost);
			connectOpts.unifiedNetworkInfo().serviceName(serverPort);

			if ((_pipeToServerChannel = Transport.connect(connectOpts, _error)) == null)
				fail("Pipe-to-server channel connect failed, check server connection config: " + TransportReturnCodes.toString(_error.errorId()) + "(" + _error.text() + ")");

			_pipeToServerChannel.selectableChannel().register(_selector, SelectionKey.OP_READ, _pipeToClientChannel);

			while (_pipeToServerChannel.state() != ChannelState.ACTIVE)
			{
				ret = _pipeToServerChannel.init(inProgInfo, _error);
				if (ret != TransportReturnCodes.SUCCESS && ret != TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
					fail("Pipe-to-server channel init failed, check server connection config: " + TransportReturnCodes.toString(_error.errorId()) + "(" + _error.text() + ")");
			}

			_pipeToServerChannel.selectableChannel().keyFor(_selector).cancel();
			_pipeToClientChannel.selectableChannel().keyFor(_selector).cancel();
		}
        catch (IOException e)
        {
            e.printStackTrace();
            fail();
        }

		_pipeToClientChannelSession = new ChannelSession(_pipeToClientChannel);
		_pipeToServerChannelSession = new ChannelSession(_pipeToServerChannel);
    }

    /** Disconnects from the client and server sides. */
	void disconnect()
	{
		if (_pipeToServerChannel != null)
		{
			assertEquals(TransportReturnCodes.SUCCESS, _pipeToServerChannel.close(_error));
			_pipeToServerChannel = null;
		}
		_pipeToServerChannelSession = null;

		if (_pipeToClientChannel != null)
		{
			assertEquals(TransportReturnCodes.SUCCESS, _pipeToClientChannel.close(_error));
			_pipeToClientChannel = null;
		}
		_pipeToClientChannelSession = null;
	}

	/** Read from pipe's client channel. */
	TransportBuffer readFromClient()
	{
		if (ENABLE_TRACE)
			System.out.println("<!-- Pipe reading from client -->");

		return read(_pipeToClientChannelSession);
	}

	/** Read from pipe's server channel. */
	TransportBuffer readFromServer()
	{
		if (ENABLE_TRACE)
			System.out.println("<!-- Pipe reading from server -->");

		return read(_pipeToServerChannelSession);

	}

    /** Forward the contents of this buffer to the client channel. */
	void forwardToClient(TransportBuffer readBuffer)
	{
		if (ENABLE_TRACE)
			System.out.println("<!-- Pipe forwarding to client -->");

		forward(_pipeToClientChannelSession, readBuffer);
	}

    /** Forward the contents of this buffer to the server channel. */
	void forwardToServer(TransportBuffer readBuffer)
	{
		if (ENABLE_TRACE)
			System.out.println("<!-- Pipe forwarding to server -->");

		forward(_pipeToServerChannelSession, readBuffer);
	}


	/** Read from client and forward any messages to server. */
	void readAndForwardToServer()
	{
		TransportBuffer readBuffer = readFromClient();
		if (readBuffer != null)
			forwardToServer(readBuffer);
	}

	/** Read from server and forward any messages to client. */
	void readAndForwardFromServer()
	{
		TransportBuffer readBuffer = readFromServer();
		if (readBuffer != null)
			forwardToClient(readBuffer);
	}

    /** Read a buffer from the channel. */
	private TransportBuffer read(ChannelSession fromChannel)
	{
		TransportBuffer readBuffer;

		if ((readBuffer = fromChannel.read()) != null)
		{
			if (ENABLE_TRACE)
				traceBufferToXml(fromChannel, readBuffer);
		}

		return readBuffer;
	}

    /** Forward a buffer to the specified channel. */
	private void forward(ChannelSession toChannel, TransportBuffer readBuffer)
	{
		TransportBuffer writeBuffer = toChannel.getBuffer(readBuffer.length(), false, _error);
		assertNotNull(writeBuffer);
		readBuffer.copy(writeBuffer.data());
		writeBuffer.data().position(writeBuffer.data().position() + readBuffer.length());

		toChannel.write(writeBuffer);
	}

    /** Dump XML trace of a buffer. */
	private void traceBufferToXml(ChannelSession channel, TransportBuffer readBuffer)
	{
		_dIter.clear();
		_dIter.setBufferAndRWFVersion(readBuffer, channel.channel().majorVersion(), channel.channel().minorVersion());
		System.out.println(_msg.decodeToXml(_dIter));
	}

}
