package com.thomsonreuters.upa.examples.consumerperf;

import java.util.List;

public interface ItemFileParser
{
    /**
     * Parses an item file, and returns {@code true} if the file was successfully parsed.
     * 
     * @param filename The full path to the file to parse
     * @param items (out) This list will be populated with (request) items
     * @param latencyItems (out) This list will be populated with latency items
     * 
     * @return {@code true} if the file was successfully parsed
     */
    public boolean parse(String filename, List<Item> items, List<Item> latencyItems);
}