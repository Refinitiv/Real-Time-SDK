/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Rdm
{
    /// <summary>
    /// Directory specific RDM definitions </summary>
    sealed public class Directory
	{
		// Directory class cannot be instantiated
		private Directory()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// Explains the content of the Data.
		/// </summary>
		public class DataTypes
		{
			// DataTypes class cannot be instantiated
			private DataTypes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// None </summary>
			public const int NONE = 0;
			/// <summary>
			/// Time </summary>
			public const int TIME = 1;
			/// <summary>
			/// Alert </summary>
			public const int ALERT = 2;
			/// <summary>
			/// Headline </summary>
			public const int HEADLINE = 3;
			/// <summary>
			/// Status </summary>
			public const int STATUS = 4;
		}

		/// <summary>
		/// Provides additional status information about the upstream source.
		/// </summary>
		public class LinkCodes
		{
			// LinkCodes class cannot be instantiated
			private LinkCodes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// None </summary>
			public const int NONE = 0;
			/// <summary>
			/// Ok </summary>
			public const int OK = 1;
			/// <summary>
			/// Recovery Started </summary>
			public const int RECOVERY_STARTED = 2;
			/// <summary>
			/// Recovery Completed </summary>
			public const int RECOVERY_COMPLETED = 3;
		}

		/// <summary>
		/// Indicates whether the upstream source that provides data to a service is up
		/// or down.
		/// </summary>
		public class LinkStates
		{
			// LinkStates class cannot be instantiated
			private LinkStates()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Down </summary>
			public const int DOWN = 0;
			/// <summary>
			/// Up </summary>
			public const int UP = 1;
		}

		/// <summary>
		/// Indicates whether the upstream source is interactive or broadcast. This does
		/// not describe whether the service itself is interactive or broadcast.
		/// </summary>
		public class LinkTypes
		{
			// LinkTypes class cannot be instantiated
			private LinkTypes()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Interactive </summary>
			public const int INTERACTIVE = 1;
			/// <summary>
			/// Broadcast </summary>
			public const int BROADCAST = 2;
		}

		/// <summary>
		/// Combination of bit values to indicate specific information about a RDM directory service.
		/// </summary>
		public class ServiceFilterFlags
		{
			// ServiceFilterFlags class cannot be instantiated
			private ServiceFilterFlags()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Source Info Filter Mask </summary>
			public const int INFO = 0x00000001;
			/// <summary>
			/// Source State Filter Mask </summary>
			public const int STATE = 0x00000002;
			/// <summary>
			/// Source Load Filter Mask </summary>
			public const int GROUP = 0x00000004;
			/// <summary>
			/// Source Load Filter Mask </summary>
			public const int LOAD = 0x00000008;
			/// <summary>
			/// Source Data Filter Mask </summary>
			public const int DATA = 0x00000010;
			/// <summary>
			/// Source Communication Link Information </summary>
			public const int LINK = 0x00000020;
            /// <summary>
            /// All of the above information </summary>
            public const int ALL_FILTERS = 0x0000003F;
}

		/// <summary>
		/// Filter IDs for RDM directory service.
		/// </summary>
		public class ServiceFilterIds
		{
			// ServiceFilterIds class cannot be instantiated
			private ServiceFilterIds()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Service Info Filter ID </summary>
			public const int INFO = 1;
			/// <summary>
			/// Source State Filter ID </summary>
			public const int STATE = 2;
			/// <summary>
			/// Source Load Filter ID </summary>
			public const int GROUP = 3;
			/// <summary>
			/// Source Load Filter ID </summary>
			public const int LOAD = 4;
			/// <summary>
			/// Source Data Filter ID </summary>
			public const int DATA = 5;
			/// <summary>
			/// Communication Link Filter ID </summary>
			public const int LINK = 6;
		}

		/// <summary>
		/// Indicates whether the original provider of the data is available to respond
		/// to new requests. If the service is down, requests for data may be handled by
		/// the immediate upstream provider (to which the consumer is directly
		/// connected). However, because the most current data might be serviced from a
		/// cached copy while the source is down, the most current data may not be
		/// immediately available.Indicates whether the original provider of the data is
		/// available to respond to new requests. If the service is down, requests for
		/// data may be handled by the immediate upstream provider (to which the consumer
		/// is directly connected). However, because the most current data might be
		/// serviced from a cached copy while the source is down, the most current data
		/// may not be immediately available.
		/// </summary>
		public class ServiceStates
		{
			// ServiceStates class cannot be instantiated
			private ServiceStates()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Service state down </summary>
			public const int DOWN = 0;
			/// <summary>
			/// Service state up </summary>
			public const int UP = 1;
		}

		/// <summary>
		/// Indicates how the downstream component is using the service.
		/// </summary>
		public class SourceMirroringMode
		{
			// SourceMirroringMode class cannot be instantiated
			private SourceMirroringMode()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// The downstream device is using the data from this service, and is not
			/// receiving it from any other service.
			/// </summary>
			public const int ACTIVE_NO_STANDBY = 0;

			/// <summary>
			/// The downstream device is using the data from this service, but is also
			/// getting it from another service.
			/// </summary>
			public const int ACTIVE_WITH_STANDBY = 1;

			/// <summary>
			/// The downstream device is getting data from this service, but is actually
			/// using data from another service.
			/// </summary>
			public const int STANDBY = 2;
		}
	}

}