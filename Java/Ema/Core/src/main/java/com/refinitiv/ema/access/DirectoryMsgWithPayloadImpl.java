/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryMsgWithFilter;
import com.refinitiv.ema.domain.directory.DirectoryMsgWithPayload;
import com.refinitiv.ema.domain.directory.DirectoryService;

import java.util.ArrayList;
import java.util.List;

/**
 * The RDM Directory Base Message with payload, that can be transformed into list of RDM Services
 *
 * @see DirectoryMsgWithFilterImpl
 * @see DirectoryMsgWithPayload
 * @see DirectoryService
 */
abstract class DirectoryMsgWithPayloadImpl<T> extends DirectoryMsgWithFilterImpl<T> implements DirectoryMsgWithPayload<T>
{
    private final Map payload = EmaFactory.createMap();
    private final List<DirectoryService> serviceList = new ArrayList<>();

    /**
     * Clears the current contents of this directory message, including the stored
     * service entries and encoded payload, and prepares it for reuse.
     *
     * @return this directory message instance
     */
    @Override
    public DirectoryMsgWithFilter<T> clear()
    {
        super.clear();
        serviceList.clear();
        payload.clear();
        return this;
    }

    protected Map payload()
    {
        return payload;
    }

    protected DirectoryService service(int serviceId)
    {
        for (DirectoryService service : serviceList)
        {
            if (service.serviceId() == serviceId)
                return service;
        }
        return null;
    }

    /**
     * List of service entries.
     *
     * @return service list
     */
    @Override
    public List<DirectoryService> serviceList()
    {
        return serviceList;
    }

    /**
     * Sets service entries into the directory message. This object's
     * Service elements will be set to Service elements from list in the
     * parameter passed in.
     *
     * @param serviceList -list of service entries.
     * @return this directory message instance
     * @throws OmmInvalidUsageException if List parameter is null
     */
    @Override
    public DirectoryMsgWithPayload<T> serviceList(List<DirectoryService> serviceList)
    {
        if (serviceList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("serviceList must be non-null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this.serviceList == serviceList)
        {
            return this;
        }

        this.serviceList.clear();
        this.serviceList.addAll(serviceList);
        return this;
    }

    protected void decodeServiceList(Map map)
    {
        for (MapEntry mapEntry : map)
        {
            if (mapEntry.key().data().dataType() != DataType.DataTypes.UINT)
            {
                throw new OmmInvalidUsageExceptionImpl().message("Unexpected map entry key type: " +
                                DataType.asString(mapEntry.key().data().dataType()),
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
            }

            if (mapEntry.loadType() == DataType.DataTypes.FILTER_LIST ||
                    (mapEntry.action() == MapEntry.MapAction.DELETE && mapEntry.loadType() == DataType.DataTypes.NO_DATA))
            {
                int serviceId = (int) mapEntry.key().uintValue();
                DirectoryService service = service(serviceId);
                if(service == null)
                {
                    service = new DirectoryServiceImpl();
                    serviceList.add(service);
                }
                if (mapEntry.loadType() == DataType.DataTypes.FILTER_LIST)
                {
                    service.decode(mapEntry.filterList());
                }
                service.serviceId(serviceId);
                service.action(mapEntry.action());
            }
            else
            {
                throw new OmmInvalidUsageExceptionImpl().message("Unexpected map entry payload: " +
                                DataType.asString(mapEntry.loadType()),
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
            }
        }
    }

    protected void encodeServiceList()
    {
        payload.clear();
        payload.keyType(DataType.DataTypes.UINT);

        for (DirectoryService service : serviceList)
        {
            MapEntry mapEntry = EmaFactory.createMapEntry();
            mapEntry.keyUInt(service.serviceId(), service.action(), service.encode());

            payload.add(mapEntry);
        }
    }
}
