///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.channels.*;
import java.util.Iterator;
import java.util.Set;

import com.thomsonreuters.upa.codec.*;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.*;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.*;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.*;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;

class ChannelSession
{
	private Channel _channel;
	private int _lastReadRet;
	private Msg _msg = CodecFactory.createMsg();
	private LoginMsg _loginMsg = LoginMsgFactory.createMsg();
	private DirectoryMsg _directoryMsg = DirectoryMsgFactory.createMsg();
	private DecodeIterator _dIter = CodecFactory.createDecodeIterator();
	private Error _error = TransportFactory.createError();
	WriteArgs _writeArgs = TransportFactory.createWriteArgs();
	ReadArgs _readArgs = TransportFactory.createReadArgs();

	private Selector _selector;
	private static final int SELECT_TIMEOUT = 1000;

	ChannelSession(Channel channel)
	{
		_channel = channel;

        try
        {
            _selector = Selector.open();
			_channel.selectableChannel().register(_selector, SelectionKey.OP_READ, _channel);
        }
        catch (IOException e)
        {
            e.printStackTrace();
            fail();
        }
	}

	Msg readMsg()
	{
		TransportBuffer readBuffer = read();
		if (readBuffer == null)
			return null;

		_dIter.clear();
		_dIter.setBufferAndRWFVersion(readBuffer, _channel.majorVersion(), _channel.minorVersion());
		assertEquals(CodecReturnCodes.SUCCESS, _msg.decode(_dIter));
		return _msg;
	}

	@SuppressWarnings("deprecation")
    LoginMsg readLoginMsg()
	{
		Msg msg;
		assertNotNull(msg = readMsg());
		assertEquals(DomainTypes.LOGIN, msg.domainType());
		switch (msg.msgClass())
		{
			case MsgClasses.REQUEST:
				_loginMsg.rdmMsgType(LoginMsgType.REQUEST);
				break;
			case MsgClasses.REFRESH:
				_loginMsg.rdmMsgType(LoginMsgType.REFRESH);
				break;
			case MsgClasses.STATUS:
				_loginMsg.rdmMsgType(LoginMsgType.STATUS);
				break;
			case MsgClasses.CLOSE:
				_loginMsg.rdmMsgType(LoginMsgType.CLOSE);
				break;
			case MsgClasses.GENERIC:
				_loginMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
				break;
			case MsgClasses.POST:
				_loginMsg.rdmMsgType(LoginMsgType.POST);
				break;
			case MsgClasses.ACK:
				_loginMsg.rdmMsgType(LoginMsgType.ACK);
				break;
		}

		assertEquals(CodecReturnCodes.SUCCESS, _loginMsg.decode(_dIter, msg));
		return _loginMsg;
	}

	DirectoryMsg readDirectoryMsg()
	{
		Msg msg = readMsg();
		assertEquals(DomainTypes.SOURCE, msg.domainType());

        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                _directoryMsg.rdmMsgType(DirectoryMsgType.REQUEST);
                break;
            case MsgClasses.REFRESH:
                _directoryMsg.rdmMsgType(DirectoryMsgType.REFRESH);
                break;
            case MsgClasses.STATUS:
                _directoryMsg.rdmMsgType(DirectoryMsgType.STATUS);
                break;
            case MsgClasses.CLOSE:
                _directoryMsg.rdmMsgType(DirectoryMsgType.CLOSE);
                break;
            case MsgClasses.GENERIC:
                _directoryMsg.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
                break;
            case MsgClasses.UPDATE:
                _directoryMsg.rdmMsgType(DirectoryMsgType.UPDATE);
                break;
        }

		assertEquals(CodecReturnCodes.SUCCESS, _directoryMsg.decode(_dIter, msg));
		return _directoryMsg;
	}

	TransportBuffer read()
	{
        int selRet = 0;
        
        if (_lastReadRet == 0)
        {
       
			// make sure the read notifier has triggered before calling read()
			try
			{
				selRet = _selector.select(SELECT_TIMEOUT);
			}
			catch (IOException e)
			{
				e.printStackTrace();
				fail();
			}
			if (selRet == 0) return null;

			assertTrue("Select error", selRet >= 0);

			Set<SelectionKey> keySet = _selector.selectedKeys();
			Iterator<SelectionKey> iter = keySet.iterator();
			if (iter.hasNext())
			{
				SelectionKey key = iter.next();
				assertEquals(_channel, key.attachment());

				iter.remove();

				/* Channel has data to read. */
				if (!key.isReadable())
					return null;
			}
		}
        
		TransportBuffer msgBuf;
		do
		{
			_readArgs.clear();
			msgBuf = _channel.read(_readArgs, _error);
			_lastReadRet = _readArgs.readRetVal();

			if (msgBuf == null)
			{
				switch(_lastReadRet)
				{
					case TransportReturnCodes.SUCCESS:
						break;
					case TransportReturnCodes.READ_PING:
						continue;
					default:
						fail("Read failed:" + TransportReturnCodes.toString(_lastReadRet));
				}
			}
		} while (msgBuf == null);
                

		return msgBuf;
                
	}

	TransportBuffer getBuffer(int length, boolean packed, Error error)
	{
		return _channel.getBuffer(length, packed, error);
	}

	void write(TransportBuffer writeBuffer)
	{
		_writeArgs.clear();

		_writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		assertEquals(TransportReturnCodes.SUCCESS, _channel.write(writeBuffer, _writeArgs, _error));
	}

	public Channel channel() { return _channel; }
}
