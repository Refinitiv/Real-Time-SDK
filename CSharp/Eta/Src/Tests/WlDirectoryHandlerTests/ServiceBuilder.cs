/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class ServiceBuilder
    {
        public static void BuildRDMServiceGroup(List<ServiceGroup> groupStateList, FilterEntryActions action)
        {
            State state = new State();
            state.Text().Data("state");
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            ServiceGroup rdmServiceGroupState = new ServiceGroup();
            rdmServiceGroupState.Clear();
            rdmServiceGroupState.Flags = ServiceGroupFlags.HAS_MERGED_TO_GROUP | ServiceGroupFlags.HAS_STATUS;
            rdmServiceGroupState.Action = action;
            rdmServiceGroupState.Group.Data("group");
            if (rdmServiceGroupState.HasMergedToGroup)
            {
                rdmServiceGroupState.MergedToGroup.Data("mergedToGroup");
            }

            if (rdmServiceGroupState.HasStatus)
            {
                rdmServiceGroupState.Status.Text().Data("state");
                rdmServiceGroupState.Status.Code(StateCodes.FAILOVER_COMPLETED);
                rdmServiceGroupState.Status.DataState(DataStates.SUSPECT);
                rdmServiceGroupState.Status.StreamState(StreamStates.OPEN);
            }

            groupStateList.Add(rdmServiceGroupState);
        }

        public static void BuildRDMServiceData(ServiceData rdmServiceData, FilterEntryActions action)
        {
            rdmServiceData.Clear();

            rdmServiceData.Flags = ServiceDataFlags.HAS_DATA;
            rdmServiceData.Action = action;
            rdmServiceData.Type = 1;
            if (rdmServiceData.HasData)
            {
                rdmServiceData.Data.Data("data");
                rdmServiceData.DataType = Codec.DataTypes.ASCII_STRING;
            }
        }

        public static void BuildRDMServiceLoad(ServiceLoad rdmServiceLoad, FilterEntryActions action)
        {
            long loadFactor = 1;
            long openLimit = 1;
            long openWindow = 1;

            rdmServiceLoad.Clear();

            rdmServiceLoad.Flags = ServiceLoadFlags.HAS_LOAD_FACTOR | ServiceLoadFlags.HAS_OPEN_LIMIT | ServiceLoadFlags.HAS_OPEN_WINDOW;
            rdmServiceLoad.Action = action;
            if (rdmServiceLoad.HasOpenLimit)
            {
                rdmServiceLoad.OpenLimit = openLimit;
            }

            if (rdmServiceLoad.HasOpenWindow)
            {
                rdmServiceLoad.OpenWindow = openWindow;
            }

            if (rdmServiceLoad.HasLoadFactor)
            {
                rdmServiceLoad.LoadFactor = loadFactor;
            }
        }

        public static void BuildRDMServiceState(ServiceState rdmServiceState, FilterEntryActions action)
        {
            long acceptingRequests = 1;
            long serviceState = 1;

            rdmServiceState.Clear();

            rdmServiceState.Flags = ServiceStateFlags.HAS_ACCEPTING_REQS | ServiceStateFlags.HAS_STATUS;
            rdmServiceState.Action = action;
            rdmServiceState.ServiceStateVal = serviceState;

            rdmServiceState.AcceptingRequests = acceptingRequests;

            if (rdmServiceState.HasStatus)
            {
                rdmServiceState.Status.Text().Data("state");
                rdmServiceState.Status.Code(StateCodes.FAILOVER_COMPLETED);
                rdmServiceState.Status.DataState(DataStates.SUSPECT);
                rdmServiceState.Status.StreamState(StreamStates.OPEN);
            }
        }

        public static void BuildRDMServiceLink(ServiceLinkInfo serviceLinkInfo, FilterEntryActions action)
        {
            long linkCode = 1;
            long linkState = 1;
            long type = DataTypes.ASCII_STRING;

            ServiceLink serviceLink = new ServiceLink();

            serviceLinkInfo.LinkList.Add(serviceLink);
            serviceLinkInfo.Action = action;
            serviceLink.Clear();

            serviceLink.Action = MapEntryActions.ADD;
            serviceLink.Flags = ServiceLinkFlags.HAS_CODE | ServiceLinkFlags.HAS_TEXT | ServiceLinkFlags.HAS_TYPE;
            serviceLink.Name.Data("name");
            serviceLink.LinkState = linkState;

            if (serviceLink.HasCode)
            {
                serviceLink.LinkCode = linkCode;
            }

            if (serviceLink.HasText)
            {
                serviceLink.Text.Data("text");
            }

            if (serviceLink.HasType)
            {
                serviceLink.Type = type;
            }
        }

        public static void BuildRDMServiceInfo(ServiceInfo rdmServiceInfo, FilterEntryActions action)
        {
            ServiceInfoFlags flags = ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS
                | ServiceInfoFlags.HAS_DICTS_PROVIDED
                | ServiceInfoFlags.HAS_DICTS_USED
                | ServiceInfoFlags.HAS_IS_SOURCE
                | ServiceInfoFlags.HAS_ITEM_LIST
                | ServiceInfoFlags.HAS_QOS
                | ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS
                | ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE
                | ServiceInfoFlags.HAS_VENDOR;

            Qos qos = new Qos();
            qos.IsDynamic = true;
            qos.Rate(QosRates.JIT_CONFLATED);
            qos.Timeliness(QosTimeliness.DELAYED);
            qos.TimeInfo(1);
            rdmServiceInfo.Clear();
            rdmServiceInfo.Action = action;
            rdmServiceInfo.Flags = flags;
            rdmServiceInfo.AcceptConsumerStatus = 1;
            rdmServiceInfo.DictionariesProvidedList.Add("dictprov1");
            rdmServiceInfo.DictionariesUsedList.Add("dictused1");
            rdmServiceInfo.IsSource = 1;
            rdmServiceInfo.ItemList.Data("itemList");
            rdmServiceInfo.QosList.Add(qos);
            rdmServiceInfo.SupportsOOBSnapshots = 1;
            rdmServiceInfo.SupportsQosRange = 1;
            rdmServiceInfo.Vendor.Data("vendor");
            rdmServiceInfo.CapabilitiesList.Add(1);
            rdmServiceInfo.ServiceName.Data("servicename");
        }

        public static void BuildRDMService(Service rdmService, ServiceFlags flags, MapEntryActions serviceAddOrDeleteAction, FilterEntryActions filterAddOrClearAction)
        {
            rdmService.Clear();
            rdmService.Action = serviceAddOrDeleteAction;
            rdmService.Flags = flags;

            // checking only set action for the filters
            // other filter unit tests cover other filter actions
            if (rdmService.HasInfo)
                BuildRDMServiceInfo(rdmService.Info, filterAddOrClearAction);
            if (rdmService.HasLink)
                BuildRDMServiceLink(rdmService.Link, filterAddOrClearAction);
            if (rdmService.HasState)
                BuildRDMServiceState(rdmService.State, filterAddOrClearAction);
            if (rdmService.HasLoad)
                BuildRDMServiceLoad(rdmService.Load, filterAddOrClearAction);
            if (rdmService.HasData)
                BuildRDMServiceData(rdmService.Data, filterAddOrClearAction);
            BuildRDMServiceGroup(rdmService.GroupStateList, filterAddOrClearAction);
        }

    }
}
