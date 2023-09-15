/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */
using LSEG.Eta.Codec;
using LSEG.Eta.Common;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal sealed class WlItemStream : WlStream
    {
        /// <summary>
        /// Indicates refresh state of the item stream.
        /// </summary>
        public enum RefreshStateFlag
        {
            NONE            = 0,   /* No refresh needed. */
            OPEN_WINDOW     = 1,    /* Need to request a refresh,
									 * but currently in excess of service OpenWindow. */
            REQUEST_REFRESH = 2,    /* Need to request a refresh. */
            PENDING_REFRESH = 3,    /* Currently waiting for a refresh .*/
            PENDING_VIEW_REFRESH = 4, /* Currently waiting for a view refresh */
            PENDING_REFRESH_COMPLETE = 5  /* Recevied partial refresh, need the rest. */
        }

        /// <summary>
        /// Indicates current status of the item stream.
        /// </summary>
        [Flags]
        public enum StatusFlags
        {
			NONE                    = 0x000,
			PENDING_SNAPSHOT        = 0x001,    /* Stream is currently waiting for a snapshot. */
			PAUSED                  = 0x002,    /* Stream is currently paused. */
			PENDING_PRIORITY_CHANGE = 0x004,    /* Stream needs to change priority. */
			VIEWED                  = 0x008,    /* Stream's view is currently active. */
			PENDING_VIEW_CHANGE     = 0x010,    /* Stream needs to update its view. */
			PENDING_VIEW_REFRESH    = 0x020,    /* Stream is expecting a refresh that changes the view. */
			PRIVATE                 = 0x040,    /* Stream is private. */
			ESTABLISHED             = 0x080,    /* Stream is established and can receive generic/post messages. */
			CLOSED          = 0x4000,   /* If closing this stream, do we need to send a close upstream? */
		}

        /// <summary>
        /// Gets or sets <see cref="RefreshState"/>
        /// </summary>
        public RefreshStateFlag RefreshState { get; set; }

        /// <summary>
        /// Gets or sets <see cref="StatusFlags"/>
        /// </summary>
        public StatusFlags Flags { get; set; }

        /// <summary>
        /// Gets item stream attributes
        /// </summary>
        public WlStreamAttributes? StreamAttributes { get; set; }

        /// <summary>
        /// Gets or sets the service associated with this item stream.
        /// </summary>
        public WlService? WlService { get; set; }

        /// <summary>
        /// Gets or set the number of paused request for this stream.
        /// </summary>
        public int NumPausedRequestsCount { get; internal set; }

        /// <summary>
        /// Gets or set <see cref="WlItemGroup"/> for this stream.
        /// </summary>
        public WlItemGroup? ItemGroup { get; set; }

        /// <summary>
        /// Waiting request list to handle cases where request could not be submitted with
        /// directory stream up but there was a pending multi-part refresh or snapshot in progress
        /// </summary>
        public LinkedList<WlItemRequest> WaitingRequestList { get; private set; } = new ();
        
        /// <summary>
        /// Gets or sets the number of requests with view
        /// </summary>
        public int RequestsWithViewCount { get; internal set; }

        /// <summary>
        /// Gets or sets <see cref="WlAggregateView"/> to aggregate requests with view
        /// </summary>
        public WlAggregateView? AggregateView { get; set; }

        private Codec.Buffer m_ViewBuffer = new();
        private ByteBuffer m_ViewByteBuffer = new(2048);
        private bool m_ViewSubsetContained;

        internal override void ChannelDown()
        {
            base.ChannelDown();

            RefreshState = RefreshStateFlag.NONE;
        }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public override void Clear()
        {
            base.Clear();
            RefreshState = RefreshStateFlag.NONE;
            Flags = StatusFlags.NONE;
            StreamAttributes = null;
            RequestMsg.MsgClass = MsgClasses.REQUEST;
            WlService = null;
            WaitingRequestList.Clear();
            NumPausedRequestsCount = 0;
            ItemGroup = null;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="msg"></param>
        /// <param name="submitOptions"></param>
        /// <param name="fromCloseMsg"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        public ReactorReturnCode SendMsg(IMsg msg, ReactorSubmitOptions submitOptions, bool fromCloseMsg, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;

            if (msg.MsgClass == MsgClasses.REQUEST)
            {
                if(RequestsWithViewCount > 0 && RequestsWithViewCount == UserRequestList.Count)
                {
                    if( ((Flags & StatusFlags.PENDING_VIEW_CHANGE) != 0) && RefreshState != RefreshStateFlag.PENDING_VIEW_REFRESH)
                    {
                        bool viewUpdated;
                        AggregateView!.MergeViews(out viewUpdated);
                        msg.Flags |= RequestMsgFlags.HAS_VIEW;
                        m_ViewSubsetContained = !viewUpdated;

                        if (!fromCloseMsg || viewUpdated)
                        {
                            m_ViewBuffer.Clear();
                            m_ViewByteBuffer.Clear();
                            m_ViewBuffer.Data(m_ViewByteBuffer);
                            m_EncodeIterator.Clear();
                            m_EncodeIterator.SetBufferAndRWFVersion(m_ViewBuffer, Watchlist!.ReactorChannel!.MajorVersion,
                                Watchlist.ReactorChannel.MinorVersion);
                            AggregateView.ViewHandler!.EncodeViewRequest(m_EncodeIterator, AggregateView);
                            msg.ContainerType = DataTypes.ELEMENT_LIST;
                            msg.EncodedDataBody = m_ViewBuffer;
                        }
                        else
                        {
                            msg.ContainerType = DataTypes.NO_DATA;
                            msg.EncodedDataBody.Clear();
                            RequestMsg.Flags |= RequestMsgFlags.NO_REFRESH;
                        }
                    }
                    else
                    {
                        // Waits until View refresh is applied.
                        WlHandler!.AddPendingRequest(null!);

                        return ReactorReturnCode.SUCCESS;
                    }
                }
                else
                {
                    RemoveViewFromMsg(msg);
                }
            }

            // encode into buffer and send out
            if(Watchlist!.IsChannelUp())
            {
                if(msg.MsgClass != MsgClasses.CLOSE)
                {
                    if(msg.MsgClass == MsgClasses.REQUEST && UserRequestList.Count > 0)
                    {
                        if (NumPausedRequestsCount == UserRequestList.Count
                            && Watchlist!.LoginHandler.SupportOptimizedPauseResume)
                        {
                            ((IRequestMsg)msg).ApplyPause();
                            Flags |= StatusFlags.PAUSED;
                        }
                        else
                        {
                            if ((Flags & StatusFlags.PAUSED) != 0)
                            {
                                ((IRequestMsg)msg).Flags &= ~RequestMsgFlags.PAUSE;
                                Flags &= ~StatusFlags.PAUSED;
                            }
                        }
                    }

                    ret = EncodeIntoBufferAndWrite((Msg)msg, submitOptions, out errorInfo);
                    
                    if(ret >= ReactorReturnCode.SUCCESS)
                    {
                        // Start request timer if refresh expected and refresh it not already pending
                        if(msg.MsgClass == MsgClasses.REQUEST)
                        {
                            // copy submitted request message to cached request message if not done already
                            if (msg != RequestMsg)
                            {
                                if ((ret = (ReactorReturnCode)CopyRequestMsg((IRequestMsg)msg)) < ReactorReturnCode.SUCCESS)
                                {
                                    return ret;
                                }
                            }

                            // start timer if request is not already pending
                            if(!RequestPending && ((IRequestMsg)msg).CheckNoRefresh() == false)
                            {
                                if (StartRequestTimer() != ReactorReturnCode.SUCCESS)
                                {
                                    return ReactorReturnCode.FAILURE;
                                }
                            }

                            if (((Flags & StatusFlags.PENDING_VIEW_CHANGE) != 0))
                            {
                                Flags &= ~StatusFlags.PENDING_VIEW_CHANGE;
                                if (!((IRequestMsg)msg).CheckNoRefresh() && !m_ViewSubsetContained)
                                    RefreshState = RefreshStateFlag.PENDING_VIEW_REFRESH;

                                if (AggregateView != null && (msg.Flags & RequestMsgFlags.HAS_VIEW) > 0)
                                    AggregateView!.CommitViews();

                                if (AggregateView != null && RequestsWithViewCount == 0)
                                {
                                    AggregateView = null;
                                }
                            }
                        }
                    }
                    else if (ret == ReactorReturnCode.NO_BUFFERS && msg.MsgClass != MsgClasses.POST)
                    {
                        if(msg.MsgClass == MsgClasses.REQUEST)
                        {

                            if(msg != RequestMsg)
                            {
                                if ((ret = (ReactorReturnCode)CopyRequestMsg((IRequestMsg)msg)) < ReactorReturnCode.SUCCESS)
                                {
                                    return ret;
                                }
                            }

                            WlHandler!.AddPendingRequest(this);
                        }

                        return ReactorReturnCode.NO_BUFFERS;
                    }
                }
                else
                {
                    ret = SendCloseMsg((Msg)msg, out errorInfo);
                }
            }
            else // channel is not up, should not send out.
            {
                return ReactorReturnCode.SUCCESS;
            }

            return ret;
        }

        void RemoveViewFromMsg(IMsg msg)
        {
            /* Clears the view flag and the payload only when there is a view for this item stream.
             * But the reissue request doesn't have a view flag to clear the view */
            if ((msg.Flags & RequestMsgFlags.HAS_VIEW) != 0)
            {
                msg.Flags &= ~RequestMsgFlags.HAS_VIEW;
                m_ViewBuffer.Clear();
                msg.EncodedDataBody = m_ViewBuffer;
                msg.ContainerType = DataTypes.NO_DATA;
                if (AggregateView != null)
                {
                    AggregateView.UnCommit();
                }

                Flags |= StatusFlags.PENDING_VIEW_CHANGE;
                m_ViewSubsetContained = false;
            }
        }

        /// <summary>
        /// Closes the item stream.
        /// </summary>
        public override void Close()
        {
            base.Close();

            NumPausedRequestsCount = 0;
            m_ViewSubsetContained = false;
        }
    }
}
