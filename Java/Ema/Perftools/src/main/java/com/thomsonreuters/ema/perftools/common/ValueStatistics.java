package com.thomsonreuters.ema.perftools.common;

/**
 * Class for calculating running statistics for a given value(such as
 * latency).
 */
public class ValueStatistics
{
	private long	_count;       // Total number of samples. 
	private double	_average;     // Current mean of samples.
	private double	_variance;    // Current variance of samples.
	private double	_maxValue;    // Highest sample value.
	private double	_minValue;    // Lowest sample value.

	double	_sum;                 // Used in calculating variance.
	double	_sumOfSquares;        // Used in calculating variance.
	
	/**
	 * Instantiates a new value statistics.
	 */
	public ValueStatistics()
	{
		_maxValue = -Double.MAX_VALUE;
		_minValue = Double.MAX_VALUE;
	}
	
	/** Clears ValueStatistics. */
	public void clear()
	{
		_count = 0;
		_average = 0;
		_variance = 0;
		_maxValue = -Double.MAX_VALUE;
		_minValue = Double.MAX_VALUE;
		_sum = 0;
		_sumOfSquares = 0;
	}
	
	/**
	 *  Recalculate stats based on new value.
	 *
	 * @param newValue the new value
	 */
	public void update(double newValue)
	{
		++_count;

		// Check against max & min
		if (newValue > _maxValue) _maxValue = newValue;
		if (newValue < _minValue) _minValue = newValue;

		/* Average and variance are calculated using online algorithms.
		 * - Average: http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average 
		 * - Variance: Devore,Farnum. "Applied Statistics for Engineers and Scientists(Second Edition)", p. 73,74 */

		_average = (newValue + _average * (_count-1))/_count;

		_sum += newValue;
		_sumOfSquares += newValue * newValue;
		_variance = _count > 1 ? 
			(_sumOfSquares - _sum * _sum / _count) / (_count - 1) : 0;
	}
	
	/**
	 *  Print a line containing all calculated statistics.
	 *
	 * @param valueStatsName the value stats name
	 * @param countUnitName the count unit name
	 * @param displayThousandths the display thousandths
	 */
	public void print(String valueStatsName, String countUnitName, boolean displayThousandths)
	{
		String outputStr = displayThousandths ? 
				   "%s: Avg:%8.3f StdDev:%8.3f Max:%8.3f Min:%8.3f, %s: %d\n"
				:  "%s: Avg:%6.1f StdDev:%6.1f Max:%6.1f Min:%6.1f, %s: %d\n";
		
		System.out.printf(outputStr,
				valueStatsName,
				_average, 
				Math.sqrt(_variance),
				_maxValue,
				_minValue,
				countUnitName,
				_count);
	}

	/**
	 *  Total number of samples.
	 *
	 * @return the long
	 */
	public long count()
	{
		return _count;
	}

	/**
	 *  Total number of samples.
	 *
	 * @param count the count
	 */
	public void count(long count)
	{
		_count = count;
	}

	/**
	 *  Current mean of samples.
	 *
	 * @return the double
	 */
	public double average()
	{
		return _average;
	}

	/**
	 *  Current mean of samples.
	 *
	 * @param average the average
	 */
	public void average(double average)
	{
		_average = average;
	}

	/**
	 *  Current variance of samples.
	 *
	 * @return the double
	 */
	public double variance()
	{
		return _variance;
	}

	/**
	 *  Current variance of samples.
	 *
	 * @param variance the variance
	 */
	public void variance(double variance)
	{
		_variance = variance;
	}

	/**
	 *  Highest sample value.
	 *
	 * @return the double
	 */
	public double maxValue()
	{
		return _maxValue;
	}

	/**
	 *  Highest sample value.
	 *
	 * @param maxValue the max value
	 */
	public void maxValue(double maxValue)
	{
		_maxValue = maxValue;
	}

	/**
	 *  Lowest sample value.
	 *
	 * @return the double
	 */
	public double minValue()
	{
		return _minValue;
	}

	/**
	 *  Lowest sample value.
	 *
	 * @param minValue the min value
	 */
	public void minValue(double minValue)
	{
		_minValue = minValue;
	}
}
