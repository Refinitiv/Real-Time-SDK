package com.refinitiv.eta.perftools.common;

/**
 * Market price item data. Allows threads to uniquely retrieve message data
 * information from cached XML file contents.
 */
public class MarketPriceItem
{
	private int _iMsg; // item data for message

    /**
     *  Item data for message.
     *
     * @return the int
     */
	public int iMsg()
	{
		return _iMsg;
	}

	/**
	 *  Item data for message.
	 *
	 * @param iMsg the i msg
	 */
	public void iMsg(int iMsg)
	{
		_iMsg = iMsg;
	}
}
