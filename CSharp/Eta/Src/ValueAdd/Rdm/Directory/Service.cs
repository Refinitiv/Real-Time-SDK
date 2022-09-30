/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using System.Diagnostics;
using System.Text;
using static Refinitiv.Eta.Rdm.Directory;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service. Contains information about a particular service.
    /// </summary>
    public class Service
    {
        private ServiceInfo _info = new ServiceInfo();
        private ServiceState _state = new ServiceState();
        private ServiceLoad _load = new ServiceLoad();
        private ServiceData _data = new ServiceData();
        private ServiceLinkInfo _link = new ServiceLinkInfo();
        private List<ServiceGroup> _groupStateList = new List<ServiceGroup>();

        /// <summary>
        /// Service flags.
        /// </summary>
        public ServiceFlags Flags { get; set; }
        /// <summary>
        /// The number identifying this service.
        /// </summary>
        public int ServiceId { get; set; }
        /// <summary>
        /// The info filter for this service.
        /// </summary>
        public ServiceInfo Info { get => _info; set { Debug.Assert(value != null); value.Copy(_info); } }
        /// <summary>
        /// The state filter for this service.
        /// </summary>
        public ServiceState State { get => _state; set { Debug.Assert(value != null); value.Copy(_state); } }
        /// <summary>
        /// The list of group filters for this service.
        /// </summary>
        public List<ServiceGroup> GroupStateList { 
            get => _groupStateList; 
            set 
            {
                Debug.Assert(value != null);
                _groupStateList.Clear();
                _groupStateList.AddRange(value);
            } 
        }
        /// <summary>
        /// The load filter for this service.
        /// </summary>
        public ServiceLoad Load { get => _load; set { Debug.Assert(value != null); value.Copy(_load); } }
        /// <summary>
        /// The data filter for this service.
        /// </summary>
        public ServiceData Data { get => _data; set { Debug.Assert(value != null); value.Copy(_data); } }
        /// <summary>
        /// The link filter for this service.
        /// </summary>
        public ServiceLinkInfo Link { get => _link; set { Debug.Assert(value != null); value.Copy(_link); } }
        /// <summary>
        /// Information about Sequenced Multicast connections with regards to EDF connections 
        /// to Snapshot server, Reference Data Server, Gap Fill and Request servers, and multicast groups.
        /// </summary>
        public ServiceSeqMcastInfo SeqMcast { get; set; } = new ServiceSeqMcastInfo();
        
        /// <summary>
        /// Action associated with this service.
        /// </summary>
        public MapEntryActions Action { get; set; } = MapEntryActions.ADD;

        private StringBuilder stringBuf = new StringBuilder();
        private const string eol = "\n";
        private const string tab = "\t";

        private FilterEntry filterEntry = new FilterEntry();
        private FilterList filterList = new FilterList();

        /// <summary>
        /// Checks the presence of the info field.
        /// </summary>
        public bool HasInfo { get => (Flags & ServiceFlags.HAS_INFO) != 0; set { if (value) Flags |= ServiceFlags.HAS_INFO; else Flags &= ~ServiceFlags.HAS_INFO; } }
        /// <summary>
        /// Checks the presence of the data field.
        /// </summary>
        public bool HasData { get => (Flags & ServiceFlags.HAS_DATA) != 0; set { if (value) Flags |= ServiceFlags.HAS_DATA; else Flags &= ~ServiceFlags.HAS_DATA; } }
        /// <summary>
        /// Checks the presence of the load field.
        /// </summary>
        public bool HasLoad { get => (Flags & ServiceFlags.HAS_LOAD) != 0; set { if (value) Flags |= ServiceFlags.HAS_LOAD; else Flags &= ~ServiceFlags.HAS_LOAD; } }
        /// <summary>
        /// Checks the presence of the link field.
        /// </summary>
        public bool HasLink { get => (Flags & ServiceFlags.HAS_LINK) != 0; set { if (value) Flags |= ServiceFlags.HAS_LINK; else Flags &= ~ServiceFlags.HAS_LINK; } }
        /// <summary>
        /// Checks the presence of the state field.
        /// </summary>
        public bool HasState { get => (Flags & ServiceFlags.HAS_STATE) != 0; set { if (value) Flags |= ServiceFlags.HAS_STATE; else Flags &= ~ServiceFlags.HAS_STATE; } }


        public Service()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current Service instance.
        /// </summary>
        public void Clear()
        {
            Flags = 0;
            Action = MapEntryActions.ADD;
            Info.Clear();
            State.Clear();
            GroupStateList.Clear();
            Load.Clear();
            Data.Clear();
            Link.Clear();
            SeqMcast.Clear();
        }

        /// <summary>
        /// Encode an RDM Service entry.
        /// </summary>
        /// <param name="encIter"><see cref="EncodeIterator"/> which is used to encode the Serivce instance.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation.</returns>
        public CodecReturnCode Encode(EncodeIterator encIter)
        {
            filterList.Clear();
            filterList.Flags = (int)FilterEntryFlags.NONE;
            filterList.ContainerType = Codec.DataTypes.ELEMENT_LIST;
            CodecReturnCode ret = filterList.EncodeInit(encIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            if (HasInfo)
            {
                ret = ServiceFilterEncode(encIter, ServiceFilterIds.INFO);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasData)
            {
                ret = ServiceFilterEncode(encIter, ServiceFilterIds.DATA);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasLink)
            {
                ret = ServiceFilterEncode(encIter, ServiceFilterIds.LINK);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasLoad)
            {
                ret = ServiceFilterEncode(encIter, ServiceFilterIds.LOAD);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasState)
            {
                ret = ServiceFilterEncode(encIter, ServiceFilterIds.STATE);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (GroupStateList.Count > 0)
            {
                ret = ServiceFilterEncode(encIter, (int)ServiceFilterIds.GROUP);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            return filterList.EncodeComplete(encIter, true);
        }

        private CodecReturnCode ServiceFilterEncode(EncodeIterator encIter, int filterId)
        {
            filterEntry.Clear();
            filterEntry.Flags = FilterEntryFlags.NONE;
            filterEntry.Id = filterId;

            switch (filterId)
            {
                case ServiceFilterIds.DATA:
                    return EncodeDataFilter(encIter);
                case ServiceFilterIds.INFO:
                    return EncodeInfoFilter(encIter);
                case ServiceFilterIds.LINK:
                    return EncodeLinkFilter(encIter);
                case ServiceFilterIds.LOAD:
                    return EncodeLoadFilter(encIter);
                case ServiceFilterIds.STATE:
                    return EncodeStateFilter(encIter);
                case ServiceFilterIds.GROUP:
                    return EncodeGroupFilter(encIter);
                default:
                    Debug.Assert(false);
                    return CodecReturnCode.FAILURE;
            }
        }

        private CodecReturnCode EncodeInfoFilter(EncodeIterator encIter)
        {
            filterEntry.Action = Info.Action;
            if (filterEntry.Action == FilterEntryActions.CLEAR)
                return filterEntry.Encode(encIter);

            CodecReturnCode ret = filterEntry.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            ret = Info.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            return filterEntry.EncodeComplete(encIter, true);
        }

        private CodecReturnCode EncodeDataFilter(EncodeIterator encIter)
        {
            filterEntry.Action = Data.Action;
            if (filterEntry.Action == FilterEntryActions.CLEAR)
                return filterEntry.Encode(encIter);

            CodecReturnCode ret = filterEntry.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            ret = Data.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            return filterEntry.EncodeComplete(encIter, true);
        }

        private CodecReturnCode EncodeStateFilter(EncodeIterator encIter)
        {
            filterEntry.Action = State.Action;
            if (filterEntry.Action == FilterEntryActions.CLEAR)
                return filterEntry.Encode(encIter);

            CodecReturnCode ret = filterEntry.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            ret = State.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            return filterEntry.EncodeComplete(encIter, true);
        }

        private CodecReturnCode EncodeLoadFilter(EncodeIterator encIter)
        {
            filterEntry.Action = Load.Action;
            if (filterEntry.Action == FilterEntryActions.CLEAR)
                return filterEntry.Encode(encIter);

            CodecReturnCode ret = filterEntry.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            ret = Load.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            return filterEntry.EncodeComplete(encIter, true);
        }

        private CodecReturnCode EncodeLinkFilter(EncodeIterator encIter)
        {
            filterEntry.Action = Link.Action;
            filterEntry.ContainerType = Codec.DataTypes.MAP;
            filterEntry.ApplyHasContainerType();

            if (filterEntry.Action == FilterEntryActions.CLEAR)
                return filterEntry.Encode(encIter);

            CodecReturnCode ret = filterEntry.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            ret = Link.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            return filterEntry.EncodeComplete(encIter, true);
        }

        private CodecReturnCode EncodeGroupFilter(EncodeIterator encIter)
        {
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            foreach (ServiceGroup group in GroupStateList)
            {
                filterEntry.Clear();
                filterEntry.Flags = FilterEntryFlags.NONE;
                filterEntry.Id = ServiceFilterIds.GROUP;
                filterEntry.Action = group.Action;
                if (filterEntry.Action == FilterEntryActions.CLEAR)
                {
                    ret = filterEntry.Encode(encIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    continue;
                }

                ret = filterEntry.EncodeInit(encIter, 0);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                ret = group.Encode(encIter);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                ret = filterEntry.EncodeComplete(encIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            return ret;
        }

        /// <summary>
        /// Decode Service entry.
        /// </summary>
        /// <param name="dIter"><see cref="DecodeIterator"/> that decodes Service entry.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation.</returns>
        public CodecReturnCode Decode(DecodeIterator dIter)
        {
            Clear();
            filterEntry.Clear();
            filterList.Clear();

            CodecReturnCode ret = filterList.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            while ((ret = filterEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                ret = DecodeFilter(dIter);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode DecodeFilter(DecodeIterator dIter)
        {
            CodecReturnCode returnCode = CodecReturnCode.SUCCESS;
            switch (filterEntry.Id)
            {
                case ServiceFilterIds.INFO:
                    ServiceInfo infoFilter = Info;
                    HasInfo = true;
                    if (filterEntry.Action != FilterEntryActions.CLEAR)
                    {
                        returnCode = infoFilter.Decode(dIter);
                    }
                    infoFilter.Action = filterEntry.Action;
                    break;
                case ServiceFilterIds.STATE:
                    ServiceState stateFilter = State;
                    HasState = true;
                    if (filterEntry.Action != FilterEntryActions.CLEAR)
                    {
                        returnCode = stateFilter.Decode(dIter);
                    }
                    stateFilter.Action = filterEntry.Action;
                    break;
                case ServiceFilterIds.GROUP:
                    ServiceGroup groupFilter = new ServiceGroup();
                    GroupStateList.Add(groupFilter);
                    if (filterEntry.Action != FilterEntryActions.CLEAR)
                    {
                        returnCode = groupFilter.Decode(dIter);
                    }
                    groupFilter.Action = filterEntry.Action;
                    break;
                case ServiceFilterIds.LOAD:
                    ServiceLoad loadFilter = Load;
                    HasLoad = true;
                    if (filterEntry.Action != FilterEntryActions.CLEAR)
                    {
                        returnCode = loadFilter.Decode(dIter);
                    }
                    loadFilter.Action = filterEntry.Action;
                    break;
                case ServiceFilterIds.DATA:
                    ServiceData dataFilter = Data;
                    HasData = true;
                    if (filterEntry.Action != FilterEntryActions.CLEAR)
                    {
                        returnCode = dataFilter.Decode(dIter);
                    }
                    dataFilter.Action = filterEntry.Action;
                    break;
                case ServiceFilterIds.LINK:
                    ServiceLinkInfo linkFilter = Link;
                    HasLink = true;
                    if (filterEntry.Action != FilterEntryActions.CLEAR)
                    {
                        returnCode = linkFilter.Decode(dIter);
                    }
                    linkFilter.Action = filterEntry.Action;
                    break;
                case ServiceFilterIds.SEQ_MCAST:
                    ServiceSeqMcastInfo SeqMcastFilter = SeqMcast;
                    HasLink = true;
                    if (filterEntry.Action != FilterEntryActions.CLEAR)
                    {
                        returnCode = SeqMcast.Decode(dIter);
                    }
                    SeqMcast.Action = filterEntry.Action;
                    break;
                default:
                    return CodecReturnCode.FAILURE;
            }

            return returnCode;
        }

        /// <summary>
        /// Performs deep copy of the current Service instance into the destination object.
        /// </summary>
        /// <param name="destService">The Service which the current instance is copied to.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation.</returns>
        public CodecReturnCode Copy(Service destService)
        {
            Debug.Assert(destService != null);
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            destService.Clear();
            destService.Action = Action;
            destService.ServiceId = ServiceId;

            destService.HasInfo = HasInfo;
            if (HasInfo)
            {
                ret = Info.Copy(destService.Info);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            destService.HasData = HasData;
            if (HasData)
            {
                ret = Data.Copy(destService.Data);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            foreach (ServiceGroup group in GroupStateList)
            {
                ServiceGroup destGroup = new ServiceGroup();
                destService.GroupStateList.Add(destGroup);
                ret = group.Copy(destGroup);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            destService.HasLink = HasLink;
            if (HasLink)
            {
                ret = Link.Copy(destService.Link);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            destService.HasLoad = HasLoad;
            if (HasLoad)
            {
                ret = Load.Copy(destService.Load);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            destService.HasState = HasState;
            if (HasState)
            {
                ret = State.Copy(destService.State);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Performs an update of this  object to the destination object. 
        /// Only updated filter entries are copied to the destination.
        /// </summary>
        /// <param name="destService">Service object to be updated by this object. It cannot be null.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation.</returns>
        public CodecReturnCode ApplyUpdate(Service destService)
        {
            Debug.Assert(destService != null);
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            destService.Action = Action;
            destService.ServiceId = ServiceId;
            destService.HasInfo = HasInfo;
            if (HasInfo)
            {
                ret = Info.Update(destService.Info);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            destService.HasData = HasData;
            if (HasData)
            {
                ret = Data.Update(destService.Data);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            foreach (ServiceGroup group in GroupStateList)
            {
                ServiceGroup destGroup = new ServiceGroup();
                destService.GroupStateList.Add(destGroup);
                ret = group.Copy(destGroup);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            destService.HasLink = HasLink;
            if (HasLink)
            {
                ret = Link.Update(destService.Link);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            destService.HasLoad = HasLoad;
            if (HasLoad)
            {
                ret = Load.Update(destService.Load);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            destService.HasState = HasState;
            if (HasState)
            {
                ret = State.Update(destService.State);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            stringBuf.Clear();
            stringBuf.Append(tab);
            stringBuf.Append("Service:");
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("serviceId: ");
            stringBuf.Append(ServiceId);
            stringBuf.Append(eol);

            if (HasInfo)
            {
                stringBuf.Append(Info.ToString());
            }

            if (HasData)
            {
                stringBuf.Append(Data.ToString());
            }

            if (HasLink)
            {
                stringBuf.Append(Link.ToString());
            }

            if (HasState)
            {
                stringBuf.Append(State.ToString());
            }

            if (HasLoad)
            {
                stringBuf.Append(Load.ToString());
            }

            if (GroupStateList.Count > 0)
            {
                foreach(var state in GroupStateList)
                {
                    stringBuf.Append(tab);
                    stringBuf.Append(state.ToString());
                }
                
            }

            return stringBuf.ToString();
        }
    }
}
