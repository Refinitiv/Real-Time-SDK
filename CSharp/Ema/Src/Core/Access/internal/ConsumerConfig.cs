/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Reflection.Metadata;

namespace LSEG.Ema.Access
{
	internal class ConsumerConfig : XmlTraceConfigurable
	{
        public ConsumerConfig()
        {
            ChannelSet = new List<string>();
            Clear();
        }

        // Deep copy constuctor.
        public ConsumerConfig(ConsumerConfig oldConfig)
        {
            ChannelSet = new List<string>();
            Name = oldConfig.Name;
            Dictionary = oldConfig.Dictionary;
            Logger = oldConfig.Logger;
            DictionaryRequestTimeOut = oldConfig.DictionaryRequestTimeOut;
            DirectoryRequestTimeOut = oldConfig.DirectoryRequestTimeOut;
            LoginRequestTimeOut = oldConfig.LoginRequestTimeOut;
            RequestTimeout = oldConfig.RequestTimeout;
            DispatchTimeoutApiThread = oldConfig.DispatchTimeoutApiThread;
            EnableRtt = oldConfig.EnableRtt;
            ItemCountHint = oldConfig.ItemCountHint;
            MaxDispatchCountApiThread = oldConfig.MaxDispatchCountApiThread;
            MaxDispatchCountUserThread = oldConfig.MaxDispatchCountUserThread;
            MaxOutstandingPosts = oldConfig.MaxOutstandingPosts;
            PostAckTimeout = oldConfig.PostAckTimeout;
            MaxEventsInPool = oldConfig.MaxEventsInPool;
            MsgKeyInUpdates = oldConfig.MsgKeyInUpdates;
            ObeyOpenWindow = oldConfig.ObeyOpenWindow;
            ReconnectAttemptLimit = oldConfig.ReconnectAttemptLimit;
            ReconnectMaxDelay = oldConfig.ReconnectMaxDelay;
            ReconnectMinDelay = oldConfig.ReconnectMinDelay;
            ReissueTokenAttemptInterval = oldConfig.ReissueTokenAttemptInterval;
            ReissueTokenAttemptLimit = oldConfig.ReissueTokenAttemptLimit;
            oldConfig.ChannelSet.ForEach(channelName => ChannelSet.Add(channelName));
            RestEnableLog = oldConfig.RestEnableLog;
            RestEnableLogViaCallback = oldConfig.RestEnableLogViaCallback;
            RestLogFileName = oldConfig.RestLogFileName;
            RestRequestTimeOut = oldConfig.RestRequestTimeOut;
            RestProxyHostName = oldConfig.RestProxyHostName;
            RestProxyPort = oldConfig.RestProxyPort;
            ServiceCountHint = oldConfig.ServiceCountHint;
            XmlTraceToStdout = oldConfig.XmlTraceToStdout;
            XmlTraceToFile = oldConfig.XmlTraceToFile;
            XmlTraceMaxFileSize = oldConfig.XmlTraceMaxFileSize;
            XmlTraceFileName = oldConfig.XmlTraceFileName;
            XmlTraceToMultipleFiles = oldConfig.XmlTraceToMultipleFiles;
            XmlTraceWrite = oldConfig.XmlTraceWrite;
            XmlTraceRead = oldConfig.XmlTraceRead;
            XmlTracePing = oldConfig.XmlTracePing;
        }

        public string Name { get; set; } = string.Empty;

        public List<string> ChannelSet { get; set; }

        public string Dictionary { get; set; } = string.Empty;

        public long DictionaryRequestTimeOut { get; set; }

        public long DirectoryRequestTimeOut { get; set; }

        public long LoginRequestTimeOut { get; set; }

        public uint RequestTimeout { get; set; }

        public long DispatchTimeoutApiThread { get; set; }

        public bool EnableRtt { get; set; } 

        public uint ItemCountHint { get; set; }

        public string Logger { get; set; } = string.Empty;

        public int MaxDispatchCountApiThread { get; set; }

        public int MaxDispatchCountUserThread { get; set; }

        public uint PostAckTimeout { get; set; }

        public uint MaxOutstandingPosts { get; set; }

        public long MaxEventsInPool { get; set; }

        public bool MsgKeyInUpdates { get; set; }

        public bool ObeyOpenWindow { get; set; }

        public int ReconnectAttemptLimit { get; set; }

        public int ReconnectMaxDelay { get; set; }

        public int ReconnectMinDelay { get; set; }

        public long ReissueTokenAttemptInterval { get; set; }

        public long ReissueTokenAttemptLimit { get; set; }

        public bool RestEnableLog { get; set; }

        public bool RestEnableLogViaCallback { get; set; }

        public string RestLogFileName { get; set; } = string.Empty;

        public ulong RestRequestTimeOut { get; set; }

        public string RestProxyHostName { get; set; } = string.Empty;

        public string RestProxyPort { get; set; } = string.Empty;

        public int ServiceCountHint { get; set; }
        public bool XmlTraceToFile { get; set; }
        public bool XmlTraceToStdout { get; set; }
        public string XmlTraceFileName { get; set; } = string.Empty;
        public ulong XmlTraceMaxFileSize { get; set; }
        public bool XmlTraceToMultipleFiles { get; set; }
        public bool XmlTraceWrite { get; set; }
        public bool XmlTraceRead { get; set; }
        public bool XmlTracePing { get; set; }

        // Clears the Consumer structure and sets the default options.
        public void Clear()
        {
            Name = string.Empty;
            Dictionary = string.Empty;
            Logger = string.Empty;
            DictionaryRequestTimeOut = 45000;
            DirectoryRequestTimeOut = 45000;
            LoginRequestTimeOut = 45000;
            RequestTimeout = 15000;
            DispatchTimeoutApiThread = 0;
            EnableRtt = false;
            ItemCountHint = 100000;
            MaxDispatchCountApiThread = 100;
            MaxDispatchCountUserThread = 100;
            MaxOutstandingPosts = 100000;
            PostAckTimeout = 15000;
            MaxEventsInPool = -1;
            MsgKeyInUpdates = true;
            ObeyOpenWindow = true;
            ReconnectAttemptLimit = -1;
            ReconnectMaxDelay = 5000;
            ReconnectMinDelay = 5000;
            ReissueTokenAttemptInterval = 5000;
            ReissueTokenAttemptLimit = -1;
            ChannelSet.Clear();
            RestEnableLog = false;
            RestEnableLogViaCallback = false;
            RestLogFileName = string.Empty;
            RestRequestTimeOut = 15000;
            RestProxyHostName = string.Empty;
            RestProxyPort = string.Empty;
            ServiceCountHint = 513;
            XmlTraceToFile = false;
            XmlTraceMaxFileSize = 100_000_000;
            XmlTraceFileName = "EmaTrace";
            XmlTraceToMultipleFiles = false;
            XmlTraceWrite = true;
            XmlTraceRead = true;
            XmlTracePing = false;
        }

        // Copy method, produces a deep copy into DestConfig.
        public void Copy(ConsumerConfig DestConfig)
        {
            DestConfig.Name = Name;
            DestConfig.Dictionary = Dictionary;
            DestConfig.Logger = Logger;
            DestConfig.DictionaryRequestTimeOut = DictionaryRequestTimeOut;
            DestConfig.DirectoryRequestTimeOut = DirectoryRequestTimeOut;
            DestConfig.LoginRequestTimeOut = LoginRequestTimeOut;
            DestConfig.RequestTimeout = RequestTimeout;
            DestConfig.DispatchTimeoutApiThread = DispatchTimeoutApiThread;
            DestConfig.EnableRtt = EnableRtt;
            DestConfig.ItemCountHint = ItemCountHint;
            DestConfig.MaxDispatchCountApiThread = MaxDispatchCountApiThread;
            DestConfig.MaxDispatchCountUserThread = MaxDispatchCountUserThread;
            DestConfig.MaxOutstandingPosts = MaxOutstandingPosts;
            DestConfig.PostAckTimeout = PostAckTimeout;
            DestConfig.MaxEventsInPool = MaxEventsInPool;
            DestConfig.MsgKeyInUpdates = MsgKeyInUpdates;
            DestConfig.ObeyOpenWindow = ObeyOpenWindow;
            DestConfig.ReconnectAttemptLimit = ReconnectAttemptLimit;
            DestConfig.ReconnectMaxDelay = ReconnectMaxDelay;
            DestConfig.ReconnectMinDelay = ReconnectMinDelay;
            DestConfig.ReissueTokenAttemptInterval = ReissueTokenAttemptInterval;
            DestConfig.ReissueTokenAttemptLimit = ReissueTokenAttemptLimit;
            ChannelSet.ForEach(channelName => DestConfig.ChannelSet.Add(channelName));
            DestConfig.RestEnableLog = RestEnableLog;
            DestConfig.RestEnableLogViaCallback = RestEnableLogViaCallback;
            DestConfig.RestLogFileName = RestLogFileName;
            DestConfig.RestRequestTimeOut = RestRequestTimeOut;
            DestConfig.RestProxyHostName = RestProxyHostName;
            DestConfig.RestProxyPort = RestProxyPort;
            DestConfig.ServiceCountHint = ServiceCountHint;

            DestConfig.XmlTraceToStdout = XmlTraceToStdout;
            DestConfig.XmlTraceToFile = XmlTraceToFile;
            DestConfig.XmlTraceMaxFileSize = XmlTraceMaxFileSize;
            DestConfig.XmlTraceFileName = XmlTraceFileName;
            DestConfig.XmlTraceToMultipleFiles = XmlTraceToMultipleFiles;
            DestConfig.XmlTraceWrite = XmlTraceWrite;
            DestConfig.XmlTraceRead = XmlTraceRead;
            DestConfig.XmlTracePing = XmlTracePing;
        }
    }
}
