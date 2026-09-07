/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

namespace LSEG.Ema.Domain.Common
{
    /// <summary>
    /// Builder for fluent list construction.
    /// </summary>
    /// <typeparam name="TItem">List item type.</typeparam>
    public interface IFluentListBuilder<TItem>
    {
        /// <summary>
        /// Adds the specified item to the list being built.
        /// </summary>
        /// <remarks>This method supports fluent syntax by returning the builder instance. Items are added
        /// in the order in which this method is called.</remarks>
        /// <param name="item">The item to add to the list. Cannot be null if the list does not support null values.</param>
        /// <returns>The current instance of the fluent list builder, enabling method chaining.</returns>
        IFluentListBuilder<TItem> Add(TItem item);
        /// <summary>
        /// Adds the elements of the specified sequence to the list builder.
        /// </summary>
        /// <param name="items">The sequence of items to add to the list builder. Cannot be null.</param>
        /// <returns>The current instance of the list builder, enabling method chaining.</returns>
        IFluentListBuilder<TItem> AddRange(IEnumerable<TItem> items);
        /// <summary>
        /// Inserts an item into the list at the specified index.
        /// </summary>
        /// <param name="index">The zero-based index at which the item should be inserted. Must be greater than or equal to 0 and less than
        /// or equal to the current number of items.</param>
        /// <param name="item">The item to insert into the list.</param>
        /// <returns>The current instance of the fluent list builder, enabling method chaining.</returns>
        IFluentListBuilder<TItem> Insert(int index, TItem item);
        /// <summary>
        /// Applies the specified update action to the item at the given index in the list.
        /// </summary>
        /// <remarks>This method allows in-place modification of an item within the list using a fluent
        /// interface. The update action is applied directly to the item at the specified index.</remarks>
        /// <param name="index">The zero-based index of the item to update. Must be greater than or equal to 0 and less than the number of
        /// items in the list.</param>
        /// <param name="updateAction">An action to perform on the item at the specified index. Cannot be null.</param>
        /// <returns>The current instance of the fluent list builder, enabling method chaining.</returns>
        IFluentListBuilder<TItem> Update(int index, Action<TItem> updateAction);
        /// <summary>
        /// Removes the item at the specified index from the list builder.
        /// </summary>
        /// <param name="index">The zero-based index of the item to remove. Must be greater than or equal to 0 and less than the number of
        /// items in the list.</param>
        /// <returns>The current instance of the list builder, enabling method chaining.</returns>
        IFluentListBuilder<TItem> RemoveAt(int index);
        /// <summary>
        /// Removes the specified item from the list being built.
        /// </summary>
        /// <param name="item">The item to remove from the list. If the item does not exist, no action is taken.</param>
        /// <returns>The current instance of the fluent list builder, enabling method chaining.</returns>
        IFluentListBuilder<TItem> Remove(TItem item);
        /// <summary>
        /// Removes all items from the list builder.
        /// </summary>
        /// <remarks>Use this method to reset the builder to an empty state before adding new items. This
        /// method supports method chaining by returning the same builder instance.</remarks>
        /// <returns>The current instance of the fluent list builder, enabling method chaining.</returns>
        IFluentListBuilder<TItem> Clear();
    }
}
