/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.common;

/**
 * Interface for Value Add application command line parsing.
 */
public interface CommandLineParser
{
    /**
     * Parses command line arguments for application.
     * 
     * @param args array of command line arguments
     * 
     * @return true, if arguments parse correctly or false, otherwise 
     */
	public boolean parseArgs(String[] args);
	
    /**
     * Prints usage for application command line.
     */
    public void printUsage();
}
