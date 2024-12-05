/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static LSEG.Eta.Rdm.SymbolList;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal sealed class WlItemRequest : WlRequest
    {
        internal GCHandle m_matchedQosHandle;
        /// <summary>
        /// Defines item request flags
        /// </summary>
        [Flags]
        public enum Flags
        {
            NONE = 0x00, /* None. */
            HAS_STATIC_QOS = 0x01, /* Request has establish a static QoS. */
            PRIVATE = 0x02, /* Request is for a private stream. */
            PROV_DRIVEN = 0x04, /* Request is provider-driven, and should be unique among 
                                      provider-driven requests. */
            REFRESHED = 0x08, /* Request has received a full refresh. */
            BATCH = 0x10, /* Request is a batch request and needs acknowledgement. */
            HAS_PROV_KEY = 0x20, /* Request is provider driven but has received a message with 
                                      a key. */
        }

        /// <summary>
        /// Defines reissue request flags
        /// </summary>
        [Flags]
        public enum ChangeFlags
        {
            /// <summary>
            /// None
            /// </summary>
            NONE = 0x00,

            /// <summary>
            /// Changes the state or attribute of the stream
            /// </summary>
            STATE_CHANGE = 0x01,

            /// <summary>
            /// Changes the view of the stream from item reissue
            /// </summary>
            REISSUE_VIEW_CHANGE = 0x02,

            /// <summary>
            /// Indicates whether to fanout solicited after view response
            /// </summary>
            FANOUT_SOLICITED_AFTER_VIEW = 0x04
        }
        
        internal sealed class ItemRequestLink : Common.VaDoubleLinkList<WlItemRequest>.ILink<WlItemRequest>
        {
            int m_itemNum;

            internal ItemRequestLink(int num)
            {
                m_itemNum = num;
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public WlItemRequest? GetPrev(WlItemRequest thisPrev) { return thisPrev.m_prev_item[m_itemNum]; }
            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public void SetPrev(WlItemRequest? thisPrev, WlItemRequest? thatPrev) { thisPrev!.m_prev_item[m_itemNum] = thatPrev; }
            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public WlItemRequest? GetNext(WlItemRequest thisNext) { return thisNext.m_next_item[m_itemNum]; }
            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public void SetNext(WlItemRequest? thisNext, WlItemRequest? thatNext) { thisNext!.m_next_item[m_itemNum] = thatNext; }
        }

        internal WlItemRequest?[] m_prev_item = new WlItemRequest?[2];
        internal WlItemRequest?[] m_next_item = new WlItemRequest?[2];

        /* Link for new WlItemRequests queue */
        internal static readonly ItemRequestLink WAIT_ITEM_REQUEST_LINK = new(0);
        /* Link for new RequestTimeout queue */
        internal static readonly ItemRequestLink TIMEOUT_ITEM_REQUEST_LINK = new(1);

        public WlItemRequest() : base()
        {
            m_matchedQosHandle = GCHandle.Alloc(MatchedQos, GCHandleType.Normal);
        }

        /// <summary>
        /// Gets or sets <see cref="Flags"/> for this item request.
        /// </summary>
        public Flags ItemReqFlags { get; set; } = Flags.NONE;

        /// <summary>
        /// Gets or sets <see cref="ChangeFlags"/> for this item request.
        /// </summary>
        public ChangeFlags ItemChangeFlags { get; set; } = ChangeFlags.NONE;

        /// <summary>
        /// Gets or sets the matched QoS, if established.
        /// </summary>
        /// <remarks>the <see cref="Flags.HAS_STATIC_QOS"/> flag is used to indicate
        /// whether this property represents the static Qos</remarks>
        public Qos MatchedQos { get; private set; } = new();

        /// <summary>
        /// Gets or sets a view for this request if any.
        /// </summary>
        public WlView? View { get; set; }

        /// <summary>
        /// Indicates whether this item request receives initial response
        /// </summary>
        public bool InitialResponseReceived { get; set; }

        /// <summary>
        /// Gets or set the view element count
        /// </summary>
        public int ViewElemCount { get; internal set; }

        /// <summary>
        /// Gets or sets the view type
        /// </summary>
        public long ViewType { get; internal set; }

        /// <summary>
        /// Gets or sets the view action
        /// </summary>
        public WlItemHandler.ViewAction ViewAction { get; internal set; }

        /// <summary>
        /// Gets or sets a list of field IDs or names
        /// </summary>
        public object? ViewFieldList { get; set; }

        /// <summary>
        /// This is used to indicate whether there is Symbolist behaviour
        /// </summary>
        public bool HasBehaviour { get; internal set; }

        /// <summary>
        /// Gets or sets the <see cref="SymbolListDataStreamRequestFlags"/>
        /// </summary>
        public int SymbolListFlags { get; internal set; }

        /// <summary>
        /// This is used to perform additional handling when there is duplicate item names in the batch request.
        /// </summary>
        public WlBatchRequestHelper? BatchRequestHelper { get; set; }

        /// <summary>
        /// Clears to default values
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void Clear()
        {
            base.Clear();

            ItemReqFlags = Flags.NONE;
            ItemChangeFlags = ChangeFlags.NONE;
            View = null;
            MatchedQos.Clear();
            InitialResponseReceived = false;
            ViewFieldList = null;
            HasBehaviour = false;
            ViewElemCount = 0;
            BatchRequestHelper = null;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void ReturnToPool()
        {
            base.ReturnToPool();
            Clear();
        }

        /* Handles the case that the full request is sent after the view has been changed to reset the flag */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void HandlePendingViewFanout(bool fanoutViewPendingRefresh)
        {
            if(fanoutViewPendingRefresh)
            {
                ItemChangeFlags &= ~ChangeFlags.FANOUT_SOLICITED_AFTER_VIEW;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool SolicitedRefreshNeededForView(bool solicitedRefresh)
        {
            if ((ItemChangeFlags & ChangeFlags.FANOUT_SOLICITED_AFTER_VIEW) != 0)
            {
                return false;
            }
            else
            {
                if (ViewElemCount > 0 && solicitedRefresh)
                {
                    ItemChangeFlags |= ChangeFlags.FANOUT_SOLICITED_AFTER_VIEW;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        internal void FreeWlItemRequest()
        {
            FreeWlRequest();
            m_matchedQosHandle.Free();
        }
    }
}
