package com.thomsonreuters.upa.valueadd.examples.common;

/** Item argument class for the Value Add consumer and
 * non-interactive provider applications. */
public class ItemArg
{
	int domain; /* Domain of an item */
	String itemName; /* Name of an item */
	boolean enablePrivateStream; /* enable private stream for this item */
	boolean symbolListData; /* enable symbollist datastream */
	
	public ItemArg(int domain, String itemName, boolean enablePrivateStream)
	{
		this.domain = domain;
		this.itemName = itemName;
		this.enablePrivateStream = enablePrivateStream;
		this.symbolListData = false;
	}
	
	public ItemArg()
	{
	}

	public int domain()
	{
		return domain;
	}
	
	public void domain(int domain)
	{
		this.domain = domain;
	}
	
	public String itemName()
	{
		return itemName;
	}
	
	public void itemName(String itemName)
	{
		this.itemName = itemName;
	}
		
	public boolean enablePrivateStream()
	{
		return enablePrivateStream;
	}

	public void enablePrivateStream(boolean enablePrivateStream)
	{
		this.enablePrivateStream = enablePrivateStream;
	}
	
	public boolean symbolListData()
	{
		return symbolListData;
	}

	public void symbolListData(boolean symbolListData)
	{
		this.symbolListData = symbolListData;
	}	
}
