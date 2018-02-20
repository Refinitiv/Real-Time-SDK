package com.thomsonreuters.upa.perftools.upajtransportperf;

import java.util.ArrayList;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.upa.perftools.common.TimeRecordQueue;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.MCastStats;
import com.thomsonreuters.upa.transport.TransportFactory;

/** High level handler for multiple transport session threads.
  * Enables a transport thread to handle multiple channels. */
public class SessionHandler
{
	private ReentrantLock			_handlerLock;		/* Lock for the handler. */
	private ArrayList<Channel>		_newChannelsList;	/* New channels from the main thread are passed to the handler through this queue. */
	private int						_openChannelsCount;	/* Total number of channels this thread is currently handling. */
	private boolean					_active;			/* Connections are active, run the test */
	private TimeRecordQueue			_latencyRecords;	/* Latency records from the TransportThread. */
	private TransportThread			_transportThread;	/* Thread associated with this handler. */
	private int						_role;				/* Role of this handler. */
	private MCastStats				_prevMCastStats;	/* Stores any multicast statistics. */
	
	/**
	 * Initializes the handler for a transport perf thread.
	 */
	public void init()
	{
		_handlerLock = new ReentrantLock();
		_newChannelsList = new ArrayList<Channel>(10000);
		_active = false;
		_latencyRecords = new TimeRecordQueue();

		_role = TransportTestRole.READER | TransportTestRole.WRITER;
		_prevMCastStats = TransportFactory.createMCastStats();
	}
	
	/**
	 * Handler lock.
	 *
	 * @return Lock for the handler.
	 */
	public ReentrantLock handlerLock()
	{
		return _handlerLock;
	}
	
	/**
	 * New channels list.
	 *
	 * @return New channels accepted from the main thread.
	 */
	public ArrayList<Channel> newChannelsList()
	{
		return _newChannelsList;
	}
	
	/**
	 * Open channels count.
	 *
	 * @return Total number of channels this thread is currently handling.
	 */
	public int openChannelsCount()
	{
		return _openChannelsCount;
	}

	/**
	 * Set the total number of channels this thread is currently handling. 
	 *
	 * @param openChannelsCount the open channels count
	 */
	public void openChannelsCount(int openChannelsCount)
	{
		_openChannelsCount = openChannelsCount;
	}

	/**
	 * Active.
	 *
	 * @return true if connections are active.
	 */
	public boolean active()
	{
		return _active;
	}

	/**
	 * Active.
	 *
	 * @param active - connections are active.
	 */
	public void active(boolean active)
	{
		_active = active;
	}

	/**
	 * Latency records.
	 *
	 * @return Latency records. Updated from transport thread.
	 */
	public TimeRecordQueue latencyRecords()
	{
		return _latencyRecords;
	}

	/**
	 * Transport thread.
	 *
	 * @return Thread associated with this handler.
	 */
	public TransportThread transportThread()
	{
		return _transportThread;
	}

	/**
	 * 
	 * @param transportThread Thread associated with this handler.
	 */
	void transportThread(TransportThread transportThread)
	{
		_transportThread = transportThread;
	}

	/**
	 * Role.
	 *
	 * @return Role of this handler.
	 */
	public int role()
	{
		return _role;
	}

	/**
	 * Role.
	 *
	 * @param role  Role of this handler.
	 */
	public void role(int role)
	{
		_role = role;
	}

	/**
	 * Prev M cast stats.
	 *
	 * @return multicast statistics. Used for multicast connection.
	 */
	public MCastStats prevMCastStats()
	{
		return _prevMCastStats;
	}

	/**
	 * Prev M cast stats.
	 *
	 * @param prevMCastStats multicast statistics.
	 */
	public void prevMCastStats(MCastStats prevMCastStats)
	{
		_prevMCastStats = prevMCastStats;
	}
}
