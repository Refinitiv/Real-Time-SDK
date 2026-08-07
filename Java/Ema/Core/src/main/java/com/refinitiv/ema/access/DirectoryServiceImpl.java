/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.*;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.FilterEntryActions;

import java.util.ArrayList;
import java.util.BitSet;
import java.util.Iterator;
import java.util.List;

import static com.refinitiv.ema.access.DirectoryValidators.isValidFilterEntryAction;
import static com.refinitiv.ema.access.DirectoryValidators.isValidMapEntryAction;

/**
 * Internal implementation of a single RDM directory service entry and its optional filters.
 * <p>
 * This implementation stores service filter content separately from the enclosing directory map
 * entry metadata. The {@link #serviceId()} and {@link #action()} values identify the enclosing
 * map entry and are not encoded by {@link #encode()}.
 *
 * @see DirectoryServiceInfo
 * @see DirectoryServiceState
 * @see DirectoryServiceGroup
 * @see DirectoryServiceLoad
 * @see DirectoryServiceData
 * @see DirectoryServiceLinkInfo
 * @see DirectoryService
 */
final class DirectoryServiceImpl implements DirectoryService
{
    private int serviceId = -1;
    private final DirectoryServiceInfo info = new DirectoryServiceInfoImpl();
    private final DirectoryServiceState state = new DirectoryServiceStateImpl();
    private final List<DirectoryServiceGroup> groupStateList = new ArrayList<>();
    private final DirectoryServiceLoad load = new DirectoryServiceLoadImpl();
    private final DirectoryServiceData data = new DirectoryServiceDataImpl();
    private final DirectoryServiceLinkInfo link = new DirectoryServiceLinkInfoImpl();
    private final BitSet flags = new BitSet();
    private int action = MapEntry.MapAction.ADD;

    private final StringBuilder stringBuilder = new StringBuilder();
    private static final int HAS_INFO_FLAG = 0;
    private static final int HAS_DATA_FLAG = 1;
    private static final int HAS_LOAD_FLAG = 2;
    private static final int HAS_LINK_FLAG = 3;
    private static final int HAS_STATE_FLAG = 4;
    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    DirectoryServiceImpl()
    {
    }

    /**
     * Clears all optional filter content from this service.
     * <p>
     * This resets all filter presence flags, clears the group-state list, restores
     * {@link #action()} to {@link MapEntry.MapAction#ADD}, and resets {@link #serviceId()} to
     * its default value of {@code -1}.
     *
     * @return this directory service instance
     */
    @Override
    public DirectoryService clear()
    {
        flags.clear();
        action = MapEntry.MapAction.ADD;
        info.clear();
        state.clear();
        load.clear();
        data.clear();
        link.clear();
        groupStateList.clear();
        serviceId = -1;
        return this;
    }

    /**
     * Returns the action associated with this service's enclosing directory map entry.
     *
     * @return the map entry action for this service
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Sets the action associated with this service's enclosing directory map entry.
     *
     * @param action one of {@link MapEntry.MapAction#ADD},
     *               {@link MapEntry.MapAction#UPDATE}, or
     *               {@link MapEntry.MapAction#DELETE}
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code action} is not one of the
     *                                  supported map entry actions
     */
    @Override
    public DirectoryService action(int action)
    {
        if (!isValidMapEntryAction(action))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + action,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.action = action;
        return this;
    }

    /**
     * Applies info presence flag.
     *
     */
    private void applyHasInfo()
    {
        flags.set(HAS_INFO_FLAG);
    }

    /**
     * Checks the presence of the info field.
     *
     * @return true - if info field exists, false - if not.
     */
    @Override
    public boolean checkHasInfo()
    {
        return flags.get(HAS_INFO_FLAG);
    }

    /**
     * Applies data presence flag.
     *
     */
    private void applyHasData()
    {
        flags.set(HAS_DATA_FLAG);
    }

    /**
     * Checks the presence of the data field.
     *
     * @return true - if data field exists, false - if not.
     */
    @Override
    public boolean checkHasData()
    {
        return flags.get(HAS_DATA_FLAG);
    }

    /**
     * Applies load presence flag.
     *
     */
    private void applyHasLoad()
    {
        flags.set(HAS_LOAD_FLAG);
    }

    /**
     * Checks the presence of the load field.
     *
     * @return true if the load field exists; false otherwise
     */
    @Override
    public boolean checkHasLoad()
    {
        return flags.get(HAS_LOAD_FLAG);
    }

    /**
     * Applies link presence flag.
     *
     */
    private void applyHasLink()
    {
        flags.set(HAS_LINK_FLAG);
    }

    /**
     * Checks the presence of the link field.
     *
     * @return true - if link field exists, false - if not.
     */
    @Override
    public boolean checkHasLink()
    {
        return flags.get(HAS_LINK_FLAG);
    }

    /**
     * Applies state presence flag.
     *
     */
    private void applyHasState()
    {
        flags.set(HAS_STATE_FLAG);
    }

    /**
     * Checks the presence of the state field.
     *
     * @return true - if state field exists, false - if not.
     */
    @Override
    public boolean checkHasState()
    {
        return flags.get(HAS_STATE_FLAG);
    }

    /**
     * Encodes this service's filter payload as an EMA {@link FilterList}.
     * <p>
     * The returned {@link FilterList} contains only the service filters currently present on
     * this object. The enclosing directory map entry metadata, including {@link #serviceId()}
     * and {@link #action()}, is not part of the encoded data produced by this method.
     *
     * @return a {@link FilterList} representing this service's filter payload
     */
    @Override
    public FilterList encode()
    {
        FilterList filterList = EmaFactory.createFilterList();
        if (checkHasInfo())
        {
            filterList.add(serviceFilterEncode(EmaRdm.SERVICE_INFO_ID));
        }

        if (checkHasData())
        {
            filterList.add(serviceFilterEncode(EmaRdm.SERVICE_DATA_ID));
        }

        if (checkHasLink())
        {
            filterList.add(serviceFilterEncode(EmaRdm.SERVICE_LINK_ID));
        }

        if (checkHasLoad())
        {
            filterList.add(serviceFilterEncode(EmaRdm.SERVICE_LOAD_ID));
        }

        if (checkHasState())
        {
            filterList.add(serviceFilterEncode(EmaRdm.SERVICE_STATE_ID));
        }

        encodeGroupFilter(filterList);

        return filterList;
    }

    private FilterEntry serviceFilterEncode(int filterId)
    {
        switch (filterId)
        {
            case EmaRdm.SERVICE_DATA_ID:
                return encodeFilterWithElementList(data);
            case EmaRdm.SERVICE_INFO_ID:
                return encodeFilterWithElementList(info);
            case EmaRdm.SERVICE_LINK_ID:
                return encodeFilterWithMap(link);
            case EmaRdm.SERVICE_LOAD_ID:
                return encodeFilterWithElementList(load);
            case EmaRdm.SERVICE_STATE_ID:
                return encodeFilterWithElementList(state);
            case EmaRdm.SERVICE_GROUP_ID:
            default:
                throw new OmmInvalidUsageExceptionImpl().message("Invalid value of filterId",
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    private FilterEntry encodeFilterWithElementList(DirectoryServiceFilter<ElementList> filter)
    {
        FilterEntry filterEntry = EmaFactory.createFilterEntry();
        if (filter.action() == FilterEntry.FilterAction.CLEAR)
        {
            return filterEntry.noData(filter.filterId(), filter.action());
        }

        return filterEntry.elementList(filter.filterId(), filter.action(), filter.encode());
    }

    private FilterEntry encodeFilterWithMap(DirectoryServiceFilter<Map> filter)
    {
        FilterEntry filterEntry = EmaFactory.createFilterEntry();
        int filterAction = mapFilterAction(filter.action());
        if (filterAction == FilterEntry.FilterAction.CLEAR)
        {
            return filterEntry.noData(filter.filterId(), filterAction);
        }

        return filterEntry.map(filter.filterId(), filterAction, filter.encode());
    }

    private int mapFilterAction(int mapAction)
    {
        if (!isValidMapEntryAction(mapAction))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + mapAction,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        switch (mapAction)
        {
            case MapEntry.MapAction.ADD:
                return FilterEntry.FilterAction.SET;
            case MapEntry.MapAction.UPDATE:
                return FilterEntry.FilterAction.UPDATE;
            case MapEntry.MapAction.DELETE:
                return FilterEntry.FilterAction.CLEAR;
            default:
                throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + mapAction,
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    private int mapMapAction(int filterAction)
    {
        if (!isValidFilterEntryAction(filterAction))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + filterAction,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        switch (filterAction)
        {
            case FilterEntry.FilterAction.SET:
                return MapEntry.MapAction.ADD;
            case FilterEntry.FilterAction.UPDATE:
                return MapEntry.MapAction.UPDATE;
            case FilterEntry.FilterAction.CLEAR:
                return MapEntry.MapAction.DELETE;
            default:
                throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + filterAction,
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    private void encodeGroupFilter(FilterList filterList)
    {
        for (DirectoryServiceGroup group : groupStateList)
        {
            filterList.add(encodeFilterWithElementList(group));
        }
    }

    /**
     * Decodes an EMA {@link FilterList} into this service's filter payload.
     * <p>
     * This implementation preserves the current {@link #serviceId()} and {@link #action()} values,
     * clears the object, decodes only the supplied filter payload, and then restores the preserved
     * metadata. The supplied {@link FilterList} therefore affects only service filter content. If
     * decoding fails, this object remains in inconsistent state and should be cleared for further
     * usage.
     *
     * @param filterList the {@link FilterList} representing a service filter payload
     * @return this directory service instance
     *
     * @throws OmmInvalidUsageException if {@code filterList} is {@code null} or decoding fails
     */
    @Override
    public DirectoryService decode(FilterList filterList)
    {
        if (filterList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("filterList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        int serviceId = this.serviceId;
        int action = this.action;

        clear();

        Iterator<FilterEntry> iterator = filterList.iteratorByRef();
        FilterEntry filterEntry;
        int filterId;
        while(iterator.hasNext())
        {
            filterEntry = iterator.next();
            filterId = filterEntry.filterId();
            switch (filterId)
            {
                case EmaRdm.SERVICE_INFO_ID:
                    if (filterEntry.action() != FilterEntryActions.CLEAR)
                    {
                        info.decode(filterEntry.elementList());
                    }
                    info.action(filterEntry.action());
                    applyHasInfo();
                    break;
                case EmaRdm.SERVICE_STATE_ID:
                    if (filterEntry.action() != FilterEntryActions.CLEAR)
                    {
                        state.decode(filterEntry.elementList());
                    }
                    state.action(filterEntry.action());
                    applyHasState();
                    break;
                case EmaRdm.SERVICE_GROUP_ID:
                    DirectoryServiceGroupImpl groupFilter = new DirectoryServiceGroupImpl();
                    if (filterEntry.action() != FilterEntryActions.CLEAR)
                    {
                        groupFilter.decode(filterEntry.elementList());
                    }
                    groupFilter.action(filterEntry.action());
                    groupStateList.add(groupFilter);
                    break;
                case EmaRdm.SERVICE_LOAD_ID:
                    if (filterEntry.action() != FilterEntryActions.CLEAR)
                    {
                        load.decode(filterEntry.elementList());
                    }
                    load.action(filterEntry.action());
                    applyHasLoad();
                    break;
                case EmaRdm.SERVICE_DATA_ID:
                    if (filterEntry.action() != FilterEntryActions.CLEAR)
                    {
                        data.decode(filterEntry.elementList());
                    }
                    data.action(filterEntry.action());
                    applyHasData();
                    break;
                case EmaRdm.SERVICE_LINK_ID:
                    if (filterEntry.action() != FilterEntryActions.CLEAR)
                    {
                        link.decode(filterEntry.map());
                    }
                    link.action(mapMapAction(filterEntry.action()));
                    applyHasLink();
                    break;
                default:
                    break;
            }
        }

        this.serviceId = serviceId;
        this.action = action;
        return this;
    }

    /**
     * Returns the numeric identifier for this service.
     * <p>
     * This value identifies the enclosing directory map entry and is not part of the
     * {@link FilterList} produced by {@link #encode()} or consumed by {@link #decode(FilterList)}.
     *
     * @return the service identifier
     */
    @Override
    public int serviceId()
    {
        return serviceId;
    }

    /**
     * Sets the numeric identifier for this service.
     * <p>
     * This value identifies the enclosing directory map entry and is not part of the
     * {@link FilterList} produced by {@link #encode()} or consumed by {@link #decode(FilterList)}.
     *
     * @param serviceId the service identifier
     * @return this directory service instance
     */
    @Override
    public DirectoryService serviceId(int serviceId)
    {
        this.serviceId = serviceId;
        return this;
    }

    /**
     * Returns the info filter for this service.
     *
     * @return the info filter
     * @throws OmmInvalidUsageException if {@link #checkHasInfo()} returns {@code false}
     */
    @Override
    public DirectoryServiceInfo info()
    {
        if (!checkHasInfo())
        {
            throw new OmmInvalidUsageExceptionImpl().message("DirectoryServiceInfo element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return info;
    }

    /**
     * Sets the info filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param info the info filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code info} is {@code null}
     */
    @Override
    public DirectoryService info(DirectoryServiceInfo info)
    {
        if (info == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("info can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.info.copy(info);
        applyHasInfo();
        return this;
    }

    /**
     * Returns the state filter for this service.
     *
     * @return the state filter
     * @throws OmmInvalidUsageException if {@link #checkHasState()} returns {@code false}
     */
    @Override
    public DirectoryServiceState state()
    {
        if (!checkHasState())
        {
            throw new OmmInvalidUsageExceptionImpl().message("DirectoryServiceState element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return state;
    }

    /**
     * Sets the state filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param state the service state filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code state} is {@code null}
     */
    @Override
    public DirectoryService state(DirectoryServiceState state)
    {
        if (state == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("state can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.state.copy(state);
        applyHasState();
        return this;
    }

    /**
     * Returns the current group-state entries for this service.
     * <p>
     * The returned list is the mutable list used internally by this service; changes made through
     * the returned list are reflected by this object.
     *
     * @return the current group-state entry list
     */
    @Override
    public List<DirectoryServiceGroup> groupStateList()
    {
        return groupStateList;
    }

    /**
     * Replaces the current group-state entries for this service.
     * <p>
     * Passing an empty list removes all current group-state entries. The supplied list must be
     * non-{@code null}.
     *
     * @param groupStateList the group-state entry list to set
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code groupStateList} is {@code null}
     */
    @Override
    public DirectoryService groupStateList(List<DirectoryServiceGroup> groupStateList)
    {
        if (groupStateList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("groupStateList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this.groupStateList == groupStateList)
        {
            return this;
        }

        this.groupStateList.clear();
        this.groupStateList.addAll(groupStateList);
        return this;
    }

    /**
     * Returns the load filter for this service.
     *
     * @return the load filter
     * @throws OmmInvalidUsageException if {@link #checkHasLoad()} returns {@code false}
     */
    @Override
    public DirectoryServiceLoad load()
    {
        if (!checkHasLoad())
        {
            throw new OmmInvalidUsageExceptionImpl().message("DirectoryServiceLoad element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return load;
    }

    /**
     * Sets the load filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param load the service load filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code load} is {@code null}
     */
    @Override
    public DirectoryService load(DirectoryServiceLoad load)
    {
        if (load == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("load can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.load.copy(load);
        applyHasLoad();
        return this;
    }

    /**
     * Returns the data filter for this service.
     *
     * @return the data filter
     * @throws OmmInvalidUsageException if {@link #checkHasData()} returns {@code false}
     */
    @Override
    public DirectoryServiceData data()
    {
        if (!checkHasData())
        {
            throw new OmmInvalidUsageExceptionImpl().message("DirectoryServiceData element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return data;
    }

    /**
     * Sets the data filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param data the service data filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code data} is {@code null}
     */
    @Override
    public DirectoryService data(DirectoryServiceData data)
    {
        if (data == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("data can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.data.copy(data);
        applyHasData();
        return this;
    }

    /**
     * Returns the link filter for this service.
     *
     * @return the link filter
     * @throws OmmInvalidUsageException if {@link #checkHasLink()} returns {@code false}
     */
    @Override
    public DirectoryServiceLinkInfo link()
    {
        if (!checkHasLink())
        {
            throw new OmmInvalidUsageExceptionImpl().message("DirectoryServiceLinkInfo element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return link;
    }

    /**
     * Sets the link filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param link the service link filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code link} is {@code null}
     */
    @Override
    public DirectoryService link(DirectoryServiceLinkInfo link)
    {
        if (link == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("link can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.link.copy(link);
        applyHasLink();
        return this;
    }

    /**
     * Performs a deep copy of another {@link DirectoryService} into this object.
     * <p>
     * If {@code sourceService} is this object, the call is a no-op. Otherwise, this
     * implementation clears this service and replaces it with the source service's current state,
     * including {@link #action()}, {@link #serviceId()}, all filters present on the source, and
     * copied group-state entries. Filters absent on the source are removed from this object.
     *
     * @param sourceService the service to copy from
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code sourceService} is {@code null}
     */
    @Override
    public DirectoryService copy(DirectoryService sourceService)
    {
        if (sourceService == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceService can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceService == this)
        {
            return this;
        }

        clear();

        action(sourceService.action());
        serviceId(sourceService.serviceId());
        if (sourceService.checkHasInfo())
        {
            info.copy(sourceService.info());
            applyHasInfo();
        }
        if (sourceService.checkHasData())
        {
            data.copy(sourceService.data());
            applyHasData();
        }
        for (DirectoryServiceGroup group : sourceService.groupStateList())
        {
            DirectoryServiceGroup newGroup = new DirectoryServiceGroupImpl();
            newGroup.copy(group);
            groupStateList.add(newGroup);
        }
        if (sourceService.checkHasLink())
        {
            link.copy(sourceService.link());
            applyHasLink();
        }
        if (sourceService.checkHasLoad())
        {
            load.copy(sourceService.load());
            applyHasLoad();
        }
        if (sourceService.checkHasState())
        {
            state.copy(sourceService.state());
            applyHasState();
        }
        return this;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);
        stringBuilder.append(TAB);
        stringBuilder.append("Service:");
        stringBuilder.append(EOL);

        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("serviceId: ");
        stringBuilder.append(serviceId());
        stringBuilder.append(EOL);

        if (checkHasInfo())
        {
            stringBuilder.append(info());
        }

        if (checkHasData())
        {
            stringBuilder.append(data());
        }

        if (checkHasLink())
        {
            stringBuilder.append(link());
        }

        if (checkHasState())
        {
            stringBuilder.append(state());
        }

        if (checkHasLoad())
        {
            stringBuilder.append(load());
        }

        if (!groupStateList().isEmpty())
        {
            stringBuilder.append(groupStateList());
        }

        return stringBuilder.toString();
    }
}
