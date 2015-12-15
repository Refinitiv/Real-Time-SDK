///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.util.Collection;

/**
 * Vector is a homogeneous container of complex data type entries.
 * 
 * <p>Vector is a collection which provides iterator over the elements in this collection.</p>
 * 
 * <br>Vector entries are identified by index.
 * 
 * <p>Vector supports two methods of adding containers; they are:
 * <br>	- adding of already populated containers, (e.g. {@link #complete()} was called on these containers) and 
 * <br>	- adding of clear containers (e.g. {@link #clear()} was called on these containers) which would be populated after that.
 * 
 * <p>The first method of adding of already populated containers allows for easy data
 * 	manipulation but incurs additional memory copy. This method is useful in
 * 	applications extracting data containers from some messages or containers and then
 * 	setting them on other containers.</p>
 * 
 * <p>The second method allows for fast container population since it avoids additional
 * 	memory copy incurred by the first method. This method is useful in source applications
 * 	setting OMM data from native data formats.</p>
 * 
 * <p>These two methods apply to containers only; e.g. ElementList, FieldList, FilterList, Map, Series, and Vector.</p>
 * 
 * <p>Objects of this class are intended to be short lived or rather transitional.
 * <br>This class is designed to efficiently perform setting and extracting of Vector and its content.
 * <br>Objects of this class are not cache-able.</p>
 */
public interface Vector extends ComplexType, Collection<VectorEntry>
{
	/**
	 * Iterates through a list of Data of any DataType.
	 * <br>Typical usage is to extract the entry during each iteration via {@link #entry()}.
	 * 
	 * @return true at the end of Vector; false otherwise
	 */
	public boolean forth();
	
	/**
	 * Resets iteration to start of container.
	 */
	public void reset();
	
	/**
	 * Indicates presence of TotalCountHint.
	 * 
	 * @return true if total count hint is set; false otherwise
	 */
	public boolean hasTotalCountHint();
	
	/**
	 * Returns Sortable.
	 * 
	 * @return true if sortable flag is set; false otherwise
	 */
	public boolean sortable();
	
	/**
	 * Returns TotalCountHint.
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasTotalCountHint()} returns false
	 * 
	 * @return total count hint
	 */
	public int totalCountHint();
	
	/**
	 * Returns the contained summaryData Data based on the summaryData DataType.
	 * <br>SummaryData contains no data if {@link SummaryData#dataType()} returns {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA}
	 * 
	 * @return {@link SummaryData}
	*/
	public SummaryData summaryData();
	
	/**
	 * Returns Entry.
	 * 
	 * @throw OmmInvalidUsageException if {@link #forth()} was not called first
	 * 
	 * @return VectorEntry
	 */
	public VectorEntry entry();

	/**
	 * Clears the Vector.
	 * <br>Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public void clear();
	
	/**
	 * Adds complex OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * 
	 * @return reference to this object
	 */
	public Vector add(int position, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a position.
	 * <br>All entries must have same complex data type.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param position specifies position of this entry in Vector
	 * @param action specifies action to be performed on this entry
	 * @param value complex type contained in this entry
	 * @param permissionData specifies permission data for this entry
	 * 
	 * @return reference to this object
	 */
	public Vector add(int position, int action, ComplexType value, ByteBuffer permissionData);
	
	/**
	 * Completes encoding of the Vector.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @return reference to this object
	 */
	public Vector complete();
	
	/**
	 * Specifies Sortable.
	 * 
	 * @param sortable specifies if this object is sortable
	 * 
	 * @return reference to this object
	*/
	public Vector sortable(boolean sortable);
	
	/**
	 * Specifies TotalCountHint.
	 * 
	 * @param totalCountHint specifies total count hint
	 * 
	 * @return reference to this object
	 */
	public Vector totalCountHint(int totalCountHint);
	
	/**
	 * Specifies the SummaryData OMM Data.
	 * <br>Call to summaryData() must happen prior to calling the add() method
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
	 * 
	 * @param data specifies complex type as summaryData
	 * @return reference to this object
	*/
	public Vector summaryData(ComplexType data);
}