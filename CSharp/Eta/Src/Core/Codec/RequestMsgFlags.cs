/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// Combination of bit values to indicate special behaviors and the presence of
    /// optional <see cref="IRequestMsg"/> content.
    /// </summary>
    /// <seealso cref="IRequestMsg"/>
    public class RequestMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private RequestMsgFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x000) No Flags set </summary>
		public const int NONE = 0x000;

		/// <summary>
		/// (0x001) Request Message has Extended Header </summary>
		public const int HAS_EXTENDED_HEADER = 0x001;

		/// <summary>
		/// (0x002) Has Priority </summary>
		public const int HAS_PRIORITY = 0x002;

		/// <summary>
		/// (0x004) Indicates whether the request is for streaming data.
		/// <ul>
		/// <li>If present, the OMM consumer wants to continue to receive changes to
		/// information after the initial refresh is complete.</li>
		/// <li>If absent, the OMM consumer wants to receive only the refresh, after
		/// which the OMM Provider should close the stream. Such a request is
		/// typically referred to as a non-streaming or snapshot data request.</li>
		/// </ul>
		/// Because a refresh can be split into multiple parts, it is possible for
		/// updates to occur between the first and last part of the refresh, even as
		/// part of a non-streaming request.
		/// </summary>
		public const int STREAMING = 0x004;

		/// <summary>
		/// (0x008) Indicates that the consumer wants to receive the full msgKey in
		/// update messages. This flag does not control whether the msgKey is present
		/// in an update message. Instead, the provider application determines
		/// whether this information is present (the consumer should be capable of
		/// handling the msgKey in any <see cref="IUpdateMsg"/>). When specified on a
		/// request to ADS, the ADS fulfils the request.
		/// </summary>
		public const int MSG_KEY_IN_UPDATES = 0x008;

		/// <summary>
		/// (0x010) Indicates that the consumer wants to receive conflation
		/// information in update messages delivered on this stream. This flag does
		/// not control whether conflation information is present in update messages.
		/// Instead, the provider application determines whether this information is
		/// present,(the consumer should be capable of handling conflation
		/// information in any <see cref="IUpdateMsg"/>).
		/// </summary>
		public const int CONF_INFO_IN_UPDATES = 0x010;

		/// <summary>
		/// (0x020) Indicates that the consumer application does not require a
		/// refresh for this request. This typically occurs after an initial request
		/// handshake is completed, usually to change stream attributes (e.g.,
		/// priority). In some instances, a Provider may still deliver a refresh
		/// message (but if the consumer does not explicitly ask for it, the message
		/// should be unsolicited).
		/// </summary>
		public const int NO_REFRESH = 0x020;

		/// <summary>
		/// (0x040) Indicates that Request has Qos </summary>
		public const int HAS_QOS = 0x040;

		/// <summary>
		/// (0x080) Indicates that Request has Worst Qos </summary>
		public const int HAS_WORST_QOS = 0x080;

		/// <summary>
		/// (0x100) Requests that the stream be opened as private </summary>
		public const int PRIVATE_STREAM = 0x100;

		/// <summary>
		/// (0x200) Indicates that the consumer would like to pause the stream,
		/// though this does not guarantee that the stream will pause. To resume data
		/// flow, the consumer must send a subsequent request message with the
		/// <see cref="STREAMING"/> flag set.
		/// </summary>
		public const int PAUSE = 0x200;

		/// <summary>
		/// (0x400) Indicates that the request message payload may contain a Dynamic
		/// View, specifying information the application wishes to receive (or that
		/// the application wishes to continue receiving a previously specified
		/// View). If this flag is not present, any previously specified view is
		/// discarded and the full View is provided.
		/// </summary>
		public const int HAS_VIEW = 0x400;

		/// <summary>
		/// (0x800) Indicates that the request message payload contains a list of
		/// items of interest, all with matching msgKey information.
		/// </summary>
		public const int HAS_BATCH = 0x800;

		/// <summary>
		/// (0x1000) Requests that the stream be opened as qualified </summary>
		public const int QUALIFIED_STREAM = 0x1000;

	}

}