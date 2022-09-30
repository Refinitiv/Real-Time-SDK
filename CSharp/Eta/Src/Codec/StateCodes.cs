/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace Refinitiv.Eta.Codec
{
	/// <summary>
	/// State Codes provide additional details about the current state.
	/// </summary>
	/// <seealso cref="State"/>
	public class StateCodes
	{
		// StateCodes class cannot be instantiated
		private StateCodes()
		{
		}

		/// <summary>
		/// code is none. Additional state code information is not required nor present.
		/// </summary>
		public const int NONE = 0;

		/// <summary>
		/// Indicates that requested information was not found, though it might be
		/// available at a later time or through changing some parameters used in the request.
		/// </summary>
		public const int NOT_FOUND = 1;

		/// <summary>
		/// Indicates that a timeout occurred somewhere in the system while processing requested data.
		/// </summary>
		public const int TIMEOUT = 2;

		/// <summary>
		/// Indicates that the request was denied due to permissioning. Typically
		/// indicates that the requesting user does not have permission to request on
		/// the service, to receive requested data, or to receive data at the requested QoS.
		/// </summary>
		public const int NOT_ENTITLED = 3;

		/// <summary>
		/// Indicates that the request includes an invalid or unrecognized parameter.
		/// Specific information should be contained in the text.
		/// </summary>
		public const int INVALID_ARGUMENT = 4;

		/// <summary>
		/// Indicates invalid usage within the system. Specific information should be
		/// contained in the text.
		/// </summary>
		public const int USAGE_ERROR = 5;

		/// <summary>
		/// Indicates the stream was preempted, possibly by a caching device.
		/// Typically indicates the user has exceeded an item limit, whether specific
		/// to the user or a component in the system. Relevant information should be
		/// contained in the text.
		/// </summary>
		public const int PREEMPTED = 6;

		/// <summary>
		/// Indicates that JIT conflation has started on the stream. User is notified
		/// when JIT Conflation ends via a <see cref="REALTIME_RESUMED"/> code.
		/// </summary>
		public const int JIT_CONFLATION_STARTED = 7;

		/// <summary>
		/// Indicates that JIT Conflation on the stream has finished.
		/// </summary>
		public const int REALTIME_RESUMED = 8;

		/// <summary>
		/// Indicates that a component is recovering due to a failover condition.
		/// User is notified when recovery finishes via a <see cref="FAILOVER_COMPLETED"/> code.
		/// </summary>
		public const int FAILOVER_STARTED = 9;

		/// <summary>
		/// Indicates that recovery from a failover condition has finished.
		/// </summary>
		public const int FAILOVER_COMPLETED = 10;

		/// <summary>
		/// Indicates a gap was detected between messages. A gap might be detected
		/// via an external reliability mechanism (e.g., transport) or using the
		/// seqNum present in ETA messages.
		/// </summary>
		public const int GAP_DETECTED = 11;

		/// <summary>
		/// Indicates that no resources are available to accommodate the stream.
		/// </summary>
		public const int NO_RESOURCES = 12;

		/// <summary>
		/// Indicates that a request cannot be processed because too many other streams are already open.
		/// </summary>
		public const int TOO_MANY_ITEMS = 13;

		/// <summary>
		/// Indicates that a stream is already open on the connection for the requested data.
		/// </summary>
		public const int ALREADY_OPEN = 14;

		/// <summary>
		/// Indicates that the requested service is not known, though the service
		/// might be available at a later point in time.
		/// </summary>
		public const int SOURCE_UNKNOWN = 15;

		/// <summary>
		/// Indicates that the stream was not opened.
		/// Additional information should be available in the text.
		/// </summary>
		public const int NOT_OPEN = 16;

		/* Reserved */
		internal const int RESERVED17 = 17;

		/* Reserved */
		internal const int RESERVED18 = 18;

		/// <summary>
		/// Indicates that a streaming request was made for non-updating data. </summary>
		public const int NON_UPDATING_ITEM = 19;

		/// <summary>
		/// Indicates that the domain on which a request is made does not support the requested viewType.
		/// </summary>
		public const int UNSUPPORTED_VIEW_TYPE = 20;

		/// <summary>
		/// code indicates that the requested view is invalid, possibly due to bad formatting.
		/// Additional information should be available in the text.
		/// </summary>
		public const int INVALID_VIEW = 21;

		/// <summary>
		/// Indicates that the full view (e.g., all available fields) is being
		/// provided, even though only a specific view was requested.
		/// </summary>
		public const int FULL_VIEW_PROVIDED = 22;

		/// <summary>
		/// Indicates that a batch request cannot be used for this request.
		/// The user can instead split the batched items into individual requests.
		/// </summary>
		public const int UNABLE_TO_REQUEST_AS_BATCH = 23;

		/* Reserved */
		internal const int RESERVED24 = 24;

		/* Reserved */
		internal const int RESERVED25 = 25;

		/// <summary>
		/// Request does not support batch and view
		/// </summary>
		public const int NO_BATCH_VIEW_SUPPORT_IN_REQ = 26;

		/// <summary>
		/// Login rejected, exceeded maximum number of mounts per user
		/// </summary>
		public const int EXCEEDED_MAX_MOUNTS_PER_USER = 27;

		/// <summary>
		/// Internal error from sender.
		/// </summary>
		public const int ERROR = 28;

		/// <summary>
		/// A21: Connection to DACS down, users are not allowed to connect
		/// </summary>
		public const int DACS_DOWN = 29;

		/// <summary>
		/// User unknown to permissioning system, it could be DACS, AAA or EED
		/// </summary>
		public const int USER_UNKNOWN_TO_PERM_SYS = 30;

		/// <summary>
		/// Maximum logins reached.
		/// </summary>
		public const int DACS_MAX_LOGINS_REACHED = 31;

		/// <summary>
		/// Application is denied access to the system
		/// </summary>
		public const int DACS_USER_ACCESS_TO_APP_DENIED = 32;

		/// <summary>
		/// Content is intended to fill a recognized gap
		/// </summary>
		public const int GAP_FILL = 33;

		/// <summary>
		/// Application Authorization Failed
		/// </summary>
		public const int APP_AUTHORIZATION_FAILED = 35;

		/* Max reserved value */
		internal const int MAX_RESERVED = 127;

		/// <summary>
		/// Provide string representation for a state code.
		/// </summary>
		/// <param name="stateCode"> <seealso cref="StateCodes"/> enumeration to convert to string
		/// </param>
		/// <returns> string representation for a state code </returns>
		public static string ToString(int stateCode)
		{
			switch (stateCode)
			{
				case NONE:
					return "NONE";
				case TIMEOUT:
					return "TIMEOUT";
				case NOT_ENTITLED:
					return "NOT_ENTITLED";
				case INVALID_ARGUMENT:
					return "INVALID_ARGUMENT";
				case USAGE_ERROR:
					return "USAGE_ERROR";
				case PREEMPTED:
					return "PREEMPTED";
				case JIT_CONFLATION_STARTED:
					return "JIT_CONFLATION_STARTED";
				case REALTIME_RESUMED:
					return "REALTIME_RESUMED";
				case FAILOVER_STARTED:
					return "FAILOVER_STARTED";
				case FAILOVER_COMPLETED:
					return "FAILOVER_COMPLETED";
				case GAP_DETECTED:
					return "GAP_DETECTED";
				case NO_RESOURCES:
					return "NO_RESOURCES";
				case TOO_MANY_ITEMS:
					return "TOO_MANY_ITEMS";
				case ALREADY_OPEN:
					return "ALREADY_OPEN";
				case SOURCE_UNKNOWN:
					return "SOURCE_UNKNOWN";
				case NOT_OPEN:
					return "NOT_OPEN";
				case NON_UPDATING_ITEM:
					return "NON_UPDATING_ITEM";
				case UNSUPPORTED_VIEW_TYPE:
					return "UNSUPPORTED_VIEW_TYPE";
				case INVALID_VIEW:
					return "INVALID_VIEW";
				case FULL_VIEW_PROVIDED:
					return "FULL_VIEW_PROVIDED";
				case UNABLE_TO_REQUEST_AS_BATCH:
					return "UNABLE_TO_REQUEST_AS_BATCH";
				case NOT_FOUND:
					return "NOT_FOUND";
				case NO_BATCH_VIEW_SUPPORT_IN_REQ:
					return "NO_BATCH_VIEW_SUPPORT_IN_REQ";
				case EXCEEDED_MAX_MOUNTS_PER_USER:
					return "EXCEEDED_MAX_MOUNTS_PER_USER";
				case ERROR:
					return "ERROR";
				case DACS_DOWN:
					return "DACS_DOWN";
				case USER_UNKNOWN_TO_PERM_SYS:
					return "USER_UNKNOWN_TO_PERM_SYS";
				case DACS_MAX_LOGINS_REACHED:
					return "DACS_MAX_LOGINS_REACHED";
				case DACS_USER_ACCESS_TO_APP_DENIED:
					return "DACS_USER_ACCESS_TO_APP_DENIED";
				case GAP_FILL:
					return "GAP_FILL";
				case APP_AUTHORIZATION_FAILED:
					return "APP_AUTHORIZATION_FAILED";
				default:
					return Convert.ToString(stateCode);
			}
		}

		/// <summary>
		/// Provide string representation of meaning associated with state code.
		/// </summary>
		/// <param name="stateCode"> <seealso cref="StateCodes"/> enumeration to get info for
		/// </param>
		/// <returns> string representation of meaning associated with state code </returns>
		public static string Info(int stateCode)
		{
			switch (stateCode)
			{
				case StateCodes.NONE:
					return "None";
				case StateCodes.NOT_FOUND:
					return "Not found";
				case StateCodes.TIMEOUT:
					return "Timeout";
				case StateCodes.NOT_ENTITLED:
					return "Not entitled";
				case StateCodes.INVALID_ARGUMENT:
					return "Invalid argument";
				case StateCodes.USAGE_ERROR:
					return "Usage error";
				case StateCodes.PREEMPTED:
					return "Preempted";
				case StateCodes.JIT_CONFLATION_STARTED:
					return "JIT conflation started";
				case StateCodes.REALTIME_RESUMED:
					return "Realtime resumed";
				case StateCodes.FAILOVER_STARTED:
					return "Failover started";
				case StateCodes.FAILOVER_COMPLETED:
					return "Failover completed";
				case StateCodes.GAP_DETECTED:
					return "Gap detected";
				case StateCodes.NO_RESOURCES:
					return "No resources";
				case StateCodes.TOO_MANY_ITEMS:
					return "Too many items";
				case StateCodes.ALREADY_OPEN:
					return "Already open";
				case StateCodes.SOURCE_UNKNOWN:
					return "Source unknown";
				case StateCodes.NOT_OPEN:
					return "Not open";
				case StateCodes.NON_UPDATING_ITEM:
					return "Non-updating item";
				case StateCodes.UNSUPPORTED_VIEW_TYPE:
					return "Unsupported view type";
				case StateCodes.INVALID_VIEW:
					return "Invalid view";
				case StateCodes.FULL_VIEW_PROVIDED:
					return "Full view provided";
				case StateCodes.UNABLE_TO_REQUEST_AS_BATCH:
					return "Unable to request as batch";
				case NO_BATCH_VIEW_SUPPORT_IN_REQ:
					return "Request does not support batch and view";
				case EXCEEDED_MAX_MOUNTS_PER_USER:
					return "Login rejected, exceeded maximum number of mounts per user";
				case ERROR:
					return "Internal error from sender";
				case DACS_DOWN:
					return "A21: Connection to DACS down, users are not allowed to connect";
				case USER_UNKNOWN_TO_PERM_SYS:
					return "User unknown to permissioning system, it could be DACS, AAA or EED";
				case DACS_MAX_LOGINS_REACHED:
					return "Maximum logins reached";
				case DACS_USER_ACCESS_TO_APP_DENIED:
					return "Application is denied access to the system";
				case GAP_FILL:
					return "Content is intended";
				case APP_AUTHORIZATION_FAILED:
					return "Application Authorization failed";
				default:
					return "";
			}
		}
	}



}