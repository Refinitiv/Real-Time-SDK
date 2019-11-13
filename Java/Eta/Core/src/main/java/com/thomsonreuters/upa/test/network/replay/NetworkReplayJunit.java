///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.test.network.replay;

import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.NotYetConnectedException;
import java.util.Arrays;

import org.junit.Test;


public class NetworkReplayJunit
{
    /**
     *  a valid, full line (16 bytes) of hex
     */
    private static final byte[] BYTE_SEQ_FULL_LINE = { 77, 65, 71, 79,   76, 68, 49, 12,
                                                        49, 48, 46, 57,   49, 46, 49, 54 };    

    /**
     * a valid, full line (16 bytes) of hex
     */
    private static final String HEX_SEQ_FULL_LINE =
        "0000: 4D 41 47 4F 4C 44 31 0C  31 30 2E 39 31 2E 31 36   MAGOLD1. 10.91.16";
    
    /**
     * verifies that given a valid, single line of hex, the correct parsed byte array is returned
     */
    @Test
    public void parseValidSingleLine()
    {
        byte[] parsed = NetworkReplayParser.parse(HEX_SEQ_FULL_LINE);
        assertArrayEquals(BYTE_SEQ_FULL_LINE, parsed);
    }
   
    /**
     * verifies that given a valid, single line of hex containing leading and trailing whitespace,
     * the correct parsed byte array is returned
     */
    @Test
    public void parseValidSingleLineLeadingAndTrailingWhitespace()
    {
        byte[] parsed = NetworkReplayParser.parse("  \t  " + HEX_SEQ_FULL_LINE + " \n");
        assertArrayEquals(BYTE_SEQ_FULL_LINE, parsed);
    }
    
    /**
     * verifies that given a null String, an empty byte array is returned
     */
    @Test
    public void parseNullString()
    {
        byte[] parsed = NetworkReplayParser.parse(null);
        assertTrue(parsed != null && parsed.length == 0);
    }
    
    /**
     * verifies that given an empty String, an empty byte array is returned
     */
    @Test
    public void parseEmptyString()
    {
        byte[] parsed = NetworkReplayParser.parse("");
        assertTrue(parsed != null && parsed.length == 0);
    }
    
    /**
     * verifies that given valid a line containing a single byte,
     * the correct parsed byte array is returned
     */
    @Test
    public void parseValidSingleLineOneByte()
    {
        // a valid line containing just one byte of hex
        final byte[] byteSequenceOneByte = { 77 };
        final String hexSequenceOneByte =
                "0000: 4D                                                 MAGOLD1. 10.91.16";

        byte[] parsed = NetworkReplayParser.parse(hexSequenceOneByte);
        assertArrayEquals(byteSequenceOneByte, parsed);
    }
    
    /**
     * verifies that a trailing, (invalid) hex character is ignored 
     */
    @Test
    public void parseIgnoreTailingInvalidChar()
    {
        // this invalid sequence has a $ trailing the last byte of hex, which should be ignored
        final String hexSequenceInvalid1 =
                "0000: 4D 41 47 4F 4C 44 31 0C  31 30 2E 39 31 2E 31 36$   MAGOLD1. 10.91.16";

        byte[] parsed = NetworkReplayParser.parse(hexSequenceInvalid1);
        assertArrayEquals(BYTE_SEQ_FULL_LINE, parsed);
    }
    
    /**
     * verifies that given a completely invalid line of input,
     * an empty byte array is returned 
     */
    @Test
    public void parseInvalidNoHexLine()
    {
        // A line containing invalid input
        final String invalidInputNoHex = "this is NOT valid input!!!";

        byte[] parsed = NetworkReplayParser.parse(invalidInputNoHex);
        assertTrue(parsed != null && parsed.length == 0);
    }
    
    /**
     * verifies that given a line containing a mix of valid and invalid hex,
     * any empty byte array is returned 
     */
    @Test
    public void parseInvalidBadHexLine()
    {
        // A line containing invalid input
        final String invalidInputBadHex =
                "0000: 4D 41 47 4F 4Cthis is bad hex                      MAGOLD1. 10.91.16";

        byte[] parsed = NetworkReplayParser.parse(invalidInputBadHex);
        assertTrue(parsed != null && parsed.length == 0);
    }
    
    /**
     * Verifies that a call to {@code read()} returns {@code null} if no data is available
     */
    @Test
    public void readReturnsNull()
    {
        NetworkReplay foo = NetworkReplayFactory.create();
        assertArrayEquals(null, foo.read());
    }
    
    /**
     * Verifies that the same data enqueued is dequeued
     */
    @Test
    public void simpleEnqueueDequeue()
    {
        byte[] sequence = { 1, 2, 3 };

        NetworkReplay foo = NetworkReplayFactory.create();
        foo.enqueue(sequence);
        assertArrayEquals(sequence, foo.read());

        // the next call should return null, because there are no more records
        assertArrayEquals(null, foo.read());
    }
    
    /**
     * Parses a valid input file, and verifies the parsed data
     */
    @Test
    public void parseValidFile()
    {
        // the byte sequences we expect to read from the file:
        final byte[] seq1 = {(byte)0x00, (byte)0x13, (byte)0x01, (byte)0x01, (byte)0x0A, (byte)0x00, (byte)0x00, (byte)0x00, 
                             (byte)0x00, (byte)0x07, (byte)0x18, (byte)0x00, (byte)0x03, (byte)0x3C, (byte)0x0E, (byte)0x00, 
                             (byte)0x00, (byte)0x00, (byte)0xFF, };
        
        final byte[] seq2 = {(byte)0x01, (byte)0x81, (byte)0x02, (byte)0x01, (byte)0x7C, (byte)0x02, (byte)0x01, (byte)0x00, 
                             (byte)0x00, (byte)0x00, (byte)0x01, (byte)0x68, (byte)0x00, (byte)0x09, (byte)0x00, (byte)0x21, 
                             (byte)0x4C, (byte)0x6F, (byte)0x67, (byte)0x69, (byte)0x6E, (byte)0x20, (byte)0x61, (byte)0x63, 
                             (byte)0x63, (byte)0x65, (byte)0x70, (byte)0x74, (byte)0x65, (byte)0x64, (byte)0x20, (byte)0x62, 
                             (byte)0x79, (byte)0x20, (byte)0x68, (byte)0x6F, (byte)0x73, (byte)0x74, (byte)0x20, (byte)0x6F, 
                             (byte)0x61, (byte)0x6B, (byte)0x77, (byte)0x61, (byte)0x67, (byte)0x6F, (byte)0x6C, (byte)0x64, 
                             (byte)0x32, (byte)0x02, (byte)0x00, (byte)0x00, (byte)0x81, (byte)0x4B, (byte)0x26, (byte)0x10, 
                             (byte)0x61, (byte)0x6E, (byte)0x64, (byte)0x72, (byte)0x65, (byte)0x77, (byte)0x2E, (byte)0x67, 
                             (byte)0x6F, (byte)0x6C, (byte)0x64, (byte)0x73, (byte)0x74, (byte)0x65, (byte)0x69, (byte)0x6E, 
                             (byte)0x01, (byte)0x05, (byte)0x81, (byte)0x35, (byte)0x08, (byte)0x00, (byte)0x0D, (byte)0x0D, 
                             (byte)0x41, (byte)0x70, (byte)0x70, (byte)0x6C, (byte)0x69, (byte)0x63, (byte)0x61, (byte)0x74, 
                             (byte)0x69, (byte)0x6F, (byte)0x6E, (byte)0x49, (byte)0x64, (byte)0x11, (byte)0x03, (byte)0x32, 
                             (byte)0x35, (byte)0x36, (byte)0x0F, (byte)0x41, (byte)0x70, (byte)0x70, (byte)0x6C, (byte)0x69, 
                             (byte)0x63, (byte)0x61, (byte)0x74, (byte)0x69, (byte)0x6F, (byte)0x6E, (byte)0x4E, (byte)0x61, 
                             (byte)0x6D, (byte)0x65, (byte)0x11, (byte)0x0C, (byte)0x72, (byte)0x73, (byte)0x73, (byte)0x6C, 
                             (byte)0x50, (byte)0x72, (byte)0x6F, (byte)0x76, (byte)0x69, (byte)0x64, (byte)0x65, (byte)0x72, 
                             (byte)0x08, (byte)0x50, (byte)0x6F, (byte)0x73, (byte)0x69, (byte)0x74, (byte)0x69, (byte)0x6F, 
                             (byte)0x6E, (byte)0x11, (byte)0x10, (byte)0x31, (byte)0x30, (byte)0x2E, (byte)0x39, (byte)0x31, 
                             (byte)0x2E, (byte)0x31, (byte)0x36, (byte)0x31, (byte)0x2E, (byte)0x33, (byte)0x33, (byte)0x2F, 
                             (byte)0x6E, (byte)0x65, (byte)0x74, (byte)0x18, (byte)0x50, (byte)0x72, (byte)0x6F, (byte)0x76, 
                             (byte)0x69, (byte)0x64, (byte)0x65, (byte)0x50, (byte)0x65, (byte)0x72, (byte)0x6D, (byte)0x69, 
                             (byte)0x73, (byte)0x73, (byte)0x69, (byte)0x6F, (byte)0x6E, (byte)0x50, (byte)0x72, (byte)0x6F, 
                             (byte)0x66, (byte)0x69, (byte)0x6C, (byte)0x65, (byte)0x04, (byte)0x01, (byte)0x01, (byte)0x1C, 
                             (byte)0x50, (byte)0x72, (byte)0x6F, (byte)0x76, (byte)0x69, (byte)0x64, (byte)0x65, (byte)0x50, 
                             (byte)0x65, (byte)0x72, (byte)0x6D, (byte)0x69, (byte)0x73, (byte)0x73, (byte)0x69, (byte)0x6F, 
                             (byte)0x6E, (byte)0x45, (byte)0x78, (byte)0x70, (byte)0x72, (byte)0x65, (byte)0x73, (byte)0x73, 
                             (byte)0x69, (byte)0x6F, (byte)0x6E, (byte)0x73, (byte)0x04, (byte)0x01, (byte)0x01, (byte)0x0A, 
                             (byte)0x53, (byte)0x69, (byte)0x6E, (byte)0x67, (byte)0x6C, (byte)0x65, (byte)0x4F, (byte)0x70, 
                             (byte)0x65, (byte)0x6E, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x10, (byte)0x41, (byte)0x6C, 
                             (byte)0x6C, (byte)0x6F, (byte)0x77, (byte)0x53, (byte)0x75, (byte)0x73, (byte)0x70, (byte)0x65, 
                             (byte)0x63, (byte)0x74, (byte)0x44, (byte)0x61, (byte)0x74, (byte)0x61, (byte)0x04, (byte)0x01, 
                             (byte)0x01, (byte)0x12, (byte)0x53, (byte)0x75, (byte)0x70, (byte)0x70, (byte)0x6F, (byte)0x72, 
                             (byte)0x74, (byte)0x50, (byte)0x61, (byte)0x75, (byte)0x73, (byte)0x65, (byte)0x52, (byte)0x65, 
                             (byte)0x73, (byte)0x75, (byte)0x6D, (byte)0x65, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x1B, 
                             (byte)0x53, (byte)0x75, (byte)0x70, (byte)0x70, (byte)0x6F, (byte)0x72, (byte)0x74, (byte)0x4F, 
                             (byte)0x70, (byte)0x74, (byte)0x69, (byte)0x6D, (byte)0x69, (byte)0x7A, (byte)0x65, (byte)0x64, 
                             (byte)0x50, (byte)0x61, (byte)0x75, (byte)0x73, (byte)0x65, (byte)0x52, (byte)0x65, (byte)0x73, 
                             (byte)0x75, (byte)0x6D, (byte)0x65, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x0E, (byte)0x53, 
                             (byte)0x75, (byte)0x70, (byte)0x70, (byte)0x6F, (byte)0x72, (byte)0x74, (byte)0x4F, (byte)0x4D, 
                             (byte)0x4D, (byte)0x50, (byte)0x6F, (byte)0x73, (byte)0x74, (byte)0x04, (byte)0x01, (byte)0x00, 
                             (byte)0x13, (byte)0x53, (byte)0x75, (byte)0x70, (byte)0x70, (byte)0x6F, (byte)0x72, (byte)0x74, 
                             (byte)0x56, (byte)0x69, (byte)0x65, (byte)0x77, (byte)0x52, (byte)0x65, (byte)0x71, (byte)0x75, 
                             (byte)0x65, (byte)0x73, (byte)0x74, (byte)0x73, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x14, 
                             (byte)0x53, (byte)0x75, (byte)0x70, (byte)0x70, (byte)0x6F, (byte)0x72, (byte)0x74, (byte)0x42, 
                             (byte)0x61, (byte)0x74, (byte)0x63, (byte)0x68, (byte)0x52, (byte)0x65, (byte)0x71, (byte)0x75, 
                             (byte)0x65, (byte)0x73, (byte)0x74, (byte)0x73, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x0E, 
                             (byte)0x53, (byte)0x75, (byte)0x70, (byte)0x70, (byte)0x6F, (byte)0x72, (byte)0x74, (byte)0x53, 
                             (byte)0x74, (byte)0x61, (byte)0x6E, (byte)0x64, (byte)0x62, (byte)0x79, (byte)0x04, (byte)0x01, 
                             (byte)0x00 };
        final byte[] seq3 = {(byte)0x02, (byte)0x51, (byte)0x02, (byte)0x00, (byte)0x38, (byte)0x02, (byte)0x04, (byte)0x00, 
                             (byte)0x00, (byte)0x00, (byte)0x02, (byte)0x81, (byte)0x68, (byte)0x09, (byte)0x09, (byte)0x00, 
                             (byte)0x22, (byte)0x53, (byte)0x6F, (byte)0x75, (byte)0x72, (byte)0x63, (byte)0x65, (byte)0x20, 
                             (byte)0x44, (byte)0x69, (byte)0x72, (byte)0x65, (byte)0x63, (byte)0x74, (byte)0x6F, (byte)0x72, 
                             (byte)0x79, (byte)0x20, (byte)0x52, (byte)0x65, (byte)0x66, (byte)0x72, (byte)0x65, (byte)0x73, 
                             (byte)0x68, (byte)0x20, (byte)0x43, (byte)0x6F, (byte)0x6D, (byte)0x70, (byte)0x6C, (byte)0x65, 
                             (byte)0x74, (byte)0x65, (byte)0x64, (byte)0x02, (byte)0x00, (byte)0x00, (byte)0x80, (byte)0x05, 
                             (byte)0x08, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x3F, (byte)0x00, (byte)0x04, (byte)0x07, 
                             (byte)0x00, (byte)0x01, (byte)0x02, (byte)0x01, (byte)0x01, (byte)0xFE, (byte)0x02, (byte)0x09, 
                             (byte)0x00, (byte)0x05, (byte)0x06, (byte)0x02, (byte)0x01, (byte)0xFE, (byte)0x01, (byte)0x0B, 
                             (byte)0x08, (byte)0x00, (byte)0x0B, (byte)0x04, (byte)0x4E, (byte)0x61, (byte)0x6D, (byte)0x65, 
                             (byte)0x11, (byte)0x0B, (byte)0x44, (byte)0x49, (byte)0x52, (byte)0x45, (byte)0x43, (byte)0x54, 
                             (byte)0x5F, (byte)0x46, (byte)0x45, (byte)0x45, (byte)0x44, (byte)0x06, (byte)0x56, (byte)0x65, 
                             (byte)0x6E, (byte)0x64, (byte)0x6F, (byte)0x72, (byte)0x11, (byte)0x0F, (byte)0x54, (byte)0x68, 
                             (byte)0x6F, (byte)0x6D, (byte)0x73, (byte)0x6F, (byte)0x6E, (byte)0x20, (byte)0x52, (byte)0x65, 
                             (byte)0x75, (byte)0x74, (byte)0x65, (byte)0x72, (byte)0x73, (byte)0x08, (byte)0x49, (byte)0x73, 
                             (byte)0x53, (byte)0x6F, (byte)0x75, (byte)0x72, (byte)0x63, (byte)0x65, (byte)0x04, (byte)0x01, 
                             (byte)0x01, (byte)0x0C, (byte)0x43, (byte)0x61, (byte)0x70, (byte)0x61, (byte)0x62, (byte)0x69, 
                             (byte)0x6C, (byte)0x69, (byte)0x74, (byte)0x69, (byte)0x65, (byte)0x73, (byte)0x0F, (byte)0xFE, 
                             (byte)0x00, (byte)0x07, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x03, (byte)0x05, (byte)0x06, 
                             (byte)0x07, (byte)0x14, (byte)0x44, (byte)0x69, (byte)0x63, (byte)0x74, (byte)0x69, (byte)0x6F, 
                             (byte)0x6E, (byte)0x61, (byte)0x72, (byte)0x69, (byte)0x65, (byte)0x73, (byte)0x50, (byte)0x72, 
                             (byte)0x6F, (byte)0x76, (byte)0x69, (byte)0x64, (byte)0x65, (byte)0x64, (byte)0x0F, (byte)0xFE, 
                             (byte)0x00, (byte)0x13, (byte)0x11, (byte)0x00, (byte)0x00, (byte)0x02, (byte)0x06, (byte)0x52, 
                             (byte)0x57, (byte)0x46, (byte)0x46, (byte)0x6C, (byte)0x64, (byte)0x07, (byte)0x52, (byte)0x57, 
                             (byte)0x46, (byte)0x45, (byte)0x6E, (byte)0x75, (byte)0x6D, (byte)0x10, (byte)0x44, (byte)0x69, 
                             (byte)0x63, (byte)0x74, (byte)0x69, (byte)0x6F, (byte)0x6E, (byte)0x61, (byte)0x72, (byte)0x69, 
                             (byte)0x65, (byte)0x73, (byte)0x55, (byte)0x73, (byte)0x65, (byte)0x64, (byte)0x0F, (byte)0xFE, 
                             (byte)0x00, (byte)0x13, (byte)0x11, (byte)0x00, (byte)0x00, (byte)0x02, (byte)0x06, (byte)0x52, 
                             (byte)0x57, (byte)0x46, (byte)0x46, (byte)0x6C, (byte)0x64, (byte)0x07, (byte)0x52, (byte)0x57, 
                             (byte)0x46, (byte)0x45, (byte)0x6E, (byte)0x75, (byte)0x6D, (byte)0x03, (byte)0x51, (byte)0x6F, 
                             (byte)0x53, (byte)0x0F, (byte)0xFE, (byte)0x00, (byte)0x06, (byte)0x0C, (byte)0x00, (byte)0x00, 
                             (byte)0x01, (byte)0x01, (byte)0x22, (byte)0x10, (byte)0x53, (byte)0x75, (byte)0x70, (byte)0x70, 
                             (byte)0x6F, (byte)0x72, (byte)0x74, (byte)0x73, (byte)0x51, (byte)0x6F, (byte)0x53, (byte)0x52, 
                             (byte)0x61, (byte)0x6E, (byte)0x67, (byte)0x65, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x08, 
                             (byte)0x49, (byte)0x74, (byte)0x65, (byte)0x6D, (byte)0x4C, (byte)0x69, (byte)0x73, (byte)0x74, 
                             (byte)0x11, (byte)0x00, (byte)0x1A, (byte)0x53, (byte)0x75, (byte)0x70, (byte)0x70, (byte)0x6F, 
                             (byte)0x72, (byte)0x74, (byte)0x73, (byte)0x4F, (byte)0x75, (byte)0x74, (byte)0x4F, (byte)0x66, 
                             (byte)0x42, (byte)0x61, (byte)0x6E, (byte)0x64, (byte)0x53, (byte)0x6E, (byte)0x61, (byte)0x70, 
                             (byte)0x73, (byte)0x68, (byte)0x6F, (byte)0x74, (byte)0x73, (byte)0x04, (byte)0x01, (byte)0x00, 
                             (byte)0x17, (byte)0x41, (byte)0x63, (byte)0x63, (byte)0x65, (byte)0x70, (byte)0x74, (byte)0x69, 
                             (byte)0x6E, (byte)0x67, (byte)0x43, (byte)0x6F, (byte)0x6E, (byte)0x73, (byte)0x75, (byte)0x6D, 
                             (byte)0x65, (byte)0x72, (byte)0x53, (byte)0x74, (byte)0x61, (byte)0x74, (byte)0x75, (byte)0x73, 
                             (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x02, (byte)0x02, (byte)0xFE, (byte)0x00, (byte)0x36, 
                             (byte)0x08, (byte)0x00, (byte)0x03, (byte)0x0C, (byte)0x53, (byte)0x65, (byte)0x72, (byte)0x76, 
                             (byte)0x69, (byte)0x63, (byte)0x65, (byte)0x53, (byte)0x74, (byte)0x61, (byte)0x74, (byte)0x65, 
                             (byte)0x04, (byte)0x01, (byte)0x01, (byte)0x11, (byte)0x41, (byte)0x63, (byte)0x63, (byte)0x65, 
                             (byte)0x70, (byte)0x74, (byte)0x69, (byte)0x6E, (byte)0x67, (byte)0x52, (byte)0x65, (byte)0x71, 
                             (byte)0x75, (byte)0x65, (byte)0x73, (byte)0x74, (byte)0x73, (byte)0x04, (byte)0x01, (byte)0x01, 
                             (byte)0x06, (byte)0x53, (byte)0x74, (byte)0x61, (byte)0x74, (byte)0x75, (byte)0x73, (byte)0x0D, 
                             (byte)0x05, (byte)0x09, (byte)0x00, (byte)0x02, (byte)0x4F, (byte)0x4B, (byte)0x02, (byte)0x03, 
                             (byte)0xFE, (byte)0x00, (byte)0x11, (byte)0x08, (byte)0x00, (byte)0x01, (byte)0x06, (byte)0x53, 
                             (byte)0x74, (byte)0x61, (byte)0x74, (byte)0x75, (byte)0x73, (byte)0x0D, (byte)0x05, (byte)0x09, 
                             (byte)0x00, (byte)0x02, (byte)0x4F, (byte)0x4B, (byte)0x02, (byte)0x04, (byte)0xFE, (byte)0x00, 
                             (byte)0x2C, (byte)0x08, (byte)0x00, (byte)0x03, (byte)0x09, (byte)0x4F, (byte)0x70, (byte)0x65, 
                             (byte)0x6E, (byte)0x4C, (byte)0x69, (byte)0x6D, (byte)0x69, (byte)0x74, (byte)0x04, (byte)0x01, 
                             (byte)0x05, (byte)0x0A, (byte)0x4F, (byte)0x70, (byte)0x65, (byte)0x6E, (byte)0x57, (byte)0x69, 
                             (byte)0x6E, (byte)0x64, (byte)0x6F, (byte)0x77, (byte)0x04, (byte)0x01, (byte)0x01, (byte)0x0A, 
                             (byte)0x4C, (byte)0x6F, (byte)0x61, (byte)0x64, (byte)0x46, (byte)0x61, (byte)0x63, (byte)0x74, 
                             (byte)0x6F, (byte)0x72, (byte)0x04, (byte)0x01, (byte)0x01, (byte)0x02, (byte)0x05, (byte)0xFE, 
                             (byte)0x00, (byte)0x13, (byte)0x08, (byte)0x00, (byte)0x02, (byte)0x04, (byte)0x54, (byte)0x79, 
                             (byte)0x70, (byte)0x65, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x04, (byte)0x44, (byte)0x61, 
                             (byte)0x74, (byte)0x61, (byte)0x04, (byte)0x01, (byte)0x00, (byte)0x22, (byte)0x06, (byte)0x09, 
                             (byte)0xFE, (byte)0x00, (byte)0x56, (byte)0x00, (byte)0x11, (byte)0x05, (byte)0x00, (byte)0x01, 
                             (byte)0x02, (byte)0x11, (byte)0x72, (byte)0x73, (byte)0x73, (byte)0x6C, (byte)0x50, (byte)0x72, 
                             (byte)0x6F, (byte)0x76, (byte)0x69, (byte)0x64, (byte)0x65, (byte)0x72, (byte)0x20, (byte)0x6C, 
                             (byte)0x69, (byte)0x6E, (byte)0x6B, (byte)0xFE, (byte)0x00, (byte)0x3B, (byte)0x08, (byte)0x00, 
                             (byte)0x04, (byte)0x04, (byte)0x54, (byte)0x79, (byte)0x70, (byte)0x65, (byte)0x04, (byte)0x01, 
                             (byte)0x01, (byte)0x09, (byte)0x4C, (byte)0x69, (byte)0x6E, (byte)0x6B, (byte)0x53, (byte)0x74, 
                             (byte)0x61, (byte)0x74, (byte)0x65, (byte)0x04, (byte)0x01, (byte)0x01, (byte)0x08, (byte)0x4C, 
                             (byte)0x69, (byte)0x6E, (byte)0x6B, (byte)0x43, (byte)0x6F, (byte)0x64, (byte)0x65, (byte)0x04, 
                             (byte)0x01, (byte)0x01, (byte)0x04, (byte)0x54, (byte)0x65, (byte)0x78, (byte)0x74, (byte)0x11, 
                             (byte)0x10, (byte)0x4C, (byte)0x69, (byte)0x6E, (byte)0x6B, (byte)0x20, (byte)0x73, (byte)0x74, 
                             (byte)0x61, (byte)0x74, (byte)0x65, (byte)0x20, (byte)0x69, (byte)0x73, (byte)0x20, (byte)0x75, 
                             (byte)0x70};
        final byte[] seq4 = {(byte)0x00, (byte)0x69, (byte)0x02, (byte)0x00, (byte)0x2F, (byte)0x02, (byte)0x06, (byte)0x00, 
                             (byte)0x00, (byte)0x00, (byte)0x06, (byte)0x80, (byte)0xE8, (byte)0x04, (byte)0x09, (byte)0x00, 
                             (byte)0x16, (byte)0x49, (byte)0x74, (byte)0x65, (byte)0x6D, (byte)0x20, (byte)0x52, (byte)0x65, 
                             (byte)0x66, (byte)0x72, (byte)0x65, (byte)0x73, (byte)0x68, (byte)0x20, (byte)0x43, (byte)0x6F, 
                             (byte)0x6D, (byte)0x70, (byte)0x6C, (byte)0x65, (byte)0x74, (byte)0x65, (byte)0x64, (byte)0x02, 
                             (byte)0x00, (byte)0x00, (byte)0x22, (byte)0x80, (byte)0x07, (byte)0x07, (byte)0x01, (byte)0x03, 
                             (byte)0x54, (byte)0x52, (byte)0x49, (byte)0x01, (byte)0x08, (byte)0x00, (byte)0x09, (byte)0x00, 
                             (byte)0x02, (byte)0x01, (byte)0x64, (byte)0x00, (byte)0x04, (byte)0x01, (byte)0x9B, (byte)0x00, 
                             (byte)0x26, (byte)0x04, (byte)0x16, (byte)0x0A, (byte)0x07, (byte)0xDA, (byte)0x00, (byte)0x06, 
                             (byte)0x02, (byte)0x0C, (byte)0x64, (byte)0x00, (byte)0x16, (byte)0x02, (byte)0x0C, (byte)0x63, 
                             (byte)0x00, (byte)0x19, (byte)0x02, (byte)0x0C, (byte)0x67, (byte)0x00, (byte)0x20, (byte)0x05, 
                             (byte)0x0C, (byte)0x00, (byte)0x98, (byte)0x96, (byte)0x80, (byte)0x00, (byte)0x0B, (byte)0x03, 
                             (byte)0x0C, (byte)0x00, (byte)0xD7, (byte)0x01, (byte)0x0B, (byte)0x03, (byte)0x0B, (byte)0x26, 
                             (byte)0x19};
        final byte[] seq5 = {(byte)0x00, (byte)0x34, (byte)0x02, (byte)0x00, (byte)0x09, (byte)0x04, (byte)0x06, (byte)0x00, 
                             (byte)0x00, (byte)0x00, (byte)0x06, (byte)0x00, (byte)0x04, (byte)0x00, (byte)0x08, (byte)0x00, 
                             (byte)0x06, (byte)0x00, (byte)0x06, (byte)0x02, (byte)0x0C, (byte)0x65, (byte)0x00, (byte)0x16, 
                             (byte)0x02, (byte)0x0C, (byte)0x64, (byte)0x00, (byte)0x19, (byte)0x02, (byte)0x0C, (byte)0x68, 
                             (byte)0x00, (byte)0x20, (byte)0x05, (byte)0x0C, (byte)0x00, (byte)0x98, (byte)0x96, (byte)0x80, 
                             (byte)0x00, (byte)0x0B, (byte)0x03, (byte)0x0C, (byte)0x00, (byte)0xD7, (byte)0x01, (byte)0x0B, 
                             (byte)0x03, (byte)0x0B, (byte)0x26, (byte)0x1A};          
        
        NetworkReplay fileReplay = NetworkReplayFactory.create();
        try
        {
            fileReplay.parseFile("../../../Java/Eta/TestTools/UnitTests/TestData/NetworkReplay/sample_capture.txt");
            assertTrue(fileReplay.recordsInQueue() == 5);

            assertArrayEquals(seq1, fileReplay.read());
            assertArrayEquals(seq2, fileReplay.read());
            assertArrayEquals(seq3, fileReplay.read());
            assertArrayEquals(seq4, fileReplay.read());
            assertArrayEquals(seq5, fileReplay.read());

            // the next call to read should return null, because there are no more records
            assertArrayEquals(null, fileReplay.read());
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
    }
    
    /**
     * Verifies an exception is thrown if an invalid path is provided
     */
    @Test
    public void parseFileInvalidPath()
    {
        boolean exceptionThrown = false;

        try
        {
            NetworkReplay fileReplay = NetworkReplayFactory.create();
            fileReplay.parseFile("This/Path/Is/Invalid.txt");
        }
        catch (IOException e)
        {
            exceptionThrown = true;
        }

        assertTrue(exceptionThrown);
    }
    
    /**
     * Verifies that calling {code read(ByteBuffer dst)} with no data returns {code 0}, and the ByteBuffer is not altered. 
     */
    @Test
    public void readNoData()
    {
        final int capacity = 100;

        try
        {
            NetworkReplay replay = NetworkReplayFactory.create();
            ByteBuffer buf = ByteBuffer.allocate(capacity);

            int bytesRead = replay.read(buf);
            assertTrue(bytesRead == 0);
            assertTrue(buf.position() == 0);
        }
        catch (NotYetConnectedException e)
        {
            assertTrue(false);
        }
        catch (IOException e)
        {
            assertTrue(false);
        }
    }

    /**
     * Read data into a buffer of the same size
     */
    @Test
    public void readDataWithSameSizedBuffer()
    {
        final int capacity = 10;

        final byte[] tenBytes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        try
        {
            NetworkReplay replay = NetworkReplayFactory.create();
            ByteBuffer buf = ByteBuffer.allocate(capacity);

            replay.enqueue(tenBytes);
            int bytesRead = replay.read(buf);

            assertTrue(bytesRead == 10);
            assertArrayEquals(tenBytes, buf.array());
        }
        catch (NotYetConnectedException e)
        {
            assertTrue(false);
        }
        catch (IOException e)
        {
            assertTrue(false);
        }
    }
    
    /**
     * Read data into a buffer with a capacity larger than the data
     */
    @Test
    public void readDataWithLargerBuffer()
    {
        final int capacity = 11;
        final int expectedBytesRead = 10;

        final byte[] tenBytes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        try
        {
            NetworkReplay replay = NetworkReplayFactory.create();
            ByteBuffer buf = ByteBuffer.allocate(capacity);

            replay.enqueue(tenBytes);
            int bytesRead = replay.read(buf);

            assertTrue(bytesRead == expectedBytesRead);
            assertArrayEquals(tenBytes, Arrays.copyOfRange(buf.array(), 0, expectedBytesRead));
        }
        catch (NotYetConnectedException e)
        {
            assertTrue(false);
        }
        catch (IOException e)
        {
            assertTrue(false);
        }
    }
    
    /**
     * Read data into a buffer with a capacity smaller than the data
     */
    @Test
    public void readDataWithSmallerBuffer()
    {
        final int capacity = 6;

        final byte[] tenBytes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        try
        {
            NetworkReplay replay = NetworkReplayFactory.create();
            ByteBuffer buf = ByteBuffer.allocate(capacity);

            replay.enqueue(tenBytes);

            // the first call should completely fill the buffer
            int bytesRead = replay.read(buf);
            assertTrue(bytesRead == capacity);
            assertArrayEquals(Arrays.copyOfRange(tenBytes, 0, capacity), buf.array());

            // clear the buffer
            buf.clear();

            // the second call should return the remainder of the buffer
            bytesRead = replay.read(buf);
            int expectedSecondReadLen = tenBytes.length - capacity;
            assertTrue(bytesRead == expectedSecondReadLen);
            assertArrayEquals(Arrays.copyOfRange(tenBytes, capacity, tenBytes.length),
                              Arrays.copyOfRange(buf.array(), 0, expectedSecondReadLen));

            // clear the buffer
            buf.clear();

            // the third call should return no data
            bytesRead = replay.read(buf);
            assertTrue(bytesRead == 0);
        }
        catch (NotYetConnectedException e)
        {
            assertTrue(false);
        }
        catch (IOException e)
        {
            assertTrue(false);
        }
    }
    
    /**
     * Verifies the forced read failure feature
     */
    @Test
    public void verifyForcedReadFailure()
    {
        final NetworkReplay replay = NetworkReplayFactory.create();
        final int expectedCallsToRead = 3;
        final int capacity = 1234;
        int readAttempts = 0;

        final String expectedErrorMsg = "the next call to read() will cause an IOException to be thrown";

        try
        {
            ByteBuffer buf = ByteBuffer.allocate(capacity);

            // the first attempt to read should succeed (though there is no data)
            ++readAttempts;
            assertEquals(0, replay.read(buf));

            // force the next call to read to fail
            replay.forceNextReadToFail(expectedErrorMsg);

            try
            {
                ++readAttempts;
                replay.read(buf); // this should throw an exception
                assertTrue(false); // control should never reach here
            }
            catch (IOException e)
            {
                assertTrue(expectedErrorMsg.equals(e.getMessage()));
            }

            // this last call should succeed
            ++readAttempts;
            assertEquals(0, replay.read(buf));
        }
        catch (IOException e)
        {
            assertTrue(false); // control should never reach here
        }

        assertEquals(expectedCallsToRead, readAttempts);
    }
    
    /**
     * Verifies the forced end of stream feature
     */
    @Test
    public void verifyForcedEndOfStream()
    {
        final NetworkReplay replay = NetworkReplayFactory.create();
        final int expectedCallsToRead = 3;
        final int capacity = 1234;
        int readAttempts = 0;

        try
        {
            ByteBuffer buf = ByteBuffer.allocate(capacity);

            // the first attempt to read should succeed (though there is no data)
            ++readAttempts;
            assertEquals(0, replay.read(buf));

            // force the next call to read to return end of stream (-1)
            replay.forceEndOfStream();

            ++readAttempts;
            assertEquals(-1, replay.read(buf));

            // this last call should also return -1
            ++readAttempts;
            assertEquals(-1, replay.read(buf));
        }
        catch (IOException e)
        {
            assertTrue(false); // control should never reach here
        }

        assertEquals(expectedCallsToRead, readAttempts);
    }
    
}
