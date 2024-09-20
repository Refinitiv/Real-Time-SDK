/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import java.io.File;
import java.io.PrintWriter;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.transport.Channel;

/** Information associated with a ProviderThread. */
public class ProviderThreadInfo
{
	private long				_threadId;					/* ID saved from thread creation. */
	private Channel				_channel;					/* ETA Channel. */
	private TimeRecordQueue		_genMsgLatencyRecords;		/* Queue of timestamp information(for generic messages), collected periodically by the main thread. */

	private int					_itemListUniqueIndex;		/* Index into the item list at which item
	 														 * requests unique to this consumer start. */
	private int					_itemListCount;				/* Number of item requests to make. */

	private DataDictionary		_dictionary;				/* Dictionary */

	private ProviderThreadStats	_stats;						/* Other stats, collected periodically by the main thread. */
	private File				_statsFile;					/* File for logging stats for this connection. */
	private PrintWriter			_statsFileWriter;			/* File writer for logging stats for this connection. */
	private File				_latencyLogFile;			/* File for logging latency for this connection. */
	private PrintWriter			_latencyLogFileWriter;		/* File writer for logging latency for this connection. */
	private volatile boolean	_shutdown;					/* Signals thread to shutdown. */
	private volatile boolean	_shutdownAck;				/* Acknowledges thread is shutdown. */
	
	{
		_genMsgLatencyRecords = new TimeRecordQueue();
		_stats = new ProviderThreadStats();
	}

	/**
	 *  ID saved from thread creation.
	 *
	 * @return the long
	 */
	public long threadId()
	{
		return _threadId;
	}
	
	/**
	 *  ID saved from thread creation.
	 *
	 * @param id the id
	 */
	public void threadId(long id)
	{
		_threadId = id;
	}

	/**
	 *  ETA Channel.
	 *
	 * @return the channel
	 */
	public Channel channel()
	{
		return _channel;
	}
    
	/**
	 *  ETA Channel.
	 *
	 * @param value the value
	 */
	public void channel(Channel value)
	{
		_channel = value;
	}

	/**
	 *  Queue of timestamp information(for generic messages), collected periodically by the main thread.
	 *
	 * @return the time record queue
	 */
	public TimeRecordQueue genMsgLatencyRecords()
	{
		return _genMsgLatencyRecords;
	}
	
	/**
	 *  Index into the item list at which item 
	 * requests unique to this consumer start.
	 *
	 * @return the int
	 */
	public int itemListUniqueIndex()
	{
		return _itemListUniqueIndex;
	}
	
	/**
	 *  Index into the item list at which item 
	 * requests unique to this consumer start.
	 *
	 * @param value the value
	 */
	public void itemListUniqueIndex(int value)
	{
		_itemListUniqueIndex = value;
	}

	/**
	 *  Number of item requests to make.
	 *
	 * @return the int
	 */
	public int itemListCount()
	{
		return _itemListCount;
	}

	/**
	 *  Number of item requests to make.
	 *
	 * @param value the value
	 */
	public void itemListCount(int value)
	{
		_itemListCount = value;
	}

	/**
	 *  Dictionary.
	 *
	 * @return the data dictionary
	 */
	public DataDictionary dictionary()
	{
		return _dictionary;
	}

	/**
	 *  Dictionary.
	 *
	 * @param value the value
	 */
	public void dictionary(DataDictionary value)
	{
		_dictionary = value;
	}

	/**
	 *  Other stats, collected periodically by the main thread.
	 *
	 * @return the provider thread stats
	 */
	public ProviderThreadStats stats()
	{
		return _stats;
	}

	/**
	 *  File for logging stats for this connection.
	 *
	 * @return the file
	 */
	public File statsFile()
	{
		return _statsFile;
	}
	
	/**
	 *  File for logging stats for this connection.
	 *
	 * @param value the value
	 */
	public void statsFile(File value)
	{
		_statsFile = value;
	}

	/**
	 *  File writer for logging stats for this connection.
	 *
	 * @return the prints the writer
	 */
	public PrintWriter statsFileWriter()
	{
		return _statsFileWriter;
	}
	
	/**
	 *  File writer for logging stats for this connection.
	 *
	 * @param value the value
	 */
	public void statsFileWriter(PrintWriter value)
	{
		_statsFileWriter = value;
	}

	/**
	 *  File for logging latency for this connection.
	 *
	 * @return the file
	 */
	public File latencyLogFile()
	{
		return _latencyLogFile;
	}
	
	/**
	 *  File for logging latency for this connection.
	 *
	 * @param value the value
	 */
	public void latencyLogFile(File value)
	{
		_latencyLogFile = value;
	}

	/**
	 *  File writer for logging latency for this connection.
	 *
	 * @return the prints the writer
	 */
	public PrintWriter latencyLogFileWriter()
	{
		return _latencyLogFileWriter;
	}
	
	/**
	 *  File writer for logging latency for this connection.
	 *
	 * @param value the value
	 */
	public void latencyLogFileWriter(PrintWriter value)
	{
		_latencyLogFileWriter = value;
	}

	/**
	 *  Submit a time record.
	 *
	 * @param recordQueue the record queue
	 * @param startTime the start time
	 * @param endTime the end time
	 * @param ticks the ticks
	 * @return the int
	 */
	public int timeRecordSubmit(TimeRecordQueue recordQueue, long startTime, long endTime, long ticks)
	{
		TimeRecord record;

		record = recordQueue.pool().poll();
		if (record == null)
		{
			record = new TimeRecord();
		}
		
		record.ticks(ticks);
		record.startTime(startTime);
		record.endTime(endTime);
		
		recordQueue.records().add(record);

		return CodecReturnCodes.SUCCESS;
	}

	/**
	 *  Signals thread to shutdown.
	 *
	 * @return true, if successful
	 */
	public boolean shutdown()
	{
		return _shutdown;
	}

	/**
	 *  Signals thread to shutdown.
	 *
	 * @param value the value
	 */
	public void shutdown(boolean value)
	{
		_shutdown = value;
	}

	/**
	 *  Acknowledges thread is shutdown.
	 *
	 * @return true, if successful
	 */
	public boolean shutdownAck()
	{
		return _shutdownAck;
	}

	/**
	 *  Acknowledges thread is shutdown.
	 *
	 * @param value the value
	 */
	public void shutdownAck(boolean value)
	{
		_shutdownAck = value;
	}
	
	/**
	 *  Clean up pools and files.
	 */
	public void cleanup()
	{
		genMsgLatencyRecords().cleanup();
		if (statsFileWriter() != null)
			statsFileWriter().close();
		if (latencyLogFileWriter() != null)
			latencyLogFileWriter().close();
	}
}
