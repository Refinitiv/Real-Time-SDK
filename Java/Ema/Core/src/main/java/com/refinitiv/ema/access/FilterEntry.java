///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;


/**
 * FilterEntry represents an entry of FilterList.
 * <p>FilterEntry associates entry's id, action, permission info, data and its data type.</p>
 * 
 * <p>Code snippet:</p>
 * <pre>
 * void decode(FilterList filterList)
 * {
 *    for(FilterEntry filterEntry : filterList)
 *    {
 *       System.out.println("ID: " + filterEntry.filterId()
 *                                 + " Action = " + filterEntry.filterActionAsString() 
 *                                 + " DataType: " + DataType.asString(filterEntry.loadType()) + " Value: ");
 *       switch(filterEntry.loadType())
 *       {
 *       case DataTypes.ELEMENT_LIST :
 *           decode(filterEntry.elementList());
 *           break;
 *       case DataTypes.MAP :
 *           decode(filterEntry.map());
 *           break;
 *       ...
 *       }
 *    }
 * }
 * </pre>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * Objects of this class are not cache-able.
 *
 * @see Data
 * @see ComplexType
 * @see ReqMsg
 * @see RefreshMsg
 * @see UpdateMsg
 * @see StatusMsg
 * @see GenericMsg
 * @see PostMsg
 * @see AckMsg
 * @see FieldList
 * @see ElementList
 * @see Map
 * @see Vector
 * @see Series
 * @see FilterList
 * @see OmmOpaque
 * @see OmmXml
 * @see OmmAnsiPage
 * @see OmmError
 */
public interface FilterEntry
{
	/**
	 * FilterAction represents filter entry action.
	 */
	public static class FilterAction
	{
		/**
		 * Indicates a partial change of the current Omm data.
		 */
		public static final int UPDATE	= 1;
	
		/**
		 * Indicates to specify or replace the current Omm data.
		 */
		public static final int SET	= 2;
	
		/**
		 * Indicates to unset the current Omm data.
		 */
		public static final int CLEAR	= 3;
	}
	
	/**
	 * Returns the FilterAction value as a string format.
	 * 
	 * @return String containing string representation of FilterAction
	 */
	public String filterActionAsString();
		
	/**
	 * Returns the DataType of the entry's load.<br>
	 * Return of {@link com.refinitiv.ema.access.DataType.DataTypes#NO_DATA} signifies no data present in load.<br>
	 * Return of {@link com.refinitiv.ema.access.DataType.DataTypes#ERROR} signifies error while extracting content of load.
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
	 * Returns FilterId.
	 * 
	 * @return filter id
	 */
	public int filterId();
	
	/**
	 * Indicates presence of PermissionData.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.ReqMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.ReqMsg} class reference to contained object
	 */
	public ReqMsg reqMsg();
	
	/** Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.RefreshMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.RefreshMsg} class reference to contained object
	*/
	public RefreshMsg refreshMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.UpdateMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.UpdateMsg} class reference to contained object
	 */
	public UpdateMsg updateMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.StatusMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.StatusMsg} class reference to contained object
	 */
	public StatusMsg statusMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.PostMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.PostMsg} class reference to contained object
	 */
	public PostMsg postMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.AckMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.AckMsg} class reference to contained object
	 */
	public AckMsg ackMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.GenericMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.GenericMsg} class reference to contained object
	 */
	public GenericMsg genericMsg();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.FieldList}
	 * 
	 * @return {@link com.refinitiv.ema.access.FieldList} class reference to contained object
	 */
	public FieldList fieldList();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.ElementList}
	 * 
	 * @return {@link com.refinitiv.ema.access.ElementList} class reference to contained object
	 */
	public ElementList elementList();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.Map}
	 * 
	 * @return {@link com.refinitiv.ema.access.Map} class reference to contained object
	 */
	public Map map();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.Vector}
	 * 
	 * @return {@link com.refinitiv.ema.access.Vector} class reference to contained object
	 */
	public Vector vector();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.Series}
	 * 
	 * @return {@link com.refinitiv.ema.access.Series} class reference to contained object
	 */
	public Series series();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.FilterList}
	 * 
	 * @return {@link com.refinitiv.ema.access.FilterList} class reference to contained object
	 */
	public FilterList filterList();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmOpaque}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmOpaque} class reference to contained object
	 */
	public OmmOpaque opaque();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmXml}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmXml} class reference to contained object
	 */
	public OmmXml xml();
	
	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmAnsiPage}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmAnsiPage} class reference to contained object
	 */
	public OmmAnsiPage ansiPage();
	
	/**
	 * Returns the contained Data based on the DataType.
	 * 
	 * @return Data class reference to contained object
	 */
	public Data load();
	
	/**
	 * Returns the current action on the OMM data.
	 * 
	 * @return FilterAction
	 */
	public int action();
	
	/**
	 * Returns PermissionData.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPermissionData()} returns false
	 * 
	 * @return ByteBuffer containing permission information
	 */
	public ByteBuffer permissionData();
	
	/**
	 * Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmError}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
	
	/** Adds a ReqMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load of for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry reqMsg(int filterId, int action, ReqMsg value);
	
	/** Adds a ReqMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry reqMsg(int filterId, int action, ReqMsg value, ByteBuffer permissionData);

	/** Adds a RefreshMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry refreshMsg(int filterId, int action, RefreshMsg value);

	/** Adds a RefreshMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry refreshMsg(int filterId, int action, RefreshMsg value, ByteBuffer permissionData);
	
	/** Adds a StatusMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry statusMsg(int filterId, int action, StatusMsg value);
	
	/** Adds a StatusMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry statusMsg(int filterId, int action, StatusMsg value, ByteBuffer permissionData);

	/** Adds a UpdateMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry updateMsg(int filterId, int action, UpdateMsg value);
	
	/** Adds a UpdateMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry updateMsg(int filterId, int action, UpdateMsg value, ByteBuffer permissionData);

	/** Adds a PostMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry postMsg(int filterId, int action, PostMsg value);
	
	/** Adds a PostMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry postMsg(int filterId, int action, PostMsg value, ByteBuffer permissionData);
	
	/** Adds a AckMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry ackMsg(int filterId, int action, AckMsg value);
	
	/** Adds a AckMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry ackMsg(int filterId, int action, AckMsg value, ByteBuffer permissionData);
	
	/** Adds a GenericMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry genericMsg(int filterId, int action, GenericMsg value);
	
	/** Adds a GenericMsg type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry genericMsg(int filterId, int action, GenericMsg value, ByteBuffer permissionData);
	
	/** Adds a FieldList type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry fieldList(int filterId, int action, FieldList value);
	
	/** Adds a FieldList type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry fieldList(int filterId, int action, FieldList value, ByteBuffer permissionData);
	
	/** Adds a ElementList type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry elementList(int filterId, int action, ElementList value);
	
	/** Adds a ElementList type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry elementList(int filterId, int action, ElementList value, ByteBuffer permissionData);
	
	/** Adds a Map type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry map(int filterId, int action, Map value);
	
	/** Adds a Map type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry map(int filterId, int action, Map value, ByteBuffer permissionData);

	/** Adds a Vector type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry vector(int filterId, int action, Vector value);
	
	/** Adds a Vector type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry vector(int filterId, int action, Vector value, ByteBuffer permissionData);
	
	/** Adds a Series type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry series(int filterId, int action, Series value);
	
	/** Adds a Series type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry series(int filterId, int action, Series value, ByteBuffer permissionData);
	
	/** Adds a FilterList type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry filterList(int filterId, int action, FilterList value);
	
	/** Adds a FilterList type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry filterList(int filterId, int action, FilterList value, ByteBuffer permissionData);
	
	/** Adds a OmmOpaque type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry opaque(int filterId, int action, OmmOpaque value);
	
	/** Adds a OmmOpaque type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry opaque(int filterId, int action, OmmOpaque value, ByteBuffer permissionData);
	
	/** Adds a OmmXml type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry xml(int filterId, int action, OmmXml value);
	
	/** Adds a OmmXml type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry xml(int filterId, int action, OmmXml value, ByteBuffer permissionData);
	
	/** Adds a OmmAnsiPage type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry ansiPage(int filterId, int action, OmmAnsiPage value);
	
	/** Adds a OmmAnsiPage type of OMM data to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param value specifies load for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry ansiPage(int filterId, int action, OmmAnsiPage value, ByteBuffer permissionData);
	
	/** Adds no payload to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry noData(int filterId, int action);
	
	/** Adds no payload to the FilterEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param filterId specifies filter id for the added FilterEntry
	 * @param action specifies action for the added FilterEntry
	 * @param permissionData specifies permissions for the added FilterEntry
	 * 
	 * @return reference to this object
	 */
	public FilterEntry noData(int filterId, int action, ByteBuffer permissionData);
}
