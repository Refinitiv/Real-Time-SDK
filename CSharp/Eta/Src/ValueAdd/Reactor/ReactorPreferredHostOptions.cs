/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Configuration options for specifying a preferred host to switch over when the connection is lost
    /// or specified detection time.
    /// </summary>
    /// <seealso cref="ReactorConnectOptions"/>
    public sealed class ReactorPreferredHostOptions
    {
        internal ReactorPreferredHostOptions() { }

        /// <summary>
        /// Gets or sets whether to enable the preferred host options.
        /// </summary>
        public bool EnablePreferredHostOptions { get; set; } = false;

        /// <summary>
        /// Gets or sets cron time schedule to switch over to a preferred host or warmstandby group. Optional.
        /// <para>
        /// <see cref="DetectionTimeInterval"/> is used instead if this property is empty.
        /// </para>
        /// </summary>
        public string DetectionTimeSchedule { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets time interval in second unit to switch over to a preferred host or warmstandby group. Optional.
        /// Defaults to 600 seconds.
        /// </summary>
        public uint DetectionTimeInterval { get; set; } = 0;

        /// <summary>
        /// Gets or sets an index in <see cref="ReactorConnectOptions.ConnectionList"/> to set as preferred host.
        /// </summary>
        public int ConnectionListIndex { get; set; } = 0;

        /// <summary>
        /// Clears this object to default.
        /// </summary>
        public void Clear()
        {
            EnablePreferredHostOptions = false;
            DetectionTimeSchedule = string.Empty;
            DetectionTimeInterval = 0;
            ConnectionListIndex = 0;
        }
    }
}
