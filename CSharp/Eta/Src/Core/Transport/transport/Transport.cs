/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Reflection;
using System.Linq;
using System.Threading;

using LSEG.Eta.Common;
using LSEG.Eta.Internal;
using System.Text;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// ETA transport is used by an OMM Interactive Provider to create listening
    /// sockets and by OMM consumer and NIP applications to establish outbound
    /// connections to a listening socket.
    /// </summary>
    public static class Transport
    {
        private static long _numInitCalls = 0;
        private static Object _initializationLock = new Object(); // lock used during Initial() and Uninitialize()

        internal static Locker GlobalLocker { get; private set; }
        private static ReaderWriterLockSlim _slimLock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);

        private static bool _globalLocking = true;

        private static ProtocolRegistry _protocolRegistry = ProtocolRegistry.Instance;

        private static Assembly _assembly;

        private static readonly FileVersionInfo m_FileVersionInfo;

        private static readonly LibraryVersionInfoImpl m_LibVersionInfo = new LibraryVersionInfoImpl();

        internal static ByteBuffer DefaultComponentVersionBuffer;

        internal static readonly byte COMP_VERSION_DIVIDER = (byte)'|';

        static Transport()
        {
            try
            {
                _assembly = Assembly.GetExecutingAssembly();
                m_FileVersionInfo = FileVersionInfo.GetVersionInfo(_assembly.Location);
            }
            catch (Exception) { }

            if(m_FileVersionInfo != null && m_FileVersionInfo.ProductVersion != null)
            {
                m_LibVersionInfo.m_ProductVersion = m_FileVersionInfo.ProductVersion;

                string[] versionNumbers = m_FileVersionInfo.ProductVersion.Split('.');

                string productVersion = string.Empty;

                if(versionNumbers.Length >= 3)
                {
                    productVersion = $"{versionNumbers[0]}.{versionNumbers[1]}.{versionNumbers[2]}";
                }

                m_LibVersionInfo.m_ProductInternalVersion = $"etacsharp{productVersion}.L1.all.rrg";
            }
            else
            {
                m_LibVersionInfo.m_ProductVersion = "ETA C# Edition";
                m_LibVersionInfo.m_ProductInternalVersion = "ETA C# Edition";
            }

            DateTime localTime = DateTime.Now.ToLocalTime();

            m_LibVersionInfo.m_ProductDate = $"{localTime.ToLongDateString()} {localTime.ToLongTimeString()} {TimeZoneInfo.Local.DisplayName}";

            byte[] byteArray = Encoding.ASCII.GetBytes(m_LibVersionInfo.m_ProductInternalVersion);

            DefaultComponentVersionBuffer = new ByteBuffer(byteArray);
        }

        /// <summary>
        /// Initialize the ETA transport API and all internal members.
        /// 
        /// This is the first method called when using the ETA. It initializes internal data structures.
        /// </summary>
        /// <param name="initArgs">Arguments for initialize</param>
        /// <param name="error">ETA Error, to be set and populated in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns> 
        public static TransportReturnCode Initialize(InitArgs initArgs, out Error error)
        {
            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            lock (_initializationLock)
            {
                if (Interlocked.Read(ref _numInitCalls) >= 0)
                {
                    if (_numInitCalls == 0)
                    {
                        Interlocked.Increment(ref _numInitCalls);

                        _globalLocking = initArgs.GlobalLocking;
                        GlobalLocker = _globalLocking
                            ? new MonitorWriteLocker(new object())
                            : new NoLocker();
                        error = null;
                    }
                    else if (initArgs.GlobalLocking != _globalLocking)
                    {
                        error = new Error(errorId: TransportReturnCode.INIT_NOT_INITIALIZED,
                                             text: $"Transport.Initialize: Attempting to change locking from ({_globalLocking}) to ({initArgs.GlobalLocking}).");
                        ret = TransportReturnCode.FAILURE;
                    }
                    else
                    {
                        Interlocked.Increment(ref _numInitCalls);
                        error = null;
                    }
                }
                else
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "Initialization underflow"
                    };

                    ret = TransportReturnCode.FAILURE;
                }
            }
            return ret;

        }

        /// <summary>
        /// Uninitialize the ETA API and all internal members.<para />
        /// 
        /// This is the last method by an application when using the ETA.
        /// If multiple threads call Initialize() on Transport, they have to
        /// call Uninitialize() when the thread finishes.
        /// The last Unitialize() call release all internally pooled resources to GC.
        /// 
        /// </summary>
        /// <returns><see cref="TransportReturnCode"/></returns> 
        public static TransportReturnCode Uninitialize()
        {
            TransportReturnCode returnCode = TransportReturnCode.FAILURE;

            lock (_initializationLock)
            {
                if (_numInitCalls == 0)
                {
                    returnCode = TransportReturnCode.INIT_NOT_INITIALIZED;
                }
                else
                {
                    --_numInitCalls;
                    if (_numInitCalls == 0)
                    {
                        try
                        {
                            GlobalLocker.Enter();

                            foreach (var (connectionType, protocol) in _protocolRegistry)
                            {
                                protocol.Uninitialize(out Error error);
                            }
                        }
                        finally
                        {
                            GlobalLocker.Exit();

                            GlobalLocker = null;
                        }
                    }

                    returnCode = TransportReturnCode.SUCCESS;
                }
                return returnCode;
            }
        }

        /// <summary>
        /// Initialize transport defined in opts if not initialized.
        /// Connects a client to a listening server.
        /// </summary>
        /// <param name="connectOptions">The connection option</param>
        /// <param name="error">The error when an error occurs</param>
        /// <returns><see cref="IChannel"/></returns>
        public static IChannel Connect(ConnectOptions connectOptions, out Error error)
        {
            error = null;
            IChannel channel = null;

            if (Interlocked.Read(ref _numInitCalls) == 0)
            {
                error = new Error(errorId: TransportReturnCode.INIT_NOT_INITIALIZED,
                                     text: "Transport not initialized.");
                return null;
            }

            try
            {
                GlobalLocker.Enter();

                if (connectOptions is null)
                {
                    error = new Error
                    {
                        Channel = null,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "connectOptions must not be null"
                    };

                    return null;
                }

                var protocol = _protocolRegistry[connectOptions.ConnectionType];
                if (protocol is null)
                {
                    error = new Error
                    {
                        Channel = null,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = $"Unsupported transport type: {connectOptions.ConnectionType}"
                    };

                    return null;
                }

                channel = protocol.CreateChannel(connectOptions, out error);

                if (channel == null)
                    return channel;
            }
            catch(Exception exp)
            {
                error = new Error(errorId: TransportReturnCode.FAILURE,
                                     text: exp.Message,
                                     channel: channel);
            }
            finally
            {
                GlobalLocker.Exit();
            }


            return channel;
        }

        /// <summary>
        /// Creates a ETA Server by binding to a port.
        /// </summary>
        /// <remarks>
        /// Usage:
        /// 1. Initialize <see cref="BindOptions"/>
        /// 2. Set <see cref="BindOptions"/> to desired values
        /// 3. Call Bind to create <see cref="IServer"/>
        /// </remarks>
        /// <param name="bindOptions">Options used when binding</param>
        /// <param name="error">ETA Error, to be set in event of an error</param>
        /// <returns><see cref="IServer"/> if a server is created successfully otherwise <c>null</c></returns>
        public static IServer Bind(BindOptions bindOptions, out Error error)
        {
            error = null;
            IServer server = null;

            if (Interlocked.Read(ref _numInitCalls) == 0)
            {
                error = new Error(errorId: TransportReturnCode.INIT_NOT_INITIALIZED,
                                     text: "Transport not initialized.");
                return null;
            }

            try
            {
                GlobalLocker.Enter();

                if(bindOptions == null)
                {
                    error = new Error
                    {
                        Channel = null,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "bindOptions must not be null"
                    };

                    return null;
                }

                var protocol = _protocolRegistry[bindOptions.ConnectionType];
                if (protocol is null)
                {
                    error = new Error
                    {
                        Channel = null,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = $"Unsupported transport type: {bindOptions.ConnectionType}"
                    };

                    return null;
                }

                server = protocol.CreateServer(bindOptions, out error);
            }
            finally
            {
                GlobalLocker.Exit();
            }

            return server;
        }

        /// <summary>
        /// Programmatically extracts library and product version information that is compiled into this library.
        /// </summary>
        /// <remarks>
        /// User can call this method to programmatically extract version information.
        /// </remarks>
        /// <returns><see cref="ILibraryVersionInfo"/></returns>
        public static ILibraryVersionInfo QueryVersion()
        {
            return m_LibVersionInfo;
        }

        /// <summary>
        /// Clears ETA Initialize Arguments
        /// </summary>
        internal static void Clear()
        {
            lock (_initializationLock)
            {
                _numInitCalls = 0;

                foreach (var protocol in _protocolRegistry.Select(i => i.protocol))
                    protocol.Uninitialize(out Error error);

                while (GlobalLocker != null && GlobalLocker.Locked)
                    GlobalLocker.Exit();
                GlobalLocker = null;
            }
        }

    }
}
