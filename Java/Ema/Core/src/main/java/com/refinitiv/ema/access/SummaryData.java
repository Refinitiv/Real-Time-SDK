///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019,2024 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * SummaryData is used to convey Omm SummaryData information optionally present on Map, Series and Vector.<br>
 * SummaryData contains objects of complex type.
 * 
 * Objects of this class are intended to be short lived or rather transitional.
 * This class is designed to efficiently perform extracting of SummaryData and its content.
 * Objects of this class are not cache-able.
 *
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
 * @see OmmJson
 * @see OmmAnsiPage
 * @see OmmError
 */
public interface SummaryData
{
	/**
	 * Returns the DataType of the contained data.
	 * <br>Return of {@link com.refinitiv.ema.access.DataType.DataTypes#NO_DATA} signifies no data present in SummaryData
	 * Return of {@link com.refinitiv.ema.access.DataType.DataTypes#ERROR} signifies error while extracting content of SummaryData.
	 * 
	 * @return data type of the contained object
	 */
	public int dataType();

	/**
	 * Returns the complex type based on the DataType.
	 * 
	 * @return ComplexType class reference to the contained object
	 */
	public ComplexType data();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.ReqMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.ReqMsg} class reference to contained object
	 */
	public ReqMsg reqMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
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
	 * 
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
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmJson}
	 *
	 * @return {@link com.refinitiv.ema.access.OmmJson} class reference to contained object
	 */
	public OmmJson json();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmAnsiPage}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmAnsiPage} class reference to contained object
	 */
	public OmmAnsiPage ansiPage();

	/** 
	 * Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmError}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
}