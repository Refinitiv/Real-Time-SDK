///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.List;

/**
 * This class defines a virtual service list name in order to send item requests and fail over to a list of concrete services.
 * This class is configured in {@link OmmConsumerConfig} for initializing an {@link OmmConsumer}. 
 *
 *@see OmmConsumerConfig
 */
public interface ServiceList
{		
	/*
	 * Returns the name of this service list
	 */
	String name();
	
	/**
	 * Sets the name of this service list
	 * @param name specifies the service list name 
	 */
	void name(String name);
	/*
	 *  A list of concrete services.
	 */
	List<String> concreteServiceList();
	
	/*
	 * Clears the ServiceList and sets all the defaults.
	 */
	void clear();
}
