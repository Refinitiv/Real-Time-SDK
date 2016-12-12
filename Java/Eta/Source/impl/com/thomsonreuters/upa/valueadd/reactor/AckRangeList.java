package com.thomsonreuters.upa.valueadd.reactor;

/* List of integer pairs. Used to store positive/negative acknowledge ranges. */

class AckRangeList
{
	private int _count;
	private int _iter;
	private int[] _rangeArray;

	public static final int MAX_RANGES = 255;

	public AckRangeList()
	{
		_count = 0;
		_iter = 0;
		_rangeArray = new int[MAX_RANGES * 2];
	}

	/* Returns the number of ranges (sequence number pairs) -- array length is twice this. */
	public int count()
	{
		return _count;
	}
	
	/* Set the number of ranges (sequence number pairs). Resets the iterator. */
	public void count(int count)
	{
		_count = count;
		_iter = 0;
	}

	/* Returns the sequence number pair array. */
	public int[] rangeArray()
	{
		return _rangeArray;
	}

	/* Returns the next sequence number in the sequence number pair array. */
	public int getNextSeqNum()
	{
		assert _iter < _count;
		return _rangeArray[_iter];
	}

	/* Moves to the next sequence number in the sequence number pair array. */
	public void updateToNextSeqNum()
	{
		assert _iter < _count;
		
		/* Ranges are stored in pairs of sequence numbers.
		 * Increment the lower number each time until it matches the
		 * higher number, then move to the next range. */
		if (TunnelStreamUtil.seqNumCompare(_rangeArray[_iter], _rangeArray[_iter + 1]) < 0)
			++_rangeArray[0];
		else
		{
			_iter += 2;
			if (_iter == _count * 2)
			{
				_iter = _count = 0;
			}
		}
	}

}
