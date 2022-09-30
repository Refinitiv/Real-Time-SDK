/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Net.Sockets;
using System.Threading;
using System.Text.RegularExpressions;

using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Common;
using Buffer = Refinitiv.Eta.Codec.Buffer;
using System.Diagnostics;
using System.Text;
using Refinitiv.Eta.Codec;

namespace Refinitiv.Eta.Tools.TransportTest
{
    public class Program
    {
        static void Main(string[] args)
        {
            TransportTest transportTest = new TransportTest();
            TransportReturnCode retVal;

            ILibraryVersionInfo libVersion = Transport.QueryVersion();
            Console.WriteLine(libVersion);

            InitArgs initArgs = new InitArgs();
            initArgs.GlobalLocking = true;
            if((retVal = Transport.Initialize(initArgs,out Error error)) < TransportReturnCode.SUCCESS)
            {
                Console.WriteLine($"Initialize(): failed with return code {retVal} <{error.Text}>");
                Environment.Exit(-1);
            }

            transportTest.StartSelectLoop();

            /* start menu loop of test */
            while (transportTest.con)
            {
                transportTest.PrintMenu();
                transportTest.ReadStdio();
            }

            foreach(var channel in transportTest.sessionList.Values)
            {
                transportTest.RemoveSession(channel);
            }

            foreach(var server in transportTest.serverList.Values)
            {
                transportTest.RemoveServer(server);
            }
        }
    }

    public class TransportTest
    {
        const int HEX_LINE_SIZE = 19;
        const int ETA_BIND = 1;
        const int ETA_SRVR_SHUTDOWN = 2;
        const int ETA_WRITE = 3;
        const int ETA_FLUSH = 4;
        const int ETA_CONNECT = 5;
        const int ETA_RECONNECT = 6;
        const int ETA_WRITE_PACKED = 7;
        const int ETA_WRITE_FRAGMENTED = 8;
        const int ETA_SESS_SHUTDOWN = 9;
        const int ETA_IOCTL = 10;
        const int ETA_SRVR_IOCTL = 11;
        const int ETA_CHAN_INFO = 12;
        const int ETA_PING = 13;
        const int ETA_SRVR_INFO = 14;
        const int ETA_WRITE_MULTIPLE = 15;
        const int ETA_WRITE_RANDOM = 16;
        const int ETA_WRITE_FULL_PACKED = 17;
        const int ETA_ENCRYPT_DECRYPT = 18;
        const int ETA_QUIT = 19;

        public bool echo;
        public bool con = true;
        public bool encryptDecrypt = false;

        EncryptDecryptHelpers? m_EncDecHelper;

        public ConcurrentDictionary<IntPtr, IServer> serverList = new ConcurrentDictionary<IntPtr, IServer>();
        public ConcurrentDictionary<IntPtr, IChannel> sessionList = new ConcurrentDictionary<IntPtr, IChannel>();
        List<Socket> m_ReadSockets = new List<Socket>(10);
        List<Socket> m_WriteSockets = new List<Socket>(10);
        List<Socket> m_ServerSockets = new List<Socket>(10);

        ReadArgs readArgs = new ReadArgs();

        Random random = new Random();

        public int numConnections = 2; // We start at 3 on first connection, similar to C (They use file descriptor to target, while we just have this ID)
        public int numServers = 2; // We start at 3 on first connection, similar to C (They use file descriptor to target, while we just have this ID)

        public void StartSelectLoop()
        {
            Thread runThread = new Thread(new ThreadStart(Run));
            runThread.Start();
        }

        public void StopSelectLoop()
        {
            con = false; ;
        }

        private void Run()
        {
            List<Socket> readSockets = new List<Socket>();
            List<Socket> writeSockets = new List<Socket>();

            while (con)
            {
                readSockets.Clear();
                writeSockets.Clear();

                for (int i = 0; i < m_ReadSockets.Count; i++)
                {
                    readSockets.Add(m_ReadSockets[i]);
                }

                for (int i = 0; i < m_WriteSockets.Count; i++)
                {
                    writeSockets.Add(m_WriteSockets[i]);
                }

                for (int i = 0; i < m_ServerSockets.Count; i++)
                {
                    readSockets.Add(m_ServerSockets[i]);
                }

                if (readSockets.Count != 0)
                {
                    Socket.Select(readSockets, writeSockets, null, 100);
                    Socket socket;

                    if (readSockets.Count > 0)
                    {
                        for (int i = 0; i < readSockets.Count; i++)
                        {
                            socket = readSockets[i];

                            if (serverList.ContainsKey(socket.Handle))
                            {
                                IServer server = serverList[socket.Handle];
                                NewSession(server);
                            }
                            else
                            {
                                if(sessionList.TryGetValue(socket.Handle, out IChannel? channel))
                                {
                                    ReadFromSession(channel);
                                }
                            }
                        }
                    }

                    if (writeSockets.Count > 0)
                    {
                        for (int i = 0; i < writeSockets.Count; i++)
                        {
                            socket = writeSockets[i];

                            if (sessionList.TryGetValue(socket.Handle, out IChannel? channel))
                            {
                                ReadFromSession(channel);
                            }
                        }
                    }
                    else
                    {
                        Thread.Sleep(100);
                    }
                }
                else
                {
                    Thread.Sleep(100);
                }
            }
        }

        public void PrintMenu()
        {
            Console.WriteLine("\n      ETA Test Generator");
            Console.WriteLine($"      ({ETA_BIND}) Initialize server");
            Console.WriteLine($"      ({ETA_SRVR_SHUTDOWN}) Server shutdown");
            Console.WriteLine($"      ({ETA_WRITE}) Send sesssion message");
            Console.WriteLine($"      ({ETA_FLUSH}) Flush sesssion");
            Console.WriteLine($"      ({ETA_CONNECT}) Connect Client");
            Console.WriteLine($"      ({ETA_WRITE_PACKED}) Write Packed Buffer");
            Console.WriteLine($"      ({ETA_WRITE_FRAGMENTED}) Write Fragmented Buffer");
            Console.WriteLine($"      ({ETA_SESS_SHUTDOWN}) Session shutdown");
            Console.WriteLine($"      ({ETA_IOCTL}) Ioctl");
            Console.WriteLine($"      ({ETA_SRVR_IOCTL}) Server Ioctl");
            Console.WriteLine($"      ({ETA_CHAN_INFO}) Session Info");
            Console.WriteLine($"      ({ETA_PING}) Send heartbeat");
            Console.WriteLine($"      ({ETA_SRVR_INFO}) Server Info");
            Console.WriteLine($"      ({ETA_WRITE_MULTIPLE}) Write Multiple");
            Console.WriteLine($"      ({ETA_WRITE_RANDOM}) Write Random");
            Console.WriteLine($"      ({ETA_WRITE_FULL_PACKED}) Write Full Packed Buffer");
            Console.WriteLine($"      ({ETA_ENCRYPT_DECRYPT}) Enable Encryption/Decryption");
            Console.WriteLine($"      ({ETA_QUIT}) Exit ETA Generator");
        }

        public void RemoveSession(IChannel channel)
        {
            if (channel.State == ChannelState.INACTIVE)
                return;

            m_ReadSockets.Remove(channel.Socket);

            sessionList.TryRemove(channel.Socket.Handle, out var value);

            if(channel.Close(out Error error) < TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("Error closing channel " + error.Text);
            }
        }

        public void RemoveServer(IServer server)
        {
            if (server.State == ChannelState.INACTIVE)
                return;

            m_ServerSockets.Remove(server.Socket);

            serverList.TryRemove(server.Socket.Handle, out var value);
        }

        public TransportReturnCode ReadStdio()
        {
            String? selection;
            IChannel? channel;
            IServer? srvr;
            int value = 0;
            String? serviceName = null;
            String? hostName = null;
            String? interfaceName = null;
            String? componentVersion = null;
            ITransportBuffer sendbuffer;
            int maxMsgSize;
            int maxBufferSize;
            CompressionType compression = 0;
            int force = 0;
            int compLevel = 0;
            IntPtr sessId;
            IOCtlCode code;
            TransportReturnCode retval = 0;
            int priority = 1;
            String flushOrder = "";
            ChannelInfo Info = new ChannelInfo();
            int pingTO = 0;
            int minPing = 0;
            int compFlag = 0;
            WriteFlags writeFlag = 0;
            int z = 0;
            int srvrIoctlOption = 0;
            ServerInfo srvrInfo = new ServerInfo();
            int majorVersion;
            int minorVersion;
            int protocolType = Codec.Codec.ProtocolType();
            WriteArgs writeArgs = new WriteArgs();

            selection = GetInput("\nSelect: ");

            if (selection == null)
            {
                retval = 0;
            }

            try
            {
                value = int.Parse(selection!);
            }
            catch (Exception)
            {
                value = 99;
            }

            switch (value)
            {
                case ETA_BIND:
                    {
                        ConnectionType connType;
                        BindOptions sopts = new BindOptions();

                        serviceName = GetInput("Enter port number: ");
                        hostName = GetInput("Enter interface name: ");
                        try
                        {
                            connType = (ConnectionType)int.Parse(GetInput("Connection Type (0: Socket 1: SSL): ")!);
                        }
                        catch (FormatException)
                        {
                            connType = ConnectionType.SOCKET;
                        }
                        try
                        {
                            maxMsgSize = int.Parse(GetInput("Max Fragment Size: ")!);
                        }
                        catch (FormatException)
                        {
                            maxMsgSize = 6144;
                        }

                        if (connType == ConnectionType.ENCRYPTED && !FillAndValidateServerEncryptedOptions(sopts))
                        {
                            break;
                        }

                        try
                        {
                            pingTO = int.Parse(GetInput("Ping Timeout: ")!);
                        }
                        catch (FormatException)
                        {
                            pingTO = 60;
                        }
                        try
                        {
                            minPing = int.Parse(GetInput("Min Ping Timeout: ")!);
                        }
                        catch (FormatException)
                        {
                            minPing = 20;
                        }
                        try
                        {
                            compression = (CompressionType)int.Parse(GetInput("Compression(ZLIB=1, LZ4=2): ")!);
                        }
                        catch (FormatException)
                        {
                            compression = 0;
                        }
                        if (compression > 0)
                        {
                            try
                            {
                                compLevel = int.Parse(GetInput("Compression Level (0:fastest - 9:most compression): ")!);
                            }
                            catch (FormatException)
                            {
                                compLevel = 0;
                            }
                            try
                            {
                                force = int.Parse(GetInput("Force Compression: ")!);
                            }
                            catch (FormatException)
                            {
                                force = 0;
                            }
                        }
                        try
                        {
                            echo = int.Parse(GetInput("Echo?: ")!) > 0;
                        }
                        catch (FormatException)
                        {
                            echo = false;
                        }
                        try
                        {
                            majorVersion = int.Parse(GetInput("Major Version?: ")!);
                        }
                        catch (FormatException)
                        {
                            majorVersion = 0;
                        }
                        try
                        {
                            minorVersion = int.Parse(GetInput("Minor Version?: ")!);
                        }
                        catch (FormatException)
                        {
                            minorVersion = 0;
                        }

                        protocolType = 0;

                        componentVersion = GetInput("Component Version?: ");


                        sopts.ServiceName = serviceName;
                        sopts.InterfaceName = hostName;
                        sopts.ConnectionType = connType;
                        sopts.CompressionType = (CompressionType)compression;
                        sopts.CompressionLevel = compLevel;
                        sopts.ForceCompression = force > 0 ? true : false;
                        sopts.PingTimeout = pingTO;
                        sopts.MinPingTimeout = minPing;
                        sopts.MaxFragmentSize = maxMsgSize;
                        sopts.ProtocolType = (Transports.ProtocolType)protocolType;

                        if (componentVersion!.Length != 0)
                        {
                            sopts.ComponentVersion = componentVersion;
                        }

                        if (majorVersion != 0)
                        {
                            sopts.MajorVersion = majorVersion;
                        }
                        else
                        {
                            sopts.MajorVersion = Codec.Codec.MajorVersion();
                        }
                        if (minorVersion != 0)
                        {
                            sopts.MinorVersion = minorVersion;
                        }
                        else
                        {
                            sopts.MinorVersion = Codec.Codec.MinorVersion();
                        }

                        if ((srvr = Transport.Bind(sopts, out Error error)) == null)
                        {
                            Console.WriteLine($"Bind(): Failed {error.Text}");
                        }
                        else
                        {
                            AddServer(srvr);
                            Console.WriteLine("\nServer IPC descriptor = " + srvr.Socket.Handle + " bound on port " + srvr.PortNumber);
                            /* add accept to select */

                            m_ServerSockets.Add(srvr.Socket);
                        }
                        break;
                    }
                case ETA_SRVR_SHUTDOWN:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter server ID to shutdown: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        srvr = GetServer(sessId);
                        if (srvr != null)
                        {
                            Console.WriteLine($"Shutting down server with ID {sessId}");
                            RemoveServer(srvr);

                            if (srvr.Close(out Error error) < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine($"Server shutdown failed {error.Text}");
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Server with ID " + sessId);
                        }
                        break;
                    }
                case ETA_CONNECT:
                    {
                        int blocking;
                        ConnectionType connType;
                        int networkType = 0;
                        ConnectOptions copts = new ConnectOptions();

                        while (networkType != 1)
                        {
                            try
                            {
                                networkType = int.Parse(GetInput("Enter 1 for unified network: ")!);
                            }
                            catch (FormatException)
                            {
                                networkType = 0;
                            }
                        }
                        try
                        {
                            connType = (ConnectionType)int.Parse(GetInput("Connection Type (0: None; 1: SSL): ")!);
                        }
                        catch (FormatException)
                        {
                            connType = ConnectionType.SOCKET;
                        }

                        if (connType == ConnectionType.ENCRYPTED)
                        {
                            FillClientEncryptedOptions(copts);
                        }

                        if (networkType == 1) // unified network
                        {
                            serviceName = GetInput("Enter server name (port number): ");
                            if (string.IsNullOrEmpty(serviceName))
                                serviceName = "14002";

                            hostName = GetInput("Enter host name: ");
                            if (string.IsNullOrEmpty(hostName))
                                hostName = "localhost";

                            interfaceName = GetInput("Enter interface name: ");
                        }

                        try
                        {
                            blocking = int.Parse(GetInput("Blocking: ")!);
                        }
                        catch (FormatException)
                        {
                            blocking = 0;
                        }
                        try
                        {
                            pingTO = int.Parse(GetInput("Ping Timeout: ")!);
                        }
                        catch (FormatException)
                        {
                            pingTO = 0;
                        }
                        try
                        {
                            compression = (CompressionType)int.Parse(GetInput("Compression(ZLIB=1, LZ4=2): ")!);
                        }
                        catch (FormatException)
                        {
                            compression = 0;
                        }
                        try
                        {
                            majorVersion = int.Parse(GetInput("Major Version?: ")!);
                        }
                        catch (FormatException)
                        {
                            majorVersion = 0;
                        }
                        try
                        {
                            minorVersion = int.Parse(GetInput("Minor Version?: ")!);
                        }
                        catch (FormatException)
                        {
                            minorVersion = 0;
                        }

                        try
                        {
                            protocolType = int.Parse(GetInput("Protocol Type?: ")!);
                        }
                        catch (FormatException)
                        {
                            protocolType = 0;
                        }

                        componentVersion = GetInput("Component Version?: ");

                        if (networkType == 1) // unified network
                        {
                            copts.UnifiedNetworkInfo.Address = hostName;
                            copts.UnifiedNetworkInfo.ServiceName = serviceName;
                            copts.UnifiedNetworkInfo.InterfaceName = interfaceName;

                            copts.PingTimeout = pingTO;
                            copts.Blocking = blocking > 0;
                            copts.ConnectionType = connType;
                            copts.CompressionType = compression;

                            if (!string.IsNullOrEmpty(componentVersion))
                            {
                                copts.ComponentVersion = componentVersion;
                            }
                            if (majorVersion != 0)
                            {
                                copts.MajorVersion = majorVersion;
                            }
                            else
                            {
                                copts.MajorVersion = Codec.Codec.MajorVersion();
                            }
                            if (minorVersion != 0)
                            {
                                copts.MinorVersion = minorVersion;
                            }
                            else
                            {
                                copts.MinorVersion = Codec.Codec.MinorVersion();
                            }
                            if (protocolType != 0)
                            {
                                copts.ProtocolType = (Transports.ProtocolType)protocolType;
                            }
                            else
                            {
                                copts.ProtocolType = (Transports.ProtocolType)Codec.Codec.ProtocolType();
                            }

                            if ((channel = Transport.Connect(copts, out Error error)) == null)
                            {
                                Console.WriteLine("Connect(): Failed " + error.Text);
                            }
                            else
                            {
                                AddSession(channel);

                                m_ReadSockets.Add(channel.Socket);
                                m_WriteSockets.Add(channel.Socket);
                            }
                        }
                        break;
                    }
                case ETA_WRITE_PACKED:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to send: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            int testType = 1; // default: standard test
                            try
                            {
                                testType = int.Parse(GetInput("Test Type [1=standard (default) 2=max buffer]: ")!);
                                if (testType == 1)
                                    StandardPackTest(channel, sessId);
                                else if (testType == 2)
                                    MaxBufferPackTest(channel, sessId);
                                else
                                    Console.WriteLine("\tUnknown Test: " + testType);
                            }
                            catch (FormatException)
                            {
                                testType = 1;
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_WRITE_FRAGMENTED:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to send: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            try
                            {
                                maxMsgSize = int.Parse(GetInput("Enter fragmented data buffer size you want: ")!);
                            }
                            catch (FormatException)
                            {
                                maxMsgSize = 0;
                            }
                            if (maxMsgSize > 0)
                            {
                                sendbuffer = channel.GetBuffer(maxMsgSize, false, out Error error);

                                if (sendbuffer == null)
                                {
                                    Console.WriteLine("GetBuffer(): Failed " + error.Text);
                                }
                                else
                                {
                                    int dataFormat = 1; // default: sequential data
                                    try
                                    {
                                        dataFormat = int.Parse(GetInput("Buffer contents [1=sequential (default), 2=uniform, 3=random]: ")!);
                                    }
                                    catch (FormatException)
                                    {
                                        dataFormat = 1; // sequential data
                                    }
                                    /* now populate the buffer */
                                    ByteBuffer bb = sendbuffer.Data;
                                    PopulateData(dataFormat, bb, maxMsgSize);

                                    if (encryptDecrypt == true)
                                    {
                                        /* get another buffer, encrypt this, then send the encrypted content */
                                        ITransportBuffer sendbuffer2 = channel.GetBuffer((int)m_EncDecHelper!.CalculateEncryptedSize(sendbuffer), false, out error);
                                        if (sendbuffer2 == null)
                                        {
                                            Console.WriteLine($"GetBuffer(): Failed {error.Text}");
                                        }
                                        else
                                        {
                                            if (m_EncDecHelper.EncryptBuffer(channel, sendbuffer, sendbuffer2, out CodecError codecErr) < CodecReturnCode.SUCCESS)
                                                Console.WriteLine($"EncryptBuffer() Failed with code {codecErr.ErrorId} and text {codecErr.Text}");
                                            else
                                            {
                                                /* release original buffer */
                                                channel.ReleaseBuffer(sendbuffer, out error);
                                                sendbuffer = sendbuffer2;
                                            }
                                        }
                                    }

                                    writeArgs.Priority = WritePriorities.HIGH;
                                    writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                                    retval = channel.Write(sendbuffer, writeArgs, out error);
                                    if (retval < TransportReturnCode.SUCCESS)
                                    {
                                        if (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                                        {
                                            while (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                                            {
                                                channel.Flush(out error);
                                                retval = channel.Write(sendbuffer, writeArgs, out error);
                                            }
                                            if (retval > 0)
                                                channel.Flush(out error);
                                        }
                                        if (retval < TransportReturnCode.SUCCESS)
                                        {
                                            Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                                            retval = channel.ReleaseBuffer(sendbuffer, out error);
                                            RemoveSession(channel);
                                        }
                                        else
                                            Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");
                                    }
                                    else
                                    {
                                        Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");
                                    }

                                    while (retval > 0)
                                    {
                                        retval = channel.Flush(out error);
                                    }
                                }
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_WRITE:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to send: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            try
                            {
                                maxMsgSize = int.Parse(GetInput("Enter output data buffer size you want: ")!);
                            }
                            catch (FormatException)
                            {
                                maxMsgSize = 0;
                            }
                            if (maxMsgSize > 0)
                            {
                                sendbuffer = channel.GetBuffer(maxMsgSize, false, out Error error);

                                if (sendbuffer == null)
                                {
                                    Console.WriteLine("GetBuffer(): Failed " + error.Text);
                                }
                                else
                                {
                                    try
                                    {
                                        priority = int.Parse(GetInput("Enter priority of data (1 - High, 2 - Medium, 3 - Low): ")!);
                                    }
                                    catch (FormatException)
                                    {
                                        priority = 1;
                                    }
                                    try
                                    {
                                        compFlag = int.Parse(GetInput("Do not compress?: ")!);
                                    }
                                    catch (FormatException)
                                    {
                                        compFlag = 0;
                                    }
                                    if (compFlag > 0)
                                    {
                                        writeFlag = WriteFlags.DO_NOT_COMPRESS;
                                    }
                                    else
                                    {
                                        writeFlag = WriteFlags.NO_FLAGS;
                                    }

                                    int dataType = 0;
                                    try
                                    {
                                        dataType = int.Parse(GetInput("buffer contents [0=key entry (default), 1=sequential, 2=uniform, 3=random]: ")!);
                                    }
                                    catch (FormatException)
                                    {
                                        dataType = 0; // default: keyboard entry
                                    }
                                    /* now populate the buffer */
                                    ByteBuffer bb = sendbuffer.Data;
                                    if (dataType == 0) // key entry
                                    {
                                        Console.WriteLine("            1234567890123456789012345678901234567890123456789012345678901234567890\n");
                                        String dataString = GetInput("Enter data: ")!;
                                        if (dataString.Length > maxMsgSize)
                                        {
                                            Console.WriteLine("Buffer too small");
                                            channel.ReleaseBuffer(sendbuffer, out error);
                                            break;
                                        }
                                        for (z = 0; z < dataString.Length; z++)
                                        {
                                            bb.Write((byte)dataString[z]);
                                        }
                                    }
                                    else
                                    {
                                        PopulateData(dataType, bb, maxMsgSize);
                                    }

                                    if (encryptDecrypt == true)
                                    {
                                        /* get another buffer, encrypt this, then send the encrypted content */
                                        ITransportBuffer sendbuffer2 = channel.GetBuffer((int)m_EncDecHelper!.CalculateEncryptedSize(sendbuffer), false, out error);
                                        if (sendbuffer2 == null)
                                        {
                                            Console.WriteLine($"GetBuffer(): Failed {error.Text}");
                                        }
                                        else
                                        {
                                            if (m_EncDecHelper.EncryptBuffer(channel, sendbuffer, sendbuffer2, out CodecError codecErr) < CodecReturnCode.SUCCESS)
                                                Console.WriteLine($"EncryptBuffer() Failed with code {codecErr.ErrorId} and text {codecErr.Text}");

                                            else
                                            {
                                                /* release original buffer */
                                                channel.ReleaseBuffer(sendbuffer, out error);
                                                sendbuffer = sendbuffer2;
                                            }
                                        }
                                    }

                                    writeArgs.Priority = (WritePriorities)(priority - 1);
                                    writeArgs.Flags = writeFlag | WriteFlags.DIRECT_SOCKET_WRITE;
                                    retval = channel.Write(sendbuffer, writeArgs, out error);
                                    if (retval < TransportReturnCode.SUCCESS)
                                    {
                                        if (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                                        {
                                            while (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                                            {
                                                channel.Flush(out error);
                                                retval = channel.Write(sendbuffer, writeArgs, out error);
                                            }
                                            if (retval > 0)
                                                channel.Flush(out error);
                                        }
                                        if (retval < TransportReturnCode.SUCCESS)
                                        {
                                            Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                                            retval = channel.ReleaseBuffer(sendbuffer, out error);
                                            RemoveSession(channel);
                                        }
                                        else
                                            Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");
                                    }
                                    else
                                    {
                                        Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");
                                    }
                                }
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_PING:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to send ping on: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {

                            retval = channel.Ping(out Error error);
                            if (retval < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("\nPing() failed with code " + retval);

                                RemoveSession(channel);
                            }
                            else
                                Console.WriteLine("\nPing() sent\n");
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_FLUSH:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to send: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            retval = channel.Flush(out Error error);
                            if (retval < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("\nsessionInactive " + channel.Socket.Handle + "<" + error.Text + ">");

                                RemoveSession(channel);
                            }
                            Console.WriteLine("\nSession flushed, " + retval + " bytes pending to be flushed");
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_SESS_SHUTDOWN:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to shutdown: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            Console.WriteLine("Shutting down session with ID " + sessId);

                            RemoveSession(channel);
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_SRVR_IOCTL:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter server ID: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }

                        srvr = GetServer(sessId);
                        if (srvr == null)
                        {
                            Console.WriteLine("No Server with ID " + sessId);
                            break;
                        }

                        try
                        {
                            srvrIoctlOption = int.Parse(GetInput("Enter 1 to change the number of server shared pool buffers\n or 2 to reset the peak number of server shared pool buffers: ")!);
                        }
                        catch (FormatException)
                        {
                            srvrIoctlOption = 0;
                        }

                        if (srvrIoctlOption == 1)
                        {
                            try
                            {
                                value = int.Parse(GetInput("Enter new value for pool size: ")!);
                            }
                            catch (FormatException)
                            {
                                value = 0;
                            }

                            /* do server ioctl */
                            if (srvr.IOCtl(IOCtlCode.SERVER_NUM_POOL_BUFFERS, value, out Error error) < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("\nServerIoctl() failed with ID " + sessId + " <" + error.Text + ">");
                            }
                            else
                                Console.WriteLine("\nServerIoctl() call successful\n");
                        }
                        else if (srvrIoctlOption == 2)
                        {
                            ///* do server ioctl */
                            if (srvr.IOCtl(IOCtlCode.SERVER_PEAK_BUF_RESET, 1, out Error error) < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine($"\nServerIoctl() failed with ID {sessId} <{error.Text}>");
                            }
                            else
                                Console.WriteLine($"\nServerIoctl() call successful\n");
                        }
                        else
                        {
                            Console.WriteLine("\nInvalid option\n");
                        }

                        break;
                    }
                case ETA_IOCTL:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        try
                        {
                            code = (IOCtlCode)int.Parse(GetInput("Enter ioctl code: ")!);
                        }
                        catch (FormatException)
                        {
                            code = 0;
                        }
                        if (code == IOCtlCode.PRIORITY_FLUSH_ORDER)
                        {
                            flushOrder = GetInput("Enter flush order: ")!;
                        }
                        else
                        {
                            try
                            {
                                value = int.Parse(GetInput("Enter value: ")!);
                            }
                            catch (FormatException)
                            {
                                value = 0;
                            }
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            if (code == IOCtlCode.PRIORITY_FLUSH_ORDER)
                            {
                                if (channel.IOCtl(code, flushOrder, out Error error) < TransportReturnCode.SUCCESS)
                                {
                                    Console.WriteLine("\nIoctl() failed with ID " + sessId + " <" + error.Text + ">");
                                }
                                else
                                {
                                    Console.WriteLine("\nIoctl() call successful");
                                }
                            }
                            else
                            {
                                if (channel.IOCtl(code, value, out Error error) < TransportReturnCode.SUCCESS)
                                {
                                    Console.WriteLine("\nIoctl() failed with ID " + sessId + " <" + error.Text + ">");
                                }
                                else
                                {
                                    Console.WriteLine("\nIoctl() call successful");
                                }
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_CHAN_INFO:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }

                        channel = Session(sessId);
                        if (channel != null)
                        {
                            if (channel.Info(Info, out Error error) < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("\nGetChannelInfo failed with ID " + sessId + " <" + error.Text + ">");
                            }
                            else
                            {
                                /* print info */
                                Console.WriteLine(Info);
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_SRVR_INFO:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter server ID: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }

                        srvr = GetServer(sessId);
                        if (srvr == null)
                        {
                            Console.WriteLine("No Server with ID " + sessId);
                            break;
                        }
                        if (srvr.Info(srvrInfo, out Error error) < TransportReturnCode.SUCCESS)
                        {
                            Console.WriteLine("\nGetServerInfo failed with ID " + sessId + " <" + error.Text + ">");
                        }
                        else
                        {
                            Console.WriteLine(srvrInfo);
                        }
                        break;
                    }
                case ETA_WRITE_MULTIPLE:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to send: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            ByteBuffer bb;

                            sendbuffer = channel.GetBuffer(30, true, out Error error);

                            if (sendbuffer == null)
                            {
                                Console.WriteLine("GetBuffer(): Failed " + error.Text);
                            }
                            else
                            {
                                bb = sendbuffer.Data;
                                bb.Put(Encoding.ASCII.GetBytes("hello1"));

                            }

                            writeArgs.Priority = WritePriorities.HIGH;
                            writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                            retval = channel.Write(sendbuffer, writeArgs, out error);
                            if (retval < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                                retval = channel.ReleaseBuffer(sendbuffer, out error);
                                RemoveSession(channel);
                            }

                            sendbuffer = channel.GetBuffer(30, true, out error);

                            if (sendbuffer == null)
                            {
                                Console.WriteLine("GetBuffer(): Failed " + error.Text);
                            }
                            else
                            {
                                bb = sendbuffer.Data;
                                bb.Put(Encoding.ASCII.GetBytes("hello3"));

                            }

                            writeArgs.Priority = WritePriorities.HIGH;
                            writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                            retval = channel.Write(sendbuffer, writeArgs, out error);
                            if (retval < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                                retval = channel.ReleaseBuffer(sendbuffer, out error);
                                RemoveSession(channel);
                            }

                            sendbuffer = channel.GetBuffer(30, true, out error);

                            if (sendbuffer == null)
                            {
                                Console.WriteLine("GetBuffer(): Failed " + error.Text);
                            }
                            else
                            {
                                bb = sendbuffer.Data;
                                bb.Put(Encoding.ASCII.GetBytes("hello2"));

                            }

                            writeArgs.Priority = WritePriorities.HIGH;
                            writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                            retval = channel.Write(sendbuffer, writeArgs, out error);
                            if (retval < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                                retval = channel.ReleaseBuffer(sendbuffer, out error);
                                RemoveSession(channel);
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }

                case ETA_WRITE_RANDOM:
                    {
                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to send: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            try
                            {
                                maxBufferSize = int.Parse(GetInput("Enter random data buffer size you want: ")!);
                            }
                            catch (FormatException)
                            {
                                maxBufferSize = 0;
                            }
                            if (maxBufferSize > 0)
                            {
                                sendbuffer = channel.GetBuffer(maxBufferSize, true, out Error error);

                                for (int i = 0; i < maxBufferSize; i++)
                                {
                                    sendbuffer.Data.Write((byte)(random.Next() % 255));
                                }

                                if (encryptDecrypt)
                                {
                                    ITransportBuffer sendbuffer2 = channel.GetBuffer((int)m_EncDecHelper!.CalculateEncryptedSize(sendbuffer), false, out error);

                                    if (sendbuffer2 == null)
                                    {
                                        Console.WriteLine($"GetBuffer(): Failed {error.Text}");
                                    }
                                    else
                                    {

                                        if (m_EncDecHelper!.EncryptBuffer(channel, sendbuffer, sendbuffer2, out CodecError codecErr) < CodecReturnCode.SUCCESS)
                                            Console.WriteLine($"EncryptBuffer() Failed with code {codecErr.ErrorId} and text {codecErr.Text}");
                                        else
                                        {
                                            /* release original buffer */
                                            channel.ReleaseBuffer(sendbuffer, out error);
                                            sendbuffer = sendbuffer2;
                                        }
                                    }
                                }

                                writeArgs.Priority = WritePriorities.HIGH;
                                writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                                retval = channel.Write(sendbuffer, writeArgs, out error);
                                if (retval < TransportReturnCode.SUCCESS)
                                {
                                    Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                                    retval = channel.ReleaseBuffer(sendbuffer, out error);
                                    RemoveSession(channel);
                                }
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }

                        break;
                    }
                case ETA_WRITE_FULL_PACKED:
                    {
                        int packHeaderSize = 2;

                        try
                        {
                            sessId = (IntPtr)int.Parse(GetInput("Enter session ID to send: ")!);
                        }
                        catch (FormatException)
                        {
                            sessId = (IntPtr)0;
                        }
                        channel = Session(sessId);
                        if (channel != null)
                        {
                            int sizePerPack;
                            int bufNumber;

                            if (channel.Info(Info, out Error error) == TransportReturnCode.SUCCESS)
                            {
                                sendbuffer = channel.GetBuffer(Info.MaxFragmentSize, true, out error);

                                if (sendbuffer == null)
                                {
                                    Console.WriteLine("GetBuffer(): Failed " + error.Text);
                                }
                                else
                                {
                                    /*We want to pack 10 messages in here,
						             *so we take max size minus header over head for 9(because one is already reserved) */

                                    sizePerPack = (Info.MaxFragmentSize - (9 * packHeaderSize)) / 10;

                                    byte[]? data = null;

                                    /* now populate the buffer */
                                    for (bufNumber = 0; bufNumber < 10; bufNumber++)
                                    {
                                        if (bufNumber == 9)
                                        {
                                            // last data segment size
                                            sizePerPack += 4;
                                        }

                                        /* fill each buffer */
                                        if (bufNumber == 0 || bufNumber == 9)
                                        {
                                            if (data != null)
                                                data = null;

                                            data = new byte[sizePerPack];

                                            for (z = 0; z < sizePerPack; z++)
                                            {
                                                data[z] = (byte)z;
                                            }
                                        }
                                        sendbuffer.Data.Put(data);

                                        // pack data
                                        if (bufNumber != 9 && (channel.PackBuffer(sendbuffer, out error) < TransportReturnCode.SUCCESS))
                                        {
                                            Console.WriteLine("PackBuffer(): Failed " + error.Text);
                                            break;
                                        }
                                        if (sendbuffer.Length < sizePerPack && bufNumber < 9)
                                        {
                                            Console.WriteLine("\nERROR: buffer is full (bufNumber={0}, sendbuffer->length={1}, sizePerPack={2} <{3}>)\n\n",
                                                bufNumber, sendbuffer.Length, sizePerPack, error.Text);
                                            break;
                                        }

                                    }

                                    writeArgs.Priority = WritePriorities.HIGH;
                                    writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                                    retval = channel.Write(sendbuffer, writeArgs, out error);
                                    if (retval < TransportReturnCode.SUCCESS)
                                    {
                                        Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                                        retval = channel.ReleaseBuffer(sendbuffer, out error);
                                        RemoveSession(channel);
                                    }
                                }
                            }
                        }
                        else
                        {
                            Console.WriteLine("No Session with ID " + sessId);
                        }
                        break;
                    }
                case ETA_ENCRYPT_DECRYPT:
                    {
                        if (m_EncDecHelper == null)
                            m_EncDecHelper = new EncryptDecryptHelpers();
                        if (m_EncDecHelper != null)
                        {
                            if (encryptDecrypt == false)
                            {
                                encryptDecrypt = true;
                                Console.WriteLine("Encryption/Decryption enabled.");
                            }
                            else
                            {
                                encryptDecrypt = false;
                                Console.WriteLine("Encryption/Decryption disabled.");
                            }
                        }

                        break;
                    }
                case ETA_QUIT:
                    {
                        con = false;
                        retval = 0;
                        break;
                    }
                default:
                    {
                        Console.WriteLine("Illegal Command\n");
                        break;
                    }
            }

            return retval;
        }

        private void AddServer(IServer server)
        {
            if(serverList.ContainsKey(server.Socket.Handle))
            {
                serverList[server.Socket.Handle] = server;
            }
            else
            {
                serverList[server.Socket.Handle] = server;
            }

            Console.WriteLine($"Added Server with ID: {server.Socket.Handle}");
        }

        private void PopulateData(int dataFormat, ByteBuffer bb, int size)
        {
            int z;
            Debug.Assert(bb.Capacity - bb.Position >= size);

            if (dataFormat == 2) // uniform
            {
                for (z = 0; z < size; z++)
                {
                    bb.Write((byte)43);
                }
            }
            else if (dataFormat == 3) // random
            {
                for (int i = 0; i < size; i++)
                {
                    bb.Write((byte)(random.Next() % 255));
                }
            }
            else // 1 or default
            {
                for (z = 0; z < size; z++)
                {
                    bb.Write((byte)z);
                }
            }
        }

        private void MaxBufferPackTest(IChannel channel, IntPtr sessId)
        {
            WriteArgs writeArgs = new WriteArgs();
            TransportReturnCode retval = 0;
            int packedMessageSize = 100; // user payload bytes
            int msgSize;
            ChannelInfo info = new ChannelInfo();
            channel.Info(info, out Error error);
            ITransportBuffer msgBuffer;
            msgBuffer = channel.GetBuffer(info.MaxFragmentSize, true, out error);

            if (msgBuffer == null)
            {
                Console.WriteLine("getBuffer(): Failed " + error.Text);
                return;
            }

            int dataFormat = 1; // default: sequential data
            try
            {
                dataFormat = int.Parse(GetInput("data [1=sequential (default), 2=uniform, 3=random]: ")!);
            }
            catch (FormatException)
            {
                dataFormat = 1;
            }

            ByteBuffer dataBuf = new ByteBuffer(packedMessageSize * 2);
            int availableRemaining = msgBuffer.Data.Limit - msgBuffer.Data.Position;
            if (availableRemaining < packedMessageSize)
            {
                Console.WriteLine("Max buffer test can only be run on a connection with a minimum MaxFragmentSize of " + packedMessageSize);
                channel.ReleaseBuffer(msgBuffer, out error);
                return;
            }
            while (availableRemaining >= packedMessageSize)
            {
                // populate buffer with payload
                dataBuf.Clear();
                if (availableRemaining <= 2 * packedMessageSize)
                {
                    // used final packed message to fill remaining buffer space
                    Console.WriteLine($"final packet filling {availableRemaining} bytes");
                    msgSize = availableRemaining;
                }
                else
                    msgSize = packedMessageSize;

                int z;
                if (dataFormat == 2) // uniform
                {
                    for (z = 0; z < msgSize; z++)
                    {
                        msgBuffer.Data.Write((byte)43);
                    }
                }
                else if (dataFormat == 3) // random
                {
                    for (int i = 0; i < msgSize; i++)
                    {
                        msgBuffer.Data.Write((byte)(random.Next() % 255));
                    }
                }
                else // 1 or default
                {
                    for (z = 0; z < msgSize; z++)
                    {
                        msgBuffer.Data.Write(((byte)z));
                    }
                }

                Console.WriteLine($"Written length: {msgBuffer.Data.Position}");

                // pack
                availableRemaining = (int)channel.PackBuffer(msgBuffer, out error);
            }

            writeArgs.Priority = WritePriorities.HIGH;
            writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
            retval = channel.Write(msgBuffer, writeArgs, out error);
            if (retval < TransportReturnCode.SUCCESS)
            {
                if (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                {
                    while (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                    {
                        channel.Flush(out error);
                        retval = channel.Write(msgBuffer, writeArgs, out error);
                    }
                    if (retval > 0)
                        channel.Flush(out error);
                }
                if (retval < TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                    retval = channel.ReleaseBuffer(msgBuffer, out error);
                    RemoveSession(channel);
                }
                else
                    Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");
            }
            else
            {
                Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");
            }
        }

        private void StandardPackTest(IChannel channel, IntPtr sessId)
        {
            WriteArgs writeArgs = new WriteArgs();
            TransportReturnCode retval = 0;
            ITransportBuffer sendbuffer = channel.GetBuffer(500, true, out Error error);

            if (sendbuffer == null)
            {
                Console.WriteLine("GetBuffer(): Failed " + error.Text);
            }
            else
            {
                TransportReturnCode retVal;
                ByteBuffer bb = sendbuffer.Data;

                bb.Put(Encoding.ASCII.GetBytes("packed test #1"));
                retVal = channel.PackBuffer(sendbuffer, out error);
                Debug.Assert(retVal != 0);

                bb.Put(Encoding.ASCII.GetBytes("packed test #2"));
                retVal = channel.PackBuffer(sendbuffer, out error);
                Debug.Assert(retVal != 0);

                bb.Put(Encoding.ASCII.GetBytes("final packing test #3"));
                
                writeArgs.Priority = WritePriorities.HIGH;
                writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                retval = channel.Write(sendbuffer, writeArgs, out error);
                if (retval < TransportReturnCode.SUCCESS)
                {
                    if (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                    {
                        while (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                        {
                            channel.Flush(out error);
                            retval = channel.Write(sendbuffer, writeArgs, out error);
                        }
                        if (retval > 0)
                            channel.Flush(out error);

                    }
                    if (retval < TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                        retval = channel.ReleaseBuffer(sendbuffer, out error);
                        RemoveSession(channel);
                    }
                    else
                        Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");

                }
                else
                {
                    Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");

                    sendbuffer = channel.GetBuffer(500, true, out error);
                    bb = sendbuffer.Data;

                    bb.Put(Encoding.ASCII.GetBytes("packing2 test #1"));
                    retVal = channel.PackBuffer(sendbuffer, out error);
                    Debug.Assert(retVal != 0);

                    bb.Put(Encoding.ASCII.GetBytes("packing2 test #2"));
                    retVal = channel.PackBuffer(sendbuffer, out error);
                    Debug.Assert(retVal != 0);

                    bb.Put(Encoding.ASCII.GetBytes("packing3 final test #3"));
                    retVal = channel.PackBuffer(sendbuffer, out error);
                    Debug.Assert(retVal != 0);

                    writeArgs.Priority = WritePriorities.HIGH;
                    writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                    retval = channel.Write(sendbuffer, writeArgs, out error);
                    if (retval < TransportReturnCode.SUCCESS)
                    {
                        if (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                        {
                            while (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                            {
                                channel.Flush(out error);
                                retval = channel.Write(sendbuffer, writeArgs, out error);
                            }
                            if (retval > 0)
                                channel.Flush(out error);
                        }
                        if (retval < TransportReturnCode.SUCCESS)
                        {
                            Console.WriteLine("\nWrite() failed with code " + retval + " and text " + error.Text);
                            retval = channel.ReleaseBuffer(sendbuffer, out error);
                            RemoveSession(channel);
                        }
                        else
                            Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");
                    }
                    else
                    {
                        Console.WriteLine("\nWrite() succeeded, " + retval + " bytes pending to be flushed, " + writeArgs.BytesWritten + " bytes written, " + writeArgs.UncompressedBytesWritten + " uncomp bytes written");
                    }
                }
            }
        }

        private void AddSession(IChannel channel)
        {
            if (sessionList.ContainsKey(channel.Socket.Handle))
            {
                sessionList[channel.Socket.Handle] = channel;
            }
            else
            {
                sessionList[channel.Socket.Handle] = channel;
            }

            Console.WriteLine($"Added Session with ID: {channel.Socket.Handle}");
        }

        private void ReadFromSession(IChannel channel)
        {
            TransportReturnCode retval;
            ITransportBuffer buffer;
            ITransportBuffer? echoBuf;
            WriteArgs writeArgs = new WriteArgs();
            InProgInfo inProg = new InProgInfo();

            if (channel.State == ChannelState.INITIALIZING)
            {
                // if non-blocking channel, cancel selector interest for write and connect
                if (!channel.Blocking)
                {
                    m_WriteSockets.Remove(channel.Socket);
                }

                // perform channel initialization
                if ((retval = channel.Init(inProg, out Error error)) < TransportReturnCode.SUCCESS)
                {
                    Console.Write("\nsessionInactive " + channel.Socket.Handle + " <" + error.Text + ">");
                    RemoveSession(channel);
                }
                else
                {
                    switch (retval)
                    {
                        case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                            {
                                if (inProg.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                                {
                                    Console.WriteLine("\nSession In Progress - New Channel: " + inProg.NewSocket.Handle + " Old Channel: " + inProg.OldSocket.Handle);

                                    /* remove old channel read select */
                                    m_ReadSockets.Remove(inProg.OldSocket);

                                    if(channel.Blocking)
                                    {
                                        m_ReadSockets.Add(inProg.NewSocket);
                                    }
                                    else
                                    {
                                        m_ReadSockets.Add(inProg.NewSocket);
                                        m_WriteSockets.Add(inProg.NewSocket);
                                    }
                                }
                                else
                                {
                                    Console.WriteLine("\nSession " + channel.Socket.Handle + " In Progress...");
                                }

                                break;
                            }
                        case TransportReturnCode.SUCCESS:
                            Console.WriteLine("\nsessionAct " + channel.Socket.Handle);
                            break;

                        default:
                            Console.WriteLine("\nBad return value hashCode=" + channel.GetHashCode()+ "<" + error.Text + ">");
                            RemoveSession(channel);
                            break;
                    }
                }
            }
            else if (channel.State == ChannelState.CLOSED)
            {
                Console.WriteLine("Session " + channel.Socket.Handle + " Closed.");
                RemoveSession(channel);
            }
            else
            {
                do
                {
                    readArgs.Clear();
                    if ((buffer = channel.Read(readArgs, out Error error)) != null)
                    {
                        if (readArgs.ReadRetVal == TransportReturnCode.FAILURE)
                        {
                            Console.WriteLine("Read() failed with return code " + readArgs.ReadRetVal + "<" + error.Text + ">");
                            RemoveSession(channel);
                        }
                        if (buffer != null) // Data came in
                        {
                            Console.WriteLine("SessionId Id=" + channel.Socket.Handle + " ");
                            Console.WriteLine("Size " + buffer.Length);
                            Console.WriteLine("bytesRead=" + readArgs.BytesRead);
                            Console.WriteLine("uncompressedBytesRead=" + readArgs.UncompressedBytesRead);

                            if (readArgs.ReadRetVal != TransportReturnCode.READ_PING)
                            {

                                DisplayHexData(buffer);

                            }
                            if (encryptDecrypt == true)
                            {
                                Buffer output = new Buffer();
                                output.Data(new ByteBuffer(buffer.Data.WritePosition - buffer.Data.ReadPosition));
                                if (m_EncDecHelper!.DecryptBuffer(channel, buffer, output, out CodecError codecErr) < CodecReturnCode.SUCCESS)
                                {
                                    Console.WriteLine($"DecryptBuffer() Failed with code {codecErr.ErrorId} and text {codecErr.Text}");
                                }
                                else
                                {

                                    Console.WriteLine("Decrypted: ");
                                    DisplayHexData(output);
                                }
                            }

                            if (echo)
                            {
                                /* get buffer */
                                echoBuf = channel.GetBuffer((buffer.Data.Limit - buffer.Data.Position), false, out error);
                                if (echoBuf != null)
                                {

                                    echoBuf.Data.Put(buffer.Data);
                                    //echoBuf.setLength(buffer.getLength());

                                    /* echo data back */
                                    writeArgs.Priority = WritePriorities.MEDIUM;
                                    writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                                    retval = channel.Write(echoBuf, writeArgs, out error);
                                    /* flush this so it sends right away */
                                    if (retval > TransportReturnCode.FAILURE)
                                    {
                                        channel.Flush(out error);
                                    }
                                    else
                                    {
                                        channel.ReleaseBuffer(echoBuf, out error);
                                        echoBuf = null;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (readArgs.ReadRetVal == TransportReturnCode.FAILURE)
                        {
                            Console.WriteLine("\nsessionInactive " + channel.Socket.Handle + " <" + error.Text + ">");
                            RemoveSession(channel);
                        }
                        else if (readArgs.ReadRetVal == TransportReturnCode.READ_FD_CHANGE)
                        {
                            Console.WriteLine("\nRead() Channel Change - Old Channel: " + channel.OldSocket.Handle + " New Channel: " + channel.Socket.Handle);

                            /* remove old channel read select */
                            m_ReadSockets.Remove(channel.OldSocket);

                            if (channel.Blocking)
                            {
                                m_ReadSockets.Add(channel.Socket);
                            }
                            else
                            {
                                m_ReadSockets.Add(channel.Socket);
                                m_WriteSockets.Add(channel.Socket);
                            }
                        }
                        else
                        {
                            if (readArgs.ReadRetVal != TransportReturnCode.READ_WOULD_BLOCK)
                            {
                                Console.Write("\nreadret: " + readArgs.ReadRetVal);
                                Console.Write(" bytesRead=" + readArgs.BytesRead);
                                Console.WriteLine(" uncompressedBytesRead=" + readArgs.UncompressedBytesRead);
                            }
                        }
                    }
                } while (readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
            }
        }

        private static void DisplayHexData(ITransportBuffer transportBuffer)
        {
            int length = transportBuffer.Length;
            byte[] buffer = new byte[length];
            int readPosition = transportBuffer.Data.ReadPosition;
            transportBuffer.Data.ReadBytesInto(buffer, 0, length);
            transportBuffer.Data.ReadPosition = readPosition;

            int line_num = (length + HEX_LINE_SIZE - 1) / HEX_LINE_SIZE;

            int i = 0;
            int k;
            int bufByte;
            int line;

            for (line = 0; line < line_num && i < length; line++)
            {
                String tempStr = i.ToString("X").ToUpper();
                if (tempStr.Length == 1)
                {
                    Console.Write("000" + tempStr + ": ");
                }
                else if (tempStr.Length == 2)
                {
                    Console.Write("00" + tempStr + ": ");
                }
                else if (tempStr.Length == 3)
                {
                    Console.Write("0" + tempStr + ": ");
                }
                else
                {
                    Console.Write(tempStr + ": ");
                }

                k = i;
                bufByte = 0;

                for (; bufByte < HEX_LINE_SIZE && i < length; bufByte++)
                {
                    String tempStr2 = (buffer[i++] & 0xFF).ToString("X").ToUpper();
                    if (tempStr2.Length > 1)
                    {
                        Console.Write(tempStr2 + " ");
                    }
                    else
                    {
                        Console.Write("0" + tempStr2 + " ");
                    }

                    if ((bufByte + 1) % (HEX_LINE_SIZE / 2) == 0)
                    {
                        Console.Write(" ");
                    }
                }

                while (bufByte++ < HEX_LINE_SIZE)
                {
                    if (bufByte % (HEX_LINE_SIZE / 2) == 0)
                    {
                        Console.Write(" ");
                    }
                    Console.Write("   ");
                }

                Console.Write(" ");
                int copyLen = (HEX_LINE_SIZE / 2 < (length - k)) ? HEX_LINE_SIZE / 2 : (length - k);
                char[] printArray = new char[copyLen];
                for(int j = 0; j < copyLen; j++)
                {
                    printArray[j] = (char)buffer[k + j];
                }
                Console.Write(Regex.Replace(new String(printArray), @"[^0-9a-zA-Z~!@#$%^&*()_+=-|\\{}\"":; ',./?>< ]", "."));
                Console.Write(" ");
                k += copyLen;
                copyLen = (HEX_LINE_SIZE / 2 < (length - k)) ? HEX_LINE_SIZE / 2 : (length - k);
                if (copyLen > 0)
                {
                    char[] printArray2 = new char[copyLen];

                    for (int j = 0; j < copyLen; j++)
                    {
                        printArray2[j] = (char)buffer[k + j];
                    }
                    Console.Write(Regex.Replace(new String(printArray2), @"[^0-9a-zA-Z~!@#$%^&*()_+=-|\\{}\"":; ',./?>< ]", "."));
                }
                Console.WriteLine();
            }
        }

        private static void DisplayHexData(Buffer inputBuffer)
        {
            int length = inputBuffer.Length;
            byte[] buffer = new byte[length];
            int readPosition = inputBuffer.Data().ReadPosition;
            inputBuffer.Data().ReadBytesInto(buffer, 0, length);
            inputBuffer.Data().ReadPosition = readPosition;

            int line_num = (length + HEX_LINE_SIZE - 1) / HEX_LINE_SIZE;

            int i = 0;
            int k;
            int bufByte;
            int line;

            for (line = 0; line < line_num && i < length; line++)
            {
                String tempStr = i.ToString("X").ToUpper();
                if (tempStr.Length == 1)
                {
                    Console.Write("000" + tempStr + ": ");
                }
                else if (tempStr.Length == 2)
                {
                    Console.Write("00" + tempStr + ": ");
                }
                else if (tempStr.Length == 3)
                {
                    Console.Write("0" + tempStr + ": ");
                }
                else
                {
                    Console.Write(tempStr + ": ");
                }

                k = i;
                bufByte = 0;

                for (; bufByte < HEX_LINE_SIZE && i < length; bufByte++)
                {
                    String tempStr2 = (buffer[i++] & 0xFF).ToString("X").ToUpper();
                    if (tempStr2.Length > 1)
                    {
                        Console.Write(tempStr2 + " ");
                    }
                    else
                    {
                        Console.Write("0" + tempStr2 + " ");
                    }

                    if ((bufByte + 1) % (HEX_LINE_SIZE / 2) == 0)
                    {
                        Console.Write(" ");
                    }
                }

                while (bufByte++ < HEX_LINE_SIZE)
                {
                    if (bufByte % (HEX_LINE_SIZE / 2) == 0)
                    {
                        Console.Write(" ");
                    }
                    Console.Write("   ");
                }

                Console.Write(" ");
                int copyLen = (HEX_LINE_SIZE / 2 < (length - k)) ? HEX_LINE_SIZE / 2 : (length - k);
                char[] printArray = new char[copyLen];
                for (int j = 0; j < copyLen; j++)
                {
                    printArray[j] = (char)buffer[k + j];
                }
                Console.Write(Regex.Replace(new String(printArray), @"[^0-9a-zA-Z~!@#$%^&*()_+=-|\\{}\"":; ',./?>< ]", "."));
                Console.Write(" ");
                k += copyLen;
                copyLen = (HEX_LINE_SIZE / 2 < (length - k)) ? HEX_LINE_SIZE / 2 : (length - k);
                if (copyLen > 0)
                {
                    char[] printArray2 = new char[copyLen];

                    for (int j = 0; j < copyLen; j++)
                    {
                        printArray2[j] = (char)buffer[k + j];
                    }
                    Console.Write(Regex.Replace(new String(printArray2), @"[^0-9a-zA-Z~!@#$%^&*()_+=-|\\{}\"":; ',./?>< ]", "."));
                }
                Console.WriteLine();
            }
        }

        private IChannel? Session(IntPtr sessId)
        {
            if (sessionList.ContainsKey(sessId))
                return sessionList[sessId];
            else
                return null;
        }

        private void NewSession(IServer srvr)
        {
            IChannel channel;
            AcceptOptions acceptOpts = new AcceptOptions();

            acceptOpts.NakMount = false;

            if ((channel = srvr.Accept(acceptOpts, out Error error)) == null)
            {
                Console.WriteLine("Accept: failed <" + error.Text + ">");
            }
            else
            {
                AddSession(channel);
                Console.WriteLine("\nServer portno=" + srvr.PortNumber + ": New client " + channel.Socket.Handle + " newSession");
                /* add read to select */
                
                m_ReadSockets.Add(channel.Socket);
            }
        }

        private IServer? GetServer(IntPtr serverId)
        {
            if (serverList.ContainsKey(serverId))
                return serverList[serverId];
            else
                return null;
        }

        private static String? GetInput(String prompt)
        {
            String? retStr = null;
            Console.Write(prompt);

            try
            {
                while (retStr == null)
                {
                    retStr = Console.ReadLine();
                }
            }
            catch (Exception exp)
            {
                Console.WriteLine($"Got exception: {exp.Message}, Exiting...");
                Environment.Exit(-1);
            }

            return retStr;
        }

        private static bool FillAndValidateServerEncryptedOptions(BindOptions serverOptions)
        {

            BindEncryptionOptions encryptionOptions = serverOptions.BindEncryptionOpts;

            String? certificateFile;
            String? privateKeyFile;

            certificateFile = GetInput("Enter path to the server certificate file: ");

            if (String.IsNullOrEmpty(certificateFile))
            {
                Console.WriteLine("Encrypted connection cannot be established without certificate file.");
                return false;
            }

            privateKeyFile = GetInput("Enter path to the server private key file: ");

            encryptionOptions.ServerCertificate = certificateFile;
            encryptionOptions.ServerPrivateKey = privateKeyFile;
            encryptionOptions.EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_TLSV1_2;
            return true;
        }

        private static bool FillClientEncryptedOptions(ConnectOptions clientOptions)
        {
            EncryptionOptions encryptionOptions = clientOptions.EncryptionOpts;
            encryptionOptions.EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_TLSV1_2;
            encryptionOptions.EncryptedProtocol = ConnectionType.SOCKET;
            return true;
        }
    }
}
