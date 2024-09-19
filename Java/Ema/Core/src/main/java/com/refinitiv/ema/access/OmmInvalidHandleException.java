///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmInvalidHandleException is thrown when application passes in an invalid handle to OmmConsumer.
 * <p>OmmConsumer uses long values, called handles to identify individual item streams.
 * <br>OmmConsumer validates each passed in handle against all open and known handles.</p>
 * 
 * @see OmmException
 */
public abstract class OmmInvalidHandleException extends OmmException
{
	private static final long serialVersionUID = -3093494896412175318L;

	/**
	 * Returns the invalid handle.
	 * 
	 * @return long value of handle causing exception
	 */
	public abstract long handle();
}