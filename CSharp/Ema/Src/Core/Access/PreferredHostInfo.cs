/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Text;

namespace LSEG.Ema.Access;

/// <summary>
/// This class represents EMA preferred host information.
/// </summary>
/// 
/// <see cref="ChannelInformation.PreferredHostInfo"/>
public sealed class PreferredHostInfo
{

    #region Public members

    /// <summary>
    /// Indicates whether preferred host feature is configured for this channel.
    /// </summary>
    ///
    /// <value>true if preferred host is enabled; false otherwise</value>
    public bool IsPreferredHostEnabled { get; internal set; }

    /// <summary>
    /// Returns Cron time schedule to switch over to a preferred host.
    /// </summary>
    ///
    /// <value>time format to switch over</value>
    public string DetectionTimeSchedule { get; internal set; } = string.Empty;

    /// <summary>
    /// Returns time interval in second to switch over to a preferred host.
    /// </summary>
    ///
    /// <value>time interval to switch over</value>
    public long DetectionTimeInterval { get; internal set; }

    /// <summary>
    /// Returns a channel name to set a preferred host.
    /// </summary>
    ///
    /// <value>the preferred host channel name</value>
    public string ChannelName { get; internal set; } = string.Empty;

    /// <summary>
    /// Returns the remaining detection time in seconds to perform fallback to preferred host.
    /// </summary>
    /// <value>the remaing time to switch over</value>
    public long RemainingDetectionTime { get; internal set; } = 0;

    /// <summary>
    /// Clears this object to default.
    /// </summary>
    public void Clear()
    {
        IsPreferredHostEnabled = false;
        DetectionTimeSchedule = string.Empty;
        DetectionTimeInterval = 0;
        ChannelName = string.Empty;
        RemainingDetectionTime = 0;
    }




    /// <summary>
    /// Converts this Preferred Host Info instance to text representation.
    /// </summary>
    /// <returns>Returns a text representation of this Preferred Host Info instance.</returns>
    public override string ToString()
    {
        if (m_StringBuilder == null)
        {
            m_StringBuilder = new StringBuilder(256);
        }
        else
        {
            m_StringBuilder.Clear();
        }

        m_StringBuilder.AppendLine().AppendLine($"\t\tEnablePreferredHostOptions={IsPreferredHostEnabled}");
        m_StringBuilder.AppendLine($"\t\tPHDetectionTimeSchedule='{DetectionTimeSchedule}'");
        m_StringBuilder.AppendLine($"\t\tPHDetectionTimeInterval={DetectionTimeInterval}");
        m_StringBuilder.AppendLine($"\t\tPreferredChannelName='{ChannelName}'");
        m_StringBuilder.AppendLine($"\t\tRemainingDetectionTime={RemainingDetectionTime}");

        return m_StringBuilder.ToString();
    }

    #endregion

    #region Implementation details

    /// <summary>
    /// Creates a Preferred Host Info instance with default parameters.
    /// </summary>
    internal PreferredHostInfo()
    {
        Clear();
    }

    private StringBuilder? m_StringBuilder;

    #endregion
}
