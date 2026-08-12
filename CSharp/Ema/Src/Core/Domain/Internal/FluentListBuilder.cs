using LSEG.Ema.Access;
using LSEG.Ema.Domain.Common;
using System;
using System.Collections.Generic;

namespace LSEG.Ema.Domain.Internal
{
    internal class FluentListBuilder<TItem> : IFluentListBuilder<TItem>
    {
        private readonly IList<TItem> m_List;

        public FluentListBuilder(IList<TItem> list)
        {
            m_List = list ?? throw new ArgumentNullException(nameof(list));
        }

        public IFluentListBuilder<TItem> Add(TItem item)
        {
            m_List.Add(item);
            return this;
        }

        public IFluentListBuilder<TItem> AddRange(IEnumerable<TItem> items)
        {
            if (items == null)
                throw new OmmInvalidUsageException($"{nameof(items)} can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            foreach (var item in items)
            {
                m_List.Add(item);
            }
            return this;
        }

        public IFluentListBuilder<TItem> Insert(int index, TItem item)
        {
            m_List.Insert(index, item);
            return this;
        }

        public IFluentListBuilder<TItem> Update(int index, Action<TItem> updateAction)
        {
            if (updateAction == null)
                throw new OmmInvalidUsageException($"{nameof(updateAction)} can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            updateAction(m_List[index]);
            return this;
        }

        public IFluentListBuilder<TItem> RemoveAt(int index)
        {
            m_List.RemoveAt(index);
            return this;
        }

        public IFluentListBuilder<TItem> Remove(TItem item)
        {
            m_List.Remove(item);
            return this;
        }

        public IFluentListBuilder<TItem> Clear()
        {
            m_List.Clear();
            return this;
        }
    }
}
