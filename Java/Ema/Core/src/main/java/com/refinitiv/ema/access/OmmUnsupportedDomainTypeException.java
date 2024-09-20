///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmUnsupportedDomainTypeException is thrown when a domain type value is greater than 255.
 *
 * @see OmmException
 */
public abstract class OmmUnsupportedDomainTypeException extends OmmException
{
	private static final long serialVersionUID = 2216879769516376466L;
	
	/**
	 * Returns the invalid DomainType. DomainTypes must be less than 256.
	 * 
	 * @return domain type value that caused the exception
	 */
	public abstract int domainType();
}