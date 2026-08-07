/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryQos;
import com.refinitiv.ema.domain.directory.DirectoryServiceInfo;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Qos;

import java.util.ArrayList;
import java.util.BitSet;
import java.util.Iterator;
import java.util.List;

import static com.refinitiv.ema.access.DirectoryValidators.isValidFilterEntryAction;

/**
 * Implementation of the RDM Source Directory service info filter.
 * <p>
 * This object stores the identifying information and content metadata carried
 * by the Source Directory Info filter, including service name, capabilities,
 * dictionaries, QoS, and optional boolean attributes.
 *
 * @see DirectoryServiceInfo
 */
final class DirectoryServiceInfoImpl implements DirectoryServiceInfo
{
    private final BitSet flags = new BitSet();
    private final List<Long> capabilitiesList = new ArrayList<>();
    private final List<String> dictionariesProvidedList = new ArrayList<>();
    private final List<String> dictionariesUsedList = new ArrayList<>();
    private final List<OmmQos> qosList = new ArrayList<>();
    private String itemList;
    private String vendor;
    private String serviceName;
    private int action;

    private final StringBuilder stringBuilder = new StringBuilder();

    private static final int HAS_VENDOR_FLAG = 0;
    private static final int IS_SOURCE_FLAG = 1;
    private static final int HAS_IS_SOURCE_FLAG = 2;
    private static final int SUPPORTS_QOS_RANGE_FLAG = 3;
    private static final int HAS_SUPPORTS_QOS_RANGE_FLAG = 4;
    private static final int SUPPORTS_OUT_OF_BAND_SNAPSHOTS_FLAG = 5;
    private static final int HAS_SUPPORTS_OUT_OF_BAND_SNAPSHOTS_FLAG = 6;
    private static final int ACCEPTING_CONSUMER_STATUS_FLAG = 7;
    private static final int HAS_ACCEPTING_CONSUMER_STATUS_FLAG = 8;
    private static final int HAS_ITEM_LIST_FLAG = 9;
    private static final int HAS_DICTIONARIES_PROVIDED_FLAG = 10;
    private static final int HAS_DICTIONARIES_USED_FLAG = 11;
    private static final int HAS_QOS_FLAG = 12;
    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    /**
     * Creates a service info filter initialized to its default state.
     */
    public DirectoryServiceInfoImpl()
    {
        clear();
    }

    /**
     * Resets this service info filter to its default state.
     * <p>
     * All optional-field presence flags are cleared, {@code action} is reset to
     * {@link FilterEntry.FilterAction#SET}, lists are emptied, and string fields
     * are set to empty strings. The internal default values for
     * {@code isSource}, {@code supportsOutOfBandSnapshots}, and
     * {@code acceptingConsumerStatus} are restored, but their corresponding
     * {@code checkHas*()} methods still return {@code false} until those fields
     * are explicitly set.
     *
     * @return this directory service info filter instance
     */
    @Override
    public DirectoryServiceInfo clear()
    {
        flags.clear();
        flags.set(IS_SOURCE_FLAG); // isSource = true
        flags.set(SUPPORTS_OUT_OF_BAND_SNAPSHOTS_FLAG); // supportsOutOfBandSnapshots = true
        flags.set(ACCEPTING_CONSUMER_STATUS_FLAG); // acceptingConsumerStatus = true
        action = FilterEntry.FilterAction.SET;
        capabilitiesList.clear();
        dictionariesProvidedList.clear();
        dictionariesUsedList.clear();
        qosList.clear();
        serviceName = "";
        itemList = "";
        vendor = "";
        return this;
    }

    /**
     * Returns the service name that identifies this service.
     * <p>
     * This value is always accessible. After {@link #clear()}, the default value
     * is an empty string.
     *
     * @return service name
     */
    @Override
    public String serviceName()
    {
        return serviceName;
    }

    /**
     * Sets the service name that identifies this service.
     *
     * @param serviceName the service name
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code serviceName} is {@code null}
     */
    @Override
    public DirectoryServiceInfo serviceName(String serviceName)
    {
        if (serviceName == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("serviceName can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.serviceName = serviceName;
        return this;
    }

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
    @Override
    public String vendor()
    {
        if (!checkHasVendor())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_VENDOR + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return vendor;
    }

    /**
     * Sets the vendor name for this service.
     * <p>
     * Calling this method marks the optional {@code vendor} field as present.
     *
     * @param vendor the vendor
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code vendor} is {@code null}
     */
    @Override
    public DirectoryServiceInfo vendor(String vendor)
    {
        if (vendor == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("vendor can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.vendor = vendor;
        applyHasVendor();
        return this;
    }

    /**
     * Marks the optional vendor field as present.
     */
    private void applyHasVendor()
    {
        flags.set(HAS_VENDOR_FLAG);
    }

    /**
     * Checks whether the optional vendor field is present.
     *
     * @return {@code true} if the vendor field is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasVendor()
    {
        return flags.get(HAS_VENDOR_FLAG);
    }

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
    @Override
    public boolean isSource()
    {
        if (!checkHasIsSource())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_IS_SOURCE + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return flags.get(IS_SOURCE_FLAG);
    }

    /**
     * Sets whether the service is provided directly by a publisher or
     * consolidated from multiple sources.
     * <p>
     * Calling this method marks the optional {@code isSource} field as present.
     *
     * @param isSource the isSource
     * @return this directory service info filter instance
     */
    @Override
    public DirectoryServiceInfo isSource(boolean isSource)
    {
        if (isSource)
        {
            flags.set(IS_SOURCE_FLAG);
        } else
        {
            flags.clear(IS_SOURCE_FLAG);
        }
        applyHasIsSource();
        return this;
    }

    /**
     * Marks the optional {@code isSource} field as present.
     */
    private void applyHasIsSource()
    {
        flags.set(HAS_IS_SOURCE_FLAG);
    }

    /**
     * Checks whether the optional {@code isSource} field is present.
     *
     * @return {@code true} if the {@code isSource} field is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasIsSource()
    {
        return flags.get(HAS_IS_SOURCE_FLAG);
    }

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
    @Override
    public boolean supportsQosRange()
    {
        if (!checkHasSupportsQosRange())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_SUPPS_QOS_RANGE + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return flags.get(SUPPORTS_QOS_RANGE_FLAG);
    }

    /**
     * Sets whether items can be requested using a QoS range.
     * <p>
     * Calling this method marks the optional {@code supportsQosRange} field as
     * present.
     *
     * @param supportsQosRange the supports qos range
     * @return this directory service info filter instance
     */
    @Override
    public DirectoryServiceInfo supportsQosRange(boolean supportsQosRange)
    {
        if (supportsQosRange)
        {
            flags.set(SUPPORTS_QOS_RANGE_FLAG);
        } else
        {
            flags.clear(SUPPORTS_QOS_RANGE_FLAG);
        }
        applyHasSupportsQosRange();
        return this;
    }

    /**
     * Marks the optional {@code supportsQosRange} field as present.
     */
    private void applyHasSupportsQosRange()
    {
        flags.set(HAS_SUPPORTS_QOS_RANGE_FLAG);
    }

    /**
     * Checks whether the optional {@code supportsQosRange} field is present.
     *
     * @return {@code true} if the {@code supportsQosRange} field is present;
     *         otherwise, {@code false}
     */
    @Override
    public boolean checkHasSupportsQosRange()
    {
        return flags.get(HAS_SUPPORTS_QOS_RANGE_FLAG);
    }

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
    @Override
    public boolean supportsOutOfBandSnapshots()
    {
        if (!checkHasSupportsOutOfBandSnapshots())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return flags.get(SUPPORTS_OUT_OF_BAND_SNAPSHOTS_FLAG);
    }

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
    @Override
    public DirectoryServiceInfo supportsOutOfBandSnapshots(boolean supportsOutOfBandSnapshots)
    {
        if (supportsOutOfBandSnapshots)
        {
            flags.set(SUPPORTS_OUT_OF_BAND_SNAPSHOTS_FLAG);
        } else
        {
            flags.clear(SUPPORTS_OUT_OF_BAND_SNAPSHOTS_FLAG);
        }
        applyHasSupportsOutOfBandSnapshots();
        return this;
    }

    /**
     * Marks the optional {@code supportsOutOfBandSnapshots} field as present.
     */
    private void applyHasSupportsOutOfBandSnapshots()
    {
        flags.set(HAS_SUPPORTS_OUT_OF_BAND_SNAPSHOTS_FLAG);
    }

    /**
     * Checks whether the optional {@code supportsOutOfBandSnapshots} field is
     * present.
     *
     * @return {@code true} if the {@code supportsOutOfBandSnapshots} field is
     *         present; otherwise, {@code false}
     */
    @Override
    public boolean checkHasSupportsOutOfBandSnapshots()
    {
        return flags.get(HAS_SUPPORTS_OUT_OF_BAND_SNAPSHOTS_FLAG);
    }

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
    @Override
    public boolean acceptingConsumerStatus()
    {
        if (!checkHasAcceptingConsumerStatus())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_ACCEPTING_CONS_STATUS + 
                            " element is not set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return flags.get(ACCEPTING_CONSUMER_STATUS_FLAG);
    }

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
    @Override
    public DirectoryServiceInfo acceptingConsumerStatus(boolean acceptingConsumerStatus)
    {
        if (acceptingConsumerStatus)
        {
            flags.set(ACCEPTING_CONSUMER_STATUS_FLAG);
        } else
        {
            flags.clear(ACCEPTING_CONSUMER_STATUS_FLAG);
        }
        applyHasAcceptingConsumerStatus();
        return this;
    }

    /**
     * Marks the optional {@code acceptingConsumerStatus} field as present.
     */
    private void applyHasAcceptingConsumerStatus()
    {
        flags.set(HAS_ACCEPTING_CONSUMER_STATUS_FLAG);
    }

    /**
     * Checks whether the optional {@code acceptingConsumerStatus} field is
     * present.
     *
     * @return {@code true} if the {@code acceptingConsumerStatus} field is
     *         present; otherwise, {@code false}
     */
    @Override
    public boolean checkHasAcceptingConsumerStatus()
    {
        return flags.get(HAS_ACCEPTING_CONSUMER_STATUS_FLAG);
    }

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
    @Override
    public String itemList()
    {
        if (!checkHasItemList())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_ITEM_LIST + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return itemList;
    }


    /**
     * Sets the item-list name for this service.
     * <p>
     * Calling this method marks the optional {@code itemList} field as present.
     *
     * @param itemList the item list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code itemList} is {@code null}
     */
    @Override
    public DirectoryServiceInfo itemList(String itemList)
    {
        if (itemList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("itemList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.itemList = itemList;
        applyHasItemList();
        return this;
    }

    /**
     * Marks the optional {@code itemList} field as present.
     */
    private void applyHasItemList()
    {
        flags.set(HAS_ITEM_LIST_FLAG);
    }

    /**
     * Checks whether the optional {@code itemList} field is present.
     *
     * @return {@code true} if the {@code itemList} field is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasItemList()
    {
        return flags.get(HAS_ITEM_LIST_FLAG);
    }

    /**
     * Returns the list of capabilities supported by this service.
     * <p>
     * This list is always accessible and represents the mutable list used by
     * this object. Capability values are typically populated using constants
     * defined by {@link EmaRdm}.
     *
     * @return capabilities list
     */
    @Override
    public List<Long> capabilitiesList()
    {
        return capabilitiesList;
    }

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
    @Override
    public DirectoryServiceInfo capabilitiesList(List<Long> capabilitiesList)
    {
        if (capabilitiesList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("capabilitiesList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this.capabilitiesList == capabilitiesList)
        {
            return this;
        }

        this.capabilitiesList.clear();
        this.capabilitiesList.addAll(capabilitiesList);
        return this;
    }

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
    @Override
    public List<String> dictionariesProvidedList()
    {
        if (!checkHasDictionariesProvided())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_DICTIONARYS_PROVIDED +
                            " element is not set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return dictionariesProvidedList;
    }

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
    @Override
    public DirectoryServiceInfo dictionariesProvidedList(List<String> dictionariesProvidedList)
    {
        if (dictionariesProvidedList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("dictionariesProvidedList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this.dictionariesProvidedList == dictionariesProvidedList)
        {
            return this;
        }

        this.dictionariesProvidedList.clear();
        this.dictionariesProvidedList.addAll(dictionariesProvidedList);
        applyHasDictionariesProvided();
        return this;
    }

    /**
     * Marks the optional {@code dictionariesProvided} field as present.
     */
    private void applyHasDictionariesProvided()
    {
        flags.set(HAS_DICTIONARIES_PROVIDED_FLAG);
    }

    /**
     * Checks whether the optional {@code dictionariesProvided} field is
     * present.
     *
     * @return {@code true} if the {@code dictionariesProvided} field is
     *         present; otherwise, {@code false}
     */
    @Override
    public boolean checkHasDictionariesProvided()
    {
        return flags.get(HAS_DICTIONARIES_PROVIDED_FLAG);
    }

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
    @Override
    public List<String> dictionariesUsedList()
    {
        if (!checkHasDictionariesUsed())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_DICTIONARYS_USED +
                    " element is not set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return dictionariesUsedList;
    }

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
    @Override
    public DirectoryServiceInfo dictionariesUsedList(List<String> dictionariesUsedList)
    {
        if (dictionariesUsedList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("dictionariesUsedList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this.dictionariesUsedList == dictionariesUsedList)
        {
            return this;
        }

        this.dictionariesUsedList.clear();
        this.dictionariesUsedList.addAll(dictionariesUsedList);
        applyHasDictionariesUsed();
        return this;
    }

    /**
     * Marks the optional {@code dictionariesUsed} field as present.
     */
    private void applyHasDictionariesUsed()
    {
        flags.set(HAS_DICTIONARIES_USED_FLAG);
    }

    /**
     * Checks whether the optional {@code dictionariesUsed} field is present.
     *
     * @return {@code true} if the {@code dictionariesUsed} field is present;
     *         otherwise, {@code false}
     */
    @Override
    public boolean checkHasDictionariesUsed()
    {
        return flags.get(HAS_DICTIONARIES_USED_FLAG);
    }

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
    @Override
    public List<OmmQos> qosList()
    {
        if (!checkHasQos())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_QOS +
                    " element is not set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return qosList;
    }

    /**
     * Replaces the qualities of service that this service provides.
     * <p>
     * The contents of the supplied list are deep-copied into this object.
     * Passing an empty list is allowed and still marks the optional
     * {@code qos} field as present.
     *
     * @param qosList the qos list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code qosList} is {@code null}
     */
    @Override
    public DirectoryServiceInfo qosList(List<OmmQos> qosList)
    {
        if (qosList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("qosList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this.qosList == qosList)
        {
            return this;
        }

        this.qosList.clear();

        // Deep copy
        for (OmmQos qos : qosList)
        {
            this.qosList.add(copyQos(qos));
        }
        applyHasQos();
        return this;
    }

    /**
     * Replaces the qualities of service that this service provides using
     * {@link DirectoryQos} objects.
     * <p>
     * The contents of the supplied list are copied into this object. Passing an
     * empty list is allowed and still marks the optional {@code qos} field as
     * present. Callers must supply instances created by
     * {@link EmaFactory.Domain#createDirectoryQos()}.
     *
     * @param directoryQosList the directory qos list
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code directoryQosList} is {@code null}
     */
    @Override
    public DirectoryServiceInfo qosListFromDirectoryQos(List<DirectoryQos> directoryQosList)
    {
        if (directoryQosList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("directoryQosList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.qosList.clear();

        for (DirectoryQos qos : directoryQosList)
        {
            this.qosList.add(((DirectoryQosImpl) qos).ommQos());
        }
        applyHasQos();
        return this;
    }

    private OmmQos copyQos(OmmQos qos)
    {
        Qos etaQos = CodecFactory.createQos();
        Utilities.toRsslQos(qos.rate(), qos.timeliness(), etaQos);

        OmmQosImpl emaQos = new OmmQosImpl();
        emaQos.decode(etaQos);

        return emaQos;
    }

    /**
     * Marks the optional {@code qos} field as present.
     */
    private void applyHasQos()
    {
        flags.set(HAS_QOS_FLAG);
    }

    /**
     * Checks whether the optional {@code qos} field is present.
     *
     * @return {@code true} if the {@code qos} field is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasQos()
    {
        return flags.get(HAS_QOS_FLAG);
    }

    /**
     * Returns the filter action associated with this service info filter.
     *
     * @return action value from {@link FilterEntry.FilterAction}
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Sets the filter action associated with this service info filter.
     *
     * @param action value from {@link FilterEntry.FilterAction}
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code action} is not a valid
     *                                  {@link FilterEntry.FilterAction} value
     */
    @Override
    public DirectoryServiceInfo action(int action)
    {
        if (!isValidFilterEntryAction(action))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + action,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.action = action;
        return this;
    }

    /**
     * Returns the RDM filter identifier for the service info filter.
     *
     * @return {@link EmaRdm#SERVICE_INFO_ID}
     */
    @Override
    public int filterId()
    {
        return EmaRdm.SERVICE_INFO_ID;
    }

    /**
     * Encodes this service info filter into an {@link ElementList}.
     * <p>
     * The encoded payload always contains the service name and capabilities.
     * Optional fields are included only when their corresponding
     * {@code checkHas*()} method returns {@code true}.
     *
     * @return element list representing this service info filter
     */
    @Override
    public ElementList encode()
    {

        ElementList elementList = EmaFactory.createElementList();

        elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, serviceName));

        if (checkHasVendor())
        {
            elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_VENDOR, vendor));
        }

        if (checkHasIsSource())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_IS_SOURCE,
                    isSource() ? 1 : 0));
        }

        OmmArray capabilities = EmaFactory.createOmmArray();
        for (long capability : capabilitiesList)
        {
            capabilities.add(EmaFactory.createOmmArrayEntry().uintValue(capability));
        }
        elementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capabilities));

        if (checkHasDictionariesProvided())
        {
            OmmArray dictionariesProvided = EmaFactory.createOmmArray();
            for (String dictionary : dictionariesProvidedList)
            {
                dictionariesProvided.add(EmaFactory.createOmmArrayEntry().ascii(dictionary));
            }
            elementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_PROVIDED, dictionariesProvided));
        }

        if (checkHasDictionariesUsed())
        {
            OmmArray dictionariesUsed = EmaFactory.createOmmArray();
            for (String dictionary : dictionariesUsedList)
            {
                dictionariesUsed.add(EmaFactory.createOmmArrayEntry().ascii(dictionary));
            }
            elementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionariesUsed));
        }

        if (checkHasQos())
        {
            OmmArray qosArray = EmaFactory.createOmmArray();
            for (OmmQos qos : qosList)
            {
                qosArray.add(EmaFactory.createOmmArrayEntry().qos(qos.timeliness(), qos.rate()));
            }
            elementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_QOS, qosArray));
        }

        if (checkHasSupportsQosRange())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPS_QOS_RANGE,
                    supportsQosRange() ? 1 : 0));
        }

        if (checkHasItemList())
        {
            elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ITEM_LIST, itemList));
        }

        if (checkHasSupportsOutOfBandSnapshots())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS,
                    supportsOutOfBandSnapshots() ? 1 : 0));
        }

        if (checkHasAcceptingConsumerStatus())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_CONS_STATUS,
                    acceptingConsumerStatus() ? 1 : 0));
        }

        return elementList;
    }

    /**
     * Decodes an {@link ElementList} into this service info filter.
     * <p>
     * The current content is cleared before decoding begins. The input must
     * contain both {@link EmaRdm#ENAME_NAME} and
     * {@link EmaRdm#ENAME_CAPABILITIES}. Optional fields are applied only when
     * present and non-blank. If decoding fails or a required element is absent,
     * an {@link OmmInvalidUsageException} is thrown; this object remains in
     * inconsistent state and should be cleared for further usage.
     *
     * @param elementList element list representing a service info filter
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code elementList} is {@code null},
     *                                  contains invalid values, or omits
     *                                  required elements
     */
    @Override
    public DirectoryServiceInfo decode(ElementList elementList)
    {
        if (elementList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("elementList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        Iterator<ElementEntry> iterator = elementList.iteratorByRef();
        boolean foundServiceName = false;
        boolean foundCapabilities = false;
        while (iterator.hasNext())
        {
            ElementEntry elementEntry = iterator.next();
            String elementName = elementEntry.name();

            switch (elementName)
            {
                case EmaRdm.ENAME_NAME:
                    if(elementEntry.code() != Data.DataCode.BLANK)
                    {
                        serviceName(elementEntry.ascii().ascii());
                        foundServiceName = true;
                    }
                    break;
                case EmaRdm.ENAME_VENDOR:
                    if(elementEntry.code() != Data.DataCode.BLANK)
                    {
                        vendor(elementEntry.ascii().ascii());
                    }
                    break;
                case EmaRdm.ENAME_IS_SOURCE:
                    long isSource = elementEntry.uintValue();
                    if (isSource == 0 || isSource == 1)
                    {
                        isSource(isSource == 1);
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    break;
                case EmaRdm.ENAME_CAPABILITIES:
                    if(elementEntry.code() != Data.DataCode.BLANK)
                    {
                        OmmArray capabilities = elementEntry.array();
                        for (OmmArrayEntry capability : capabilities)
                        {
                            capabilitiesList.add(capability.uintValue());
                        }
                        foundCapabilities = true;
                    }
                    break;
                case EmaRdm.ENAME_DICTIONARYS_PROVIDED:
                    if(elementEntry.code() != Data.DataCode.BLANK)
                    {
                        OmmArray dictionaries = elementEntry.array();
                        for (OmmArrayEntry dictionary : dictionaries)
                        {
                            dictionariesProvidedList.add(dictionary.ascii().ascii());
                        }
                        applyHasDictionariesProvided();
                    }
                    break;
                case EmaRdm.ENAME_DICTIONARYS_USED:
                    if(elementEntry.code() != Data.DataCode.BLANK)
                    {
                        OmmArray dictionaries = elementEntry.array();
                        for (OmmArrayEntry dictionary : dictionaries)
                        {
                            dictionariesUsedList.add(dictionary.ascii().ascii());
                        }
                        applyHasDictionariesUsed();
                    }
                    break;
                case EmaRdm.ENAME_QOS:
                    if(elementEntry.code() != Data.DataCode.BLANK)
                    {
                        OmmArray qosArray = elementEntry.array();
                        for (OmmArrayEntry qos : qosArray)
                        {
                            qosList.add(copyQos(qos.qos()));
                        }
                        applyHasQos();
                    }
                    break;
                case EmaRdm.ENAME_SUPPS_QOS_RANGE:
                    long supportsQosRange = elementEntry.uintValue();
                    if (supportsQosRange == 0 || supportsQosRange == 1)
                    {
                        supportsQosRange(supportsQosRange == 1);
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    break;
                case EmaRdm.ENAME_ITEM_LIST:
                    if(elementEntry.code() != Data.DataCode.BLANK)
                    {
                        itemList(elementEntry.ascii().ascii());
                    }
                    break;
                case EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS:
                    long supportsOOBSnapshots = elementEntry.uintValue();
                    if (supportsOOBSnapshots == 0 || supportsOOBSnapshots == 1)
                    {
                        supportsOutOfBandSnapshots(supportsOOBSnapshots == 1);
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    break;
                case EmaRdm.ENAME_ACCEPTING_CONS_STATUS:
                    long acceptingConsumerStatus = elementEntry.uintValue();
                    if (acceptingConsumerStatus == 0 || acceptingConsumerStatus == 1)
                    {
                        acceptingConsumerStatus(acceptingConsumerStatus == 1);
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    break;
                default:
                    break;
            }
        }

        if(!foundServiceName || !foundCapabilities)
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_NAME + " or " +
                    EmaRdm.ENAME_CAPABILITIES + " element is absent", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
        return this;
    }

    /**
     * Replaces the contents of this object with a deep copy of another service
     * info filter.
     * <p>
     * This method clears the current content before copying values from
     * {@code sourceServiceInfo}.
     *
     * @param sourceServiceInfo source service info filter to copy from
     * @return this directory service info filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceInfo} is {@code null}
     */
    @Override
    public DirectoryServiceInfo copy(DirectoryServiceInfo sourceServiceInfo)
    {
        if (sourceServiceInfo == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceServiceInfo can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceServiceInfo == this)
        {
            return this;
        }

        clear();

        serviceName(sourceServiceInfo.serviceName());
        action(sourceServiceInfo.action());
        capabilitiesList(sourceServiceInfo.capabilitiesList());

        if (sourceServiceInfo.checkHasAcceptingConsumerStatus())
        {
            acceptingConsumerStatus(sourceServiceInfo.acceptingConsumerStatus());
        }

        if (sourceServiceInfo.checkHasDictionariesProvided())
        {
            dictionariesProvidedList(sourceServiceInfo.dictionariesProvidedList());
        }

        if (sourceServiceInfo.checkHasDictionariesUsed())
        {
            dictionariesUsedList(sourceServiceInfo.dictionariesUsedList());
        }

        if (sourceServiceInfo.checkHasIsSource())
        {
            isSource(sourceServiceInfo.isSource());
        }

        if (sourceServiceInfo.checkHasItemList())
        {
            itemList(sourceServiceInfo.itemList());
        }

        if (sourceServiceInfo.checkHasQos())
        {
            qosList(sourceServiceInfo.qosList());
        }

        if (sourceServiceInfo.checkHasSupportsOutOfBandSnapshots())
        {
            supportsOutOfBandSnapshots(sourceServiceInfo.supportsOutOfBandSnapshots());
        }

        if (sourceServiceInfo.checkHasSupportsQosRange())
        {
            supportsQosRange(sourceServiceInfo.supportsQosRange());
        }

        if (sourceServiceInfo.checkHasVendor())
        {
            vendor(sourceServiceInfo.vendor());
        }
        return this;
    }

    /**
     * Returns a human-readable representation of this service info filter.
     *
     * @return formatted service info contents
     */
    @Override
    public String toString()
    {
        stringBuilder.setLength(0);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("InfoFilter:");
        stringBuilder.append(EOL);

        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("serviceName: ");
        stringBuilder.append(serviceName());
        stringBuilder.append(EOL);

        if (checkHasVendor())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("vendor: ");
            stringBuilder.append(vendor());
            stringBuilder.append(EOL);
        }

        if (checkHasIsSource())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("isSource: ");
            stringBuilder.append(isSource());
            stringBuilder.append(EOL);
        }

        if (checkHasSupportsQosRange())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("supportsQosRange: ");
            stringBuilder.append(supportsQosRange());
            stringBuilder.append(EOL);
        }

        if (checkHasSupportsOutOfBandSnapshots())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("supportsOutOfBandSnapshots: ");
            stringBuilder.append(supportsOutOfBandSnapshots());
            stringBuilder.append(EOL);
        }

        if (checkHasAcceptingConsumerStatus())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("acceptingConsumerStatus: ");
            stringBuilder.append(acceptingConsumerStatus());
            stringBuilder.append(EOL);
        }

        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("capabilities: ");
        stringBuilder.append(capabilitiesList());
        stringBuilder.append(EOL);

        if (checkHasDictionariesProvided())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("dictionariesProvided: ");
            stringBuilder.append(dictionariesProvidedList());
            stringBuilder.append(EOL);
        }

        if (checkHasDictionariesUsed())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("dictionariesUsed: ");
            stringBuilder.append(dictionariesUsedList());
            stringBuilder.append(EOL);
        }

        if (checkHasQos())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("qos: ");
            stringBuilder.append(qosList());
            stringBuilder.append(EOL);
        }

        return stringBuilder.toString();
    }
}
