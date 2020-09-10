package com.rtsdk.eta.codec;

class PriorityImpl implements Priority
{
    int _class;
    int _count;
	
    @Override
    public void clear()
    {
        _class = 0;
        _count = 0;
    }

    @Override
    public void priorityClass(int priorityClass)
    {
        assert (priorityClass >= 0 && priorityClass <= 255) : "priorityClass is out of range (0-255)"; // uint8

        _class = priorityClass;
    }

    @Override
    public int priorityClass()
    {
        return _class;
    }

    @Override
    public void count(int count)
    {
        assert (count >= 0 && count <= 65535) : "priorityCount is out of range (0-65535)"; // uint16

        _count = count;
    }

    @Override
    public int count()
    {
        return _count;
    }
}
