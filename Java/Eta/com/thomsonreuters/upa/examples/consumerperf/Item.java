package com.thomsonreuters.upa.examples.consumerperf;

/**
 * Represents an item that will be requested by the consumer
 */
public interface Item
{
    /**
     * The domain of the item (e.g. MarketPrice)
     * 
     * @return The domain of the item (e.g. MarketPrice)
     */
    Domain domain();
    
    /**
     * The name of the item (e.g. TRI.N)
     * 
     * @return The name of the item (e.g. TRI.N)
     */
    String name();
}
