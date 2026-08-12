using LSEG.Ema.Access;
using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Internal;
using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using System.Text;
using FilterEntryActions = LSEG.Eta.Codec.FilterEntryActions;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Service. Contains information about a particular service.
    /// </summary>
    public sealed class DirectoryService : ICloneable
    {
        private readonly StringBuilder m_ToString = new();
        private DirectoryMapAction m_Action = DirectoryMapAction.ADD;
        private ushort m_ServiceId;
        private readonly DirectoryServiceInfo m_Info = new();
        private readonly DirectoryServiceState m_State = new();
        private readonly List<DirectoryServiceGroup> m_GroupStateList = new();
        private readonly FluentListBuilder<DirectoryServiceGroup> m_GroupStateListBuilder;
        private readonly DirectoryServiceLoad m_Load = new();
        private readonly DirectoryServiceData m_Data = new();
        private readonly DirectoryServiceLinkInfo m_Link = new();

        /// <summary>
        /// Initializes a new instance of the DirectoryService class.
        /// </summary>
        public DirectoryService()
        {
            m_GroupStateListBuilder = new(m_GroupStateList);
        }

        /// <summary>
        /// Action associated with this service.
        /// </summary>
        public DirectoryMapAction Action() => m_Action;
        /// <summary>
        /// Action associated with this service.
        /// </summary>
        public DirectoryService Action(DirectoryMapAction value)
        {
            if (!System.Enum.IsDefined(value))
                throw new OmmInvalidUsageException($"Invalid action value: {value}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Action = value;
            return this;
        }

        /// <summary>
        /// Gets or sets info presence flag.
        /// </summary>
        public bool HasInfo { get; private set; }

        /// <summary>
        /// Gets or sets presence of the data field.
        /// </summary>
        public bool HasData { get; private set; }

        /// <summary>
        /// Gets or sets presence of the load field.
        /// </summary>
        public bool HasLoad { get; private set; }

        /// <summary>
        /// Gets or sets presence of the link field.
        /// </summary>
        public bool HasLink { get; private set; }

        /// <summary>
        /// Gets or sets presence of the state field.
        /// </summary>
        public bool HasState { get; private set; }

        /// <summary>
        /// Number identifying this service.
        /// It's not a part of encoded data.
        /// </summary>
        public ushort ServiceId() => m_ServiceId;
        /// <summary>
        /// Number identifying this service.
        /// It's not a part of encoded data.
        /// </summary>
        public DirectoryService ServiceId(ushort value)
        {
            m_ServiceId = value;
            return this;
        }

        /// <summary>
        /// List of group filters for this service.
        /// </summary>
        public IList<DirectoryServiceGroup> GroupStateList() => m_GroupStateList;
        /// <summary>
        /// List of group filters for this service.
        /// </summary>
        public DirectoryService GroupStateList(IList<DirectoryServiceGroup> value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("GroupStateList cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_GroupStateList.Clear();
            m_GroupStateList.AddRange(value);
            return this;
        }
        /// <summary>
        /// List of group filters for this service.
        /// </summary>
        public DirectoryService GroupStateList(Action<IFluentListBuilder<DirectoryServiceGroup>> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            buildAction(m_GroupStateListBuilder);
            return this;
        }

        /// <summary>
        /// Info filter for this service.
        /// </summary>
        public DirectoryServiceInfo Info()
        {
            if (!HasInfo)
                throw new OmmInvalidUsageException("Info element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_Info;
        }
        /// <summary>
        /// Info filter for this service.
        /// </summary>
        public DirectoryService Info(DirectoryServiceInfo value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("Info cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Info.CopyFrom(value);
            HasInfo = true;
            return this;
        }
        /// <summary>
        /// Info filter for this service.
        /// </summary>
        public DirectoryService Info(Action<DirectoryServiceInfo> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            buildAction(m_Info);
            HasInfo = true;
            return this;
        }

        /// <summary>
        /// State filter for this service.
        /// </summary>
        public DirectoryServiceState State()
        {
            if (!HasState)
                throw new OmmInvalidUsageException("State element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_State;
        }
        /// <summary>
        /// State filter for this service.
        /// </summary>
        public DirectoryService State(DirectoryServiceState value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("State cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_State.CopyFrom(value);
            HasState = true;
            return this;
        }
        /// <summary>
        /// State filter for this service.
        /// </summary>
        public DirectoryService State(Action<DirectoryServiceState> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            buildAction(m_State);
            HasState = true;
            return this;
        }

        /// <summary>
        /// Load filter for this service.
        /// </summary>
        public DirectoryServiceLoad Load()
        {
            if (!HasLoad)
                throw new OmmInvalidUsageException("Load element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_Load;
        }
        /// <summary>
        /// Load filter for this service.
        /// </summary>
        public DirectoryService Load(DirectoryServiceLoad value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("Load cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Load.CopyFrom(value);
            HasLoad = true;
            return this;
        }
        /// <summary>
        /// Load filter for this service.
        /// </summary>
        public DirectoryService Load(Action<DirectoryServiceLoad> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            buildAction(m_Load);
            HasLoad = true;
            return this;
        }

        /// <summary>
        /// Data filter for this service.
        /// </summary>
        public DirectoryServiceData Data()
        {
            if (!HasData)
                throw new OmmInvalidUsageException("Data element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_Data;
        }
        /// <summary>
        /// Data filter for this service.
        /// </summary>
        public DirectoryService Data(DirectoryServiceData value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("Data cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Data.CopyFrom(value);
            HasData = true;
            return this;
        }
        /// <summary>
        /// Data filter for this service.
        /// </summary>
        public DirectoryService Data(Action<DirectoryServiceData> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            buildAction(m_Data);
            HasData = true;
            return this;
        }

        /// <summary>
        /// Link filter for this service.
        /// </summary>
        public DirectoryServiceLinkInfo Link()
        {
            if (!HasLink)
                throw new OmmInvalidUsageException("Link element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_Link;
        }
        /// <summary>
        /// Link filter for this service.
        /// </summary>
        public DirectoryService Link(DirectoryServiceLinkInfo value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("Link cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Link.CopyFrom(value);
            HasLink = true;
            return this;
        }
        /// <summary>
        /// Link filter for this service.
        /// </summary>
        public DirectoryService Link(Action<DirectoryServiceLinkInfo> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            buildAction(m_Link);
            HasLink = true;
            return this;
        }

        /// <summary>
        /// Decode an EMA service entry into an RDM service entry.
        /// </summary>
        /// <param name="filterList">object that represents service entry</param>
        internal DirectoryService DecodeFrom(FilterList filterList)
        {
            if (filterList == null)
                throw new OmmInvalidUsageException($"{nameof(filterList)} can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            Clear();

            foreach (var filterEntry in filterList)
            {
                switch (filterEntry.FilterId)
                {
                    case EmaRdm.SERVICE_INFO_ID:
                        DecodeFilterEntry(filterEntry, m_Info, fe => fe.ElementList());
                        HasInfo = true;
                        break;
                    case EmaRdm.SERVICE_STATE_ID:
                        DecodeFilterEntry(filterEntry, m_State, fe => fe.ElementList());
                        HasState = true;
                        break;
                    case EmaRdm.SERVICE_GROUP_ID:
                        var groupFilter = new DirectoryServiceGroup();
                        DecodeFilterEntry(filterEntry, groupFilter, fe => fe.ElementList());
                        m_GroupStateList.Add(groupFilter);
                        break;
                    case EmaRdm.SERVICE_LOAD_ID:
                        DecodeFilterEntry(filterEntry, m_Load, fe => fe.ElementList());
                        HasLoad = true;
                        break;
                    case EmaRdm.SERVICE_DATA_ID:
                        DecodeFilterEntry(filterEntry, m_Data, fe => fe.ElementList());
                        HasData = true;
                        break;
                    case EmaRdm.SERVICE_LINK_ID:
                        DecodeFilterEntry(filterEntry, m_Link, fe => fe.Map());
                        HasLink = true;
                        break;
                    default:
                        break;
                }
            }

            return this;
        }

        /// <summary>
        /// Encode a RDM Service entry.
        /// </summary>
        /// <returns></returns>
        internal DirectoryService EncodeTo(FilterList filterList)
        {
            if (HasInfo)
            {
                EncodeFilterEntry(filterList, m_Info, d => d.Complete(), d => d.Clear());
            }

            if (HasData)
            {
                EncodeFilterEntry(filterList, m_Data, d => d.Complete(), d => d.Clear());
            }

            if (HasLink)
            {
                EncodeFilterEntry(filterList, m_Link, d => d.Complete(), d => d.Clear());
            }

            if (HasLoad)
            {
                EncodeFilterEntry(filterList, m_Load, d => d.Complete(), d => d.Clear());
            }

            if (HasState)
            {
                EncodeFilterEntry(filterList, m_State, d => d.Complete(), d => d.Clear());
            }

            EncodeGroupFilter(filterList);

            return this;
        }

        /// <summary>
        /// Clears an RDMService.
        /// </summary>
        public DirectoryService Clear()
        {
            m_Action = DirectoryMapAction.ADD;
            m_ServiceId = 0;
            m_Info.Clear();
            HasInfo = false;
            m_State.Clear();
            HasState = false;
            m_Load.Clear();
            HasLoad = false;
            m_Data.Clear();
            HasData = false;
            m_Link.Clear();
            HasLink = false;
            m_GroupStateList.Clear();
            return this;
        }

        /// <summary>
        /// Copies the contents from another DirectoryService instance.
        /// </summary>
        /// <param name="source">The source DirectoryService to copy from.</param>
        /// <returns>The current DirectoryService instance to support method chaining.</returns>
        public DirectoryService CopyFrom(DirectoryService source)
        {
            m_Action = source.m_Action;
            m_ServiceId = source.m_ServiceId;
            m_Info.CopyFrom(source.m_Info);
            HasInfo = source.HasInfo;
            m_State.CopyFrom(source.m_State);
            HasState = source.HasState;
            m_Load.CopyFrom(source.m_Load);
            HasLoad = source.HasLoad;
            m_Data.CopyFrom(source.m_Data);
            HasData = source.HasData;
            m_Link.CopyFrom(source.m_Link);
            HasLink = source.HasLink;
            m_GroupStateList.CopyFrom(source.m_GroupStateList);

            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryService"/> instance that is a copy of this instance.</returns>
        public DirectoryService Clone() => new DirectoryService().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        public override string ToString()
        {
            m_ToString.Clear();
            AppendToString(m_ToString, 0);
            return m_ToString.ToString();
        }

        internal void AppendToString(StringBuilder builder, int indent)
        {
            builder
                .AddIndent(indent).AppendLine($"Action: {m_Action}")
                .AddIndent(indent).AppendLine($"ServiceId: {m_ServiceId}");
            if (HasInfo)
            {
                builder.AddIndent(indent).AppendLine("Info:");
                m_Info.AppendToString(builder, indent + 1);
            }
            if (HasState)
            {
                builder.AddIndent(indent).AppendLine("State:");
                m_State.AppendToString(builder, indent + 1);
            }
            if (HasLoad)
            {
                builder.AddIndent(indent).AppendLine("Load:");
                m_Load.AppendToString(builder, indent + 1);
            }
            if (HasData)
            {
                builder.AddIndent(indent).AppendLine("Data:");
                m_Data.AppendToString(builder, indent + 1);
            }
            if (HasLink)
            {
                builder.AddIndent(indent).AppendLine("Link:");
                m_Link.AppendToString(builder, indent + 1);
            }
            builder.AddIndent(indent).AppendLine("GroupStateList:");
            m_GroupStateList.AppendToString(builder, indent,
                (b, group, i) => group.AppendToString(b, i),
                wrapInBraces: true);
        }

        private void DecodeFilterEntry<TData, TSelf>(
                FilterEntry filterEntry,
                DirectoryServiceFilter<TData, TSelf> filter,
                Func<FilterEntry, TData> getData)
            where TData : ComplexType, new()
            where TSelf : DirectoryServiceFilter<TData, TSelf>
        {
            if (filterEntry.Action != (int)FilterEntryActions.CLEAR)
            {
                filter.DecodeFrom(getData(filterEntry));
            }
            filter.Action((DirectoryFilterAction)filterEntry.Action);
        }

        private void EncodeFilterEntry<TData, TSelf>(
                FilterList filterList,
                DirectoryServiceFilter<TData, TSelf> filter,
                Action<TData> completeAction,
                Action<TData> clearAction)
            where TData : ComplexType, new()
            where TSelf : DirectoryServiceFilter<TData, TSelf>
        {
            if (filter.Action() == DirectoryFilterAction.CLEAR)
                filterList.AddEntry(filter.FilterId, (int)filter.Action());
            else
            {
                var data = new TData();
                filter.EncodeTo(data);
                completeAction(data);
                filterList.AddEntry(filter.FilterId, (int)filter.Action(), data);
                clearAction(data);
            }
        }

        private void EncodeGroupFilter(FilterList filterList)
        {
            foreach (var group in m_GroupStateList)
            {
                EncodeFilterEntry(filterList, group, d => d.Complete(), d => d.Clear());
            }
        }
    }

    /// <summary>
    /// Represents a map entry action.
    /// </summary>
    public enum DirectoryMapAction
    {
        /// <summary>
        /// Indicates a partial change of the current Omm data.
        /// </summary>
        ADD = MapAction.ADD,
        /// <summary>
        /// Indicates to append or replace the current Omm data.
        /// </summary>
        UPDATE = MapAction.UPDATE,
        /// <summary>
        /// Indicates to remove current Omm data.
        /// </summary>
        DELETE = MapAction.DELETE,
    }
}