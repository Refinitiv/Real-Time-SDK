///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * MapEntry represents an entry of Map.
 * MapEntry associates entry's key, permission information, action, data and its data type.
 */
public interface MapEntry
{
	/**
	 * A MapAction represents a map entry action.
	 */
	public static class MapAction
	{
		/**
		 * Indicates a partial change of the current Omm data.
		 */
		public static final int UPDATE = 1;

		/**
		 * Indicates to append or replace the current Omm data.
		 */
		public static final int ADD = 2;

		/**
		 * Indicates to remove current Omm data.
		 */
		public static final int DELETE = 3;
	}

	/**
	 * Returns the MapAction value as a string format.
	 * 
	 * @return String containing string representation of MapAction
	 */
	public String mapActionAsString();

	/**
	 * Returns the DataType of the entry's load.
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA} signifies no data present in load.
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#ERROR}
	 * signifies error while extracting content of load
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
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.ReqMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.ReqMsg} class reference to contained object
	 */
	public ReqMsg reqMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.RefreshMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.RefreshMsg} class reference to contained entry's load object
	 */
	public RefreshMsg refreshMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.UpdateMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.UpdateMsg} class reference to contained entry's load object
	 */
	public UpdateMsg updateMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.StatusMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.StatusMsg} class reference to contained entry's load object
	 */
	public StatusMsg statusMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.PostMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.PostMsg} class reference to contained entry's load object
	 */
	public PostMsg postMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.AckMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.AckMsg} class reference to contained entry's load object
	 */
	public AckMsg ackMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.GenericMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.GenericMsg} class reference to contained entry's load object
	 */
	public GenericMsg genericMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.FieldList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.FieldList} class reference to contained entry's load object
	 */
	public FieldList fieldList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.ElementList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.ElementList} class reference to contained entry's load object
	 */
	public ElementList elementList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Map}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Map} class reference to contained entry's load object
	 */
	public Map map();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Vector}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Vector} class reference to contained entry's load object
	 */
	public Vector vector();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Series}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Series} class reference to contained entry's load object
	 */
	public Series series();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.FilterList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.FilterList} class reference to contained entry's load object
	 */
	public FilterList filterList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmOpaque}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmOpaque} class reference to contained entry's load object
	 */
	public OmmOpaque opaque();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmXml}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmXml} class reference to contained entry's load object
	 */
	public OmmXml xml();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmAnsiPage}
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
	 * Returns the contained key Data based on the key DataType.
	 * 
	 * @return Key class reference to contained entry's Key object
	 */
	public Key key();

	/**
	 * Returns the current action on the OMM data.
	 * 
	 * @return MapAction
	 */
	public int action();

	/**
	 * Returns PermissionData.
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasPermissionData()} returns false
	 * 
	 * @return ByteBuffer containing permission information
	 */
	public ByteBuffer permissionData();

	/**
	 * Returns Error.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmError}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmError} class reference to contained entry's load object
	 */
	public OmmError error();
}