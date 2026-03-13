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
/// Used to specify Preferred Host configuration options.
/// </summary>
///
/// <see cref="OmmConsumer.ModifyIOCtl(LSEG.Ema.Access.IOCtlCode, object)"/>
/// <see cref="IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS"/>
public sealed class PreferredHostOptions
{
    #region Public members

    /// <summary>
    /// Consturcts new preferred host options instance with default values.
    /// </summary>
    public PreferredHostOptions()
    {
        Clear();
    }

    /// <summary>
    /// Gets or sets whether preferred host feature is configured for this channel.
    /// </summary>
    /// <value>true if preferred host is enabled; false otherwise</value>
    public bool EnablePreferredHostOptions { get; set; } = false;

    /// <summary>
    /// Gets or sets Cron time schedule to switch over to a preferred host or WSB group.
    /// </summary>
    public string DetectionTimeSchedule { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets time interval in second to switch over to a preferred host or WSB group.
    /// </summary>
    public long DetectionTimeInterval { get; set; } = 0;

    /// <summary>
    /// Gets or sets the preferred host channel name.
    /// </summary>
    public string ChannelName { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the session channel name for the preferred host options.
    /// </summary>
    public string SessionChannelName { get; set; } = string.Empty;

    /// <summary>
    /// Clears this object to default.
    /// </summary>
    public void Clear()
    {
        EnablePreferredHostOptions = false;
        DetectionTimeSchedule = string.Empty;
        DetectionTimeInterval = 0;
        ChannelName = string.Empty;
        SessionChannelName = string.Empty;
    }

    /// <summary>
    /// Converts object to its textual representation.
    /// </summary>
    /// <returns>Returns textual representation of this instance parameters.</returns>
    public override string ToString()
    {
        if (m_StringBuilder is null)
        {
            m_StringBuilder = new StringBuilder(256);
        }
        else
        {
            m_StringBuilder.Clear();
        }

        m_StringBuilder.AppendLine($"\t\tEnablePreferredHostOptions={EnablePreferredHostOptions}");
        m_StringBuilder.AppendLine($"\t\tPHDetectionTimeSchedule='{DetectionTimeSchedule}'");
        m_StringBuilder.AppendLine($"\t\tPHDetectionTimeInterval={DetectionTimeInterval}");
        m_StringBuilder.AppendLine($"\t\tPreferredChannelName='{ChannelName}'");
        m_StringBuilder.AppendLine($"\t\tSessionChannelName='{SessionChannelName}'");

        return m_StringBuilder.ToString();
    }

    #endregion

    #region Implementation details

    private StringBuilder? m_StringBuilder;

    #endregion

}
