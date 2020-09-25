///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import static org.junit.Assert.assertEquals;

import java.nio.BufferOverflowException;

import org.junit.Test;

import com.refinitiv.eta.codec.Codec;

public class MulticastSocketChannelJunit
{
    
    @Test
    public void basicReadWriteTest()
    {
        System.out.println("\nbasicReadWriteTest");
		Error error = TransportFactory.createError();
		ReadArgs readArgs = TransportFactory.createReadArgs();
		int port = 30000;
		String interfaceName = "";
		String group = "235.1.1.1";
		Channel channelSession = null;
		TransportBuffer readIn = null;
		TransportBuffer toSend = new TransportBufferImpl(3000);
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		ConnectOptions copts = TransportFactory.createConnectOptions();
		
		try
		{
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			toSend = channelSession.getBuffer(500, true, error);
			
			toSend.data().put("1234".getBytes());
			
			channelSession.write(toSend, writeArgs, error);
			System.out.println("Wrote " + writeArgs.bytesWritten() + " bytes.");
			
			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Read in " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs.bytesRead());
		}
		finally
		{
            if (channelSession != null)
                channelSession.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
        
    }
    

    
    @Test
    public void packingOverrunTest()
    {
        System.out.println("\npackingOverrunTest");
		
		Error error = TransportFactory.createError();
		ReadArgs readArgs = TransportFactory.createReadArgs();
		int port = 30000;
		String interfaceName = "";
		String group = "235.1.1.1";
		Channel channelSession = null;
		TransportBuffer readIn = null;
		TransportBuffer toSend = new TransportBufferImpl(3000);
		WriteArgs writeArgs = TransportFactory.createWriteArgs();
		int maxMessageSize = 10;
        
		try
		{
			ConnectOptions copts = TransportFactory.createConnectOptions();
			
			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			toSend = channelSession.getBuffer(maxMessageSize, true, error);
			
			toSend.data().put("1234".getBytes());
			
			channelSession.packBuffer(toSend, error);
			
			try
			{
				System.out.println("Attempting to overflow buffer...");
				toSend.data().put("567890123456789".getBytes());
			}
			catch (BufferOverflowException e)
			{
				System.out.println("Buffer overflow exception occured");
				System.out.println("Buffer position: " + toSend.data().position() + " Buffer size: " + toSend.data().limit());
			}
			
			channelSession.write(toSend, writeArgs, error);
			System.out.println("Wrote " + writeArgs.bytesWritten() + " bytes.");
			
			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Read in " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs.bytesRead());
		}
		finally
		{
            if (channelSession != null)
                channelSession.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
        
    }
    
    @Test
    public void emptyMessageTest()
    {
        System.out.println("\nemptyMessageTest");
        
        Error error = TransportFactory.createError();
        int port = 30000;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;
        TransportBuffer toSend = new TransportBufferImpl(3000);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        int maxMessageSize = 10;
     
        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();
			
			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			toSend = channelSession.getBuffer(maxMessageSize, true, error);
			
			assert (TransportReturnCodes.FAILURE == channelSession.write(toSend, writeArgs, error));
			
			System.out.println("Nothing sent because buffer was not written into before write()");
        }
		finally
		{
            if (channelSession != null)
                channelSession.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
        
    }
    
    @Test
    public void multipleConnectionsOnSameAddressTest()
    {
        System.out.println("\nmultipleConnectionsOnSameAddressTest");
        
        Error error = TransportFactory.createError();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        int port = 30000;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;
		Channel channelSession2 = null;
        TransportBuffer readIn = null;
        TransportBuffer toSend = new TransportBufferImpl(3000);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();
			
			ReadArgs readArgs2 = TransportFactory.createReadArgs();
			TransportBuffer readIn2 = null;

			ConnectOptions copts2 = TransportFactory.createConnectOptions();
			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			copts2 = copts;
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			if ( (channelSession2 = Transport.connect(copts2, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			
			toSend = channelSession.getBuffer(500, true, error);
			
			toSend.data().put("1234".getBytes());
			
			channelSession.write(toSend, writeArgs, error);
			System.out.println("Wrote " + writeArgs.bytesWritten() + " bytes to " + group  + ":" + port + " .");
			
			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Read in (1) on address " + group  + ":" + port+ " is " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs.bytesRead());
			
			readIn2 = channelSession2.read(readArgs2, error);
			readIn2.data().position(0);
			System.out.println("Read in (2) on address " + group  + ":" + port+ " is " + readArgs2.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs2.bytesRead());

        }
		finally
		{
            if (channelSession != null)
                channelSession.close(error);
			if (channelSession2 != null)
                channelSession2.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
        
        
    }
    
    @Test
    public void multipleConnectionsOnDifferentAddressTest()
    {
        System.out.println("\nmultipleConnectionsOnDifferentAddressTest");
        
        Error error = TransportFactory.createError();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        int port = 30000;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;
		Channel channelSession2 = null;
        TransportBuffer readIn = null;
        TransportBuffer toSend = new TransportBufferImpl(3000);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();
			
			ReadArgs readArgs2 = TransportFactory.createReadArgs();

			TransportBuffer readIn2 = null;
			String group2 = "235.1.1.2";

			ConnectOptions copts2 = TransportFactory.createConnectOptions();
			
			
			
			
			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			copts2.guaranteedOutputBuffers(500);
			copts2.majorVersion(Codec.majorVersion());
			copts2.minorVersion(Codec.minorVersion());
			copts2.protocolType(Codec.protocolType());

			copts2.unifiedNetworkInfo().address(group2);
			copts2.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts2.unifiedNetworkInfo().interfaceName(interfaceName);
			copts2.seqMCastOpts().maxMsgSize(3000);
			copts2.blocking(false);
			copts2.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}

			
			if ( (channelSession2 = Transport.connect(copts2, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			
			toSend = channelSession.getBuffer(500, true, error);
			
			toSend.data().put("1234".getBytes());
			
			channelSession.write(toSend, writeArgs, error);
			System.out.println("Wrote " + writeArgs.bytesWritten() + " bytes to " + group  + ":" + port + " .");
			
			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Read in (1) on address " + group  + ":" + port+ " is " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs.bytesRead());
			
			readIn2 = channelSession2.read(readArgs2, error);
			if (readIn2 != null)
				readIn2.data().position(0);
			System.out.println("Read in (2) on address " + group2 + ":" + port + " is " + readArgs2.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() != readArgs2.bytesRead());
		}
		finally
        {
            if (channelSession != null)
                channelSession.close(error);
			if (channelSession2 != null)
                channelSession2.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    @Test
    public void multipleConnectionsOnDifferentPortTest()
    {
        System.out.println("\nmultipleConnectionsOnDifferentPortTest");
        
        Error error = TransportFactory.createError();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        int port = 30000;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;
		Channel channelSession2 = null;
        TransportBuffer readIn = null;
        TransportBuffer toSend = new TransportBufferImpl(3000);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();
			
			ReadArgs readArgs2 = TransportFactory.createReadArgs();
			TransportBuffer readIn2 = null;
			int port2 = 30001;

			ConnectOptions copts2 = TransportFactory.createConnectOptions();
			
			
			
			
			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			copts2.guaranteedOutputBuffers(500);
			copts2.majorVersion(Codec.majorVersion());
			copts2.minorVersion(Codec.minorVersion());
			copts2.protocolType(Codec.protocolType());

			copts2.unifiedNetworkInfo().address(group);
			copts2.unifiedNetworkInfo().serviceName(String.valueOf(port2));
			copts2.unifiedNetworkInfo().interfaceName(interfaceName);
			copts2.seqMCastOpts().maxMsgSize(3000);
			copts2.blocking(false);
			copts2.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}

			
			if ( (channelSession2 = Transport.connect(copts2, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			
			toSend = channelSession.getBuffer(500, true, error);
			
			toSend.data().put("1234".getBytes());
			
			channelSession.write(toSend, writeArgs, error);
			System.out.println("Wrote " + writeArgs.bytesWritten() + " bytes to " + group  + ":" + port + " .");
			
			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Read in (1) on address " + group  + ":" + port+ " is " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs.bytesRead());
			
			readIn2 = channelSession2.read(readArgs2, error);
			if (readIn2 != null)
				readIn2.data().position(0);
			System.out.println("Read in (2) on address " + group + ":" + port2 + " is " + readArgs2.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() != readArgs2.bytesRead());
		}
        finally
        {
            if (channelSession != null)
                channelSession.close(error);
			if (channelSession2 != null)
                channelSession2.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    
    @Test
    public void twoWritersOneReader()
    {
        System.out.println("\ntwoWritersOneReader");
        
        Error error = TransportFactory.createError();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        int port = 30000;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;
		Channel channelSession2 = null;
        TransportBuffer readIn = null;
        TransportBuffer toSend = new TransportBufferImpl(3000);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();
			TransportBuffer toSend2 = new TransportBufferImpl(3000);
			WriteArgs writeArgs2 = TransportFactory.createWriteArgs();
			
			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}

			
			if ( (channelSession2 = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			toSend = channelSession.getBuffer(500, true, error);
			
			toSend.data().put("1234".getBytes());
			
			toSend2 = channelSession2.getBuffer(500, true, error);
			
			toSend2.data().put("1234".getBytes());
			
			channelSession.write(toSend, writeArgs, error);
			System.out.println("Writer 1 wrote  " + writeArgs.bytesWritten() + " bytes.");
			
			channelSession2.write(toSend2, writeArgs2, error);
			System.out.println("Writer 2 wrote  " + writeArgs2.bytesWritten() + " bytes.");
			
			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Reader read " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs.bytesRead());
			
			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Reader read " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs2.bytesWritten() == readArgs.bytesRead());
		}
        finally
        {
            if (channelSession != null)
                channelSession.close(error);
			if (channelSession2 != null)
                channelSession2.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    @Test
    public void oneWriterTwoReaders()
    {
        System.out.println("\noneWriterTwoReaders");
        
        Error error = TransportFactory.createError();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        int port = 30000;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;
        Channel channelSession2 = null;
		TransportBuffer readIn = null;
        TransportBuffer toSend = new TransportBufferImpl(3000);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();
			
			ReadArgs readArgs2 = TransportFactory.createReadArgs();
			TransportBuffer readIn2 = null;

			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}

			
			if ( (channelSession2 = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			toSend = channelSession.getBuffer(500, true, error);
			
			toSend.data().put("1234".getBytes());
			
			channelSession.write(toSend, writeArgs, error);
			System.out.println("Writer wrote " + writeArgs.bytesWritten() + " bytes.");

			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Reader 1 read " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs.bytesRead());
			
			readIn2 = channelSession2.read(readArgs2, error);
			readIn2.data().position(0);
			System.out.println("Reader 2 read " + readArgs2.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs2.bytesRead());

		}
        finally
        {
            if (channelSession != null)
                channelSession.close(error);
			if (channelSession2 != null)
                channelSession2.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    @Test
    public void invalidBufferTest()
    {
       System.out.println("\ninvalidBufferTest");
        
        Error error = TransportFactory.createError();
        int port = 30000;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;
        TransportBuffer toSend = new TransportBufferImpl(3000);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
        ConnectOptions copts = TransportFactory.createConnectOptions();

        
        copts.majorVersion(Codec.majorVersion());
        copts.minorVersion(Codec.minorVersion());
        copts.protocolType(Codec.protocolType());
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(false);
        Transport.initialize(initArgs, error);
        
        copts.unifiedNetworkInfo().address(group);
        copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
        copts.unifiedNetworkInfo().interfaceName(interfaceName);
        copts.seqMCastOpts().maxMsgSize(3000);
        copts.blocking(true);
        copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
        
        if ( (channelSession = Transport.connect(copts, error)) == null)
        {
            System.out.println("Connection failure: " + error.text());
        }

        toSend = channelSession.getBuffer(5000, true, error);
        
        assert(toSend == null);
        assert(error.errorId() == TransportReturnCodes.FAILURE);
        
        channelSession.write(toSend, writeArgs, error);
        
        assert(error.errorId() == TransportReturnCodes.FAILURE);
        assert(writeArgs.bytesWritten() == 0);
        System.out.println("Writer wrote " + writeArgs.bytesWritten() + " bytes.");
        
        channelSession.close(error);
    }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    @Test
    public void invalidServiceNameTest()
    {
       System.out.println("\ninvalidServiceNameTest");
        
        Error error = TransportFactory.createError();
        int port = 0;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;

        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();

			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure with port " + port + ": " + error.text());
			}

			assert(error.errorId() == TransportReturnCodes.FAILURE);
			assert(channelSession == null);

			port = 65536;
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));

			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure with port " + port + ": " + error.text());
			}

			assert(error.errorId() == TransportReturnCodes.FAILURE);
			assert(channelSession == null);
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }        
    }
    
    @Test
    public void invalidAddressTest()
    {
       System.out.println("\ninvalidAddressTest");
        
        Error error = TransportFactory.createError();
        int port = 30000;
        String interfaceName = "";
        String group = "192.0.0.1";
        Channel channelSession = null;

        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();

			
			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.unifiedNetworkInfo().address(group);
			copts.unifiedNetworkInfo().serviceName(String.valueOf(port));
			copts.unifiedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure with address " + group + ": " + error.text());
			}

			assert(error.errorId() == TransportReturnCodes.FAILURE);
			assert(channelSession == null);
		   
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure with address " + group + ": " + error.text());
			}

			assert(error.errorId() == TransportReturnCodes.FAILURE);
			assert(channelSession == null);
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
        
    }
    
    @Test
    public void segmentedNetworkReadWriteTest()
    {
        System.out.println("\nsegmentedNetworkReadWriteTest");
        
        
        Error error = TransportFactory.createError();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        int port = 30000;
        String interfaceName = "";
        String group = "235.1.1.1";
        Channel channelSession = null;
		Channel channelSession2 = null;
        TransportBuffer readIn = null;
        TransportBuffer toSend = new TransportBufferImpl(3000);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
			ConnectOptions copts = TransportFactory.createConnectOptions();

			ReadArgs readArgs2 = TransportFactory.createReadArgs();
			TransportBuffer readIn2 = null;
			String group2 = "235.1.1.2";

			ConnectOptions copts2 = TransportFactory.createConnectOptions();

			copts.majorVersion(Codec.majorVersion());
			copts.minorVersion(Codec.minorVersion());
			copts.protocolType(Codec.protocolType());
			InitArgs initArgs = TransportFactory.createInitArgs();
			initArgs.globalLocking(false);
			Transport.initialize(initArgs, error);
			
			copts.segmentedNetworkInfo().sendAddress(group);
			copts.segmentedNetworkInfo().recvAddress(group);
			copts.segmentedNetworkInfo().recvServiceName(String.valueOf(port));
			copts.segmentedNetworkInfo().sendServiceName(String.valueOf(port));
			copts.segmentedNetworkInfo().interfaceName(interfaceName);
			copts.seqMCastOpts().maxMsgSize(3000);
			copts.blocking(true);
			copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			copts2.guaranteedOutputBuffers(500);
			copts2.majorVersion(Codec.majorVersion());
			copts2.minorVersion(Codec.minorVersion());
			copts2.protocolType(Codec.protocolType());

			copts2.segmentedNetworkInfo().recvAddress(group);
			copts2.segmentedNetworkInfo().recvServiceName(String.valueOf(port));
			copts2.segmentedNetworkInfo().interfaceName(interfaceName);
			copts2.seqMCastOpts().maxMsgSize(3000);
			copts2.blocking(false);
			copts2.connectionType(ConnectionTypes.SEQUENCED_MCAST);
			
			if ( (channelSession = Transport.connect(copts, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}

			
			if ( (channelSession2 = Transport.connect(copts2, error)) == null)
			{
				System.out.println("Connection failure: " + error.text());
			}
			
			
			toSend = channelSession.getBuffer(500, true, error);
			
			toSend.data().put("1234".getBytes());
			
			channelSession.write(toSend, writeArgs, error);
			System.out.println("Wrote " + writeArgs.bytesWritten() + " bytes to " + group  + ":" + port + " .");
			
			readIn = channelSession.read(readArgs, error);
			readIn.data().position(0);
			System.out.println("Read in (1) on address " + group  + ":" + port+ " is " + readArgs.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs.bytesRead());
			
			readIn2 = channelSession2.read(readArgs2, error);
			if (readIn2 != null)
				readIn2.data().position(0);
			System.out.println("Read in (2) on address " + group2 + ":" + port + " is " + readArgs2.bytesRead() + " bytes.");
			
			assert(writeArgs.bytesWritten() == readArgs2.bytesRead());

		}
        finally
        {
            if (channelSession != null)
                channelSession.close(error);
			if (channelSession2 != null)
                channelSession2.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    

}
