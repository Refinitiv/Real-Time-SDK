/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Rdm
{
	/// <summary>
	/// Login specific RDM definitions </summary>
	sealed public class Login
	{
		// Login class cannot be instantiated
		private Login()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// Indicates the role of the application logging onto the system.
		/// </summary>
		public class RoleTypes
		{
			// RoleTypes class cannot be instantiated
			private RoleTypes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Application logs in as a consumer </summary>
			public const int CONS = 0;

			/// <summary>
			/// Application logs in as a provider </summary>
			public const int PROV = 1;
		}

		/// <summary>
		/// RDM Login Server Types.
		/// </summary>
		public class ServerTypes
		{
			// ServerTypes class cannot be instantiated
			private ServerTypes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Active Server </summary>
			public const int ACTIVE = 0;

			/// <summary>
			/// Standby Server </summary>
			public const int STANDBY = 1;
		}

		/// <summary>
		/// Indicates nametype in RDM login messages.
		/// </summary>
		public enum UserIdTypes : int
		{
			/// <summary>
			/// DACS user name. This can be used to authenticate and permission a user. </summary>
			NAME = 1,

			/// <summary>
			/// Email address </summary>
			EMAIL_ADDRESS = 2,

			/// <summary>
			/// The user token is retrieved from a AAAAPI gateway and then passed through
			/// the system via the msgKey.name. To validate users, a provider application
			/// can pass this user token to an Authentication Manager application.
			/// </summary>
			TOKEN = 3,

			/// <summary>
			/// This indicates user information is specified in a cookie.
			/// If msgKey.name is present it contains cookie file information.
			/// If not present, cookie information may be well known or externally specified.
			/// </summary>
			COOKIE = 4,

            /// <summary>
            /// String defining User Authentication Token is specified as the AUTHN_TOKEN
            /// login attribute.
            /// </summary>
            AUTHN_TOKEN = 5
        }

        /// <summary>
        /// Provider batch support flags. Any combination can be set by provider. </summary>
        public class BatchSupportFlags
		{
			// BatchSupportFlags class cannot be instantiated
			private BatchSupportFlags()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Provider does not support batching. </summary>
			public const int NONE = 0x0;

			/// <summary>
			/// Provider supports batch requests. </summary>
			public const int SUPPORT_REQUESTS = 0x1;

			/// <summary>
			/// Provider supports batch reissue requests. </summary>
			public const int SUPPORT_REISSUES = 0x2;

			/// <summary>
			/// Provider supports batch closes. </summary>
			public const int SUPPORT_CLOSES = 0x4;
		}

		/// <summary>
		/// Enhanced Symbol List behavior support flags. </summary>
		public class EnhancedSymbolListSupportFlags
		{
			// EnhancedSymbolListSupportFlags class cannot be instantiated
			private EnhancedSymbolListSupportFlags()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Supports names only, no additional functionality. </summary>
			public const int NAMES_ONLY = 0x0;

			/// <summary>
			/// Supports symbol list data streams. </summary>
			public const int DATA_STREAMS = 0x1;
		}
	}

}