///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.util.Collection;

/**
 * Series is a homogeneous container of complex data type entries.<br>
 * Series is a collection which provides iterator over the elements in this
 * collection.
 * 
 * <p>
 * Series entries have no explicit identification. They are implicitly indexed
 * inside Series.</p>
 * 
 * The following code snippet shows addition of entry and summaryData to Series.
 * <pre>
 * Series series = EmaFactory.createSeries();
 * FieldList fl = EmaFactory.createFieldList();
 * fl.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
 * series.summaryData(fl);
 * fl.clear();
 * fl.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
 * series.add(EmaFactory.createSeriesEntry().fieldList(fl));
 * </pre>
 *
 * The following code snippet shows extracting data from Series and its content.
 * <pre>
 * void decode(Series series)
 * {
 *    switch(series.summaryData().dataType())
 *    {
 *    case DataTypes.FIELD_LIST:
 *         decode(series.summaryData().fieldList());
 *         break;
 *    case DataTypes.NO_DATA:
 *         break;
 *    }
 *
 *    for(SeriesEntry seriesEntry : series)
 *    {
 *       System.out.println(" DataType: " + DataType.asString(seriesEntry.loadType()) + " Value: ");
 *
 *       switch(seriesEntry.loadType())
 *       {
 *       case DataTypes.FIELD_LIST:
 *           decode(seriesEntry.fieldList());
 *           break;
 *       case DataTypes.NO_DATA:
 *           break;
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
 * @see SeriesEntry
 * @see SummaryData
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
	 * {@link com.rtsdk.ema.access.DataType.DataTypes#NO_DATA}
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
	 * Specifies the SummaryData OMM Data.
	 * 
	 * @param data
	 *            specifies complex type as summaryData
	 * @return reference to this object
	 */
	public Series summaryData(ComplexType data);
}