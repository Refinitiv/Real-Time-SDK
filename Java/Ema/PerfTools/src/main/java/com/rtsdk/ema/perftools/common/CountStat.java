package com.rtsdk.ema.perftools.common;

/**
 * Keeps an ongoing count that can be used to get both total and periodic
 * counts. The count can also be collected by a different thread than the one
 * counting.
 */
public class CountStat
{
	private long _currentValue;	// The current value. 
	private long _prevValue;	// Value returned from the previous call to countStatGetChange. 
	
	/**
	 * Inits the.
	 */
	public void init()
    {
	    _currentValue = 0;
	    _prevValue = 0;
    }
	
    /**
     * Get the difference between the current count and that of the previous
     * call.
     *
     * @return the change
     */
	public long getChange()
	{
		long currentValue = _currentValue;
		
		currentValue -= _prevValue;
		_prevValue = _currentValue;
		
		return currentValue;
	}
	
    /**
     *  Add to the count.
     *
     * @param addend the addend
     */
	public void add(long addend)
	{
		_currentValue += addend;
	}
	
    /**
     *  Get the current overall count.
     *
     * @return the total
     */
	public long getTotal()
	{
		return _currentValue;
	}
	
    /** Increment the count. */
	public void increment()
	{
		++_currentValue;
	}
}
