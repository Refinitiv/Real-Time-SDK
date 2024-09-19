///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.codec;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.refinitiv.eta.transport.ByteRoutines;

public class ParseHexFile
{

    /**
     * Parse and extract hex data from a file.
     * <p>
     * The format of the file is one or more lines of data. Each line has at
     * least one byte up two 20 bytes. The bytes are arranged in pairs of up to
     * 10 pairs, with spaces between each pair. Blank lines and lines that start
     * with '#' will be ignored.
     * <p>
     * Example file format:
     * <br># a comment line that will be ignored.
     * <br>4920 5374 7269 6e67 2074 6573 7406 4e61 6d65 3137 &nbsp;&nbsp;&nbsp;I String test.Name17
     * <br>0b07 0f09 07ae 0508 05 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;.........
     * <p>For this example, a byte[] of length 29 would be returned.
     * @param fileName
     * @return byte[] containing the bytes parsed, or null on parsing error.
     */
    public static byte[] parse(String fileName)
    {
        // hold all byte arrays returned from parseHexDataFromString
        List<byte[]> byteArray = new ArrayList<byte[]>();
        
        try
        {
            FileInputStream fstream = new FileInputStream(fileName);
            DataInputStream in = new DataInputStream(fstream);
            BufferedReader br = new BufferedReader(new InputStreamReader(in));
            
            String line;

            //Read File Line By Line
            while ((line = br.readLine()) != null)
            {
                //System.out.println(line);
                
                if(line.length() == 0 || line.startsWith("#"))
                		continue;
                
                byte[] parsed = parseString(line);
                if (parsed != null && parsed.length > 0)
                {
                    byteArray.add(parsed);
                }
            }
            
            in.close();
        }
        catch (Exception e)
        {
            System.out.println("Error parsing data file: " + e.getMessage());
            return null;
        }
        
        if (byteArray.size() == 0)
        {
            System.out.println("Error parsing data file: no hex data found");
            return null;
        }
        
        // determine total number of bytes.
        int totalBytes = 0;
        for(byte[] b : byteArray)
        {
            totalBytes += b.length;
        }
        
        // copy 
        int allIdx = 0;
        byte[] allBytes = new byte[totalBytes];
        for(byte[] b : byteArray)
        {
            for(int idx = 0; idx < b.length; idx++)
                allBytes[allIdx++] = b[idx];
        }
        
        return allBytes;
    }
    
    private static byte[] parseString(String singleLine)
    {
        final Pattern HEX_FORMAT = Pattern.compile("(.{1,50})"); 
        
        //String s = "0b07 0f09 07ae 0508 05                               .........";
        byte[] converted = null;
        
        Matcher matcher = HEX_FORMAT.matcher(singleLine);
        
        if (matcher.find() && matcher.groupCount() >= 1)
        {
            String justHex = matcher.group(1);
            converted = ByteRoutines.fromHexString(justHex.getBytes(), 0, justHex.length());
        }
        
        return converted;
    }
}
