///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;


/**
 * Attrib is read only and is used for decoding only.
 * 
 * <p>The following code snippet shows extracting of Attrib and its content while processing RefreshMsg.</p>
 * <pre>
 * class AppClient implements OmmConsumerClient
 * {
 *     ...
 *
 *     public void onRefreshMsg( RefreshMsg refreshMsg, OmmConsumerEvent Event )
 *     {
 *         Attrib attrib = refreshMsg.attrib();
 *			
 *         switch( attrib.dataType() )
 *         {
 *         case DataTypes.FIELD_LIST :
 *            decode( attrib.fieldList() );
 *            break;
 *         case DataTypes.MAP :
 *            decode( attrib.map() );
 *            break;
 *         case DataTypes.NO_DATA :
 *            break;
 *         }
 *     }
 *     
 *     ...
 *
 * };
 * </pre>
 * 
 * This class is designed to efficiently perform extracting of Attrib and its content.<br>
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * Objects of this class are not cache-able.
 *
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
 * @see OmmAnsiPage
 * @see OmmError
 * 
 */
public interface Attrib
{
	/**
	 * Returns the DataType of the contained data.<br>
	 * Return of {@link com.rtsdk.ema.access.DataType.DataTypes#NO_DATA} signifies no data present in Attrib.<br>
	 * Return of {@link com.rtsdk.ema.access.DataType.DataTypes#ERROR} signifies error while extracting content of Attrib.
	 * 
	 * @return data type of the contained object
	 */
	public int dataType();

	/**
	 * Returns the complex type based on the DataType.
	 * 
	 * @return ComplexType class reference to contained object
	 */
	public ComplexType data();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.ReqMsg}
	 * 
	 * @return {@link com.rtsdk.ema.access.ReqMsg} class reference to contained object
	 */
	public ReqMsg reqMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.RefreshMsg}
	 * 
	 * @return {@link com.rtsdk.ema.access.RefreshMsg} class reference to contained object
	 */
	public RefreshMsg refreshMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.UpdateMsg}
	 * 
	 * @return {@link com.rtsdk.ema.access.UpdateMsg} class reference to contained object
	 */
	public UpdateMsg updateMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.StatusMsg}
	 * 
	 * @return {@link com.rtsdk.ema.access.StatusMsg} class reference to contained object
	 */
	public StatusMsg statusMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.PostMsg}
	 * 
	 * @return {@link com.rtsdk.ema.access.PostMsg} class reference to contained object
	 */
	public PostMsg postMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.AckMsg}
	 * 
	 * @return {@link com.rtsdk.ema.access.AckMsg} class reference to contained object
	 */
	public AckMsg ackMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.GenericMsg}
	 * 
	 * @return {@link com.rtsdk.ema.access.GenericMsg} class reference to contained object
	 */
	public GenericMsg genericMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.FieldList}
	 * 
	 * @return {@link com.rtsdk.ema.access.FieldList} class reference to contained object
	 */
	public FieldList fieldList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.ElementList}
	 * 
	 * @return {@link com.rtsdk.ema.access.ElementList} class reference to contained object
	 */
	public ElementList elementList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.Map}
	 * 
	 * @return {@link com.rtsdk.ema.access.Map} class reference to contained object
	 */
	public Map map();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.Vector}
	 * 
	 * @return {@link com.rtsdk.ema.access.Vector} class reference to contained object
	 */
	public Vector vector();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.Series}
	 * 
	 * @return {@link com.rtsdk.ema.access.Series} class reference to contained object
	 */
	public Series series();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.FilterList}
	 * 
	 * @return {@link com.rtsdk.ema.access.FilterList} class reference to contained object
	 */
	public FilterList filterList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.OmmOpaque}
	 * 
	 * @return {@link com.rtsdk.ema.access.OmmOpaque} class reference to contained object
	 */
	public OmmOpaque opaque();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.OmmXml}
	 * 
	 * @return {@link com.rtsdk.ema.access.OmmXml} class reference to contained object
	 */
	public OmmXml xml();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.OmmAnsiPage}
	 * 
	 * @return {@link com.rtsdk.ema.access.OmmAnsiPage} class reference to contained object
	 */
	public OmmAnsiPage ansiPage();

	/**
	 * Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.rtsdk.ema.access.OmmError}
	 * 
	 * @return {@link com.rtsdk.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
}