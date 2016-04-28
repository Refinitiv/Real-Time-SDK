package com.thomsonreuters.upa.perftools.upajconsperf;

import java.io.File;
import java.io.PrintWriter;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.perftools.common.TimeRecord;
import com.thomsonreuters.upa.perftools.common.TimeRecordQueue;
import com.thomsonreuters.upa.transport.Channel;

/** Contains information about a consumer thread and its connection. */
public class ConsumerThreadInfo
{
	private long				_threadId;					/* ID saved from thread creation. */
	private Channel				_channel;					/* UPA Channel. */
	private TimeRecordQueue		_latencyRecords;			/* Queue of timestamp information, collected periodically by the main thread. */
	private TimeRecordQueue		_postLatencyRecords;		/* Queue of timestamp information(for posts), collected periodically by the main thread. */
	private TimeRecordQueue		_genMsgLatencyRecords;		/* Queue of timestamp information(for generic messages), collected periodically by the main thread. */

	private int					_itemListUniqueIndex;		/* Index into the item list at which item
	 														 * requests unique to this consumer start. */
	private int					_itemListCount;				/* Number of item requests to make. */

	private DataDictionary		_dictionary;				/* Dictionary */

	private ConsumerStats		_stats;						/* Other stats, collected periodically by the main thread. */
	private File				_statsFile;					/* File for logging stats for this connection. */
	private PrintWriter			_statsFileWriter;			/* File writer for logging stats for this connection. */
	private File				_latencyLogFile;			/* File for logging latency for this connection. */
	private PrintWriter			_latencyLogFileWriter;		/* File writer for logging latency for this connection. */
	private volatile boolean	_shutdown;					/* Signals thread to shutdown. */
	private volatile boolean	_shutdownAck;				/* Acknowledges thread is shutdown. */
	
	{
		_latencyRecords = new TimeRecordQueue();
		_postLatencyRecords = new TimeRecordQueue();
		_genMsgLatencyRecords = new TimeRecordQueue();
		_stats = new ConsumerStats();
	}

	/** ID saved from thread creation. */
	public long threadId()
	{
		return _threadId;
	}
	
	/** ID saved from thread creation. */
	public void threadId(long id)
	{
		_threadId = id;
	}

	/** UPA Channel. */
	public Channel channel()
	{
		return _channel;
	}
    
	/** UPA Channel. */
	public void channel(Channel value)
	{
		_channel = value;
	}

    /** Queue of timestamp information, collected periodically by the main thread. */
	public TimeRecordQueue latencyRecords()
	{
		return _latencyRecords;
	}
	
	/** Queue of timestamp information(for posts), collected periodically by the main thread. */
	public TimeRecordQueue postLatencyRecords()
	{
		return _postLatencyRecords;
	}
	
	/** Queue of timestamp information(for generic messages), collected periodically by the main thread. */
	public TimeRecordQueue genMsgLatencyRecords()
	{
		return _genMsgLatencyRecords;
	}
	
	/** Index into the item list at which item 
	  * requests unique to this consumer start. */
	public int itemListUniqueIndex()
	{
		return _itemListUniqueIndex;
	}
	
	/** Index into the item list at which item 
	  * requests unique to this consumer start. */
	public void itemListUniqueIndex(int value)
	{
		_itemListUniqueIndex = value;
	}

	/** Number of item requests to make. */
	public int itemListCount()
	{
		return _itemListCount;
	}

	/** Number of item requests to make. */
	public void itemListCount(int value)
	{
		_itemListCount = value;
	}

	/** Dictionary */
	public DataDictionary dictionary()
	{
		return _dictionary;
	}

	/** Dictionary */
	public void dictionary(DataDictionary value)
	{
		_dictionary = value;
	}

	/** Other stats, collected periodically by the main thread. */
	public ConsumerStats stats()
	{
		return _stats;
	}

	/** File for logging stats for this connection. */
	public File statsFile()
	{
		return _statsFile;
	}
	
	/** File for logging stats for this connection. */
	public void statsFile(File value)
	{
		_statsFile = value;
	}

	/** File writer for logging stats for this connection. */
	public PrintWriter statsFileWriter()
	{
		return _statsFileWriter;
	}
	
	/** File writer for logging stats for this connection. */
	public void statsFileWriter(PrintWriter value)
	{
		_statsFileWriter = value;
	}

	/** File for logging latency for this connection. */
	public File latencyLogFile()
	{
		return _latencyLogFile;
	}
	
	/** File for logging latency for this connection. */
	public void latencyLogFile(File value)
	{
		_latencyLogFile = value;
	}

	/** File writer for logging latency for this connection. */
	public PrintWriter latencyLogFileWriter()
	{
		return _latencyLogFileWriter;
	}
	
	/** File writer for logging latency for this connection. */
	public void latencyLogFileWriter(PrintWriter value)
	{
		_latencyLogFileWriter = value;
	}

	/** Submit a time record */
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

	/** Signals thread to shutdown. */
	public boolean shutdown()
	{
		return _shutdown;
	}

	/** Signals thread to shutdown. */
	public void shutdown(boolean value)
	{
		_shutdown = value;
	}

	/** Acknowledges thread is shutdown. */
	public boolean shutdownAck()
	{
		return _shutdownAck;
	}

	/** Acknowledges thread is shutdown. */
	public void shutdownAck(boolean value)
	{
		_shutdownAck = value;
	}
	
	/** Clean up pools and files */
	public void cleanup()
	{
		latencyRecords().cleanup();
		postLatencyRecords().cleanup();
		genMsgLatencyRecords().cleanup();
		if (statsFileWriter() != null)
			statsFileWriter().close();
		if (latencyLogFileWriter() != null)
			latencyLogFileWriter().close();
	}
}
