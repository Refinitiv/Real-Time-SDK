package com.refinitiv.eta.perftools.common;

import java.util.Random;

/**
 * Generates a randomized array that can be used to determine which message in a
 * burst should contain latency information.
 * <p>
 * Create the array using the {@link #create(LatencyRandomArrayOptions)} method.
 * Iterate over the array using the {@link #next()} method.
 */
public class LatencyRandomArray
{
	private int[] _array;				// The array of values.
	private int _valueCount;			// Total number of values in the array.
	private int _randArrayIndex = 0;	// Index for the randomized latency array.
	private Random _generator = new Random(System.currentTimeMillis());

	/**
	 * Creates a LatencyRandomArray. 
	 * 
	 * @param opts options for generating latency array.
	 *  
	 * @return &lt; 0 if latency array options are invalid, 0 otherwise. 
	 */
	public int create(LatencyRandomArrayOptions opts)
	{
		if(opts.totalMsgsPerSec() == 0)
		{
			System.err.printf("Random Array: Total message rate is zero.\n");
			return PerfToolsReturnCodes.FAILURE;
		}

		if(opts.latencyMsgsPerSec() == 0)
		{
			System.err.printf("Random Array: Latency message rate is zero.\n");
			return PerfToolsReturnCodes.FAILURE;
		}

		if(opts.latencyMsgsPerSec() > opts.totalMsgsPerSec())
		{
			System.err.printf("Random Array: Latency message rate is greater than total message rate.\n");
			return PerfToolsReturnCodes.FAILURE;
		}

		if(opts.arrayCount() == 0)
		{
			System.err.printf("Random Array: Array count is zero.\n");
			return PerfToolsReturnCodes.FAILURE;
		}

		int totalMsgsPerTick = opts.totalMsgsPerSec() / opts.ticksPerSec();
		int totalMsgsPerTickRemainder = opts.totalMsgsPerSec() % opts.ticksPerSec();

		_valueCount = opts.ticksPerSec() * opts.arrayCount();
		_array = new int[_valueCount];

		// Build random array.
        // The objective is to create an array that will be used for each second.
        // It will contain one value for each tick, which indicates which message
        // during the tick should contain latency information. 
		for (int setPos = 0; setPos < _valueCount; setPos += opts.ticksPerSec())
		{
			// Each array cell represents one tick.
            // Fill the array '1'with as many latency messages as we will send per second. 
			for(int i = 0; i < opts.latencyMsgsPerSec(); ++i)
				_array[setPos + i] = 1;

			// Fill the rest with -1
			for(int i = opts.latencyMsgsPerSec(); i < opts.ticksPerSec(); ++i)
				_array[setPos + i] = -1;

			// Shuffle array to randomize which ticks send latency message
			for (int i = 0; i < opts.ticksPerSec(); ++i)
			{
				int pos1 = Math.abs(_generator.nextInt() % opts.ticksPerSec());
				int pos2 = Math.abs(_generator.nextInt() % opts.ticksPerSec());
				int tmpB = _array[setPos + pos1];
				_array[setPos + pos1] = _array[setPos + pos2];
				_array[setPos + pos2] = tmpB;
			}

			// Now, for each tick that sends a latency message, determine which message that will be 
			for (int i = 0; i < opts.ticksPerSec(); ++i)
				if (_array[setPos + i] == 1)
					_array[setPos + i] = Math.abs(_generator.nextInt() % totalMsgsPerTick + ((i < totalMsgsPerTickRemainder) ? 1 : 0));
		}

		return PerfToolsReturnCodes.SUCCESS;
	}
	
	
    /**
     * Iterate over the LatencyRandomArray. The value returned indicates which
     * message in the tick should contain latency information. If the value is
     * -1, no latency message should be sent in that tick. The iteration starts
     * over when the end of the array is reached.
     * 
     * @return value indicating which message in the tick contains latency information
     */
	public int next()
	{
		_randArrayIndex++;
		if (_randArrayIndex == _valueCount)
		{
			_randArrayIndex = 0;
		}		
		return _array[_randArrayIndex];
	}

	/**
	 * Total number of values in the array.
	 * 
	 * @return Total number of values in the array
	 */
	public int valueCount()
	{
		return _valueCount;
	}

	/**
	 * The array of values.
	 * 
	 * @return The array of values
	 */
	public int[] array()
	{
		return _array;
	}
}
