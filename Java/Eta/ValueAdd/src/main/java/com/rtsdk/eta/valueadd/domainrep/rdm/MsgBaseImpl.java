package com.rtsdk.eta.valueadd.domainrep.rdm;

/**
 * Implements {@link MsgBase}.
 * 
 * @see MsgBase
 */
public abstract class MsgBaseImpl implements MsgBase
{
    private int streamId;
    
    protected MsgBaseImpl()
    {
    }
    
    public int streamId()
    {
        return streamId;
    }

    public void streamId(int streamId)
    {
        this.streamId = streamId;
    }


    public void clear()
    {
        streamId = 0;
    }

   
    protected StringBuilder buildStringBuffer()
    {
        stringBuf.setLength(0);
        stringBuf.append(tab);
        stringBuf.append("streamId: ");
        stringBuf.append(streamId());
        stringBuf.append(eol);
        return stringBuf;
    }
    
    public String toString()
    {
        return buildStringBuffer().toString();
    }
    
    private StringBuilder stringBuf = new StringBuilder();
    private final static String eol = "\n";
    private final static String tab = "\t";
}
