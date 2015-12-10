package com.thomsonreuters.upa.examples.consumerperf;

/**
 * Represents an item that will be requested by the consumer
 */
class ItemImpl implements Item
{
    private final Domain _domain;
    private final String _name;
    
    public ItemImpl(Domain domain, String name)
    {
        if (name == null || name.isEmpty())
        {
            throw new IllegalArgumentException("Item name must be specified");
        }
        
        _domain = domain;
        _name = name;
    }
    
    @Override
    public Domain domain()
    {
        return _domain;
    }

    @Override
    public String name()
    {
        return _name;
    }
}
