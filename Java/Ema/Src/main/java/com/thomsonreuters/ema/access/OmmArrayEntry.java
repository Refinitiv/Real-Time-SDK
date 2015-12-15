///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmArrayEntry represents an entry of OmmArray.
 * OmmArrayEntry associates entry's data and its data type.
 */
public interface OmmArrayEntry
{
	/**
	 * Returns the DataType of the entry's load.
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#ERROR} signifies error while extracting content of load.
	 * 
	 * @return data type of the contained object
	 */
	public int loadType();

	/**
	 * Returns the Code of the entry's load. The code indicates a special state of a Data.
	 * Attempts to extract data will cause OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK} is returned.
	 * 
	 * @return data code of the contained object
	 */
	public int code();
		
	/**
	 * Returns a string representation of the class instance.
	 * 
	 * @return string representation of the class instance
	 */
	public String toString();

	/**
	 * Returns the contained Data based on the DataType.
	 * 
	 * @return Data class reference to contained object
	 */
	public Data load();

	/** Returns the current OMM data represented as a specific simple type.
	 * 
	 * 	@throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmInt}
	 * 	@throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long intValue();
	
	/** Returns the current OMM data represented as a specific simple type.
	 * 
	 * 	@throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmInt}
	 * 	@throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmInt}
	 */
	public OmmInt ommIntValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUInt}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long uintValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUInt}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUInt}
	 */
	public OmmUInt ommUIntValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmReal}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmReal} class reference to contained object
	 */
	public OmmReal real();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return float
	 */
	public float floatValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmFloat}
	 */
	public OmmFloat ommFloatValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return double
	 */
	public double doubleValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDouble}
	 */
	public OmmDouble ommDoubleValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDate}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDate} class reference to contained object
	 */
	public OmmDate date();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmTime}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmTime} class reference to contained object
	 */
	public OmmTime time();

	/** 
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDateTime}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDateTime} class reference to contained object
	 */
	public OmmDateTime dateTime();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmQos}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmQos} class reference to contained object
	 */
	public OmmQos qos();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmState}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmState} class reference to contained object
	 */
	public OmmState state();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmEnum}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return int
	 */
	public int enumValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmEnum}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmEnum}
	 */
	public OmmEnum ommEnumValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmBuffer}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmBuffer}
	 */
	public OmmBuffer buffer();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmAscii}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmAscii}
	 */
	public OmmAscii ascii();

	/** 
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUtf8}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUtf8}
	 */
	public OmmUtf8 utf8();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmRmtes}
	 * @throw OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmRmtes}
	 */
	public OmmRmtes rmtes();

	/** Returns Error.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmError}
	 * @return {@link com.thomsonreuters.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
}