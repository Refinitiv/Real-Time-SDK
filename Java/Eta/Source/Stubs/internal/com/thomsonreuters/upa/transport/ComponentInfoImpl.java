package com.thomsonreuters.upa.transport;


import com.thomsonreuters.upa.codec.Buffer;


class ComponentInfoImpl implements ComponentInfo
{

      
    @Override
    public Buffer componentVersion()
    {
        return null;
    }
    
    /**
     * Create a clone of the ComponentInfo. This is a deep copy where new memory
     * is created for all members.
     * 
     * @return a new ComponentInfo object.
     */
    @Override
    protected ComponentInfo clone()
    {
        return null;
    }
    
}
