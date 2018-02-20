///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2017. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.rdm;

/** 
 * Enumerations used to define Marketfeed field type
 *
*/

public class MfFieldTypes
{
	private MfFieldTypes()
	{
		throw new AssertionError();
	}
	
	public static final int NONE = -1;
	
	public static final int TIME_SECONDS = 0;
	
	public static final int INTEGER = 1;
	
	public static final int DATE = 3;
	
	public static final int PRICE = 4;
	
	public static final int ALPHANUMERIC = 5;
	
	public static final int ENUMERATED = 6;
	
	public static final int TIME = 7;
	
	public static final int BINARY = 8;
	
	public static final int LONG_ALPHANUMERIC	= 9;
			
	public static final int	OPAQUE				= 10;
}
