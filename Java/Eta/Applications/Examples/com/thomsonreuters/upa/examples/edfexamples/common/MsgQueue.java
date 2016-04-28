package com.thomsonreuters.upa.examples.edfexamples.common;

import java.util.LinkedList;
import java.util.List;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.Msg;

/** Message queue used by the EDFConsumer application. */
public class MsgQueue
{
    private List<Msg> msgQueue = new LinkedList<Msg>();
    
    public void msgQueueClear()
    {
        for (int i = 0; i < msgQueue.size(); ++i)
        {
            msgQueueCleanupMsg(msgQueue.get(i));
            msgQueue.remove(i);
        }
    }
    
   public void msgQueueAdd(Msg msg)
    {
        Msg newMsg = CodecFactory.createMsg();
        msg.copy(newMsg, msg.flags());
        msgQueue.add(newMsg);
    }
    
    public int msgQueuePop(int popCmd, Msg msg)
    {
        if (msgQueue.isEmpty())
            return CodecReturnCodes.FAILURE;
        
        msg = CodecFactory.createMsg();
        msgQueue.get(0).copy(msg, msgQueue.get(0).flags());
        msgQueue.remove(0);
        
        return CodecReturnCodes.SUCCESS;
    }
    
    public void msgQueueCleanupMsg(Msg msg)
    {
        msg.clear();
    }
    
    public boolean msgQueueHasMsgs()
    {
        return !msgQueue.isEmpty();
    }
    
}
