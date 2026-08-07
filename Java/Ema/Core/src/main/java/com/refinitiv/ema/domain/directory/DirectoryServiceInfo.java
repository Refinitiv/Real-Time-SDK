/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.List;

/**
 * Represents the RDM Source Directory service info filter.
 * <p>
 * This filter conveys a service's identifying information and content metadata,
 * such as its service name, supported domain capabilities, dictionaries,
 * available qualities of service, and several optional boolean attributes.
 * <p>
 * Within this contract, {@link #serviceName()} and {@link #capabilitiesList()}
 * are always accessible. Most other fields are optional and should be queried
 * only after the corresponding {@code checkHas*()} method returns {@code true}.
 *
 * @see DirectoryServiceFilter
 */
public interface DirectoryServiceInfo extends DirectoryServiceFilter<ElementList>
{
    /**
     * Clears this service-info filter and resets it to its default state.
     * <p>
     * This removes all optional fields, empties the mutable lists maintained by
     * the filter, restores required values to their defaults, and resets the
     * filter-entry action.
     *
     * @return this directory service info filter instance
     */
    @Override
    DirectoryServiceInfo clear();

    /**
     * Sets the filter-entry action associated with this service-info filter.
     * <p>
     * Valid values are defined by the concrete implementation and typically come
     * from {@link com.refinitiv.ema.access.FilterEntry.FilterAction}.
     *
     * @param action the filter-entry action
     * @return this directory service info filter instance
     */
    @Override
    DirectoryServiceInfo action(int action);

    /**
     * Decodes an {@link ElementList} payload into this service-info filter.
     * <p>
     * Implementations replace the current content with the decoded info-filter
     * data.
     *
     * @param struct encoded {@link ElementList} representing the service-info filter
     * @return this directory service info filter instance
     */
    @Override
    DirectoryServiceInfo decode(ElementList struct);

    /**
     * Returns the service name that identifies this service.
     * <p>
     * This value is always accessible. After {@link #clear()}, the default value
     * is an empty string.
     *
     * @return service name
     */
    String serviceName();

    /**
     * Sets the service name that identifies this service.
     *
     * @param serviceName the service name
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code serviceName} is {@code null}
     */
    DirectoryServiceInfo serviceName(String serviceName);

    /**
     * Returns the vendor name of data provided by this service.
     * <p>
     * This is an optional field. Call {@link #checkHasVendor()} before calling
     * this method.
     *
     * @return vendor name
     * @throws OmmInvalidUsageException if {@link #checkHasVendor()} returns
     *                                  {@code false}
     */
    String vendor();

    /**
     * Sets the vendor name for this service.
     * <p>
     * Calling this method marks the optional {@code vendor} field as present.
     *
     * @param vendor the vendor
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code vendor} is {@code null}
     */
    DirectoryServiceInfo vendor(String vendor);

    /**
     * Checks the presence of the vendor field.
     *
     * @return {@code true} if the vendor field is present; otherwise,
     *         {@code false}
     */
    boolean checkHasVendor();

    /**
     * Returns whether the service is provided directly by a publisher or
     * consolidated from multiple sources.
     * <p>
     * This is an optional field. Call {@link #checkHasIsSource()} before
     * calling this method.
     *
     * @return isSource
     * @throws OmmInvalidUsageException if {@link #checkHasIsSource()} returns
     *                                  {@code false}
     */
    boolean isSource();

    /**
     * Sets whether the service is provided directly by a publisher or
     * consolidated from multiple sources.
     * <p>
     * Calling this method marks the optional {@code isSource} field as present.
     *
     * @param isSource the isSource
     * @return this directory service info filter instance
     */
    DirectoryServiceInfo isSource(boolean isSource);

    /**
     * Checks the presence of the isSource field.
     *
     * @return {@code true} if the {@code isSource} field is present; otherwise,
     *         {@code false}
     */
    boolean checkHasIsSource();

    /**
     * Returns whether items can be requested using a QoS range, meaning both
     * the {@code qos} and {@code worstQos} members of a {@link ReqMsg} may be
     * used.
     * <p>
     * This is an optional field. Call {@link #checkHasSupportsQosRange()}
     * before calling this method.
     *
     * @return {@code true} if QoS range requests are supported; otherwise,
     *         {@code false}
     * @throws OmmInvalidUsageException if
     *                                  {@link #checkHasSupportsQosRange()}
     *                                  returns {@code false}
     */
    boolean supportsQosRange();

    /**
     * Sets whether items can be requested using a QoS range.
     * <p>
     * Calling this method marks the optional {@code supportsQosRange} field as
     * present.
     *
     * @param supportsQosRange the supports qos range
     * @return this directory service info filter instance
     */
    DirectoryServiceInfo supportsQosRange(boolean supportsQosRange);

    /**
     * Checks the presence of the supportsQosRange field.
     *
     * @return {@code true} if the {@code supportsQosRange} field is present;
     *         otherwise, {@code false}
     */
    boolean checkHasSupportsQosRange();

    /**
     * Returns whether snapshot requests, meaning requests without the
     * streaming flag, may be made when the open limit is reached.
     * <p>
     * This is an optional field. Call
     * {@link #checkHasSupportsOutOfBandSnapshots()} before calling this method.
     *
     * @return {@code true} if out-of-band snapshots are supported; otherwise,
     *         {@code false}
     * @throws OmmInvalidUsageException if
     *                                  {@link #checkHasSupportsOutOfBandSnapshots()}
     *                                  returns {@code false}
     */
    boolean supportsOutOfBandSnapshots();

    /**
     * Sets whether snapshot requests may be made when the open limit is
     * reached.
     * <p>
     * Calling this method marks the optional
     * {@code supportsOutOfBandSnapshots} field as present.
     *
     * @param supportsOutOfBandSnapshots the supports out of band snapshots
     * @return this directory service info filter instance
     */
    DirectoryServiceInfo supportsOutOfBandSnapshots(boolean supportsOutOfBandSnapshots);

    /**
     * Checks the presence of the supportsOutOfBandSnapshots field.
     *
     * @return {@code true} if the {@code supportsOutOfBandSnapshots} field is
     *         present; otherwise, {@code false}
     */
    boolean checkHasSupportsOutOfBandSnapshots();

    /**
     * Returns whether the service accepts consumer status messages related to
     * source mirroring.
     * <p>
     * This is an optional field. Call
     * {@link #checkHasAcceptingConsumerStatus()} before calling this method.
     *
     * @return acceptingConsumerStatus
     * @throws OmmInvalidUsageException if
     *                                  {@link #checkHasAcceptingConsumerStatus()}
     *                                  returns {@code false}
     */
    boolean acceptingConsumerStatus();

    /**
     * Sets whether the service accepts consumer status messages related to
     * source mirroring.
     * <p>
     * Calling this method marks the optional
     * {@code acceptingConsumerStatus} field as present.
     *
     * @param acceptingConsumerStatus the accepting consumer status
     * @return this directory service info filter instance
     */
    DirectoryServiceInfo acceptingConsumerStatus(boolean acceptingConsumerStatus);

    /**
     * Checks the presence of the acceptingConsumerStatus field.
     *
     * @return {@code true} if the {@code acceptingConsumerStatus} field is
     *         present; otherwise, {@code false}
     */
    boolean checkHasAcceptingConsumerStatus();

    /**
     * Returns the item-list name a consumer can request to obtain a symbol list
     * of all item names available from this service.
     * <p>
     * This is an optional field. Call {@link #checkHasItemList()} before
     * calling this method.
     *
     * @return itemList
     * @throws OmmInvalidUsageException if {@link #checkHasItemList()} returns
     *                                  {@code false}
     */
    String itemList();

    /**
     * Sets the item-list name for this service.
     * <p>
     * Calling this method marks the optional {@code itemList} field as present.
     *
     * @param itemList the item list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code itemList} is {@code null}
     */
    DirectoryServiceInfo itemList(String itemList);

    /**
     * Checks the presence of the itemList field.
     *
     * @return {@code true} if the {@code itemList} field is present; otherwise,
     *         {@code false}
     */
    boolean checkHasItemList();

    /**
     * Returns the list of capabilities supported by this service.
     * <p>
     * This list is always accessible and represents the mutable list used by
     * this object. Capability values are typically populated using constants
     * defined by {@link EmaRdm}.
     *
     * @return capabilities list
     */
    List<Long> capabilitiesList();

    /**
     * Replaces the current capabilities for this service.
     * <p>
     * The contents of the supplied list are copied into this object. Passing an
     * empty list is allowed and results in an empty capabilities list.
     *
     * @param capabilitiesList the capabilities list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code capabilitiesList} is
     *                                  {@code null}
     */
    DirectoryServiceInfo capabilitiesList(List<Long> capabilitiesList);

    /**
     * Returns the list of dictionary names that this service provides.
     * <p>
     * This is an optional field. Call {@link #checkHasDictionariesProvided()}
     * before calling this method. The returned list is the mutable list used by
     * this object.
     *
     * @return dictionariesProvided list
     * @throws OmmInvalidUsageException if
     *                                  {@link #checkHasDictionariesProvided()}
     *                                  returns {@code false}
     */
    List<String> dictionariesProvidedList();

    /**
     * Replaces the dictionary names provided by this service.
     * <p>
     * The contents of the supplied list are copied into this object. Passing an
     * empty list is allowed and still marks the optional
     * {@code dictionariesProvided} field as present.
     *
     * @param dictionariesProvidedList the dictionaries provided list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code dictionariesProvidedList} is
     *                                  {@code null}
     */
    DirectoryServiceInfo dictionariesProvidedList(List<String> dictionariesProvidedList);

    /**
     * Checks the presence of the dictionariesProvided field.
     *
     * @return {@code true} if the {@code dictionariesProvided} field is
     *         present; otherwise, {@code false}
     */
    boolean checkHasDictionariesProvided();

    /**
     * Returns the list of dictionary names a consumer requires to decode this
     * service's market data.
     * <p>
     * This is an optional field. Call {@link #checkHasDictionariesUsed()}
     * before calling this method. The returned list is the mutable list used by
     * this object.
     *
     * @return list of dictionary names
     * @throws OmmInvalidUsageException if
     *                                  {@link #checkHasDictionariesUsed()}
     *                                  returns {@code false}
     */
    List<String> dictionariesUsedList();

    /**
     * Replaces the dictionary names required to decode this service's market
     * data content.
     * <p>
     * The contents of the supplied list are copied into this object. Passing an
     * empty list is allowed and still marks the optional
     * {@code dictionariesUsed} field as present.
     *
     * @param dictionariesUsedList the dictionaries used list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code dictionariesUsedList} is
     *                                  {@code null}
     */
    DirectoryServiceInfo dictionariesUsedList(List<String> dictionariesUsedList);

    /**
     * Checks the presence of the {@code dictionariesUsed} field.
     *
     * @return {@code true} if the {@code dictionariesUsed} field is present;
     *         otherwise, {@code false}
     */
    boolean checkHasDictionariesUsed();

    /**
     * Returns the list of qualities of service that this service provides.
     * <p>
     * This is an optional field. Call {@link #checkHasQos()} before calling
     * this method. The returned list is the mutable list used by this object.
     *
     * @return qosList
     * @throws OmmInvalidUsageException if {@link #checkHasQos()} returns
     *                                  {@code false}
     */
    List<OmmQos> qosList();

    /**
     * Replaces the qualities of service that this service provides.
     * <p>
     * The contents of the supplied list are copied into this object. Passing an
     * empty list is allowed and still marks the optional {@code qos} field as
     * present.
     *
     * @param qosList the qos list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code qosList} is {@code null}
     */
    DirectoryServiceInfo qosList(List<OmmQos> qosList);

    /**
     * Replaces the qualities of service that this service provides using
     * {@link DirectoryQos} objects.
     * <p>
     * The contents of the supplied list are copied into this object. Passing an
     * empty list is allowed and still marks the optional {@code qos} field as
     * present. Callers must supply instances created by
     * {@link com.refinitiv.ema.access.EmaFactory.Domain#createDirectoryQos()}.
     *
     * @param directoryQosList the directory qos list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code directoryQosList} is {@code null}
     */
    DirectoryServiceInfo qosListFromDirectoryQos(List<DirectoryQos> directoryQosList);

    /**
     * Checks the presence of the qosList field.
     *
     * @return {@code true} if the {@code qos} field is present; otherwise,
     *         {@code false}
     */
    boolean checkHasQos();

    /**
     * Replaces the contents of this object with a deep copy of another service
     * info filter.
     * <p>
     * Implementations typically clear the current content before copying values
     * from {@code sourceServiceInfo}.
     *
     * @param sourceServiceInfo source service info filter to copy from
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceInfo} is {@code null}
     */
    DirectoryServiceInfo copy(DirectoryServiceInfo sourceServiceInfo);
}
