using LSEG.Ema.Access;
using LSEG.Ema.Domain.Internal;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    internal class DirectoryServiceList : ICloneable
    {
        private readonly StringBuilder m_ToString = new();
        private readonly List<DirectoryService> m_List = new();

        public IList<DirectoryService> Value
        {
            get => m_List;
            set
            {
                m_List.Clear();
                m_List.AddRange(value);
            }
        }

        public void Clear()
        {
            m_List.Clear();
        }

        internal void DecodeFrom(Map map)
        {
            Clear();
            foreach (var mapEntry in map)
            {
                if (mapEntry.LoadType == DataType.DataTypes.FILTER_LIST || // ADD and UPDATE
                    mapEntry.LoadType == DataType.DataTypes.NO_DATA) // DELETE
                {
                    var serviceId = mapEntry.GetServiceId();
                    var service = m_List.FirstOrDefault(_ => _.ServiceId() == serviceId);
                    if (service == null)
                    {
                        service = new DirectoryService();
                        m_List.Add(service);
                    }
                    if (mapEntry.Action != MapAction.DELETE)
                    {
                        service.DecodeFrom(mapEntry.FilterList());
                    }
                    service.ServiceId(serviceId);
                    service.Action((DirectoryMapAction)mapEntry.Action);
                }
            }
        }

        internal void EncodeTo(Map map)
        {
            map.Clear();
            map.KeyType(DataType.DataTypes.UINT);

            var filterList = new FilterList();
            foreach (var service in m_List)
            {
                service.EncodeTo(filterList);
                map.AddKeyUInt(service.ServiceId(), (int)service.Action(), filterList.Complete());
                filterList.Clear();
            }
        }

        public void CopyFrom(DirectoryServiceList source)
        {
            m_List.CopyFrom(source.m_List);
        }

        /// <inheritdoc />
        public object Clone()
        {
            var result = new DirectoryServiceList();
            result.CopyFrom(this);
            return result;
        }

        public override string ToString()
        {
            m_ToString.Clear();
            AppendToString(m_ToString, 0);
            return m_ToString.ToString();
        }

        internal void AppendToString(StringBuilder builder, int indent)
        {
            m_List.AppendToString(builder, indent,
                (sb, item, indent) => item.AppendToString(sb, indent),
                wrapInBraces: true);
        }
    }
}
