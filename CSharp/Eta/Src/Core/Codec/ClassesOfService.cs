/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.Rdm
{
	/// <summary>
	/// Qualified stream class of service specific RDM definitions. </summary>
	sealed public class ClassesOfService
	{
		// ClassesOfService class cannot be instantiated
		private ClassesOfService()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// Filter IDs for class of service.
		/// </summary>
		public class FilterIds
		{
			// FilterIds class cannot be instantiated
			private FilterIds()
			{
                throw new System.NotImplementedException();
			}

			/// <summary>
			/// Common Properties Filter ID </summary>
			public const int COMMON_PROPERTIES = 1;
			/// <summary>
			/// Authentication Filter ID </summary>
			public const int AUTHENTICATION = 2;
			/// <summary>
			/// Flow Control Filter ID </summary>
			public const int FLOW_CONTROL = 3;
			/// <summary>
			/// Data Integrity Filter ID </summary>
			public const int DATA_INTEGRITY = 4;
			/// <summary>
			/// Guarantee Filter ID </summary>
			public const int GUARANTEE = 5;
		}

		/// <summary>
		/// Filter flags for class of service.
		/// </summary>
		public class FilterFlags
		{
			// FilterFlags class cannot be instantiated
			internal FilterFlags()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Common Properties Filter Mask </summary>
			public const int COMMON_PROPERTIES = 0x00000001;
			/// <summary>
			/// Authentication Filter Mask </summary>
			public const int AUTHENTICATION = 0x00000002;
			/// <summary>
			/// Flow Control Filter Mask </summary>
			public const int FLOW_CONTROL = 0x00000004;
			/// <summary>
			/// Data Integrity Filter Mask </summary>
			public const int DATA_INTEGRITY = 0x00000008;
			/// <summary>
			/// Guarantee Filter Mask </summary>
			public const int GUARANTEE = 0x00000010;
		}

		/// <summary>
		/// Element names for class of service.
		/// </summary>
		public class ElementNames
		{
			// ElementNames class cannot be instantiated
			internal ElementNames()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// :StreamVersion </summary>
			public static readonly Buffer STREAM_VERSION = new Buffer();

			/// <summary>
			/// :MaxMsgSize </summary>
			public static readonly Buffer MAX_MSG_SIZE = new Buffer();

			/// <summary>
			/// :ProtocolType </summary>
			public static readonly Buffer PROTOCOL_TYPE = new Buffer();

			/// <summary>
			/// :ProtocolMajorVersion </summary>
			public static readonly Buffer PROTOCOL_MAJOR_VERSION = new Buffer();

			/// <summary>
			/// :ProtocolMinorVersion </summary>
			public static readonly Buffer PROTOCOL_MINOR_VERSION = new Buffer();

			/// <summary>
			/// :RecvWindowSize </summary>
			public static readonly Buffer RECV_WINDOW_SIZE = new Buffer();

			/// <summary>
			/// :Type </summary>
			public static readonly Buffer TYPE = new Buffer();

            /// <summary>
            /// 
            /// </summary>
			static ElementNames()
            {
				// :StreamVersion
				STREAM_VERSION.Data(":StreamVersion");

				// :MaxMsgSize
				MAX_MSG_SIZE.Data(":MaxMsgSize");

                // :ProtocolType
                PROTOCOL_TYPE.Data(":ProtocolType");

				// :ProtocolMajorVersion
				PROTOCOL_MAJOR_VERSION.Data(":ProtocolMajorVersion");

				// :ProtocolMinorVersion </summary>
				PROTOCOL_MINOR_VERSION.Data(":ProtocolMinorVersion");

				// :RecvWindowSize
				RECV_WINDOW_SIZE.Data(":RecvWindowSize");

				// :Type </summary>
				TYPE.Data(":Type");
			}
		}

		/// <summary>
		/// Authentication type enumerations for class of service.
		/// </summary>
		public class AuthenticationTypes
		{
			// AuthenticationTypes class cannot be instantiated
			private AuthenticationTypes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
            /// Not required
            /// </summary>
			public const int NOT_REQUIRED = 0;

			/// <summary>
            /// OMM Login
            /// </summary>
			public const int OMM_LOGIN = 1;
		}

		/// <summary>
		/// Flow control type enumerations for class of service.
		/// </summary>
		public class FlowControlTypes
		{
			// FlowControlTypes class cannot be instantiated
			private FlowControlTypes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// None </summary>
			public const int NONE = 0;

			/// <summary>
			/// Bidirectional </summary>
			public const int BIDIRECTIONAL = 1;
		}

		/// <summary>
		/// Data integrity type enumerations for class of service.
		/// </summary>
		public class DataIntegrityTypes
		{
			// DataIntegrityTypes class cannot be instantiated
			private DataIntegrityTypes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Best Effort </summary>
			public const int BEST_EFFORT = 0;

			/// <summary>
			/// Reliable </summary>
			public const int RELIABLE = 1;
		}

		/// <summary>
		/// Guarantee type enumerations for class of service.
		/// </summary>
		public class GuaranteeTypes
		{
			// Guarantee classTypes cannot be instantiated
			private GuaranteeTypes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// None </summary>
			public const int NONE = 0;

			/// <summary>
			/// PersistentQueue </summary>
			public const int PERSISTENT_QUEUE = 1;
		}
	}

}