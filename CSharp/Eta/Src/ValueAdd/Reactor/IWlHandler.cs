/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Defines base interfaces for all watchlist handlers
    /// </summary>
    internal interface IWlHandler
    {
        /// <summary>
        /// Submits a <see cref="IRequestMsg"/> to a watchlist handler
        /// </summary>
        /// <param name="request">the watchlist request information</param>
        /// <param name="isReissue"><c>true</c> if this is reissue request</param>
        /// <param name="requestMsg">the request message</param>
        /// <param name="submitOptions">the submit options</param>
        /// <param name="errorInfo">set the error info if any</param>
        /// <returns><see cref="ReactorReturnCode"/></returns>
        ReactorReturnCode SubmitRequest(WlRequest request, IRequestMsg requestMsg, bool isReissue, ReactorSubmitOptions submitOptions,
            out ReactorErrorInfo? errorInfo);

        /// <summary>
        /// Submits a <see cref="IMsg"/> to a watchlist handler
        /// </summary>
        /// <param name="request">the watchlist request information</param>
        /// <param name="msg">the message</param>
        /// <param name="submitOptions">the submit options</param>
        /// <param name="errorInfo">set the error info if any</param>
        /// <returns><see cref="ReactorReturnCode"/></returns>
        ReactorReturnCode SubmitMsg(WlRequest request, IMsg msg, ReactorSubmitOptions submitOptions,
            out ReactorErrorInfo? errorInfo);

        /// <summary>
        /// Reads the <see cref="IMsg"/> from the network.
        /// </summary>
        /// <param name="wlStream">the watchlist stream</param>
        /// <param name="decodeIt">the decode iterator</param>
        /// <param name="msg">the message</param>
        /// <param name="errorInfo">set the error info if any</param>
        /// <returns><see cref="ReactorReturnCode"/></returns>
        ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator decodeIt, IMsg msg, out ReactorErrorInfo? errorInfo);

        /// <summary>
        /// Callback to user with <see cref="Msg"/> from a specified location.
        /// </summary>
        /// <param name="location">the location which generates the message</param>
        /// <param name="msg">the Codec message</param>
        /// <param name="wlRequest">the watchlist request information</param>
        /// <param name="errorInfo">set the error info if any</param>
        /// <returns><see cref="ReactorReturnCode"/></returns>
        ReactorReturnCode CallbackUserWithMsg(string location, IMsg msg, WlRequest wlRequest,
            out ReactorErrorInfo? errorInfo);

        /// <summary>
        /// Callback to user with <see cref="IRdmMsg"/> from a specified location.
        /// </summary>
        /// <param name="location">the location which generates the message</param>
        /// <param name="msg">the Codec message</param>
        /// <param name="msgBase">the Rdm message</param>
        /// <param name="wlRequest">the watchlist request information</param>
        /// <param name="errorInfo">set the error info if any</param>
        /// <returns><see cref="ReactorReturnCode"/></returns>
        ReactorReturnCode CallbackUserWithMsgBase(String location, IMsg msg, IRdmMsg msgBase, WlRequest wlRequest,
            out ReactorErrorInfo? errorInfo);

        /// <summary>
        /// Dispatches all streams for the handler.
        /// </summary>
        /// <param name="errorInfo">set the error info if any</param>
        /// <returns><see cref="ReactorReturnCode"/></returns>
        ReactorReturnCode Dispatch(out ReactorErrorInfo? errorInfo);

        /// <summary>
        /// Handles the channel up event by the handler.
        /// </summary>
        /// <param name="errorInfo">set the error info if any</param>
        void ChannelUp(out ReactorErrorInfo? errorInfo);

        /// <summary>
        /// Handles the channel up event by the handler.
        /// </summary>
        void ChannelDown();

        /// <summary>
        /// Clears to default values
        /// </summary>
        void Clear();

        /// <summary>
        /// This is used to add unsent message to the pending list.
        /// </summary>
        /// <param name="wlStream">The watchlist stream</param>
        void AddPendingRequest(WlStream wlStream);

        /// <summary>
        /// Callback invoked upon timed our request.
        /// </summary>
        /// <param name="stream">stream, on which the request timed out</param>
        /// <seealso cref="WlStream.OnRequestTimeout(WlTimeoutTimer)"/>
        void OnRequestTimeout(WlStream stream);
    }
}
