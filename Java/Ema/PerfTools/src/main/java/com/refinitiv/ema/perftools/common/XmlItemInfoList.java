/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.io.IOException;

import org.xmlpull.v1.XmlPullParserException;
import com.refinitiv.ema.rdm.EmaRdm;;

/** Builds a list of items from an XML file. This may be used by
 * applications to determine what items to open when starting a session. */
public class XmlItemInfoList
{
	class XmlItemInfoListReader  extends XMLReader
	{
		/** XML Pull Parser beginSectionRead(). */
		void beginSectionRead() throws IOException, XmlPullParserException
		{
			String tag = XPP.getName();
			
			if( tag.equals("item"))
	    	{
	     		if(getAttributeCount() == 0)
	     			return;

	     		XmlItemInfo itemInfo = new XmlItemInfo();
	     		_itemInfoList[_currentItemCount] = itemInfo;
	 			_currentItemCount++; 			
	 			
	     		for (int i = 0; i < getAttributeCount(); i++)
	     		{
	    			String ntag = XPP.getAttributeName(i);
	    			String nvalue = XPP.getAttributeValue(i);
	    			
	    			if (ntag.equals("domain"))
	    			{
	    				itemInfo.domainType(domainTypeValue(nvalue));
	    			}
	    			else if (ntag.equals("name"))
	    			{
	    				itemInfo.name(nvalue);
	    			}
	    			else if (ntag.equals("post"))
	    			{
	    				if (nvalue.equalsIgnoreCase("true"))
	    				{
	    					itemInfo.isPost(true);
	    					_postItemCount++;
	    				}
	    			}
	    			else if (ntag.equals("generic"))
	    			{
	    				if (nvalue.equalsIgnoreCase("true"))
	    				{
	    					itemInfo.isGenMsg(true);
	    					_genMsgItemCount++;
	    				}
	    			}
	    			else if (ntag.equals("snapshot"))
	    			{
	    				if (nvalue.equalsIgnoreCase("true"))
	    				{
	    					itemInfo.isSnapshot(true);
	    				}
	    			}
	     		}
	    	}
	    	
	    	if(_currentItemCount == _itemInfoCount)
	    	{
	    		_bStopRead = true;
	    	}
		}
		
		/* Converts domain type string to domain type value. */
	    private int domainTypeValue(String domainTypeString)
	    {
	        int ret = 0;

	        if (domainTypeString.equals("MarketPrice"))
	        {
	            ret = EmaRdm.MMT_MARKET_PRICE;
	        }
	        else if (domainTypeString.equals("MarketByOrder"))
	        {
	            ret = EmaRdm.MMT_MARKET_BY_ORDER;
	        }
	        else if (domainTypeString.equals("MarketByPrice"))
	        {
	            ret = EmaRdm.MMT_MARKET_BY_PRICE;
	        }

	        return ret;
	    }
	}
	
	private int					_itemInfoCount;	/* The total number of items in the list */
	private int					_postItemCount;	/* Number of items in list for posting. */
	private int					_genMsgItemCount;	/* Number of items in list for sending generic msgs. */
	private XmlItemInfo[]		_itemInfoList;	/* The list of items */
	
	private int _currentItemCount; /* current item count of list */
	
	private XmlItemInfoListReader _xmlReader = new XmlItemInfoListReader();
	
	/**
	 * Instantiates a new xml item info list.
	 *
	 * @param itemCount the item count
	 */
	public XmlItemInfoList(int itemCount)
	{
		_currentItemCount = 0;
		_itemInfoCount = itemCount;
		_itemInfoList = new XmlItemInfo[_itemInfoCount];
	}

	/**
	 * Parses xml message data file.
	 *
	 * @param filename the filename
	 * @return {@link PerfToolsReturnCodes}
	 */
	public int parseFile( String filename) 
	{
		if (_xmlReader.parseFile(filename) != PerfToolsReturnCodes.SUCCESS)
			return PerfToolsReturnCodes.FAILURE;
		
		// check if the xml file contains the number of requested items
		if (_currentItemCount < _itemInfoCount)
		{
    		System.out.printf("Error: Item file contained %d items, but consumer wants %d items.\n", _currentItemCount, 
    				_itemInfoCount);
			return PerfToolsReturnCodes.FAILURE;
		}
		
		return PerfToolsReturnCodes.SUCCESS;
	}

	/**
	 *  The total number of items in the list.
	 *
	 * @return the int
	 */
	public int itemInfoCount()
	{
		return _itemInfoCount;
	}

	/**
	 *  Number of items in list for posting.
	 *
	 * @return the int
	 */
	public int postItemCount()
	{
		return _postItemCount;
	}

	/**
	 *  Number of items in list for sending generic msgs.
	 *
	 * @return the int
	 */
	public int genMsgItemCount()
	{
		return _genMsgItemCount;
	}
	
	/**
	 *  The list of items.
	 *
	 * @return the xml item info[]
	 */
	public XmlItemInfo[] itemInfoList()
	{
		return _itemInfoList;
	}
		
}
