/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;

using System.Collections.Generic;
using System.Text;

namespace LSEG.Ema.Access
{
    internal class DirectoryCache
    {
        internal string DirectoryName { get; set; } = string.Empty;

        internal DirectoryRefresh DirectoryRefresh { get; set; }

        internal DirectoryCache()
        {
            DirectoryRefresh = new DirectoryRefresh();
            Clear();
        }

        internal void Clear()
        {
            DirectoryName = string.Empty;
            DirectoryRefresh.Clear();
            DirectoryRefresh.StreamId = 0;
            DirectoryRefresh.State.StreamState(StreamStates.OPEN);
            DirectoryRefresh.State.DataState(DataStates.OK);
            DirectoryRefresh.State.Code(StateCodes.NONE);

            Buffer stateText = new Buffer();
            stateText.Data("Source Directory Refresh Completed");
            DirectoryRefresh.State.Text(stateText);
        }

        internal void AddService(Service service)
        {
            DirectoryRefresh.ServiceList.Add(service);
        }

        internal Service? GetService(int serviceId)
        {
            for(int index = 0; index < DirectoryRefresh.ServiceList.Count; index++)
            {
                if(DirectoryRefresh.ServiceList[index].ServiceId == serviceId)
                {
                    return DirectoryRefresh.ServiceList[index];
                }
            }

            return null;
        }

        internal void RemoveService(int serviceId)
        {
            for (int index = 0; index < DirectoryRefresh.ServiceList.Count; index++)
            {
                if (DirectoryRefresh.ServiceList[index].ServiceId == serviceId)
                {
                    DirectoryRefresh.ServiceList.RemoveAt(index);
                }
            }
        }
    }

    internal interface IDirectoryServiceStoreClient
    {
        void OnServiceDelete(ClientSession? session, int serviceId);

        void OnServiceStateChange(ClientSession? session, int serviceId, ServiceState state);

        void OnServiceGroupChange(ClientSession? session, int serviceId, IList<ServiceGroup> serviceGroupList);
    }

    internal abstract class DirectoryServiceStore
    {
        protected IDictionary<string, int> ServiceNameAndIdDict;
        protected IDictionary<int, string> ServiceIdAndNameDict;
        protected DirectoryCache? m_DirectoryCache;
        protected bool m_bUsingDefaultService;

        private OmmProviderConfig.ProviderRoleEnum m_ProviderRole;
        private IOmmCommonImpl m_OmmCommonImpl;
        private DecodeIterator m_DecodeIterator;
        private DirectoryMsg m_SubmittedDirectoryMsg;
        private EmaObjectManager m_EmaObjectManager;

        private Queue<Service> _servicePool = new Queue<Service>(5);

        private MonitorWriteLocker CacheLock { get; set; } = new MonitorWriteLocker(new object());

        protected abstract bool CheckExistingServiceId(int serviceId, StringBuilder errorText, out int errorCode);
        protected abstract bool AddServiceIdAndNamePair(int serviceId, string serviceName, StringBuilder errorText, out int errorCode);

        internal DirectoryMsg SubmittedDirectoryMsg => m_SubmittedDirectoryMsg;

        internal IDirectoryServiceStoreClient? DirectoryServiceStoreClient;

        internal DirectoryServiceStore(EmaObjectManager emaObjectManager, OmmProviderConfig.ProviderRoleEnum providerRole, IOmmCommonImpl ommCommonImpl)
        {
            m_EmaObjectManager = emaObjectManager;
            m_ProviderRole = providerRole;
            m_OmmCommonImpl = ommCommonImpl;
            m_DecodeIterator = new DecodeIterator();
            m_SubmittedDirectoryMsg = new DirectoryMsg();

            ServiceNameAndIdDict = new Dictionary<string, int>();
            ServiceIdAndNameDict = new Dictionary<int, string>();
        }

        internal void AddToServiceMap(Service service)
        {
            if (service.HasInfo)
            {
                AddToServiceMap(service.ServiceId, service.Info.ServiceName.ToString());
            }
        }

        internal void AddToServiceMap(int serviceId, string serviceName)
        {
            CacheLock.Enter();
            try
            {
                ServiceIdAndNameDict[serviceId] = serviceName;
                ServiceNameAndIdDict[serviceName] = serviceId;
            }
            finally
            {
                CacheLock.Exit();
            }
        }

        internal void RemoveService(int serviceId)
        {
            CacheLock.Enter();
            try
            {
                if(ServiceIdAndNameDict.TryGetValue(serviceId, out var serviceName))
                {
                    if(serviceName != null)
                    {
                        ServiceNameAndIdDict.Remove(serviceName);
                    }

                    ServiceIdAndNameDict.Remove(serviceId);

                }
            }
            finally
            {
                CacheLock.Exit();
            }
        }

        internal bool GetServiceIdByName(string serviceName, out int serviceId)
        {
            try
            {
                CacheLock.Enter();
                return ServiceNameAndIdDict.TryGetValue(serviceName, out serviceId);
            }
            finally { CacheLock.Exit(); }
        }

        internal bool GetServiceNameById(int serviceId, out string? serviceName)
        {
            try
            {
                CacheLock.Enter();
                return ServiceIdAndNameDict.TryGetValue(serviceId, out serviceName);
            }
            finally { CacheLock.Exit(); }
        }

        internal void ClearServiceNameAndId()
        {
            CacheLock.Enter();

            try
            {
                ServiceIdAndNameDict.Clear();
                ServiceNameAndIdDict.Clear();
            }
            finally
            {
                CacheLock.Exit();
            }
        }

        internal void NotifyOnServiceStateChange(ClientSession? clientSession, Service service)
        {
            if(DirectoryServiceStoreClient != null && service.State.Action != FilterEntryActions.CLEAR)
            {
                DirectoryServiceStoreClient.OnServiceStateChange(clientSession, service.ServiceId, service.State);
            }
        }

        internal void NotifyOnServiceGroupChange(ClientSession? clientSession, Service service)
        {
            if (DirectoryServiceStoreClient != null && service.GroupStateList.Count != 0)
            {
                DirectoryServiceStoreClient.OnServiceGroupChange(clientSession, service.ServiceId, service.GroupStateList);
            }
        }

        internal void NotifyServiceDelete(ClientSession? clientSession, Service service)
        {
            if (DirectoryServiceStoreClient != null)
            {
                DirectoryServiceStoreClient.OnServiceDelete(clientSession, service.ServiceId);
            }
        }

        internal DirectoryCache GetDirectoryCache()
        {
            return m_DirectoryCache!;
        }

        internal static DirectoryRefresh GetDirectoryRefreshMsg(DirectoryServiceStore directoryServiceStore, DirectoryRefresh directoryRefresh, bool clearCache)
        {
            directoryServiceStore.CacheLock.Enter();

            try
            {
                if(clearCache)
                {
                    directoryRefresh.ClearCache = true;
                }

                int filters = 0;
                IList<Service> serviceList = directoryRefresh.ServiceList;
                Service service;
                for (int i = 0; i < serviceList.Count; i++)
                {
                    service = serviceList[i];
                    if (service.HasInfo)
                        filters |= Directory.ServiceFilterFlags.INFO;
                    if(service.HasState)
                        filters |= Directory.ServiceFilterFlags.STATE;
                    if(service.HasData)
                        filters |= Directory.ServiceFilterFlags.DATA;
                    if(service.HasLink)
                        filters |= Directory.ServiceFilterFlags.LINK;
                }

                directoryRefresh.Filter = filters;
            }
            finally
            {
                directoryServiceStore.CacheLock.Exit();
            }

            return directoryRefresh;
        }

        public bool DecodeSourceDirectory(IMsg msg, StringBuilder errorText, out int errorCode)
        {
            errorCode = 0;
            CodecReturnCode retCode;
            m_DecodeIterator.Clear();

            retCode = m_DecodeIterator.SetBufferAndRWFVersion(msg.EncodedDataBody, Codec.MajorVersion(), Codec.MinorVersion());

            if(retCode != CodecReturnCode.SUCCESS)
            {
                errorText.Append($"Internal error. Failed to set decode iterator buffer and version in " +
                    $"DirectoryServiceStore.DecodeSourceDirectory(). Reason = {retCode.GetAsString()}.");
                errorCode = (int)retCode;
                return false;
            }

            Eta.Codec.Map map = new();

            if (m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
            {
                m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, "Begin decoding of SourceDirectory.");
            }

            retCode = map.Decode(m_DecodeIterator);

            if(retCode < CodecReturnCode.SUCCESS)
            {
                errorText.Append("Internal error. Failed to decode Map in GetDirectoryServiceStore.DecodeSourceDirectory()." +
                    $" Reason = {retCode.GetAsString()}");
                errorCode = (int)retCode;
                return false;
            }
            else if (retCode == CodecReturnCode.NO_DATA)
            {
                if(m_OmmCommonImpl.GetLoggerClient().IsWarnEnabled)
                {
                    m_OmmCommonImpl.GetLoggerClient().Warn(m_OmmCommonImpl.InstanceName, "Passed in SourceDirectory map" +
                        " contains no entries (e.g. there is no service specified).");
                }

                if (m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
                {
                    m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, "End decoding of SourceDirectory.");
                }

                return true;
            }

            switch (map.KeyPrimitiveType)
            {
                case DataTypes.UINT:
                    {
                        if(!DecodeSourceDirectoryKeyUInt(map, m_DecodeIterator, errorText, out errorCode))
                        {
                            return false;
                        }
                        break;
                    }
                default:
                    {
                        errorText.Append($"Attempt to specify SourceDirectory info with a Map using key DataType of " +
                            $"{DataTypes.ToString(map.KeyPrimitiveType)} while the expected key DataType is {DataTypes.ToString(DataTypes.UINT)}");

                        if(m_OmmCommonImpl.GetLoggerClient().IsErrorEnabled)
                        {
                            m_OmmCommonImpl.GetLoggerClient().Error(m_OmmCommonImpl.InstanceName, errorText.ToString());
                        }

                        errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT;
                        return false;
                    }
            }

            if (m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
            {
                m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, "End decoding of SourceDirectory.");
            }

            return true;
        }

        public bool SubmitSourceDirectory(ClientSession? clientSession, IMsg msg, StringBuilder errorText, bool storeUserSubmitted, out int errorCode)
        {
            errorCode = 0;
            m_DecodeIterator.Clear();

            CodecReturnCode retCode = m_DecodeIterator.SetBufferAndRWFVersion(msg.EncodedDataBody, Codec.MajorVersion(), 
                Codec.MinorVersion());

            if(retCode != CodecReturnCode.SUCCESS)
            {
                errorText.Append($"Internal error. Failed to set decode iterator buffer and version in GetDirectoryServiceStore.SubmitSourceDirectory()." +
                    $" Reason = {retCode.GetAsString()}.");
                errorCode = (int)retCode;
                return false;
            }

            if(msg.MsgClass == MsgClasses.REFRESH)
            {
                m_SubmittedDirectoryMsg.Clear();
                m_SubmittedDirectoryMsg.DirectoryMsgType = DirectoryMsgType.REFRESH;
            }
            else if(msg.MsgClass == MsgClasses.UPDATE)
            {
                m_SubmittedDirectoryMsg.Clear();
                m_SubmittedDirectoryMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
            }

            retCode = m_SubmittedDirectoryMsg.Decode(m_DecodeIterator, msg);

            if(retCode < CodecReturnCode.SUCCESS)
            {
                errorText.Append($"Internal error: Failed to decode Source Directory message in GetDirectoryServiceStore.SubmitSourceDirectory()." +
                    $" Reason = {retCode.GetAsString()}.");
                errorCode = (int)retCode;
                return false;
            }

            IList<Service>? serviceList;

            switch (m_SubmittedDirectoryMsg.DirectoryMsgType)
            {
                case DirectoryMsgType.REFRESH:
                    {
                        DirectoryRefresh directoryRefresh = m_SubmittedDirectoryMsg.DirectoryRefresh!;
                        serviceList = directoryRefresh.ServiceList;
                        break;
                    }
                case DirectoryMsgType.UPDATE:
                    {
                        DirectoryUpdate directoryUpdate = m_SubmittedDirectoryMsg.DirectoryUpdate!;
                        serviceList = directoryUpdate.ServiceList;
                        break;
                    }
                default:
                    {
                        errorText.Append($"Received unexpected message type {m_SubmittedDirectoryMsg.DirectoryMsgType}.");
                        errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT;
                        return false;
                    }
            }

            if (!storeUserSubmitted)
            {
                Service submittedService;

                for (int index = 0; index < serviceList.Count; index++)
                {
                    submittedService = serviceList[index];
                    switch(submittedService.Action)
                    {
                        case MapEntryActions.ADD:

                            if (submittedService.HasState)
                                NotifyOnServiceStateChange(clientSession, submittedService);

                            NotifyOnServiceGroupChange(clientSession, submittedService);
                            break;
                        case MapEntryActions.DELETE:

                            NotifyServiceDelete(clientSession, submittedService);
                            break;
                        case MapEntryActions.UPDATE:

                            if (submittedService.HasState)
                                NotifyOnServiceStateChange(clientSession, submittedService);

                            NotifyOnServiceGroupChange(clientSession, submittedService);
                            break;
                    }
                }
            }
            else
            {
                Service submittedService;

                for (int index = 0; index < serviceList.Count; index++)
                {
                    Service? cacheService;
                    submittedService = serviceList[index];
                    switch (submittedService.Action)
                    {
                        case MapEntryActions.ADD:
                            {
                                cacheService = m_DirectoryCache!.GetService(submittedService.ServiceId);

                                if(cacheService is null)
                                {
                                    cacheService = new Service();
                                    cacheService.ServiceId = submittedService.ServiceId;

                                    if(!ApplyService(cacheService, submittedService, clientSession, errorText, out errorCode))
                                    {
                                        return false;
                                    }

                                    m_DirectoryCache.AddService(cacheService);
                                }

                                if (submittedService.HasState) NotifyOnServiceStateChange(clientSession, submittedService);

                                NotifyOnServiceGroupChange(clientSession, submittedService);
                                break;
                            }
                        case MapEntryActions.DELETE:

                            m_DirectoryCache!.RemoveService(submittedService.ServiceId);

                            NotifyServiceDelete(clientSession, submittedService);
                            break;
                        case MapEntryActions.UPDATE:

                            cacheService = m_DirectoryCache!.GetService(submittedService.ServiceId);

                            if (cacheService is not null)
                            {
                                if (!ApplyService(cacheService, submittedService, clientSession, errorText, out errorCode))
                                {
                                    return false;
                                }
                            }
                            break;
                    }
                }
            }

            return true;
        }

        private bool ApplyService(Service cacheService, Service submittedService, ClientSession? clientSession, StringBuilder errorText, out int errorCode)
        {
            errorCode = 0;
            CodecReturnCode retCode = CodecReturnCode.SUCCESS;

            if (submittedService.HasState)
            {
                if (submittedService.State.Action == FilterEntryActions.UPDATE)
                {
                    ServiceStateFlags flags = cacheService.State.Flags | submittedService.State.Flags;
                    retCode = submittedService.State.Update(cacheService.State);
                    if (retCode == CodecReturnCode.SUCCESS)
                        cacheService.State.Flags = flags;
                }
                else
                {
                    retCode = submittedService.State.Update(cacheService.State);
                }

                if (retCode < CodecReturnCode.SUCCESS)
                {
                    errorText.AppendLine($"Internal error: Failed to Update State filter in DirectoryServiceStroe.ApplyService()" +
                        $" for Service Id = {submittedService.ServiceId}").Append($"Reason = {retCode.GetAsString()}.");
                    errorCode = (int)retCode;
                    return false;
                }

                cacheService.HasState = true;

                NotifyOnServiceStateChange(clientSession!, submittedService);
            }

            if (submittedService.HasInfo)
            {
                if (submittedService.Info.Action == FilterEntryActions.UPDATE)
                {
                    ServiceInfoFlags flags = cacheService.Info.Flags | submittedService.Info.Flags;
                    retCode = submittedService.Info.Update(cacheService.Info);
                    if(retCode == CodecReturnCode.SUCCESS)
                        cacheService.Info.Flags = flags;
                }
                else
                {
                    retCode = submittedService.Info.Update(cacheService.Info);
                }

                if(retCode < CodecReturnCode.SUCCESS)
                {
                    errorText.AppendLine($"Internal error: Failed to Update Info filter in DirectoryServiceStroe.ApplyService()" +
                        $" for Service Id = {submittedService.ServiceId}").Append($"Reason = {retCode.GetAsString()}.");
                    errorCode = (int)retCode;
                    return false;
                }

                cacheService.HasInfo = true;
            }

            if (submittedService.HasLoad)
            {
                if (submittedService.Load.Action == FilterEntryActions.UPDATE)
                {
                    ServiceLoadFlags flags = cacheService.Load.Flags | submittedService.Load.Flags;
                    retCode = submittedService.Load.Update(cacheService.Load);
                    if (retCode == CodecReturnCode.SUCCESS)
                        cacheService.Load.Flags = flags;
                }
                else
                {
                    retCode = submittedService.Load.Update(cacheService.Load);
                }

                if (retCode < CodecReturnCode.SUCCESS)
                {
                    errorText.AppendLine($"Internal error: Failed to Update Load filter in DirectoryServiceStroe.ApplyService()" +
                        $" for Service Id = {submittedService.ServiceId}").Append($"Reason = {retCode.GetAsString()}.");
                    errorCode = (int)retCode;
                    return false;
                }

                cacheService.HasLoad = true;
            }

            if (submittedService.HasLink)
            {
                if (submittedService.Link.Action == FilterEntryActions.UPDATE)
                {
                    cacheService.Link.Action = FilterEntryActions.UPDATE;

                    ServiceLink? submittedLink, cacheLink = null;

                    for (int submittedIndex =0; submittedIndex < submittedService.Link.LinkList.Count; submittedIndex++)
                    {
                        submittedLink = submittedService.Link.LinkList[submittedIndex];
                        bool foundLink = false;

                        for (int cacheIndex = 0; cacheIndex < cacheService.Link.LinkList.Count; cacheIndex++)
                        {
                            cacheLink = cacheService.Link.LinkList[cacheIndex];

                            if (submittedLink.Name.Equals(cacheLink.Name))
                            {
                                foundLink = true;
                                break;
                            }
                        }

                        if (foundLink && cacheLink != null)
                        {
                            if (submittedLink.HasCode)
                            {
                                cacheLink.HasCode = true;
                                cacheLink.LinkCode = submittedLink.LinkCode;
                            }

                            if (submittedLink.HasText)
                            {
                                cacheLink.HasText = true;
                                ByteBuffer byteBuffer = new(submittedLink.Text.Length);
                                retCode = submittedLink.Text.Copy(byteBuffer);
                                cacheLink.Text.Data(byteBuffer);
                            }

                            if (submittedLink.HasType)
                            {
                                cacheLink.HasType = true;
                                cacheLink.Type = submittedLink.Type;
                            }

                            cacheLink.LinkState = submittedLink.LinkState;
                        }
                        else
                        {
                            cacheLink = new ServiceLink();
                            retCode = submittedLink.Copy(cacheLink);
                            cacheService.Link.LinkList.Add(cacheLink);
                        }
                    }
                }
                else
                {
                    retCode = submittedService.Link.Update(cacheService.Link);
                }

                if (retCode < CodecReturnCode.SUCCESS)
                {
                    errorText.AppendLine($"Internal error: Failed to Update Link filter in DirectoryServiceStroe.ApplyService()" +
                        $" for Service Id = {submittedService.ServiceId}").Append($"Reason = {retCode.GetAsString()}.");
                    errorCode = (int)retCode;
                    return false;
                }

                cacheService.HasLink = true;
            }

            NotifyOnServiceGroupChange(clientSession!, submittedService);

            return true;
        }

        private bool DecodeSourceDirectoryKeyUInt(Eta.Codec.Map map, DecodeIterator decodeIt, StringBuilder errorText, out int errorCode)
        {
            errorCode = 0;
            CodecReturnCode retCode = CodecReturnCode.SUCCESS;
            UInt serviceId = new();
            Eta.Codec.MapEntry mapEntry = new();
            Eta.Codec.FilterList filterList = new();
            Eta.Codec.FilterEntry filterEntry = new();
            Eta.Codec.ElementList elementList = new();
            Eta.Codec.ElementEntry elementEntry = new();
            StringBuilder text = new StringBuilder();

            while ((retCode = mapEntry.Decode(decodeIt, serviceId)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if(retCode != CodecReturnCode.SUCCESS)
                {
                    errorText.Append($"Internal error: Failed to Decode Map Entry. Reason = {retCode.GetAsString()}.");
                    errorCode = (int)retCode;
                    return false;
                }

                if (m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
                {
                    text.Clear();
                    text.Append($"Begin decoding of Service with id of {serviceId}. Action= ");
                    switch (mapEntry.Action)
                    {
                        case MapEntryActions.UPDATE:
                            text.Append("Update");
                            break;
                        case MapEntryActions.ADD:
                            text.Append("Add");
                            break;
                        case MapEntryActions.DELETE:
                            text.Append("Delete");
                            break;
                    }

                    m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, text.ToString());
                }

                int srvcId = (int)serviceId.ToULong();
                if (mapEntry.Action == MapEntryActions.DELETE)
                {
                    if (GetServiceNameById(srvcId, out string? serviceName))
                    {
                        RemoveService(srvcId);
                    }

                    if (m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        text.Clear();
                        text.Append($"End decoding of Service with id of {srvcId}");
                        m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, text.ToString());
                    }

                    continue;
                }
                else if (mapEntry.Action == MapEntryActions.ADD)
                {
                    if(CheckExistingServiceId(srvcId, errorText, out errorCode) == false)
                    {
                        return false;
                    }
                }

                if(map.ContainerType != DataTypes.FILTER_LIST)
                {
                    errorText.Append($"Attempt to specify Service with a container of {DataTypes.ToString(map.ContainerType)}" +
                        $" rather than the expected {DataTypes.ToString(DataTypes.FILTER_LIST)}");
                    errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT;
                    return false;
                }

                filterList.Clear();
                filterEntry.Clear();

                retCode = filterList.Decode(decodeIt);
                if (retCode < CodecReturnCode.SUCCESS)
                {
                    errorText.Append($"Internal error: Failed to Decode FilterList. Reason = {retCode.GetAsString()}.");
                    errorCode = (int)retCode;
                    return false;
                }
                else if (retCode == CodecReturnCode.NO_DATA)
                {
                    if (m_OmmCommonImpl.GetLoggerClient().IsWarnEnabled)
                    {
                        text.Clear();
                        text.Append($"Service with id of {srvcId} contains no FilterEntries. Skipping this service.");
                        m_OmmCommonImpl.GetLoggerClient().Warn(m_OmmCommonImpl.InstanceName, text.ToString());
                    }

                    if (m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        text.Clear();
                        text.Append($"End decoding of Service with id of {srvcId}");
                        m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, text.ToString());
                    }

                    continue;
                }

                while ((retCode = filterEntry.Decode(decodeIt)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if(retCode < CodecReturnCode.SUCCESS)
                    {
                        errorText.Append($"Internal error: Failed to Decode Filter Entry. Reason = {retCode.GetAsString()}.");
                        errorCode = (int)retCode;
                        return false;
                    }

                    if(m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        text.Clear();
                        text.Append($"Begin decoding of FilterEntry with id of {filterEntry.Id}");
                        m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, text.ToString());
                    }

                    if(filterEntry.Id == Directory.ServiceFilterIds.INFO)
                    {
                        if(mapEntry.Action == MapEntryActions.UPDATE)
                        {
                            errorText.Append($"Attempt to update Infofilter of service with id of {serviceId}" +
                                $" while this is not allowed.");
                            errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION;
                            return false;
                        }

                        if(filterEntry.CheckHasContainerType() && (filterEntry.ContainerType != DataTypes.ELEMENT_LIST)
                           && filterList.ContainerType != DataTypes.ELEMENT_LIST)
                        {
                            int containerType = filterEntry.CheckHasContainerType() ? filterEntry.ContainerType :
                                filterList.ContainerType;

                            errorText.Append($"Attempt to specify Service InfoFilter with a container of {DataTypes.ToString(containerType)}" +
                                $" rather than the expected {DataTypes.ToString(DataTypes.ELEMENT_LIST)}.");
                            errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT;
                            return false;
                        }

                        elementList.Clear();
                        elementEntry.Clear();

                        if((retCode = elementList.Decode(decodeIt, null)) < CodecReturnCode.SUCCESS)
                        {
                            errorText.Append($"Internal error: Failed to Decode ElementList. Reason = {retCode.GetAsString()}.");
                            errorCode = (int)retCode;
                            return false;
                        }

                        bool bServiceNameEntryFound = false;
                        Buffer serviceNameBuffer = new();
                        while ( (retCode = elementEntry.Decode(decodeIt)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            if(retCode < CodecReturnCode.SUCCESS)
                            {
                                errorText.Append($"Internal error: Failed to Decode ElementEntry. Reason = {retCode.GetAsString()}.");
                                errorCode = (int)retCode;
                                return false;
                            }

                            if(m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
                            {
                                text.Clear();
                                text.Append($"Decoding of ElementEntry with name of {elementEntry.Name.ToString()}");
                                m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, text.ToString());
                            }

                            if (!bServiceNameEntryFound && elementEntry.Name.Equals(ElementNames.NAME))
                            {
                                if (elementEntry.DataType != DataTypes.ASCII_STRING)
                                {
                                    errorText.Append($"Attempt to specify Service Name with a {DataTypes.ToString(elementEntry.DataType)}" +
                                        $" rather than the expected {DataTypes.ToString(DataTypes.ASCII_STRING)}");
                                    errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT;
                                    return false;
                                }

                                serviceNameBuffer.Clear();
                                retCode = serviceNameBuffer.Decode(decodeIt);
                                if (retCode < CodecReturnCode.SUCCESS)
                                {
                                    errorText.Append($"Internal error: Failed to Decode Buffer. Reason = {retCode.GetAsString()}.");
                                    errorCode = (int)retCode;
                                    return false;
                                }
                                else if (retCode == CodecReturnCode.BLANK_DATA)
                                {
                                    errorText.Append($"Attempt to specify Service Name with a blank ascii string for service id of " +
                                        $"{serviceId}.");
                                    errorCode = (int)retCode;
                                    return false;
                                }

                                bServiceNameEntryFound = true;

                                if (AddServiceIdAndNamePair(srvcId, serviceNameBuffer.ToString(), errorText, out errorCode) == false)
                                {
                                    return false;
                                }
                            }
                        }

                        if(!bServiceNameEntryFound)
                        {
                            errorText.Append($"Attempt to specify service InfoFilter without required Service Name for service" +
                                $" id of {serviceId}");
                            errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT;
                            return false;
                        }
                    }
                }

                if(m_OmmCommonImpl.GetLoggerClient().IsTraceEnabled)
                {
                    text.Clear();
                    text.Append($"End decoding of FilterEntry with id of {filterEntry.Id}");

                    m_OmmCommonImpl.GetLoggerClient().Trace(m_OmmCommonImpl.InstanceName, text.ToString());
                }
            }

            return true;
        }

        internal long EncodeDirectoryMsg(List<Service> inputServiceList, 
            List<Service> outputServiceList, 
            long directoryServiceFilter, 
            bool initialResp, 
            bool specifiedServiceId, 
            int serviceId)
        {
            CacheLock.Enter();

            Service? service = null;
            Service respService;
            long filters = 0;


            if (specifiedServiceId)
            {
                foreach (var s in inputServiceList)
                {
                    if (s.ServiceId == serviceId)
                    {
                        service = s;
                        break;
                    }
                }

                if (service == null)
                {
                    outputServiceList.Clear();
                    return filters;
                }
            }

            if (service != null)
            {
                if (service.Action == MapEntryActions.DELETE)
                {
                    respService = GetService();
                    service.Copy(respService);
                    outputServiceList.Add(respService);
                }
                else
                {
                    respService = GetService();
                    filters = ApplyDirectoryService(directoryServiceFilter, initialResp, service, respService);

                    if (filters != 0) outputServiceList.Add(respService);
                    else ReturnService(respService);
                }
            }
            else
            {
                long applyFilters;
                foreach (var s in inputServiceList)
                {
                    if (s.Action == MapEntryActions.DELETE)
                    {
                        respService = GetService();
                        s.Copy(respService);
                        outputServiceList.Add(respService);
                    }
                    else
                    {
                        respService = GetService();
                        applyFilters = ApplyDirectoryService(directoryServiceFilter, initialResp, s, respService);

                        if (applyFilters != 0)
                        {
                            filters |= applyFilters;
                            outputServiceList.Add(respService);
                        }
                        else
                        {
                            ReturnService(respService);
                        }
                    }
                }
            }

            CacheLock.Exit();

            return filters;
        }

        internal long ApplyDirectoryService(long directoryServiceFilter, bool initialResp, Service service, Service respService)
        {
            long filter = 0;

            if (service.HasInfo && ((directoryServiceFilter & Directory.ServiceFilterFlags.INFO) == Directory.ServiceFilterFlags.INFO))
            {
                respService.HasInfo = true;
                service.Info.Copy(respService.Info);
                if (initialResp)
                {
                    respService.Info.Action = FilterEntryActions.SET;
                }

                filter |= directoryServiceFilter & Directory.ServiceFilterFlags.INFO;
            }

            if (service.HasState && ((directoryServiceFilter & Directory.ServiceFilterFlags.STATE) == Directory.ServiceFilterFlags.STATE))
            {
                respService.HasState = true;
                respService.State = service.State;

                if (initialResp)
                {
                    respService.State.Action = FilterEntryActions.SET;
                }

                filter |= directoryServiceFilter & Directory.ServiceFilterFlags.STATE;
            }

            if (service.HasLoad && ((directoryServiceFilter & Directory.ServiceFilterFlags.LOAD) == Directory.ServiceFilterFlags.LOAD))
            {
                respService.HasLoad = true;
                service.Load.Copy(respService.Load);

                if (initialResp)
                {
                    respService.Load.Action = FilterEntryActions.SET;
                }

                filter |= directoryServiceFilter & Directory.ServiceFilterFlags.LOAD;
            }

            if (service.HasLink && ((directoryServiceFilter & Directory.ServiceFilterFlags.LINK) == Directory.ServiceFilterFlags.LINK))
            {
                respService.HasLink = true;
                service.Link.Copy(respService.Link);

                if (initialResp)
                {
                    respService.Link.Action = FilterEntryActions.SET;
                }

                filter |= directoryServiceFilter & Directory.ServiceFilterFlags.LINK;
            }

            if (service.HasData && ((directoryServiceFilter & Directory.ServiceFilterFlags.DATA) == Directory.ServiceFilterFlags.DATA))
            {
                respService.HasData = true;
                service.Data.Copy(respService.Data);

                if (initialResp)
                {
                    respService.Data.Action = FilterEntryActions.SET;
                }

                filter |= directoryServiceFilter & Directory.ServiceFilterFlags.DATA;
            }

            if ((service.GroupStateList.Count != 0) && ((directoryServiceFilter & Directory.ServiceFilterFlags.GROUP) == Directory.ServiceFilterFlags.GROUP))
            {
                respService.GroupStateList = service.GroupStateList;

                filter |= directoryServiceFilter & Directory.ServiceFilterFlags.GROUP;
            }

            if (filter != 0)
            {
                if (initialResp)
                {
                    respService.Action = MapEntryActions.ADD;
                }
                else
                {
                    respService.Action = service.Action;
                }
                respService.ServiceId = service.ServiceId;
            }

            return filter;
        }

        internal void ReturnServiceToPool(List<Service> serviceList)
        {
            CacheLock.Enter();

            foreach (var service in serviceList)
            {
                ReturnService(service);
            }
            serviceList.Clear();

            CacheLock.Exit();
        }

        internal void ReturnService(Service service)
        {
            _servicePool.Enqueue(service);
        }

        private Service GetService()
        {
            Service service;

            if (_servicePool.Count == 0) service = new Service();
            else service = _servicePool.Dequeue();

            service.Clear();
            return service;
        }
    }

    internal class OmmNiProviderDirectoryStore : DirectoryServiceStore
    {
        private OmmNiProviderImpl m_OmmNiProviderImpl;
        private DirectoryCache m_DirectoryCacheAPIControl;

        public OmmNiProviderDirectoryStore(EmaObjectManager emaObjectManager, OmmNiProviderImpl ommNiProviderImpl, IOmmCommonImpl ommCommonImpl) 
            : base(emaObjectManager, OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE, ommCommonImpl)
        {
            m_OmmNiProviderImpl = ommNiProviderImpl;

            m_DirectoryCacheAPIControl = ommNiProviderImpl.NiProviderConfigImpl.DirectoryCache!;

            // The directory cache should always have at least one service here, even if it is the default service.
            if (m_DirectoryCacheAPIControl.DirectoryRefresh.ServiceList.Count != 0)
            {
                foreach (Service service in m_DirectoryCacheAPIControl.DirectoryRefresh.ServiceList)
                    AddToServiceMap(service);
            }

            m_DirectoryCache = new DirectoryCache(); /* This is used for user submitted source directory */
        }

        protected override bool AddServiceIdAndNamePair(int serviceId, string serviceName, StringBuilder errorText, out int errorCode)
        {
            errorCode = OmmInvalidUsageException.ErrorCodes.NONE;

            if(GetServiceIdByName(serviceName, out _))
            {
                errorText.Clear();
                errorText.Append($"Attempt to add a service with name of {serviceName} and Id of {serviceId}")
                    .Append(" while a service with the same id is already added.");
                errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION;
            }

            AddToServiceMap(serviceId, serviceName);

            if(m_OmmNiProviderImpl.LoggerClient != null && m_OmmNiProviderImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder text = m_OmmNiProviderImpl.GetStrBuilder();
                text.Append($"Detected Service with name of {serviceName} and Id of {serviceId}");
                m_OmmNiProviderImpl.LoggerClient.Trace(m_OmmNiProviderImpl.InstanceName, text.ToString());
            }

            return true;
        }

        protected override bool CheckExistingServiceId(int serviceId, StringBuilder errorText, out int errorCode)
        {
            errorCode = OmmInvalidUsageException.ErrorCodes.NONE;
            if (GetServiceNameById(serviceId, out var serviceName))
            {
                errorText.Clear();
                errorText.Append($"Attempt to add a service with name of {serviceName} and Id of {serviceId}")
                    .Append(" while a service with the same id is already added.");
                errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION;
                return false;
            }

            return true;
        }

        public DirectoryCache GetApiControlDirectory()
        {
            return m_DirectoryCacheAPIControl;
        }

    }

    internal class OmmIProviderDirectoryStore : DirectoryServiceStore
    {
        private OmmIProviderImpl m_OmmIProviderImpl;

        public OmmIProviderDirectoryStore(EmaObjectManager emaObjectManager, OmmIProviderImpl ommIProviderImpl) :
            base(emaObjectManager, OmmProviderConfig.ProviderRoleEnum.INTERACTIVE, ommIProviderImpl)
        {
            m_OmmIProviderImpl = ommIProviderImpl;

            /* m_DirectoryCache is used for both EMA's configuration and user submitted source directory */

            if (m_OmmIProviderImpl.ConfigImpl.AdminControlDirectory == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            {
                m_DirectoryCache = ommIProviderImpl.ConfigImpl.DirectoryCache;

                // The directory cache should always have at least one service here, even if it is the default service.
                if (m_DirectoryCache!.DirectoryRefresh.ServiceList.Count != 0)
                {
                    foreach (Service service in m_DirectoryCache.DirectoryRefresh.ServiceList)
                        AddToServiceMap(service);
                }
            }
            else
            {
                m_DirectoryCache = new DirectoryCache();
            }
        }

        public bool IsAcceptingRequests(int serviceId)
        {
            m_OmmIProviderImpl.GetUserLocker().Enter();
            bool acceptingRequests = true;

            try
            {
                Service? service = m_DirectoryCache!.GetService(serviceId);

                if (service != null)
                {
                    if(service.HasState && service.State.HasAcceptingRequests)
                    {
                        acceptingRequests = service.State.AcceptingRequests != 0;
                    }
                }
                else
                {
                    acceptingRequests = false;
                }

                return acceptingRequests;
            }
            finally
            {
                m_OmmIProviderImpl.GetUserLocker().Exit();
            }
        }

        public bool IsValidQosRange(int serviceId, IRequestMsg requestMsg)
        {
            m_OmmIProviderImpl.GetUserLocker().Enter();
            bool result = false;

            try
            {
                Service? service = m_DirectoryCache!.GetService(serviceId);

                if (service != null)
                {
                    if(service.HasInfo)
                    {
                        if(requestMsg.CheckHasQos() && requestMsg.CheckHasWorstQos())
                        {
                            if(service.Info.HasSupportQosRange)
                            {
                                if (service.Info.HasQos)
                                {
                                    var qosList = service.Info.QosList;
                                    foreach (var entry in qosList)
                                    {
                                        if (entry.IsInRange(requestMsg.Qos, requestMsg.WorstQos))
                                        {
                                            result = true;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    Qos qos = new ();
                                    qos.Rate(QosRates.TICK_BY_TICK);
                                    qos.Timeliness(QosTimeliness.REALTIME);

                                    if(qos.IsInRange(requestMsg.Qos, requestMsg.WorstQos))
                                    {
                                        result = true;
                                    }
                                }
                            }
                        }
                        else if(requestMsg.CheckHasQos())
                        {
                            if (service.Info.HasQos)
                            {
                                var qosList = service.Info.QosList;
                                foreach (var entry in qosList)
                                {
                                    if (entry.Equals(requestMsg.Qos))
                                    {
                                        result = true;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                Qos qos = new ();
                                qos.Rate(QosRates.TICK_BY_TICK);
                                qos.Timeliness(QosTimeliness.REALTIME);

                                if (qos.Equals(requestMsg.Qos))
                                {
                                    result = true;
                                }
                            }
                        }
                        else
                        {
                            Qos qos = new();
                            qos.Rate(QosRates.TICK_BY_TICK);
                            qos.Timeliness(QosTimeliness.REALTIME);

                            if (service.Info.HasQos)
                            {
                                var qosList = service.Info.QosList;
                                foreach (var entry in qosList)
                                {
                                    if (entry.Equals(qos))
                                    {
                                        result = true;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                return true;
                            }
                        }
                    }
                    else
                    {
                        result = true;
                    }
                    
                }

                return result;
            }
            finally
            {
                m_OmmIProviderImpl.GetUserLocker().Exit();
            }
        }

        protected override bool AddServiceIdAndNamePair(int serviceId, string serviceName, StringBuilder errorText, out int errorCode)
        {
            errorCode = 0;

            if (m_OmmIProviderImpl.ConfigImpl.AdminControlDirectory == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            {
                if(ServiceNameAndIdDict.ContainsKey(serviceName))
                {
                    errorText.Clear();
                    errorText.Append($"Attempt to add a service with name of {serviceName} and id of ")
                        .Append($"{serviceId}  while a service with the same id is already added.");
                    errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT;

                    return false;
                }

                AddToServiceMap(serviceId, serviceName);
            }
            else
            {
                if(ServiceNameAndIdDict.ContainsKey(serviceName) == false)
                {
                    AddToServiceMap(serviceId, serviceName);
                }
            }

            if(errorCode != 0 && m_OmmIProviderImpl.GetLoggerClient().IsTraceEnabled)
            {
                var stringBuilder = new StringBuilder(255);
                stringBuilder.Append($"Detected Service with name of {serviceName} and Id of {serviceId}");
                m_OmmIProviderImpl.GetLoggerClient().Trace(m_OmmIProviderImpl.InstanceName, stringBuilder.ToString());
            }

            return true;
        }

        protected override bool CheckExistingServiceId(int serviceId, StringBuilder errorText, out int errorCode)
        {
            errorCode = 0;

            if(m_OmmIProviderImpl.ConfigImpl.AdminControlDirectory == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            {
                if(ServiceIdAndNameDict.TryGetValue(serviceId, out string? serviceName))
                {
                    errorText.Clear();
                    errorText.Append($"Attempt to add a service with name of {serviceName}")
                        .Append($" and id of {serviceId} while a service with the same id is already added.");
                    errorCode = OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION;
                    return false;
                }
            }

            return true;
        }
    }

}
