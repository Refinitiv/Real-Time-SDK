package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgFlags;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

/* Connection recovery handler for the Reactor */
class ConnectionRecoveryHandler
{
	private final int NO_RECONNECT_LIMIT = -1;
    private ReactorConnectOptions _reactorConnectOptions;
    private InProgInfo _inProg = TransportFactory.createInProgInfo();
    private Error _error = TransportFactory.createError();
    private int _reconnectAttempts;
    private int _reconnectDelay;
    private long _nextRecoveryTime;
    private boolean _recoveryAttemptLimitReached;
    private int _listIndex;

	void reactorConnectOptions(ReactorConnectOptions reactorConnectOptions)
	{
		_reactorConnectOptions = reactorConnectOptions;
		_reconnectDelay = 0;
		_nextRecoveryTime = 0;
		if (reactorConnectOptions.reconnectAttemptLimit() == 0)
		{
		    _recoveryAttemptLimitReached = true;
		}
    }

	boolean recoverConnection(ReactorChannel reactorChannel, Selector selector)
	{
		int ret = TransportReturnCodes.FAILURE;
		boolean retVal = false;

		if (_recoveryAttemptLimitReached == false &&
			(_reactorConnectOptions.reconnectAttemptLimit() == NO_RECONNECT_LIMIT ||
			 _reconnectAttempts < _reactorConnectOptions.reconnectAttemptLimit()))
		{
			if (System.currentTimeMillis() >= _nextRecoveryTime)
			{
				System.out.println("Reactor attempting to recover connection...");
				_reconnectAttempts++;
				if (reactorChannel.channel() != null)
				{
				    reactorChannel.channel().close(_error);
				}
				// enable channel read/write locking for reactor since it's multi-threaded with worker thread
	            ConnectOptions connectOptions = _reactorConnectOptions.connectionList().get(_listIndex).connectOptions();
	            connectOptions.channelReadLocking(true);
	            connectOptions.channelWriteLocking(true);
	            // connect
				Channel channel = Transport.connect(connectOptions, _error);
				reactorChannel.selectableChannelFromChannel(channel);
	            reactorChannel.initializationTimeout(_reactorConnectOptions.connectionList().get(_listIndex).initTimeout());
	            if (reactorChannel.channel() != null)
	            {
    				while ((ret = reactorChannel.channel().init(_inProg, _error)) == TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
    				{
    	                if (_inProg.flags() == InProgFlags.SCKT_CHNL_CHANGE)
    	                {
    	                    // cancel old channel read select
    	                    try
    	                    {
    	                        SelectionKey key = _inProg.oldSelectableChannel().keyFor(selector);
    	                        if (key != null)
    	                        {
    	                            key.cancel();
    	                        }
    	                    }
    	                    catch (Exception e) {}

    	                    // add new channel read select
    	                    try
    	                    {
    	                        reactorChannel.channel().selectableChannel()
    	                        .register(selector, SelectionKey.OP_CONNECT|SelectionKey.OP_READ, reactorChannel);            
    	                    }
    	                    catch (Exception e)
    	                    {
    	                        _error.text(e.getMessage());
    	                        return retVal;
    	                    }
    	                  
    	                    // reset selectable channel on ReactorChannel to new one
    	                    reactorChannel.selectableChannelFromChannel(reactorChannel.channel());
    	                    
    	                    // set oldSelectableChannel on ReactorChannel
    	                    reactorChannel.oldSelectableChannel(_inProg.oldSelectableChannel());
    	                }
    	                else
    	                {
                            // break out of loop if initialization timeout reached
                            if (System.currentTimeMillis() > reactorChannel.initializationEndTimeMs())
                            {
                                break;
                            }
                            try
                            {
                                Thread.sleep(100);
                            }
                            catch (InterruptedException e) {}
    	                }
    				}
	            }
				
				if (ret == TransportReturnCodes.SUCCESS)
				{
					retVal = true;
					reset();
				}
				else
				{
				    System.out.println("\nRecovery attempt failed: " + _error.text() + "\n");
					calculateNextRecoveryTime();
					if (_reactorConnectOptions.reconnectAttemptLimit() != NO_RECONNECT_LIMIT &&
						_reconnectAttempts == _reactorConnectOptions.reconnectAttemptLimit())
					{
						System.out.println("Reactor Reconnect Attempt Limit Reached");
						_recoveryAttemptLimitReached = true;
					}
					if (++_listIndex == _reactorConnectOptions.connectionList().size())
					{
						_listIndex = 0;
					}
				}
			}
		}

		return retVal;
	}
	
	boolean recoveryAttemptLimitReached()
	{
		return _recoveryAttemptLimitReached;
	}
	
	private void calculateNextRecoveryTime()
	{
		if (_reconnectDelay < _reactorConnectOptions.reconnectMaxDelay())
		{
		    if (_reconnectDelay != 0)
		    {
		        _reconnectDelay *= 2;
		    }
		    else // set equal to reconnectMinDelay first time through
		    {
		        _reconnectDelay = _reactorConnectOptions.reconnectMinDelay();
		    }
		    
			if (_reconnectDelay > _reactorConnectOptions.reconnectMaxDelay())
			{
				_reconnectDelay = _reactorConnectOptions.reconnectMaxDelay();
			}
		}
		_nextRecoveryTime = System.currentTimeMillis() + _reconnectDelay;
	}

	private void reset()
	{
		_reconnectAttempts = 0;
		_reconnectDelay = 0;
		_nextRecoveryTime = 0;
		_listIndex = 0;
	}

	void clear()
	{
		_reconnectAttempts = 0;
		_reconnectDelay = 0;
		_nextRecoveryTime = 0;
		_reactorConnectOptions = null;
		_recoveryAttemptLimitReached = false;
		_listIndex = 0;
	}
}
