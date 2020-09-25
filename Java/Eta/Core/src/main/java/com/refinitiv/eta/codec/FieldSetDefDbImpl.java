package com.refinitiv.eta.codec;

class FieldSetDefDbImpl implements FieldSetDefDb
{
    /* Maximum local message scope set identifier */
    int MAX_LOCAL_ID;
    
    /* Blank set identifier */
    final static int BLANK_ID = 65536;

    FieldSetDefImpl[]     _definitions;

    int maxSetId = 0;     /* Maximum set definition index */
   
    int maxLocalId = MAX_LOCAL_ID;
    
    FieldSetDefDbImpl(int maxID)
    {
        MAX_LOCAL_ID = maxID;
        _definitions = new FieldSetDefImpl[MAX_LOCAL_ID + 1];
        for (int i = 0; i <= MAX_LOCAL_ID; ++i)
        {
            _definitions[i] = new FieldSetDefImpl();
            _definitions[i]._setId = BLANK_ID;
        }
        maxSetId = 0;
    }
    
    @Override
    public void clear()
    {
        for (int i = 0; i < maxLocalId; ++i)
        {
            _definitions[i]._setId = BLANK_ID;
        }

        maxSetId = 0;
    }
    
    @Override
    public FieldSetDef[] definitions()
    {
        return _definitions;
    }

    @Override
    public int maxSetId()
    {
        return maxSetId;
    }
    
    @Override
    public void maxSetId(int setMaxSetId)
    {
        maxSetId = setMaxSetId;
    }
    
}
