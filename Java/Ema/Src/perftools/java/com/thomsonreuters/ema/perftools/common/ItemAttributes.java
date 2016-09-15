package com.thomsonreuters.ema.perftools.common;

/** Class containing the attributes that uniquely identify an item. */
public class ItemAttributes
{
	private int _domainType;   // Domain of this item
	private String _serviceName;
	private int _serviceId;
	private int _nameType;
	private String _name;
	
	public void clear()
	{
		_domainType = 0;
		_serviceId = 0;
		_nameType = 0;
	}
	
    /** Domain of this item */
	public int domainType()
	{
		return _domainType;
	}
	
    /** Domain of this item */
	public void domainType(int domainType)
	{
		_domainType = domainType;
	}
	
	 /** NameType of this item */
	public int nameType()
	{
		return _nameType;
	}
	
    /** NameType of this item */
	public void nameType(int nameType)
	{
		_nameType = nameType;
	}
	 /** Name of this item */
	public String name()
	{
		return _name;
	}
	
    /** Name of this item */
	public void name(String name)
	{
		_name = name;
	}
	
	 /** ServiceName of this item */
	public String serviceName()
	{
		return _serviceName;
	}
	
    /** ServiceName of this item */
	public void serviceName(String serviceName)
	{
		_serviceName = serviceName;
	}
	
	 /** Domain of this item */
	public int serviceId()
	{
		return _serviceId;
	}
	
    /** Domain of this item */
	public void serviceId(int serviceId)
	{
		_serviceId = serviceId;
	}
}
