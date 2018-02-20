package com.thomsonreuters.upa.transport;

class SeqMCastOptsImpl implements SeqMCastOpts
{
    private int maxMsgSize;
    private int instanceId;

    SeqMCastOptsImpl()
    {
    }

    @Override
    public void maxMsgSize(int size)
    {
        maxMsgSize = size;
    }

    @Override
    public int maxMsgSize()
    {
        return maxMsgSize;
    }

    @Override
    public void instanceId(int id)
    {
        instanceId = id;
    }

    @Override
    public int instanceId()
    {
        return instanceId;
    }

    void copy(SeqMCastOptsImpl destOpts)
    {
        destOpts.maxMsgSize = maxMsgSize;
        destOpts.instanceId = instanceId;
    }

    @Override
    public String toString()
    {
        return "SeqMCastOpts" + "\n" + 
               "\t\tmaxMsgSize: " + maxMsgSize + 
               "\t\tinstanceId: " + instanceId;
    }
}
