/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;
using Xunit;
using Xunit.Categories;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class WlServiceCacheTests
    {
        [Fact]
        [Category("WlServiceCacheTest")]
        public void TestProcessServiceListMethod()
        {
            int servicesAdded = 0;
            int servicesUpdated = 0;
            int servicesDeleted = 0;

            WlServiceCache serviceCache = new WlServiceCache();
            serviceCache.ServiceAddedCallback = wlService => servicesAdded++;
            serviceCache.ServiceUpdatedCallback = wlService => servicesUpdated++;
            serviceCache.ServiceRemovedCallback = (wlService, arg) => servicesDeleted++;

            List<Service> serviceList = new List<Service>();

            Service service1 = new Service();
            service1.ServiceId = 1;
            ServiceBuilder.BuildRDMService(service1, ServiceFlags.HAS_INFO | ServiceFlags.HAS_DATA | ServiceFlags.HAS_STATE, 
                Codec.MapEntryActions.ADD,
                Codec.FilterEntryActions.SET);
            if (service1.HasInfo)
            {
                service1.Info.ServiceName.Data("Service1");
            }
            Service service2 = new Service();
            service2.ServiceId = 2;
            ServiceBuilder.BuildRDMService(service2, ServiceFlags.HAS_INFO | ServiceFlags.HAS_DATA | ServiceFlags.HAS_LOAD,
                Codec.MapEntryActions.ADD,
                Codec.FilterEntryActions.SET);
            if (service2.HasInfo)
            {
                service2.Info.ServiceName.Data("Service2");
            }

            serviceList.Add(service1);
            serviceList.Add(service2);

            ReactorErrorInfo errorInfo;
            serviceCache.ProcessServiceList(serviceList, out errorInfo);

            Assert.Null(errorInfo);
            Assert.Equal(2, servicesAdded);
            Assert.Equal(2, serviceCache.ServiceList.Count);
            Assert.Equal(2, serviceCache.m_ServicesByNameTable.Count);

            service1.Action = Codec.MapEntryActions.DELETE;
            serviceList.Remove(service2);

            serviceCache.ProcessServiceList(serviceList, out errorInfo);
            Assert.Null(errorInfo);
            Assert.Equal(2, servicesAdded);
            Assert.Equal(1, servicesDeleted);
            Assert.Single(serviceCache.ServiceList);
            Assert.Single(serviceCache.m_ServicesByNameTable);

            ServiceBuilder.BuildRDMService(service2, ServiceFlags.HAS_INFO | ServiceFlags.HAS_DATA | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK,
                Codec.MapEntryActions.UPDATE,
                Codec.FilterEntryActions.SET);
            service2.Info.ServiceName.Data("Service2");

            serviceList.Add(service2);
            serviceList.Remove(service1);

            serviceCache.ProcessServiceList(serviceList, out errorInfo);
            Assert.Null(errorInfo);
            Assert.Equal(2, servicesAdded);
            Assert.Equal(1, servicesDeleted);
            Assert.Equal(1, servicesUpdated);
            Assert.Single(serviceCache.ServiceList);
            Assert.Single(serviceCache.m_ServicesByNameTable);
            Assert.True(serviceCache.m_ServicesByIdTable.ContainsKey(service2.ServiceId));
            Assert.True(serviceCache.m_ServicesByNameTable.ContainsKey("Service2"));
            serviceCache.m_ServicesByIdTable.TryGetValue(service2.ServiceId, out var s);
            Assert.Equal(ServiceFlags.HAS_INFO | ServiceFlags.HAS_DATA | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK, s.RdmService.Flags);
        }

        [Fact]
        [Category("WlServiceCacheTest")]
        public void TestFillDirectoryRefreshServiceListFromCache()
        {
            int servicesAdded = 0;
            int servicesUpdated = 0;
            int servicesDeleted = 0;

            WlServiceCache serviceCache = new WlServiceCache();
            serviceCache.ServiceAddedCallback = wlService => servicesAdded++;
            serviceCache.ServiceUpdatedCallback = wlService => servicesUpdated++;
            serviceCache.ServiceRemovedCallback = (wlService, arg) => servicesDeleted++;

            List<Service> serviceList = new List<Service>();

            Service service1 = new Service();
            service1.ServiceId = 1;
            ServiceBuilder.BuildRDMService(service1, ServiceFlags.HAS_INFO | ServiceFlags.HAS_DATA | ServiceFlags.HAS_STATE | ServiceFlags.HAS_LOAD,
                Codec.MapEntryActions.ADD,
                Codec.FilterEntryActions.SET);
            if (service1.HasInfo)
            {
                service1.Info.ServiceName.Data("Service1");
            }
            Service service2 = new Service();
            service2.ServiceId = 2;
            ServiceBuilder.BuildRDMService(service2, ServiceFlags.HAS_INFO | ServiceFlags.HAS_DATA | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK,
                Codec.MapEntryActions.ADD,
                Codec.FilterEntryActions.SET);
            if (service2.HasInfo)
            {
                service2.Info.ServiceName.Data("Service2");
            }

            serviceList.Add(service1);
            serviceList.Add(service2);

            ReactorErrorInfo errorInfo;
            serviceCache.ProcessServiceList(serviceList, out errorInfo);

            DirectoryRefresh directoryRefresh = new DirectoryRefresh();
            directoryRefresh.ServiceId = 2;
            directoryRefresh.HasServiceId = true;
            directoryRefresh.Solicited = true;
            directoryRefresh.StreamId = 5;
            directoryRefresh.Filter = Rdm.Directory.ServiceFilterFlags.INFO | Rdm.Directory.ServiceFilterFlags.LINK;

            serviceCache.FillDirectoryRefreshServiceListFromCache(directoryRefresh, "Service2");

            Assert.Single(directoryRefresh.ServiceList);
            Assert.Equal(2, directoryRefresh.ServiceList[0].ServiceId);
            Assert.True(directoryRefresh.ServiceList[0].HasInfo);
            Assert.True(directoryRefresh.ServiceList[0].HasLink);
            Assert.False(directoryRefresh.ServiceList[0].HasData);
            Assert.False(directoryRefresh.ServiceList[0].HasLoad);
        }

        [Fact]
        [Category("WlServiceCacheTest")]
        public void TestFillDirectoryUpdateServiceListFromUpdateMsgServices()
        {
            int servicesAdded = 0;
            int servicesUpdated = 0;
            int servicesDeleted = 0;

            WlServiceCache serviceCache = new WlServiceCache();
            serviceCache.ServiceAddedCallback = wlService => servicesAdded++;
            serviceCache.ServiceUpdatedCallback = wlService => servicesUpdated++;
            serviceCache.ServiceRemovedCallback = (wlService, arg) => servicesDeleted++;

            List<Service> serviceList = new List<Service>();

            Service service1 = new Service();
            service1.ServiceId = 1;
            ServiceBuilder.BuildRDMService(service1, ServiceFlags.HAS_INFO | ServiceFlags.HAS_DATA | ServiceFlags.HAS_STATE,
                Codec.MapEntryActions.ADD,
                Codec.FilterEntryActions.SET);
            if (service1.HasInfo)
            {
                service1.Info.ServiceName.Data("Service1");
            }

            Service service2 = new Service();
            service2.ServiceId = 2;
            ServiceBuilder.BuildRDMService(service2, ServiceFlags.HAS_INFO | ServiceFlags.HAS_DATA | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK,
                Codec.MapEntryActions.UPDATE,
                Codec.FilterEntryActions.SET);
            if (service2.HasInfo)
            {
                service2.Info.ServiceName.Data("Service2");
            }

            serviceList.Add(service1);
            serviceList.Add(service2);

            ReactorErrorInfo errorInfo;
            serviceCache.ProcessServiceList(serviceList, out errorInfo);

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.ServiceId = 2;
            directoryUpdate.HasServiceId = true;
            directoryUpdate.StreamId = 5;
            directoryUpdate.Filter = Rdm.Directory.ServiceFilterFlags.INFO | Rdm.Directory.ServiceFilterFlags.LINK;

            serviceCache.FillDirectoryUpdateServiceListFromUpdateMsgServices(directoryUpdate, serviceList);

            Assert.Single(directoryUpdate.ServiceList);
            Assert.Equal(2, directoryUpdate.ServiceList[0].ServiceId);
            Assert.True(directoryUpdate.ServiceList[0].HasInfo);
            Assert.True(directoryUpdate.ServiceList[0].HasLink);
            Assert.False(directoryUpdate.ServiceList[0].HasData);
            Assert.False(directoryUpdate.ServiceList[0].HasLoad);
        }
    }
}
