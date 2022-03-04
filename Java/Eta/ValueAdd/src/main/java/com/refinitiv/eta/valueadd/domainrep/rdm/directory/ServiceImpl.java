/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FilterEntry;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.FilterEntryFlags;
import com.refinitiv.eta.codec.FilterList;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.rdm.Directory;

/*
 * Implementation of Directory Service
 */
class ServiceImpl implements Service
{
    private int serviceId;
    private ServiceInfo info;
    private ServiceState state;
    private List<Service.ServiceGroup> groupStateList;
    private ServiceLoad load;
    private ServiceData data;
    private ServiceLinkInfo link;
    private ServiceSeqMcastInfo seqMcast;
    private int flags;
    private int action;
   
    private StringBuilder stringBuf = new StringBuilder();
    private final static String eol = "\n";
    private final static String tab = "\t";

    private FilterEntry filterEntry = CodecFactory.createFilterEntry();
    private FilterList filterList = CodecFactory.createFilterList();
    
    ServiceImpl()
    {
        info = new ServiceInfo();
        state = new ServiceState();
        groupStateList = new ArrayList<Service.ServiceGroup>();
        load = new ServiceLoad();
        data = new Service.ServiceData();
        link = new ServiceLinkInfo();
        seqMcast = new ServiceSeqMcastInfo();
        action = MapEntryActions.ADD;
    }

    public void clear()
    {
        flags = 0;
        action = MapEntryActions.ADD;
        info.clear();
        state.clear();
        groupStateList.clear();
        load.clear();
        data.clear();
        link.clear();
        seqMcast.clear();
    }

    public int action()
    {
        return action;
    }

    public void action(int action)
    {
        this.action = action;
    }

    public int flags()
    {
        return flags;
    }

    public void flags(int flags)
    {
        this.flags = flags;
    }

    public void applyHasInfo()
    {
        flags |= ServiceFlags.HAS_INFO;
    }

    public boolean checkHasInfo()
    {
        return (flags & ServiceFlags.HAS_INFO) != 0;
    }

    public void applyHasData()
    {
        flags |= ServiceFlags.HAS_DATA;
    }

    public boolean checkHasData()
    {
        return (flags & ServiceFlags.HAS_DATA) != 0;
    }

    public void applyHasLoad()
    {
        flags |= ServiceFlags.HAS_LOAD;
    }

    public boolean checkHasLoad()
    {
        return (flags & ServiceFlags.HAS_LOAD) != 0;
    }

    public void applyHasLink()
    {
        flags |= ServiceFlags.HAS_LINK;
    }

    public boolean checkHasLink()
    {
        return (flags & ServiceFlags.HAS_LINK) != 0;
    }

    public void applyHasState()
    {
        flags |= ServiceFlags.HAS_STATE;
    }

    public boolean checkHasState()
    {
        return (flags & ServiceFlags.HAS_STATE) != 0;
    }

    public int encode(EncodeIterator encIter)
    {
        filterList.clear();
        filterList.flags(FilterEntryFlags.NONE);
        filterList.containerType(DataTypes.ELEMENT_LIST);
        int ret = filterList.encodeInit(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        if (checkHasInfo())
        {
            ret = serviceFilterEncode(encIter, Directory.ServiceFilterIds.INFO);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasData())
        {
            ret = serviceFilterEncode(encIter, Directory.ServiceFilterIds.DATA);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasLink())
        {
            ret = serviceFilterEncode(encIter, Directory.ServiceFilterIds.LINK);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasLoad())
        {
            ret = serviceFilterEncode(encIter, Directory.ServiceFilterIds.LOAD);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasState())
        {
            ret = serviceFilterEncode(encIter, Directory.ServiceFilterIds.STATE);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (groupStateList().size() > 0)
        {
            ret = serviceFilterEncode(encIter, Directory.ServiceFilterIds.GROUP);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }
        return filterList.encodeComplete(encIter, true);
    }

    private int serviceFilterEncode(EncodeIterator encIter, int filterId)
    {
        filterEntry.clear();
        filterEntry.flags(FilterEntryFlags.NONE);
        filterEntry.id(filterId);

        switch (filterId)
        {
            case Directory.ServiceFilterIds.DATA:
                return encodeDataFilter(encIter);
            case Directory.ServiceFilterIds.INFO:
                return encodeInfoFilter(encIter);
            case Directory.ServiceFilterIds.LINK:
                return encodeLinkFilter(encIter);
            case Directory.ServiceFilterIds.LOAD:
                return encodeLoadFilter(encIter);
            case Directory.ServiceFilterIds.STATE:
                return encodeStateFilter(encIter);
            case Directory.ServiceFilterIds.GROUP:
                return encodeGroupFilter(encIter);
            default:
                assert (false);
                return CodecReturnCodes.FAILURE;
        }
    }

    private int encodeInfoFilter(EncodeIterator encIter)
    {
        filterEntry.action(info().action());
        if (filterEntry.action() == FilterEntryActions.CLEAR)
            return filterEntry.encode(encIter);

        int ret = filterEntry.encodeInit(encIter, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        ret = info().encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        return filterEntry.encodeComplete(encIter, true);
    }

    private int encodeDataFilter(EncodeIterator encIter)
    {
        filterEntry.action(data().action());
        if (filterEntry.action() == FilterEntryActions.CLEAR)
            return filterEntry.encode(encIter);

        int ret = filterEntry.encodeInit(encIter, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        ret = data().encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        return filterEntry.encodeComplete(encIter, true);
    }

    private int encodeStateFilter(EncodeIterator encIter)
    {
        filterEntry.action(state().action());
        if (filterEntry.action() == FilterEntryActions.CLEAR)
            return filterEntry.encode(encIter);

        int ret = filterEntry.encodeInit(encIter, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        ret = state().encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        return filterEntry.encodeComplete(encIter, true);
    }

    private int encodeLoadFilter(EncodeIterator encIter)
    {
        filterEntry.action(load().action());
        if (filterEntry.action() == FilterEntryActions.CLEAR)
            return filterEntry.encode(encIter);

        int ret = filterEntry.encodeInit(encIter, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        ret = load().encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        return filterEntry.encodeComplete(encIter, true);
    }

    private int encodeLinkFilter(EncodeIterator encIter)
    {
        filterEntry.action(link().action());
        filterEntry.containerType(DataTypes.MAP);
        filterEntry.applyHasContainerType();
        
        if (filterEntry.action() == FilterEntryActions.CLEAR)
            return filterEntry.encode(encIter);

        int ret = filterEntry.encodeInit(encIter, 0);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        ret = link().encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        return filterEntry.encodeComplete(encIter, true);
    }

    private int encodeGroupFilter(EncodeIterator encIter)
    {
        int ret = CodecReturnCodes.SUCCESS;
        for (Service.ServiceGroup group : groupStateList)
        {
            filterEntry.clear();
            filterEntry.flags(FilterEntryFlags.NONE);
            filterEntry.id(Directory.ServiceFilterIds.GROUP);
            filterEntry.action(group.action());
            if (filterEntry.action() == FilterEntryActions.CLEAR)
            {
                ret = filterEntry.encode(encIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                continue;
            }

            ret = filterEntry.encodeInit(encIter, 0);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = group.encode(encIter);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            ret = filterEntry.encodeComplete(encIter, true);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }
        return ret;
    }

    public int decode(DecodeIterator dIter)
    {
        clear();
        filterEntry.clear();
        filterList.clear();

        int ret = filterList.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        while ((ret = filterEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            ret = decodeFilter(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }
        return CodecReturnCodes.SUCCESS;
    }

    private int decodeFilter(DecodeIterator dIter)
    {
        int returnCode = CodecReturnCodes.SUCCESS;
        switch (filterEntry.id())
        {
            case Directory.ServiceFilterIds.INFO:
                ServiceInfo infoFilter = info();
                applyHasInfo();
                if (filterEntry.action() != FilterEntryActions.CLEAR)
                {
                    returnCode =  infoFilter.decode(dIter);
                }
                infoFilter.action(filterEntry.action());
                break;
            case Directory.ServiceFilterIds.STATE:
                ServiceState stateFilter = state();
                applyHasState();
                if (filterEntry.action() != FilterEntryActions.CLEAR)
                {
                    returnCode = stateFilter.decode(dIter);
                }
                stateFilter.action(filterEntry.action());
                break;
            case Directory.ServiceFilterIds.GROUP:
                Service.ServiceGroup groupFilter = new Service.ServiceGroup();
                groupStateList().add(groupFilter);
                if (filterEntry.action() != FilterEntryActions.CLEAR)
                {
                    returnCode = groupFilter.decode(dIter);
                }
                groupFilter.action(filterEntry.action());
                break;
            case Directory.ServiceFilterIds.LOAD:
                ServiceLoad loadFilter = load();
                applyHasLoad();
                if (filterEntry.action() != FilterEntryActions.CLEAR)
                {
                    returnCode = loadFilter.decode(dIter);
                }
                loadFilter.action(filterEntry.action());
                break;
            case Directory.ServiceFilterIds.DATA:
                ServiceData dataFilter = data();
                applyHasData();
                if (filterEntry.action() != FilterEntryActions.CLEAR)
                {
                    returnCode = dataFilter.decode(dIter);
                }
                dataFilter.action(filterEntry.action());
                break;
            case Directory.ServiceFilterIds.LINK:
                ServiceLinkInfo linkFilter = link();
                applyHasLink();
                if (filterEntry.action() != FilterEntryActions.CLEAR) {
                    returnCode = linkFilter.decode(dIter);
                }
                linkFilter.action(filterEntry.action());
                break;
            case Directory.ServiceFilterIds.SEQ_MCAST:
                //ServiceSeqMcastInfo SeqMcastFilter = seqMcastInfo();
                applyHasLink();
                if (filterEntry.action() != FilterEntryActions.CLEAR)
                {
                    returnCode = seqMcast.decode(dIter);
                }
                seqMcast.action(filterEntry.action());
                break;
            default:
                return CodecReturnCodes.FAILURE;
        }

        return returnCode;
    }

    public int serviceId()
    {
        return serviceId;
    }

    public void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    public ServiceInfo info()
    {
        return info;
    }
    
    public void info(ServiceInfo info)
    {
        copyServiceInfoRefFrom(info);
    }

    public ServiceState state()
    {
        return state;
    }
    
    public void state(ServiceState state)
    {
        copyServiceStateRefFrom(state);
    }

    public List<ServiceGroup> groupStateList()
    {
        return groupStateList;
    }
    
    public void groupStateList(List<ServiceGroup> groupStateList)
    {
        assert (groupStateList != null) : "groupStateList can not be null";

        groupStateList().clear();
       
        for (ServiceGroup serviceGroup : groupStateList)
        {
            groupStateList().add(serviceGroup);
        }
    }

    public ServiceLoad load()
    {
        return load;
    }

    public void load(ServiceLoad load)
    {
        copyServiceLoadRefFrom(load);
    }
    
    public ServiceData data()
    {
        return data;
    }

    public void data(ServiceData data)
    {
        copyServiceDataRefFrom(data);
    }
    
    public ServiceLinkInfo link()
    {
        return link;
    }
    
    @Override
    public ServiceSeqMcastInfo seqMcastInfo()
    {
        return seqMcast;
    }

    public void link(ServiceLinkInfo link)
    {
        copyServiceLinkRefFrom(link);
    }
    
    public int copy(Service destService)
    {
        assert (destService != null) : "destService can not be null";
        int ret = CodecReturnCodes.SUCCESS;
        destService.clear();
        destService.action(action());
        destService.serviceId(serviceId());
        if (checkHasInfo())
        {
            ret = info().copy(destService.info());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasInfo();
        }
        if (checkHasData())
        {
            ret = data().copy(destService.data());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasData();
        }
        for (Service.ServiceGroup group : groupStateList())
        {
            Service.ServiceGroup destGroup = new Service.ServiceGroup();
            destService.groupStateList().add(destGroup);
            ret = group.copy(destGroup);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (checkHasLink())
        {
            ret = link().copy(destService.link());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasLink();
        }
        if (checkHasLoad())
        {
            ret = load().copy(destService.load());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasLoad();
        }
        if (checkHasState())
        {
            ret = state().copy(destService.state());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasState();
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    public int applyUpdate(Service destService)
    {
        assert (destService != null) : "destService can not be null";
        int ret = CodecReturnCodes.SUCCESS;
        destService.action(action());
        destService.serviceId(serviceId());
        if (checkHasInfo())
        {
            ret = info().update(destService.info());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasInfo();
        }
        
        if (checkHasData())
        {
            ret = data().update(destService.data());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasData();
        }
        
        for (Service.ServiceGroup group : groupStateList())
        {
            Service.ServiceGroup destGroup = new Service.ServiceGroup();
            destService.groupStateList().add(destGroup);
            ret = group.copy(destGroup);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (checkHasLink())
        {
            ret = link().update(destService.link());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasLink();
        }

        if (checkHasLoad())
        {
            ret = load().update(destService.load());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasLoad();
        }
        
        if (checkHasState())
        {
            ret = state().update(destService.state());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destService.applyHasState();
        }

        return CodecReturnCodes.SUCCESS;
    }

    public String toString()
    {
        stringBuf.setLength(0);
        stringBuf.append(tab);
        stringBuf.append("Service:");
        stringBuf.append(eol);

        stringBuf.append(tab);
        stringBuf.append(tab);
        stringBuf.append("serviceId: ");
        stringBuf.append(serviceId());
        stringBuf.append(eol);

        if (checkHasInfo())
        {
            stringBuf.append(info());
        }

        if (checkHasData())
        {
            stringBuf.append(data());
        }

        if (checkHasLink())
        {
            stringBuf.append(link());
        }

        if (checkHasState())
        {
            stringBuf.append(state());
        }

        if (checkHasLoad())
        {
            stringBuf.append(load());
        }

        if (!groupStateList().isEmpty())
        {
            stringBuf.append(groupStateList());
        }

        return stringBuf.toString();
    }
    
    //copies references from src service info into this object.
    private void copyServiceInfoRefFrom(ServiceInfo srcServiceInfo)
    {
        assert (srcServiceInfo != null) : "srcServiceInfo can not be null";
        info().serviceName(srcServiceInfo.serviceName());
        info().action(srcServiceInfo.action());
        info().capabilitiesList(srcServiceInfo.capabilitiesList());
        if (srcServiceInfo.checkHasAcceptingConsumerStatus())
        {
            info().applyHasAcceptingConsumerStatus();
            info().acceptingConsumerStatus(srcServiceInfo.acceptingConsumerStatus());
        }
        if (srcServiceInfo.checkHasDictionariesProvided())
        {
            info().applyHasDictionariesProvided();
            info().dictionariesProvidedList(srcServiceInfo.dictionariesProvidedList());
            
        }
        if (srcServiceInfo.checkHasDictionariesUsed())
        {
            info().applyHasDictionariesUsed();
            info().dictionariesUsedList(srcServiceInfo.dictionariesUsedList());
        }
        if (srcServiceInfo.checkHasIsSource())
        {
            info().applyHasIsSource();
            info().isSource(srcServiceInfo.isSource());
        }
        if (srcServiceInfo.checkHasItemList())
        {
            info().applyHasItemList();
            info().itemList(srcServiceInfo.itemList());
        }
        if (srcServiceInfo.checkHasQos())
        {
            info().applyHasQos();
            info().qosList(srcServiceInfo.qosList());
        }
        if (srcServiceInfo.checkHasSupportsOutOfBandSnapshots())
        {
            info().applyHasSupportsOutOfBandSnapshots();
            info().supportsOutOfBandSnapshots(srcServiceInfo.supportsOutOfBandSnapshots());
        }
        if (srcServiceInfo.checkHasSupportsQosRange())
        {
            info().applyHasSupportsQosRange();
            info().supportsQosRange(srcServiceInfo.supportsQosRange());
        }
        if (srcServiceInfo.checkHasVendor())
        {
            info().applyHasVendor();
            info().vendor(srcServiceInfo.vendor());
        }
    }
    
    private void copyServiceDataRefFrom(ServiceData srcData)
    {
        assert (srcData != null) : "srcData can not be null";
        data().action(srcData.action());
        data().type(srcData.type());
        data().flags(srcData.flags());
        if (srcData.checkHasData())
        {
            data().applyHasData();
            data().dataType(srcData.dataType());
            data().data(srcData.data());
        }
    }
    
    private void copyServiceLinkRefFrom(ServiceLinkInfo srcLink)
    {
        assert (srcLink != null) : "srcLink can not be null";
        link().action(srcLink.action());
        link().linkList(srcLink.linkList());
    }
    
    private void copyServiceLoadRefFrom(ServiceLoad srcLoad)
    {
        assert (srcLoad != null) : "srcLoad can not be null";
        
        load().action(srcLoad.action());
        
        if (srcLoad.checkHasLoadFactor())
        {
            load().applyHasLoadFactor();
            load().loadFactor(srcLoad.loadFactor());
        }

        if (srcLoad.checkHasOpenLimit())
        {
            load().applyHasOpenLimit();
            load().openLimit(srcLoad.openLimit());
        }

        if (srcLoad.checkHasOpenWindow())
        {
            load().applyHasOpenWindow();
            load().openWindow(srcLoad.openWindow());
        }
    }
    
    private void copyServiceStateRefFrom(ServiceState srcState)
    {
        assert (srcState != null) : "srcState can not be null";
        state().flags(srcState.flags());
        state().action(srcState.action());
        state().serviceState(srcState.serviceState());

        if (srcState.checkHasAcceptingRequests())
        {
            state().applyHasAcceptingRequests();
            state().acceptingRequests(srcState.acceptingRequests());
        }

        if (srcState.checkHasStatus())
        {
            state().applyHasStatus();
            state().status(srcState.status());
        }
    }

}
