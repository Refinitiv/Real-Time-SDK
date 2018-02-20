///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * Key conveys MapEntry key information.
 * <p>Key contains objects of primitive type (e.g. they are not complex type).</p>
 */
public interface Key
{
	/**
	 * Returns the DataType of the contained data.
	 * <br>
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#ERROR}
	 * signifies error while extracting content of Key.
	 * 
	 * @return data type of the contained object
	 */
	public int dataType();

	/**
	 * Returns the simple type based on the DataType.
	 * 
	 * @return Data class reference to the contained object
	 */
	public Data data();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#INT}
	 * 
	 * @return long
	 */
	public long intValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#INT}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmInt}
	 */
	public OmmInt ommIntValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#UINT}
	 * 
	 * @return long
	 */
	public long uintValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#UINT}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUInt}
	 */
	public OmmUInt ommUIntValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#REAL}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmReal} class reference to the contained object
	 */
	public OmmReal real();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * 
	 * @return float
	 */
	public float floatValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#FLOAT}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmFloat}
	 */
	public OmmFloat ommFloatValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#DOUBLE}
	 * 
	 * @return double
	 */
	public double doubleValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDouble}
	 */
	public OmmDouble ommDoubleValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#DOUBLE}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDate} class reference to the contained object
	 */
	public OmmDate date();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#TIME}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmTime} class reference to the contained object
	 */
	public OmmTime time();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#DATETIME}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDateTime} class reference to the contained object
	 */
	public OmmDateTime dateTime();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#QOS}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmQos} class reference to the contained object
	 */
	public OmmQos qos();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#STATE}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmState} class reference to the contained object
	 */
	public OmmState state();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#ENUM}
	 * 
	 * @return int
	 */
	public int enumValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#ENUM}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmEnum}
	 */
	public OmmEnum ommEnumValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#BUFFER}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmBuffer} class reference to the contained object
	 */
	public OmmBuffer buffer();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#ASCII}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmAscii} class reference to the contained object
	 */
	public OmmAscii ascii();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#UTF8}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUtf8} class reference to the contained object
	 */
	public OmmUtf8 utf8();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#RMTES}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmRmtes} class reference to the contained object
	 */
	public OmmRmtes rmtes();

	/**
	 * Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link DataType.DataTypes#ERROR}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
}