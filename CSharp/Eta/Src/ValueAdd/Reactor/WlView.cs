/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Watchlist storage for a single view.
    /// </summary>
    internal sealed class WlView
    {
        public WlView()
        {
            ViewNode = new LinkedListNode<WlView>(this);
        }

        /// <summary>
        /// Gets or sets a view node.
        /// </summary>
        public LinkedListNode<WlView> ViewNode { get; set; }

        /// <summary>
        /// Gets or sets a <see cref="ViewHandler"/>
        /// </summary>
        public WlViewHandler? ViewHandler { get; set; }

        /// <summary>
        /// Gets or sets the number of the element for this view
        /// </summary>
        public int ElemCount { get; set; }

        /// <summary>
        /// Gets or set the type of this view
        /// </summary>
        public int ViewType { get; set; }

        /// <summary>
        /// Gets a list of Fidld IDs or names depending upon the view type.
        /// </summary>
        public object? FieldList { get; set; }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public void Clear()
        {
            ElemCount = 0;
            ViewType = 0;
            ViewHandler = null;
        }
    }
}
