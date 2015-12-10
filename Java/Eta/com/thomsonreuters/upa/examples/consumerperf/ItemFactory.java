package com.thomsonreuters.upa.examples.consumerperf;

public class ItemFactory
{
    private ItemFactory()
    {
        throw new UnsupportedOperationException();
    }
    
    /**
     * Creates a new {@link Item}
     * 
     * @param domain The {@link Domain} of the item (e.g. MarketPrice)
     * @param name The name of the item, which may not be {@code null} or empty
     * 
     * @return A new  {@link Item}
     */
    public static Item createItem(Domain domain, String name)
    {
        return new ItemImpl(domain, name);
    }
    
    /**
     * Creates a new {@link ItemFileParser}
     * 
     * @return a new {@link ItemFileParser}
     */
    public static ItemFileParser createItemFileParser()
    {
        return new ItemFileParserImpl();
    }
}
