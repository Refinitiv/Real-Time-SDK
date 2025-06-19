/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.test.network.replay.util;

import java.io.IOException;
import java.io.PrintWriter;
import java.nio.ByteBuffer;

import com.refinitiv.eta.test.network.replay.NetworkReplay;
import com.refinitiv.eta.test.network.replay.NetworkReplayFactory;
import com.refinitiv.eta.transport.Transport;

/**
 * This simple utility program reads all the hex dump data in a NetworkReplay
 * file, merges it into a single hex dump, and writes the hex dump to the
 * specified output file. The merged output file can be used with a diff utility
 * to compare data captured across runs, or to compare the data sent by a
 * provider with the data sent by a consumer.
 */
class MergeNetworkReplayFile
{
    private static void showUsage()
    {
        System.out.println("\nUsage:");
        System.out.println("<inputFile> <outputFile>");
        System.out.println("\nThis simple utility program reads all the hex dump data in a NetworkReplay");
        System.out.println("file, merges it into a single hex dump, and writes the hex dump to the");
        System.out.println("specified output file. The merged output file can be used with a diff");
        System.out.println("utility to compare data captured across runs, or to compare the data sent");
        System.out.println("by a provider with the data sent by a consumer.");
    }

    /**
     * @param args
     */
    public static void main(String[] args)
    {
        final String EXITING = "Exiting.";
        
        if (args.length != 2)
        {
            System.out.println("Error: invalid number of arguments");
            showUsage();
            System.out.println(EXITING);
            System.exit(-1);
        }

        final String inputFile = args[0];
        final String outputFile = args[1];

        if (inputFile.equals(outputFile))
        {
            System.out.println("Error: the input file and output file must be different\n" + EXITING);
            System.exit(-1);
        }

        try
        {
            mergeFile(inputFile, outputFile);
        }
        catch (IOException e)
        {
            System.out.println("Error merging file: " + e.getMessage());
        }
        finally
        {
            System.out.println(EXITING);
        }
    }

    private static void mergeFile(final String inputFile, final String outputFile)
            throws IOException
    {
        System.out.println("reading input file \"" + inputFile + "\"...");
        byte[] merged = readAllData(inputFile);
        System.out.println("...finished reading " + merged.length + " bytes.");

        System.out.println("writing output file \"" + outputFile + "\"...");
        ByteBuffer temp = ByteBuffer.wrap(merged);

        PrintWriter provOut = new PrintWriter(outputFile);
        provOut.write(Transport.toHexString(temp, 0, temp.limit()));
        provOut.close();
        System.out.println("...finished writing output file.");
    }

    /**
     * Reads all the hex data in the specified replay file and returns it as a
     * {@code byte[]} array
     * 
     * @param replayFilename The replay filename
     * @return A {@code byte[]} array containing all the data read from the
     *         replay file
     * 
     * @throws IOException
     */
    private static byte[] readAllData(String replayFilename) throws IOException
    {
        final NetworkReplay replay = NetworkReplayFactory.create();
        final int totalBytes = replay.parseFile(replayFilename);
        final int recordCount = replay.recordsInQueue();

        byte[] allData = new byte[totalBytes];
        int allDataPos = 0;
        
        for (int i = 0; i < recordCount; i++)
        {
            final byte[] data = replay.read();
            if (data != null)
            {
                for (byte b : data)
                {
                    allData[allDataPos++] = b;
                }
            }
        }

        return allData;
    }

}
