/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

/** Class containing the attributes that uniquely identify an item. */
public class ItemAttributes
{
	private int _domainType;   // Domain of this item
	private String _serviceName;
	private int _serviceId;
	private int _nameType;
	private String _name;
	private boolean _nameTypeSet;

	/**
	 * Clear.
	 */
	public void clear()
	{
		_domainType = 0;
		_serviceId = 0;
		_nameType = 1;
		_serviceName = null;
		_name = null;
		_nameTypeSet = false;
	}
	
    /**
     *  Domain of this item.
     *
     * @return the int
     */
	public int domainType()
	{
		return _domainType;
	}
	
    /**
     *  Domain of this item.
     *
     * @param domainType the domain type
     */
	public void domainType(int domainType)
	{
		_domainType = domainType;
	}
	
	 /**
 	 *  NameType of this item.
 	 *
 	 * @return the int
 	 */
	public int nameType()
	{
		return _nameType;
	}
	
    /**
     *  NameType of this item.
     *
     * @param nameType the name type
     */
	public void nameType(int nameType)
	{
		_nameType = nameType;
		_nameTypeSet = true;
	}
	
	/**
	 *  Checks whether the name type is set for this item.
	 * @return True if the name type is set; otherwise false
	 */
	public boolean hasNameType()
	{
		return _nameTypeSet;
	}
	 
 	/**
 	 *  Name of this item.
 	 *
 	 * @return the string
 	 */
	public String name()
	{
		return _name;
	}
	
    /**
     *  Name of this item.
     *
     * @param name the name
     */
	public void name(String name)
	{
		_name = name;
	}
	
	 /**
 	 *  ServiceName of this item.
 	 *
 	 * @return the string
 	 */
	public String serviceName()
	{
		return _serviceName;
	}
	
    /**
     *  ServiceName of this item.
     *
     * @param serviceName the service name
     */
	public void serviceName(String serviceName)
	{
		_serviceName = serviceName;
	}
	
	 /**
 	 *  Domain of this item.
 	 *
 	 * @return the int
 	 */
	public int serviceId()
	{
		return _serviceId;
	}
	
    /**
     *  Domain of this item.
     *
     * @param serviceId the service id
     */
	public void serviceId(int serviceId)
	{
		_serviceId = serviceId;
	}
}
