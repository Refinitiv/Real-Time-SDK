package com.thomsonreuters.upa.perftools.common;

/** Stores time information. This class along with {@link TimeRecordQueue}
 * are used to collect individual time differences for statistical calculation
 * in a thread-safe manner -- one thread can store information by adding to
 * the records queue and another can retrieve the information from the records
 * queue and do any desired calculation. */
public class TimeRecord
{
	private long _startTime;	/* Recorded start time. */
	private long _endTime;		/* Recorded end time. */
	private long _ticks;		/* Units per microsecond. */
	
	/** Recorded start time. */
	public long startTime()
	{
		return _startTime;
	}
	
	/** Recorded start time. */
	public void startTime(long startTime)
	{
		_startTime = startTime;
	}
	
	/** Recorded end time. */
	public long endTime()
	{
		return _endTime;
	}
	
	/** Recorded end time. */
	public void endTime(long endTime)
	{
		_endTime = endTime;
	}

	/** Units per microsecond. */
	public long ticks()
	{
		return _ticks;
	}

	/** Units per microsecond. */
	public void ticks(long ticks)
	{
		_ticks = ticks;
	}
}
