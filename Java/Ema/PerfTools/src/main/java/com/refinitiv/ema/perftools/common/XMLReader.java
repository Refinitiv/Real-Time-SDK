/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

/**
 * Base class for utilities reading from XML files.
 */
public abstract class XMLReader 
{   
	protected XmlPullParser XPP;
  
	protected boolean _bStopRead;
    
	
	/**
	 * Parses xml message data file.
	 *
	 * @param filename the filename
	 * @return {@link PerfToolsReturnCodes}
	 */
	public int parseFile( String filename) 
	{
		if( filename == null )
		{
			System.err.println("XML filename is null");
			return PerfToolsReturnCodes.FAILURE;
		}
	
		try
		{
			XmlPullParserFactory factory = XmlPullParserFactory.newInstance(
					System.getProperty(XmlPullParserFactory.PROPERTY_NAME), null);
			factory.setNamespaceAware(true);
			XPP = factory.newPullParser();
			FileReader fr = null;
			fr = new FileReader( filename );
			XPP.setInput (fr);
			
			for (int eventType = XPP.getEventType(); eventType != XmlPullParser.END_DOCUMENT; eventType = XPP.next())
			{
				if( _bStopRead == true )
					break;
				
				if(eventType == XmlPullParser.START_TAG)
				{
					beginSectionRead();
				}
				else if(eventType == XmlPullParser.END_TAG)
				{
					endSectionRead();
				}
			}
		}
		catch (FileNotFoundException e)
		{
			System.err.println( "Unable to find file " + filename);
			return PerfToolsReturnCodes.FAILURE;
		}
	  	catch (Exception e)
	  	{
	  		System.err.println( e.getMessage() );
	  		return PerfToolsReturnCodes.FAILURE;
	  	}
	  
		return PerfToolsReturnCodes.SUCCESS;
	}

	abstract void beginSectionRead() throws IOException, XmlPullParserException;
	
	/*
	 * Method called by parse method when end of section tag reached.
	 */
	void endSectionRead() throws IOException, XmlPullParserException
	{
	}
	
	/**
	 * Gets the attribute count.
	 *
	 * @return Number of attributes in the xml file.
	 */
  	protected int getAttributeCount()
	{
  		if (XPP != null)
  			return XPP.getAttributeCount();
  		else
  			return 0;
	}
}   
