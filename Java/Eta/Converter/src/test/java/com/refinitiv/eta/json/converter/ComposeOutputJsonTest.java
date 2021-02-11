package com.refinitiv.eta.json.converter;

import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.assertEquals;

public class ComposeOutputJsonTest {

    JsonConverterError error;

    @Before
    public void init() {
        error = ConverterFactory.createJsonConverterError();
    }

    @Test
    public void streamIdReplacementTest() {
        String message = "{\"ID\":5,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        String correct = "{\"ID\":25,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        byte[] bytes = new byte[message.length() + 5];
        for (int i = 0; i < message.length(); i++)
            bytes[i] = (byte)message.charAt(i);
        JsonBuffer buffer = new JsonBuffer();
        buffer.data = bytes;
        buffer.position = message.length();

        BufferHelper.replaceStreamId(buffer, 25, error);
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)buffer.data[i]);
        assertEquals(buffer.position, correct.length());
    }

    @Test
    public void streamIdReplacementTest1() {
        String message = "{\"ID\":5,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        String correct = "{\"ID\":2556,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        byte[] bytes = new byte[message.length() + 5];
        for (int i = 0; i < message.length(); i++)
            bytes[i] = (byte)message.charAt(i);
        JsonBuffer buffer = new JsonBuffer();
        buffer.data = bytes;
        buffer.position = message.length();

        BufferHelper.replaceStreamId(buffer, 2556, error);
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)buffer.data[i]);
        assertEquals(buffer.position, correct.length());
    }

    @Test
    public void streamIdReplacementTest2() {
        String message = "{\"ID\":5,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        String correct = "{\"ID\":1,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        byte[] bytes = new byte[message.length() + 5];
        for (int i = 0; i < message.length(); i++)
            bytes[i] = (byte)message.charAt(i);
        JsonBuffer buffer = new JsonBuffer();
        buffer.data = bytes;
        buffer.position = message.length();

        BufferHelper.replaceStreamId(buffer, 1, error);
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)buffer.data[i]);
        assertEquals(buffer.position, correct.length());
    }

    @Test
    public void streamIdReplacementTest3() {
        String message = "{\"ID\":5,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        String correct = "{\"ID\":-1,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        byte[] bytes = new byte[message.length() + 5];
        for (int i = 0; i < message.length(); i++)
            bytes[i] = (byte)message.charAt(i);
        JsonBuffer buffer = new JsonBuffer();
        buffer.data = bytes;
        buffer.position = message.length();

        BufferHelper.replaceStreamId(buffer, -1, error);
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)buffer.data[i]);
        assertEquals(buffer.position, correct.length());
    }

    @Test
    public void streamIdReplacementTest4() {
        String message = "{\"ID\":55,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        String correct = "{\"ID\":1,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        byte[] bytes = new byte[message.length() + 5];
        for (int i = 0; i < message.length(); i++)
            bytes[i] = (byte)message.charAt(i);
        JsonBuffer buffer = new JsonBuffer();
        buffer.data = bytes;
        buffer.position = message.length();

        BufferHelper.replaceStreamId(buffer, 1, error);
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)buffer.data[i]);
        assertEquals(buffer.position, correct.length());
    }

    @Test
    public void streamIdReplacementTest5() {
        String message = "{\"ID\":1098,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        String correct = "{\"ID\":465,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        byte[] bytes = new byte[message.length() + 5];
        for (int i = 0; i < message.length(); i++)
            bytes[i] = (byte)message.charAt(i);
        byte[] bytes1 = new byte[message.length() + 20];
        JsonBuffer buffer = new JsonBuffer();
        buffer.data = bytes;
        buffer.position = message.length();

        BufferHelper.composeMessage(bytes1, 465, buffer, error);
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)bytes1[i]);
        assertEquals(bytes1[correct.length()], 0);
    }

    @Test
    public void composeMessageTest() {
        String message = "{\"ID\":1098,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";
        String correct = "{\"ID\":11111,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Item Refresh Completed\"},\"ClearCache\":false,\"PartNumber\":10,\"Solicited\":false}";

        byte[] bytes = new byte[message.length() + 5];
        for (int i = 0; i < message.length(); i++)
            bytes[i] = (byte)message.charAt(i);
        ByteBuffer output = ByteBuffer.allocate(500);
        output.position(10);
        JsonBuffer buffer = new JsonBuffer();
        buffer.data = bytes;
        buffer.position = message.length();
        BufferHelper.composeMessage(output, 11111, buffer, error);
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)output.array()[i + 10]);
        assertEquals(output.array()[correct.length() + 10], 0);
    }
}
