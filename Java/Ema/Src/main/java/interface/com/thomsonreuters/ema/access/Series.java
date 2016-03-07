///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Collection;

/**
 * Series is a homogeneous container of complex data type entries.
 * 
 * <p>
 * Series is a collection which provides iterator over the elements in this
 * collection.
 * </p>
 * 
 * <p>
 * Series entries have no explicit identification. They are implicitly indexed
 * inside Series.
 * </p>
 * 
 * <p>
 * Series supports two methods of adding containers; they are: <br>
 * - adding of already populated containers, (e.g. {@link #complete()} was
 * called) and <br>
 * - adding of clear containers (e.g. {@link #clear()} was called) which would
 * be populated after that.
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
 * 
 * <p>
 * These two methods apply to containers only, e.g. ElementList, FieldList,
 * FilterList, Map, Series, and Vector.
 * </p>
 * 
 * <p>
 * Objects of this class are intended to be short lived or rather transitional.
 * <br>
 * This class is designed to efficiently perform setting and extracting of
 * Series and its content. <br>
 * Objects of this class are not cache-able.
 * </p>
 */
public interface Series extends ComplexType, Collection<SeriesEntry>
{
	/**
	 * Indicates presence of TotalCountHint.
	 * 
	 * @return true if total count hint is set; false otherwise
	 */
	public boolean hasTotalCountHint();

	/**
	 * Returns TotalCountHint.
	 * 
	 * throw OmmInvalidUsageException if {@link #hasTotalCountHint()} returns
	 * false
	 * 
	 * @return total count hint
	 */
	public int totalCountHint();

	/**
	 * Returns the contained summaryData Data based on the summaryData DataType.
	 * SummaryData contains no data if {@link SummaryData#dataType()} returns
	 * {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA}
	 * 
	 * @return {@link SummaryData}
	 */
	public SummaryData summaryData();

	/**
	 * Clears the Series. Invoking clear() method clears all the values and
	 * resets all the defaults.
	 */
	public void clear();

	/**
	 * Specifies TotalCountHint.
	 * 
	 * @param totalCountHint
	 *            specifies total count hint
	 * @return reference to this object
	 */
	public Series totalCountHint(int totalCountHint);

	/**
	 * Specifies the SummaryData OMM Data. Call to {@link #summaryData()} must
	 * happen prior to calling the {@link #add(ComplexType)} method
	 * 
	 * @param data
	 *            specifies complex type as summaryData
	 * @return reference to this object
	 */
	public Series summaryData(ComplexType data);
}