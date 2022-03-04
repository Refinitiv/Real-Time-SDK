/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

/** Stores a queue and pool of TimeRecord objects. This class along with
 * {@link TimeRecord} are used to collect individual time differences
 * for statistical calculation in a thread-safe manner -- one thread can
 * store information by adding to the records queue and another can retrieve
 * the information from the records queue and do any desired calculation. */
public class TimeRecordQueue 
{
	private ConcurrentLinkedQueue<TimeRecord> _pool; /* Reusable pool of TimeRecord objects. */
	private ConcurrentLinkedQueue<TimeRecord> _records;	/* Queue of submitted TimeRecord objects. */
	
	{
		_pool = new ConcurrentLinkedQueue<TimeRecord>();
		_records = new ConcurrentLinkedQueue<TimeRecord>();
		for (int i = 0; i < 1000; i++)
		{
			_pool.add(new TimeRecord());
		}
	}
	
	/**
	 *  Queue of submitted TimeRecord objects.
	 *
	 * @return the queue
	 */
	public Queue<TimeRecord> records()
	{
		return _records;
	}

	/**
	 *  Reusable pool of TimeRecord objects.
	 *
	 * @return the queue
	 */
	public Queue<TimeRecord> pool()
	{
		return _pool;
	}

	/**
	 *  Cleans up TimeRecordQueue.
	 */
	public void cleanup()
	{
		while (!_records.isEmpty())
		{
			_pool.add(_records.poll());
		}
	}
}
