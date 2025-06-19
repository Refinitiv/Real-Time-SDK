/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    // This class is the super-class for the Provider group configurations, containing all 
    // common elements between the OMMNiProvider and OMMIProviders.
    internal class ProviderConfig : XmlTraceConfigurable
    {
        // Name of this provider config.
        public string Name { get; set; } = string.Empty;
        public string Directory { get; set; } = string.Empty;
        public long DispatchTimeoutApiThread { get; set; }
        public ulong ItemCountHint { get; set; }
        public string Logger { get; set; } = string.Empty;
        public int MaxDispatchCountApiThread { get; set; }
        public int MaxDispatchCountUserThread { get; set; }
        public int MaxEventsInPool { get; set; }
        public bool RefreshFirstRequired { get; set; }
        public ulong RequestTimeout { get; set; }
        public int ServiceCountHint { get; set; }

        public bool XmlTraceToFile { get; set; }
        public bool XmlTraceToStdout { get; set; }
        public string XmlTraceFileName { get; set; } = string.Empty;
        public ulong XmlTraceMaxFileSize { get; set; }
        public bool XmlTraceToMultipleFiles { get; set; }
        public bool XmlTraceWrite { get; set; }
        public bool XmlTraceRead { get; set; }
        public bool XmlTracePing { get; set; }

        internal ProviderConfig()
        {
            Clear();
        }

        internal ProviderConfig(ProviderConfig oldConfig)
        {
            Name = oldConfig.Name;
            Directory = oldConfig.Directory;
            DispatchTimeoutApiThread = oldConfig.DispatchTimeoutApiThread;
            ItemCountHint = oldConfig.ItemCountHint;
            Logger = oldConfig.Logger;
            MaxDispatchCountApiThread = oldConfig.MaxDispatchCountApiThread;
            MaxDispatchCountUserThread = oldConfig.MaxDispatchCountUserThread;
            MaxEventsInPool = oldConfig.MaxEventsInPool;
            RefreshFirstRequired = oldConfig.RefreshFirstRequired;
            RequestTimeout = oldConfig.RequestTimeout;
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

        internal void Copy(ProviderConfig destConfig)
        {
            destConfig.Name = Name;
            destConfig.Directory = Directory;
            destConfig.DispatchTimeoutApiThread = DispatchTimeoutApiThread;
            destConfig.ItemCountHint = ItemCountHint;
            destConfig.Logger = Logger;
            destConfig.MaxDispatchCountApiThread = MaxDispatchCountApiThread;
            destConfig.MaxDispatchCountUserThread = MaxDispatchCountUserThread;
            destConfig.MaxEventsInPool = MaxEventsInPool;
            destConfig.RefreshFirstRequired = RefreshFirstRequired;
            destConfig.RequestTimeout = RequestTimeout;
            destConfig.ServiceCountHint = ServiceCountHint;

            destConfig.XmlTraceToStdout = XmlTraceToStdout;
            destConfig.XmlTraceToFile = XmlTraceToFile;
            destConfig.XmlTraceMaxFileSize = XmlTraceMaxFileSize;
            destConfig.XmlTraceFileName = XmlTraceFileName;
            destConfig.XmlTraceToMultipleFiles = XmlTraceToMultipleFiles;
            destConfig.XmlTraceWrite = XmlTraceWrite;
            destConfig.XmlTraceRead = XmlTraceRead;
            destConfig.XmlTracePing = XmlTracePing;
        }

        internal void Clear()
        {
            Name = string.Empty;
            Directory = string.Empty;
            DispatchTimeoutApiThread = -1;
            ItemCountHint = 100000;
            Logger = string.Empty;
            MaxDispatchCountApiThread = 100;
            MaxDispatchCountUserThread = 100;
            MaxEventsInPool = -1;
            RefreshFirstRequired = true;
            RequestTimeout = 15000;
            ServiceCountHint = 513;

            XmlTraceToStdout = false;
            XmlTraceToFile = false;
            XmlTraceMaxFileSize = 100_000_000;
            XmlTraceFileName = "EmaTrace";
            XmlTraceToMultipleFiles = false;
            XmlTraceWrite = true;
            XmlTraceRead = true;
            XmlTracePing = false;
        }
    }
}