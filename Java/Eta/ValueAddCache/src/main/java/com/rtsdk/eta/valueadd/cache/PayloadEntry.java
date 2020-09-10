package com.rtsdk.eta.valueadd.cache;

import java.io.PrintWriter;

import com.rtsdk.eta.codec.DataDictionary;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;


/**
 * {@link PayloadEntry} stores the last known or applied value of an OMM item.
 * PayloadEntry efficiently stores OMM containers (one per entry), e.g. FieldList, Map,
 * Series, Vector, FilterList or ElementList.
 * 
 * Note<br>
 * - usage of OMM FieldList requires that RDM Field Dictionary information is set
 * 	 with the cache instance prior to storing this FieldList in PayloadEntry.<br>
 * - OMM data types of Opaque and XML are not supported.<br>
 * 
 * @see PayloadCache
 * @see PayloadCursor
 * @see CacheError
 */
public interface PayloadEntry
{
	/**
	 * Trace format options in which to display a decoded output of the payload entry data.
	 */
	public class PayloadEntryTraceFormat
	{
		/**
		 * Trace format option to get a decoded output of the payload entry data in XML format
		 */
		public static final int PAYLOAD_ENTRY_TRACE_OPTION_XML = 1;
	}
	
	
	/**
	 * Applies the partially decoded Msg to the payload entry. The
	 * OMM container in the message payload is applied to the cache entry
	 * following the OMM rules of the message and the container type.
	 *
	 * When this function returns an error (&lt; CodecReturnCodes.SUCCESS), some 
	 * or all of the data could not be cached. Some errors returned by this function
	 * can be handled as warnings, depending on application requirements. When
	 * the function returns an error code, the CacheError errorId can be
	 * tested for a warning indication.  For errors, errorId will have the
	 * same value as the function return value. For warnings, errorId will be
	 * set to CodecReturnCodes.SUCCESS. The CacheError text can be reviewed for further
	 * explanation about the errors or warnings. Examples of warning conditions: data
	 * truncated for exceeding size allocated in the field dictionary; fields
	 * in an update Msg ignored if they were not in the refresh Msg.
	 *
	 * @param dIter The decode iterator positioned at the payload of the message 
	 * @param msg The partially decoded message structure
	 * @param error The error information structure will be populated if the
	 * payload data from the message could not be written to the cache entry
	 * @return Returns failure codes (&lt; CodecReturnCodes.SUCCESS) if the data could not be applied to the cache entry
	 */
	public int apply( DecodeIterator dIter, Msg msg, CacheError error );

	/**
	 * Retrieves encoded OMM data from the cache payload entry.
	 *
	 * This function encodes data from the cache entry into the buffer provided
	 * with the eIter parameter. The retrieval process will attempt to encode
	 * the entire container into the given buffer, but also supports
	 * multi-part retrieval for OMM containers that support fragmentation.
	 * (Note that FieldList and ElementList containers must be retrieved
	 * in a single part). If the application does not wish to use fragmentation,
	 * it should pass a null PayloadCursor.
	 *
	 * For multi-part retrieval, a PayloadCursor is required. The application will
	 * call this function to retrieve fragments of the cached container until the
	 * cursor indicates the final part has been retrieved (PayloadCursor.isComplete). 
	 * The size of the buffer provided for retrieval will determine the number of 
	 * fragments needed to encode the entire container.
	 *
	 * @param eIter The OMM payload data in the cache entry will be encoded into the
	 * buffer associated with this iterator.
	 * @param cursor The cursor is required to support multi-part (fragmented)
	 * retrieval of the OMM data in the cache entry. Clear the cursor (PayloadCursor.clear)
	 * prior to the first retrieval from the payload entry. The cursor is not needed for
	 * single part retrieval.
	 * @param error The error in formation structure will be populated if payload
	 * entry cache data could not be retrieved.
	 * @return Failure codes (&lt; CodecReturnCodes.SUCCESS) if data could not be
	 * retrieved from the container.
	 * - CodecReturnCodes.BUFFER_TOO_SMALL indicates that the buffer size should be
	 * increased in order to retrieve the data from the entry (single part or multi-part). 
	 * - CodecReturnCodes.SUCCESS is returned when data is successfully encoded into the buffer,
	 * or if there is no data to retrieve from the entry.
	 * - Note that for multi-part retrieval, CodecReturnCodes.SUCCESS only indicates the
	 * successful encoding of the current fragment. Check the state of the PayloadCursor
	 * to determine if the current fragment is the final part.
	 */
	public int retrieve( EncodeIterator eIter, PayloadCursor cursor, CacheError error );
	
	/**
	 * Returns the payload entry data type
	 * When the entry is first created, or cleared, returns DataTypes.UNKNOWN.
	 * 
	 * @return The OMM data type of this container.
	 */
	public short dataType();
		
	/**
	 * Removes the payload entry from its cache and destroys the entry.
	 */
	public void destroy();

	/**
	 * Clears the OMM data contents of this entry. The entry remains in
	 * the cache and can be re-used.
	 */
	public void clear();

	/**
	 * Display the cache payload entry contents by writing to the given fileWriter.
	 * 
	 * @param traceFormat Option for specifying the type of information displayed for the entry
	 * @param fileWriter The file writer where the trace information will be written. Supports
	 * write into a file or standard output.
	 * @param dictionary The dictionary structure used to decode the OMM data from the cache entry
	 * @return Returns failure code if trace function fails
	 */
	public int trace( int traceFormat, PrintWriter fileWriter, DataDictionary dictionary );
}