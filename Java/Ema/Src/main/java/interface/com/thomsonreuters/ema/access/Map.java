///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Collection;

/**
 * Map is a homogeneous container of complex data type entries.
 * 
 * <p>
 * Map is a collection which provides iterator over the elements in this
 * collection.
 * </p>
 * 
 * <p>
 * Map entries are identified by a map key. All entries must have key of the
 * same primitive data type. All entries must have same complex data type
 * (except for delete action).
 * </p>
 * 
 * <p>
 * Map supports two methods of adding containers; they are: <br>
 * - adding of already populated containers, (e.g. complete() was called) and
 * <br>
 * - adding of clear containers (e.g. clear() was called) which would be
 * populated after that.
 * </p>
 * 
 * <p>
 * The first method of adding of already populated containers allows for easy
 * data manipulation but incurs additional memory copy. This method is useful in
 * applications extracting data containers from some messages or containers and
 * then setting them on other containers.
 * </p>
 * 
 * <p>
 * The second method allows for fast container population since it avoids
 * additional memory copy incurred by the first method. This method is useful in
 * source applications setting OMM data from native data formats.
 * </p>
 */
public interface Map extends ComplexType, Collection<MapEntry>
{
	/**
	 * Indicates presence of KeyFieldId.
	 * 
	 * @return true if key field id is set; false otherwise
	 */
	public boolean hasKeyFieldId();

	/**
	 * Indicates presence of TotalCountHint.
	 * 
	 * @return true if total count hint is set; false otherwise
	 */
	public boolean hasTotalCountHint();

	/**
	 * Returns KeyFieldId.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if hasKeyFieldId() returns false
	 * 
	 * @return key field id
	 */
	public int keyFieldId();

	/**
	 * Returns TotalCountHint.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if hasTotalCountHint() returns false
	 * 
	 * @return total count hint
	 */
	public int totalCountHint();

	/**
	 * Returns the contained summaryData Data based on the summaryData DataType.
	 * SummaryData contains no data if
	 * {@link com.thomsonreuters.ema.access.SummaryData#dataType()} returns
	 * {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.SummaryData}
	 */
	public SummaryData summaryData();

	/**
	 * Clears the Map. Invoking clear() method clears all the values and resets
	 * all the defaults
	 */
	public void clear();

	/**
	 * Specifies KeyFieldId.
	 * 
	 * @param fieldId
	 *            specifies key field id
	 * @return reference to this object
	 */
	public Map keyFieldId(int fieldId);

	/**
	 * Specifies TotalCountHint.
	 * 
	 * @param totalCountHint
	 *            specifies total count hint
	 * @return reference to this object
	 */
	public Map totalCountHint(int totalCountHint);

	/**
	 * Specifies the SummaryData OMM Data. Call to summaryData() must happen
	 * prior to calling any add***() method
	 * 
	 * @throws OmmInvalidUsageException
	 *             if an error is detected (exception will specify the cause of
	 *             the error)
	 * 
	 * @param summaryData
	 *            specifies complex type as summaryData
	 * @return reference to this object
	 */
	public Map summaryData(ComplexType summaryData);

}