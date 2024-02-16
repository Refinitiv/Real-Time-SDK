/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.common;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.transport.ConnectionTypes;

/** Connection arguments parser for the Value Add consumer and
 * non-interactive application command line.
 */
public class ConnectionArgsParser
{
	final int ERROR_RETURN_CODE = -1;
	List<ConnectionArg> connectionList = new ArrayList<ConnectionArg>();
	
	/**
	 * Returns true if argOffset is start of connection arguments.
	 * 
	 * @param args array of command line arguments
     * @param argOffset offset into array of command line arguments
     * 
     * @return true if argOffset is start of connection arguments and false otherwise
	 */
	public boolean isStart(String[] args, int argOffset)
	{
		if ("-c".equals(args[argOffset]) ||
			"-tcp".equals(args[argOffset]) ||
			"-segmentedMulticast".equals(args[argOffset]))
		{
			return true;
		}
		
		return false;
	}
	
	/**
	 * Parses connection arguments.
	 * 
     * @param args array of command line arguments
     * @param argOffset offset into array of command line arguments
     * 
	 * @return argument offset after connection arguments or -1 if error
	 */
	public int parse(String[] args, int argOffset)
	{
		int offset = 0;
		
		if ("-c".equals(args[argOffset]) ||
			"-tcp".equals(args[argOffset]))
		{
			offset = parseSocketConnectionArgs(args, argOffset);
		}
		else if ("-segmentedMulticast".equals(args[argOffset]))
		{
			offset = parseMulticastConnectionArgs(args, argOffset);
		}
		else
		{
			offset = ERROR_RETURN_CODE;
		}
		
		return offset;
	}
	
	// parses TCP socket connection arguments
	private int parseSocketConnectionArgs(String[] args, int argOffset)
	{
		int retCode = ERROR_RETURN_CODE;
		ConnectionArg connectionArg = null;
		
		if ((args.length-1) >= argOffset+3)
		{
			if (args[argOffset+1].contains(":") && !args[argOffset+2].contains(":"))
			{
				String[] tokens = args[argOffset+1].split(":");
				connectionArg = new ConnectionArg();
				connectionArg.connectionType = ConnectionTypes.SOCKET;
				if (!args[argOffset+2].startsWith("-"))
				{
					connectionArg.service = args[argOffset+2];
					retCode = argOffset + 3;
				}
				else
				{
					retCode = argOffset + 2;
				}
				if (tokens.length == 2)
				{
					connectionArg.hostname = tokens[0];
					connectionArg.port = tokens[1];
					connectionList.add(connectionArg);
				}
				else {
					connectionArg.hostname = "";
					connectionArg.port = "";
					connectionList.add(connectionArg);
				}
			}
			else
			{
				return retCode;
			}
		}
		else
		{
			return retCode;
		}
			
		// parse item arguments for this connection
		ItemArgsParser itemArgsParser = new ItemArgsParser();
		if (itemArgsParser.isStart(args, retCode))
		{
			retCode = itemArgsParser.parse(args, retCode);
			connectionArg.itemList = itemArgsParser.itemList();
		}
        else if (!args[retCode].equals("-tsServiceName") && !args[retCode].equals("-tunnel") && !args[retCode].equals("-tsAuth") && !args[retCode].equals("-tsDomain"))
        {
            retCode = ERROR_RETURN_CODE;
        }
		
		int argsCount = retCode;
		while (argsCount < args.length && !args[argsCount].equals("-c"))
		{
            if (args[argsCount].equals("-tsServiceName"))
            {
                connectionArg.tsService = args[argsCount + 1];
                retCode += 2;
            }
            if (args[argsCount].equals("-tunnel"))
            {
                connectionArg.tunnel = true;
                retCode += 1;           
            }
            if (args[argsCount].equals("-tsAuth"))
            {
                connectionArg.tunnelAuth = true;
                retCode += 1;           
            }
            if (args[argsCount].equals("-tsDomain"))
            {
                connectionArg.tunnelDomain = Integer.valueOf(args[argsCount + 1]);
                retCode += 2;           
            }
            if (argsCount < retCode)
            {
                argsCount = retCode;
            }
            else
            {
                argsCount++;
            }
		}
		
		if (connectionArg.tsService == null)
		{
		    connectionArg.tsService = connectionArg.service;
		}
		
		return retCode;
	}

	// parses reliable multicast connection arguments
	private int parseMulticastConnectionArgs(String[] args, int argOffset)
	{
		int retCode = ERROR_RETURN_CODE;
		ConnectionArg connectionArg = null;
		
		if (args[argOffset+1].contains(":") && args[argOffset+2].contains(":") &&
			!args[argOffset+3].contains(":") && !args[argOffset+4].contains(":"))
		{
			String[] sendAddrTokens = args[argOffset+1].split(":");
			if (sendAddrTokens.length == 2 || sendAddrTokens.length == 3)
			{
				connectionArg = new ConnectionArg();
				connectionArg.connectionType = ConnectionTypes.RELIABLE_MCAST;
				connectionArg.sendAddress = sendAddrTokens[0];
				connectionArg.sendPort = sendAddrTokens[1];
				if (sendAddrTokens.length == 3)
				{
					connectionArg.interfaceName = sendAddrTokens[2];
				}
			}
			else
			{
				return retCode;
			}
			String[] recvAddrTokens = args[argOffset+2].split(":");
			if (recvAddrTokens.length == 2)
			{
				connectionArg.recvAddress = recvAddrTokens[0];
				connectionArg.recvPort = recvAddrTokens[1];
				connectionArg.unicastPort = args[argOffset+3];
				connectionArg.service = args[argOffset+4];
				connectionList.add(connectionArg);
				retCode = argOffset + 5;
			}
			else
			{
				return retCode;
			}
		}
		else
		{
			return retCode;
		}
			
		// parse item arguments for this connection
		ItemArgsParser itemArgsParser = new ItemArgsParser();
				
		connectionArg.itemList = itemArgsParser.itemList();
		
		if (itemArgsParser.isStart(args, retCode))
		{
			retCode = itemArgsParser.parse(args, retCode);		
		}
		else
		{
			retCode = ERROR_RETURN_CODE;
		}
		
		return retCode;
	}

	/**
	 * Returns list of parsed connection arguments.
	 *
	 * @return the list
	 */
	public List<ConnectionArg> connectionList()
	{
		return connectionList;
	}
}
