///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * VectorEntry represents an entry of Vector.
 * <p>VectorEntry associates entry's position, action, permission information, data and its data type.</p>
 * 
 * Code snippet:
 * <pre>
 * void decode(Vector vector)
 * {
 *    for(VectorEntry vectorEntry : vector)
 *    {
 *       System.out.println(" DataType: " + DataType.asString(vectorEntry.loadType()) + " Value: ");
 *
 *       switch(vectorEntry.loadType())
 *       {
 *       case DataTypes.FIELD_LIST:
 *           decode(vectorEntry.fieldList());     
 *           break;
 *           
 *       ...
 *       
 *       }
 *    }
 * }
 * </pre>
 * 
 * Objects of this class are intended to be short lived or rather transitional.
 * This class is designed to efficiently perform setting and extracting of Map and its content.
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
public interface VectorEntry
{
	/**
	 * VectorAction represents vector entry action.
	 */
	public static class VectorAction
	{
		/**
		 * Indicates a partial change of the entry.
		 */
		public final static int UPDATE	= 1;
	
		/**
		 * Indicates to replace the entry.
		 */
		public final static int SET		= 2;
	
		/**
		 * Indicates to empty the entry. Contains no data.
		 */
		public final static int CLEAR	= 3;
	
		/**
		 * Indicates to place the entry in between other entries.
		 * <br>Increases any higher-ordered position by one.
		 * <br>May leave gaps if previous lower-ordered position is not populated. 
		 */
		public final static int INSERT	= 4;
	
		/**
		 * Indicates to remove the entry.
		 * <br>Decreases any higher-ordered position by one.
		 * <br>Contains no data.
		 */
		public final static int DELETE	= 5;
	}
	
	/**
	 * Returns the VectorAction value as a string format.
	 * 
	 * @return String containing string representation of VectorAction
	 */
	public String vectorActionAsString();
		
	/**
	 * Returns the DataType of the entry's load.
	 * <br>Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA} signifies no data present in load.
	 * <br>Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#ERROR} signifies error while extracting content of load.
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
	 * Indicates presence of PermissionData.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();
	
	/**
	 * Returns position of the entry.
	 * 
	 * @return entry's position
	 */
	public long position();
	
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
	 * Returns the current action on the OMM data.
	 * 
	 * @return VectorEntry Action
	 */
	public int action();
	
	/**
	 * Returns PermissionData.
	 * 
	 * @throws OmmInvalidUsageException if hasPermissionData() returns false
	 * 
	 * @return ByteBuffer containing permission information
	 */
	public ByteBuffer permissionData();
	
	/**
	 * Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmError}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmError} class reference to contained entry's load object
	 */
	public OmmError error();
	
	/** Adds a ReqMsg type of OMM data identified by a position.
	* <br>All entries must have same complex data type.
	* 
	* @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	* 
	* @param position specifies position of this entry in Vector
	* @param action specifies action to be performed on this entry
	* @param value complex type contained in this entry
	* 
	* @return reference to this object
	*/
	public VectorEntry reqMsg(long position, int action, ReqMsg value);
	
	/** Adds a ReqMsg type of OMM data  identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action for the added VectorEntry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry reqMsg(long position, int action, ReqMsg value, ByteBuffer permissionData);

	/** Adds a RefreshMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry refreshMsg(long position, int action, RefreshMsg value);

	/** Adds a RefreshMsg type of OMM data identified by a position.
	* <br>All entries must have same complex data type.
	* 
	* @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	* 
	* @param position specifies position of this entry in Vector
	* @param action specifies action to be performed on this entry
	* @param value complex type contained in this entry
	* @param permissionData specifies permission data for this entry
	* 
	* @return reference to this object
	*/
	public VectorEntry refreshMsg(long position, int action, RefreshMsg value, ByteBuffer permissionData);
	
	/** Adds a StatusMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry statusMsg(long position, int action, StatusMsg value);
	
	/** Adds a StatusMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry statusMsg(long position, int action, StatusMsg value, ByteBuffer permissionData);

	/** Adds a UpdateMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry updateMsg(long position, int action, UpdateMsg value);
	
	/** Adds a UpdateMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry updateMsg(long position, int action, UpdateMsg value, ByteBuffer permissionData);

	/** Adds a PostMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry postMsg(long position, int action, PostMsg value);
	
	/** Adds a PostMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry postMsg(long position, int action, PostMsg value, ByteBuffer permissionData);
	
	/** Adds a AckMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry ackMsg(long position, int action, AckMsg value);
	
	/** Adds a AckMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry ackMsg(long position, int action, AckMsg value, ByteBuffer permissionData);
	
	/** Adds a GenericMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry genericMsg(long position, int action, GenericMsg value);
	
	/** Adds a GenericMsg type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry genericMsg(long position, int action, GenericMsg value, ByteBuffer permissionData);
	
	/** Adds a FieldList type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry fieldList(long position, int action, FieldList value);
	
	/** Adds a FieldList type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry fieldList(long position, int action, FieldList value, ByteBuffer permissionData);
	
	/** Adds a ElementList type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry elementList(long position, int action, ElementList value);
	
	/** Adds a ElementList type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry elementList(long position, int action, ElementList value, ByteBuffer permissionData);
	
	/** Adds a Map type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry map(long position, int action, Map value);
	
	/** Adds a Map type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry map(long position, int action, Map value, ByteBuffer permissionData);

	/** Adds a Vector type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry vector(long position, int action, Vector value);
	
	/** Adds a Vector type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry vector(long position, int action, Vector value, ByteBuffer permissionData);
	
	/** Adds a Series type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry series(long position, int action, Series value);
	
	/** Adds a Series type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry series(long position, int action, Series value, ByteBuffer permissionData);
	
	/** Adds a FilterList type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry filterList(long position, int action, FilterList value);
	
	/** Adds a FilterList type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry filterList(long position, int action, FilterList value, ByteBuffer permissionData);
	
	/** Adds a OmmOpaque type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry opaque(long position, int action, OmmOpaque value);
	
	/** Adds a OmmOpaque type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry opaque(long position, int action, OmmOpaque value, ByteBuffer permissionData);
	
	/** Adds a OmmXml type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry xml(long position, int action, OmmXml value);
	
	/** Adds a OmmXml type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry xml(long position, int action, OmmXml value, ByteBuffer permissionData);
	
	/** Adds a OmmAnsiPage type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry ansiPage(long position, int action, OmmAnsiPage value);
	
	/** Adds a OmmAnsiPage type of OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry ansiPage(long position, int action, OmmAnsiPage value, ByteBuffer permissionData);
	
	/** Adds no payload identified by a position.
	 *
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry noData(long position, int action);
	
	/** Adds no payload identified by a position.
	 *  
	 * @throws OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public VectorEntry noData(long position, int action, ByteBuffer permissionData);
}
