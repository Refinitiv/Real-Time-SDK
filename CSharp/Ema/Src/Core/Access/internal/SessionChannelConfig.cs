/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|              Copyright (C) 2025 LSEG. All rights reserved.                --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

namespace LSEG.Ema.Access;

/// <summary>
/// This class maps to the "SessionChannelInfo" entry in the consumer configuration.
/// </summary>
///
/// "SessionChannelGroup" contains a "SessionChannelList" that contains
/// "SessionChannelInfo" objects.
///
/// For Programmatic Configuration, the top level Map contains a Map Entry that contains
/// an Element List with the key of "SessionChannelGroup".  The SessionChannelGroup
/// Element List has a single Element with a Map data type and a Name of
/// "SessionChannelList".
///
/// The SessionChannelList be an ElementList data type Map containing Map Entries that are
/// keyed by ASCII string config names.
///
internal class SessionChannelConfig
{
    #region Configuration properties

    /// Specifies the unique name of this SessionChannelList component.
    public string Name { get; set; } = string.Empty;

    /// Specifies the maximum number of times that this Session Channel will attempt to
    /// reconnect to a channel when it fails.
    ///
    /// If set to -1, this channel will continually attempt to reconnect.
    public long ReconnectAttemptLimit { get; set; }

    /// Sets the maximum amount of time this Session Channel will wait (in milliseconds)
    /// before attempting to reconnect a failed channel. Refer also to the
    /// ReconnectMinDelay parameter.
    public long ReconnectMaxDelay { get; set; }

    /// Specifies the minimum amount of time this Session Channel will wait wait (in
    /// milliseconds) before attempting to reconnect a failed channel. This wait time
    /// increases with each connection attempt, from ReconnectMinDelay to
    /// ReconnectMaxDelay.
    public long ReconnectMinDelay { get; set; }

    /// Specifies a comma separated set of channel names.  Channels in the set will be
    /// tried with each reconnection attempt until a successful connection is made.
    public List<string> ChannelSet { get; set; }

    // These flags are used to check whether these property are set by file or programmatic
    public bool ReconnectAttemptLimitSet { get; internal set; }
    public bool ReconnectMaxDelaySet { get; internal set; }
    public bool ReconnectMinDelaySet { get; internal set; }
    #endregion

    public SessionChannelConfig()
    {
        ChannelSet = new List<string>();

        Clear();
    }

    // Deep copy constuctor.
    public SessionChannelConfig(SessionChannelConfig oldConfig)
    {
        Name = oldConfig.Name;
        ReconnectAttemptLimit = oldConfig.ReconnectAttemptLimit;
        ReconnectMaxDelay = oldConfig.ReconnectMaxDelay;
        ReconnectMinDelay = oldConfig.ReconnectMinDelay;
        ReconnectAttemptLimitSet = oldConfig.ReconnectAttemptLimitSet;
        ReconnectMaxDelaySet = oldConfig.ReconnectMaxDelaySet;
        ReconnectMinDelaySet = oldConfig.ReconnectMinDelaySet;

        ChannelSet = new List<string>(oldConfig.ChannelSet.Count);
        ChannelSet.AddRange(oldConfig.ChannelSet);
    }

    // Clears the Consumer structure and sets the default options.
    public void Clear()
    {
        Name = string.Empty;
        ReconnectAttemptLimit = -1;
        ReconnectMaxDelay = 5000;
        ReconnectMinDelay = 5000;

        ChannelSet.Clear();

        ReconnectAttemptLimitSet = false;
        ReconnectMaxDelaySet = false;
        ReconnectMinDelaySet = false;
    }

    // Copy method, produces a deep copy into DestConfig.
    public void Copy(SessionChannelConfig DestConfig)
    {
        DestConfig.Name = Name;
        DestConfig.ReconnectAttemptLimit = ReconnectAttemptLimit;
        DestConfig.ReconnectMaxDelay = ReconnectMaxDelay;
        DestConfig.ReconnectMinDelay = ReconnectMinDelay;
        DestConfig.ReconnectAttemptLimitSet = ReconnectAttemptLimitSet;
        DestConfig.ReconnectMaxDelaySet = ReconnectMaxDelaySet;
        DestConfig.ReconnectMinDelaySet = ReconnectMinDelaySet;

        DestConfig.ChannelSet.AddRange(ChannelSet);
    }
}
