///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


/**
 * SeriesEntry represents an entry of Series.
 * <br>SeriesEntry associates entry's data and its data type.
 */
public interface SeriesEntry
{
	/**
	 * Returns the DataType of the entry's load. 
	 * <br>Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#ERROR}
	 * signifies error while extracting content of load.
	 * 
	 * @return data type of the contained object
	 */
	public int loadType();
	
	/**
	 * Returns a string representation of the class instance.
	 * 
	 * @return string representation of the class instance
	 */
	public String toString();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.ReqMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.ReqMsg} class reference to contained object
	 */
	public ReqMsg reqMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.RefreshMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.RefreshMsg} class reference to contained entry's load object
	 */
	public RefreshMsg refreshMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.UpdateMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.UpdateMsg} class reference to contained entry's load object
	 */
	public UpdateMsg updateMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.StatusMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.StatusMsg} class reference to contained entry's load object
	 */
	public StatusMsg statusMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.PostMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.PostMsg} class reference to contained entry's load object
	 */
	public PostMsg postMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.AckMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.AckMsg} class reference to contained entry's load object
	 */
	public AckMsg ackMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.GenericMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.GenericMsg} class reference to contained entry's load object
	 */
	public GenericMsg genericMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.FieldList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.FieldList} class reference to contained entry's load object
	 */
	public FieldList fieldList();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.ElementList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.ElementList} class reference to contained entry's load object
	 */
	public ElementList elementList();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Map}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Map} class reference to contained entry's load object
	 */
	public Map map();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Vector}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Vector} class reference to contained entry's load object
	 */
	public Vector vector();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Series}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Series} class reference to contained entry's load object
	 */
	public Series series();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.FilterList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.FilterList} class reference to contained entry's load object
	 */
	public FilterList filterList();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmOpaque}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmOpaque} class reference to contained entry's load object
	 */
	public OmmOpaque opaque();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmXml}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmXml} class reference to contained entry's load object
	 */
	public OmmXml xml();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmAnsiPage}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmAnsiPage} class reference to contained entry's load object
	 */
	public OmmAnsiPage ansiPage();
	
	/**
	 * Returns the contained Data based on the DataType.
	 * 
	 * @return Data class reference to contained entry's load object
	 */
	public Data load();
	
	/**
	 * Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmError}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmError} class reference to contained entry's load object
	 */
	public OmmError error();
	
	/** Adds a ReqMsg type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry reqMsg(ReqMsg value);
	
	/** Adds a RefreshMsg type of OMM data to the SeriesEntry.
	  * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry refreshMsg(RefreshMsg value);

	/** Adds a StatusMsg type of OMM data to the SeriesEntry.
	   * <br>All entries must have same complex data type.
	  * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry statusMsg(StatusMsg value);
	
	/** Adds a UpdateMsg type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry updateMsg(UpdateMsg value);
	
	/** Adds a PostMsg type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry postMsg(PostMsg value);
	
	/** Adds a AckMsg type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry ackMsg(AckMsg value);
	
	/** Adds a GenericMsg type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry genericMsg(GenericMsg value);
	
	/** Adds a FieldList type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *   
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry fieldList(FieldList value);
	
	/** Adds a ElementList type of OMM data to the SeriesEntry.
	 * <br>All entries  must have same complex data type.
	 *   
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry elementList(ElementList value);
	
	/** Adds a Map type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *   
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry map(Map value);
	
	/** Adds a Vector type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *   
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry vector(Vector value);
	
	/** Adds a Series type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry series(Series value);
	
	/** Adds a FilterList type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry filterList(FilterList value);
	
	/** Adds a OmmOpaque type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry opaque(OmmOpaque value);
	
	/** Adds a OmmXml type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry xml(OmmXml value);
	
	/** Adds a OmmAnsiPage type of OMM data to the SeriesEntry.
	 * <br>All entries must have same complex data type.
	 *   
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 *
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public SeriesEntry ansiPage(OmmAnsiPage value);
}