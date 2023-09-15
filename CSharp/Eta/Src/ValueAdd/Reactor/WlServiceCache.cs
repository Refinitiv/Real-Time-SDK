/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using System.Collections.Generic;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// The watchlist service cache
    /// </summary>
    sealed internal class WlServiceCache
    {
        internal IDictionary<string, WlService> m_ServicesByNameTable = new Dictionary<string, WlService>();
        internal IDictionary<int, WlService> m_ServicesByIdTable = new Dictionary<int, WlService>();       

        internal Action<WlService>? ServiceAddedCallback;
        internal Action<WlService, bool>? ServiceRemovedCallback;
        internal Action<WlService>? ServiceUpdatedCallback;

        internal Queue<Service> ServicePool = new Queue<Service>();

        public LinkedList<WlService> ServiceList { get; private set; } = new LinkedList<WlService>();       

        /// <summary>
        /// Processes the list of services
        /// </summary>
        /// <param name="serviceList">the list of services obtained from the SOURCE domain message</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> object htat carries error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value representing the state of the operation</returns>
        public ReactorReturnCode ProcessServiceList(List<Service> serviceList, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode reactorReturnCode = ReactorReturnCode.SUCCESS;
            WlService? wlService;

            foreach (Service service in serviceList)
            {
                switch (service.Action)
                {
                    case MapEntryActions.ADD:
                        if ((reactorReturnCode = AddToCache(service, out wlService, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            return reactorReturnCode;
                        } 
                        else if (wlService != null)
                        {
                            ServiceAddedCallback!(wlService);
                        }                 
                        
                        break;
                    case MapEntryActions.UPDATE:
                        if (m_ServicesByIdTable.TryGetValue(service.ServiceId, out wlService)) 
                        {
                            if (wlService != null) // this is a change to an existing service
                            {
                                if (wlService!.RdmService!.HasInfo && service.HasInfo) // handle possible service name change (though provider shouldn't do this)
                                { 
                                    if (!wlService!.RdmService!.Info.ServiceName.Equals(service.Info.ServiceName))
                                    {
                                        m_ServicesByNameTable.Remove(wlService!.RdmService!.Info.ServiceName.ToString());
                                        if (!m_ServicesByNameTable.TryAdd(service.Info.ServiceName.ToString(), wlService))
                                        {
                                            return Reactor.PopulateErrorInfo(out errorInfo,
                                                ReactorReturnCode.FAILURE,
                                                "WlServiceCache.AddService",
                                                $"Failed to add service to cache: service with name {wlService!.RdmService!.Info.ServiceName} already exists.");
                                        }
                                    }
                                }
                                service.ApplyUpdate(wlService!.RdmService!);
                                ServiceUpdatedCallback!(wlService);
                            }
                            
                        }
                        else // service not in tables, this is the same as an add
                        {
                            if ((reactorReturnCode = AddToCache(service, out wlService, out errorInfo)) < ReactorReturnCode.SUCCESS)
                            {
                                return reactorReturnCode;
                            } 
                            else if (wlService != null)
                            {
                                ServiceAddedCallback!(wlService);
                            }
                        }
                           
                        break;
                    case MapEntryActions.DELETE:
                        if (m_ServicesByIdTable.TryGetValue(service.ServiceId, out wlService))
                        {
                            m_ServicesByIdTable.Remove(service.ServiceId);
                            if (wlService != null)
                            {
                                string? serviceName = null;
                                if (wlService!.RdmService!.HasInfo)
                                {
                                    serviceName = wlService.RdmService.Info.ServiceName.ToString();
                                    m_ServicesByNameTable.Remove(serviceName);
                                }
                                ServiceList.Remove(wlService);
                                ServiceRemovedCallback!(wlService, false);
                            }
                        }                       
                        break;
                    default:
                        Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE,
                            "WlServiceCache.ProcessServiceList",
                            $"Invalid map entry action {service.Action} received on directory service");
                        break;
                }

                // break out of loop when error encountered
                if (reactorReturnCode < ReactorReturnCode.SUCCESS)
                {
                    break;
                }
            }

            errorInfo = null;
            return reactorReturnCode;
        }

        /// <summary>
        /// Adds service to cache
        /// </summary>
        /// <param name="service">the service instance to be added</param>
        /// <param name="wlService">object associated with the RDM Service instance</param>
        /// <param name="errorInfo">error object carrying error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> indicating the status of the operation</returns>
        public ReactorReturnCode AddToCache(Service service, out WlService? wlService, out ReactorErrorInfo? errorInfo)
        {
            wlService = new WlService();
            wlService.RdmService = GetRdmServiceFormPool();
            service.Copy(wlService!.RdmService!);
            
            if (!m_ServicesByIdTable.TryAdd(service.ServiceId, wlService))
            {
                ServicePool.Enqueue(wlService.RdmService);
                wlService = null;
                return Reactor.PopulateErrorInfo(out errorInfo, 
                    ReactorReturnCode.FAILURE, 
                    "WlServiceCache.AddService", 
                    $"Failed to add service to cache: service with id {service.ServiceId} already exists.");
            }
            if (service.HasInfo)
            {
                if (!m_ServicesByNameTable.TryAdd(wlService!.RdmService!.Info.ServiceName.ToString(), wlService))
                {
                    ServicePool.Enqueue(wlService.RdmService);
                    wlService = null;
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        "WlServiceCache.AddService",
                        $"Failed to add service to cache: service with name {service.Info.ServiceName} already exists.");
                }
            }
            ServiceList.AddLast(wlService);

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Retrieve service id from service name.
        /// </summary>
        /// <param name="serviceName">the name of the service</param>
        /// <returns>the id of the service with the provided name</returns>
        public int ServiceId(string serviceName)
        {
            if (m_ServicesByNameTable.TryGetValue(serviceName, out var wlService))
            {
                if (wlService != null)
                {
                    return wlService!.RdmService!.ServiceId;
                }
                else
                {
                    return (int)ReactorReturnCode.PARAMETER_INVALID;
                }
            }
            return (int)ReactorReturnCode.PARAMETER_INVALID;
        }

        /// <summary>
        /// Retrieve service name from service id.
        /// </summary>
        /// <param name="serviceId">the id of the service</param>
        /// <returns>the name of the service corresponding to the provided id</returns>
        public string? ServiceName(int serviceId)
        {
            string? serviceName = null;
            if (m_ServicesByIdTable.TryGetValue(serviceId, out var wlService))
            {
                if (wlService != null && wlService!.RdmService!.HasInfo)
                {
                    serviceName = wlService.RdmService.Info.ServiceName.ToString();
                }
            }            
            return serviceName;
        }

        /// <summary>
        /// Retrieve service by service name.
        /// </summary>
        /// <param name="serviceName">the name of the service</param>
        /// <returns>the service corresponding to the provided name</returns>
        public WlService? Service(string serviceName)
        {
            if (m_ServicesByNameTable.TryGetValue(serviceName, out var service))
            {
                return service;
            }
            return null;
        }

        /// <summary>
        /// Retrieve service by service id
        /// </summary>
        /// <param name="serviceId">the id of the service</param>
        /// <returns>the service corresponding to the provided id</returns>
        public WlService? Service(int serviceId)
        {
            if (m_ServicesByIdTable.TryGetValue(serviceId, out var service))
            {
                return service;
            }
            return null;
        }

        /// <summary>
        /// Prepares DirectoryRefresh message based on the list of services received and on the service name requested
        /// </summary>
        /// <param name="directoryRefresh">the <see cref="DirectoryRefresh"/> message</param>
        /// <param name="serviceName">the service name</param>
        public void FillDirectoryRefreshServiceListFromCache(DirectoryRefresh directoryRefresh, string serviceName)
        {
            Service service;
            if (serviceName != null)
            {
                if (m_ServicesByNameTable.TryGetValue(serviceName, out var wlService))
                {
                    if (wlService != null)
                    {
                        service = GetRdmServiceFormPool();
                        wlService!.RdmService!.Copy(service);
                        directoryRefresh.ServiceList.Add(service);
                        SetFilterFlagsRefresh(directoryRefresh.Filter, directoryRefresh.ServiceList.First());
                    }
                }                
            }
            else if (directoryRefresh.HasServiceId)
            {
                if (m_ServicesByIdTable.TryGetValue(directoryRefresh.ServiceId, out var wlService))
                {
                    if (wlService != null)
                    {
                        service = GetRdmServiceFormPool();
                        wlService!.RdmService!.Copy(service);
                        directoryRefresh.ServiceList.Add(service);
                        SetFilterFlagsRefresh(directoryRefresh.Filter, directoryRefresh.ServiceList.First());
                    }
                }               
            }
            else
            {
                if (ServiceList.Count > 0)
                {
                    // Copy the service services here.
                    foreach (var wlService in ServiceList)
                    {
                        service = GetRdmServiceFormPool();
                        wlService.RdmService!.Copy(service);
                        SetFilterFlagsRefresh(directoryRefresh.Filter, service);
                        directoryRefresh.ServiceList.Add(service);
                    }
                }               
            }
        }

        public void ReturnServicesToPool(List<Service> serviceList)
        {
            serviceList.ForEach(s => ServicePool.Enqueue(s));
            serviceList.Clear();
        }

        private void AddServiceToUpdateMsgServiceList(Service s, DirectoryUpdate directoryUpdate)
        {
            Service service = GetRdmServiceFormPool();

            s.Copy(service);
            int ret = SetFilterFlagsUpdate(directoryUpdate.Filter, service, s);
            if (s.Action == MapEntryActions.DELETE || s.Action == MapEntryActions.ADD)
            {
                directoryUpdate.ServiceList.Add(service);
            }
            else // UPDATE
            {
                if (ret > 0 || directoryUpdate.Filter == 0)
                {
                    directoryUpdate.ServiceList.Add(service);
                }
                else
                {
                    // service shouldn't be added to directory update
                    ServicePool.Enqueue(service);
                }
            }
        }

        /// <summary>
        /// Fills DirectoryUpdate message based on the list of services received
        /// </summary>
        /// <param name="directoryUpdate">the <see cref="DirectoryUpdate"/> message</param>
        /// <param name="services">the list of services</param>
        public void FillDirectoryUpdateServiceListFromUpdateMsgServices(DirectoryUpdate directoryUpdate, List<Service> services)
        {
            Service s;
            if (directoryUpdate.HasServiceId)
            {
                for (int i = 0; i < services.Count; i++)
                {
                    s = services[i];
                    if (s.ServiceId == directoryUpdate.ServiceId)
                    {
                        AddServiceToUpdateMsgServiceList(s, directoryUpdate);
                        break;
                    }
                }
            }
            else
            {
                for (int i = 0; i < services.Count; i++)
                {
                    AddServiceToUpdateMsgServiceList(services[i], directoryUpdate);
                }
            }
        }

        /// <summary>
        /// Sets provided service flags based on the provided filter
        /// </summary>
        /// <param name="filter">the filter provided by the user</param>
        /// <param name="service">the Service instance to be modified</param>
        public void SetFilterFlagsRefresh(long filter, Service service)
        {
            // One service selected
            if (service.HasInfo && (filter & Eta.Rdm.Directory.ServiceFilterFlags.INFO) == 0)
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_INFO;
            }

            if (service.HasData && (filter & Eta.Rdm.Directory.ServiceFilterFlags.DATA) == 0)
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_DATA;
            }

            if (service.GroupStateList.Count > 0 && ((filter & Eta.Rdm.Directory.ServiceFilterFlags.GROUP) == 0))
            {
                service.GroupStateList.Clear(); // Remove group
            }

            if (service.HasLink && ((filter & Eta.Rdm.Directory.ServiceFilterFlags.LINK) == 0))
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_LINK;
            }

            if (service.HasLoad && (filter & Eta.Rdm.Directory.ServiceFilterFlags.LOAD) == 0)
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_LOAD;
            }

            if (service.HasState && (filter & Eta.Rdm.Directory.ServiceFilterFlags.STATE) == 0)
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_STATE;
            }
        }

        /// <summary>
        /// Sets current service's flags based on the flags of the service received during update
        /// </summary>
        /// <param name="filter">the filter provided</param>
        /// <param name="service">the current service</param>
        /// <param name="serviceReceived">the service received during update</param>
        /// <returns>the number of flags applied</returns>
        public int SetFilterFlagsUpdate(long filter, Service service, Service serviceReceived)
        {
            int retNumFilters = 0;
            if (serviceReceived.HasInfo && (filter & Eta.Rdm.Directory.ServiceFilterFlags.INFO) != 0)
            {
                service.HasInfo = true;
                retNumFilters++;
            }
            else
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_INFO;
            }

            if (serviceReceived.HasData && (filter & Eta.Rdm.Directory.ServiceFilterFlags.DATA) != 0)
            {
                service.HasData = true;
                retNumFilters++;
            }
            else
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_DATA;
            }

            if ((serviceReceived.GroupStateList.Count > 0) && (filter & Eta.Rdm.Directory.ServiceFilterFlags.GROUP) != 0)
            {
                retNumFilters++;
            }
            else
            {
                service.GroupStateList.Clear();
            }

            if (serviceReceived.HasLink && ((filter & Eta.Rdm.Directory.ServiceFilterFlags.LINK) != 0))
            {
                service.HasLink = true;
                retNumFilters++;
            }
            else
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_LINK;
            }

            if (serviceReceived.HasLoad && ((filter & Eta.Rdm.Directory.ServiceFilterFlags.LOAD) != 0))
            {
                service.HasLoad = true;
                retNumFilters++;
            }
            else
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_LOAD;
            }

            if (serviceReceived.HasState && (filter & Eta.Rdm.Directory.ServiceFilterFlags.STATE) != 0)
            {
                service.HasState = true;
                retNumFilters++;
            }
            else
            {
                service.Flags = service.Flags & ~ServiceFlags.HAS_STATE;
            }

            return retNumFilters;
        }

        /// <summary>
        /// Clear the service cache
        /// </summary>
        /// <param name="channelIsDown">true if the channel is down, false otherwise</param>
        public void ClearCache(bool channelIsDown)
        {   
            if (ServiceList.Count > 0)
            {
                foreach (var wlService in ServiceList)
                {
                    ServiceRemovedCallback!(wlService, channelIsDown);
                    wlService!.RdmService!.Clear();
                }
                ServiceList.Clear();
            }
            
            m_ServicesByNameTable.Clear();
            m_ServicesByIdTable.Clear();
        }

        /// <summary>
        /// Either gets a free Rdm Service instance from the pool or creates a new one if the pool is empty
        /// </summary>
        /// <returns>Free <see cref="Rdm.Service"/> instance</returns>
        public Service GetRdmServiceFormPool()
        {
            Service service;
            if (ServicePool.Count == 0)
            {
                service = new Service();
            }
            else
            {
                service = ServicePool.Dequeue();
                service.Clear();
            }
            return service;
        }

        /// <summary>
        /// Clears service cache for re-use.
        /// </summary>
        public void Clear()
        {
            m_ServicesByNameTable.Clear();
            m_ServicesByIdTable.Clear();
            ServiceList.Clear();
        }
    }
}
