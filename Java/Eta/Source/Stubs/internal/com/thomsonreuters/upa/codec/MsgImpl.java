package com.thomsonreuters.upa.codec;


import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.PostUserInfo;
import com.thomsonreuters.upa.codec.Priority;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.UpdateMsg;

/**
 * This class provides the implementation for the superset of message classes
 * (e.g. {@link RequestMsg}, {@link RefreshMsg}, etc.). Representing
 * the superset of message types in a single class affords users the ability
 * to maintain a single pool of messages that can be re-used across message
 * types. (Users cast to the appropriate message-class specific interface.
 * For example, an instance of this class may initially represent a
 * request message, and later, a response message.
 */
final class MsgImpl implements Msg, AckMsg, CloseMsg, GenericMsg, PostMsg, RefreshMsg, RequestMsg, StatusMsg, UpdateMsg
{
  
    @Override
    public MsgKey msgKey()
	{
    	return null;
	}

	@Override
    public int encode(EncodeIterator	iter)
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
    public int encodeInit(EncodeIterator	iter, int dataMaxSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public int encodeComplete(EncodeIterator iter, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public int encodeKeyAttribComplete(EncodeIterator iter, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public int encodeExtendedHeaderComplete(EncodeIterator iter, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void msgClass(int msgClass)
	{
	    
	}

	@Override
    public int msgClass()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void domainType(int domainType)
	{
	   
	}

	@Override
    public int domainType()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void containerType(int containerType)
	{
	    
	}

	@Override
    public int containerType()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void streamId(int streamId)
	{
	    
	}

	@Override
    public int streamId()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void extendedHeader(Buffer extendedHeader)
	{
	    
	}

	@Override
    public Buffer extendedHeader()
	{
       return null;
	}

	@Override
    public void encodedDataBody(Buffer encodedDataBody)
	{
	  
	}

	@Override
    public Buffer encodedDataBody()
	{
		return null;
	}

	

	@Override
    public Buffer encodedMsgBuffer()
	{
		return null;
	}
	
    @Override
    public int decode(DecodeIterator iter)
    {
    	return CodecReturnCodes.FAILURE;	
    }

    @Override
    public int decodeKeyAttrib(DecodeIterator iter, MsgKey key)
    {
    	return CodecReturnCodes.FAILURE;	
    }

   
    @Override
    public int copy(Msg destMsg, int copyMsgFlags)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public boolean isFinalMsg()
    {
    	
    	return false;
    }

    @Override
    public boolean validateMsg()
    {
    	return false;
    }

    @Override
    public String decodeToXml(DecodeIterator iter)
    {
		return null;		
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
		return null;		
    }

    @Override
    public boolean checkHasExtendedHdr()
    {
       return false;
    }    
    
    @Override
    public void applyHasExtendedHdr()
    {
        
    }    

    @Override
    public void flags(int flags)
    {
       
    }
    
    @Override
    public int flags()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public boolean checkHasMsgKey()
    {
       return false;
    }    
   
    @Override
    public void applyHasMsgKey()
    {
       
    }

    @Override
    public boolean checkHasSeqNum()
    {
        return false;
    }    
    
    @Override
    public void applyHasSeqNum()
    {
      
    }
    
    @Override
    public void seqNum(long seqNum)
    {
       
    }
    
    @Override
    public long seqNum()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public boolean checkAck()
    {
       return false;
    }
    
    @Override
    public boolean checkHasPermData()
    {
       return false;
    }
    
    @Override
    public void applyAck()
    {
       
    }
    
    @Override
    public void applyHasPermData()
    {
       
    }

    @Override
    public void partNum(int partNum)
    {
      
    }
    
    @Override
    public int partNum()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public void permData(Buffer permData)
    {
        
    }
    
    @Override
    public Buffer permData()
    {
    	return null;
    }
    
    @Override
    public void applyHasPartNum()
    {
       
    }
    
    @Override
    public boolean checkPrivateStream()
    {
       return false;
    }
    
    @Override
    public boolean checkQualifiedStream()
    {
       return false;
    }

    @Override
    public void groupId(Buffer groupId)
    {
       
    }
    
    @Override
    public Buffer groupId()
    {
       return null;
    }
    
    @Override
    public void applyPrivateStream()
    {
        
    }
    
    @Override
    public void applyQualifiedStream()
    {
       
    }
    
    @Override
    public boolean checkHasPartNum()
    {
       return false;   
    }    
    
    @Override
    public PostUserInfo postUserInfo()
    {
        return null;
    }
    
    @Override
    public boolean checkHasQos()
    {
       return false;
    }    
    
    @Override
    public boolean checkClearCache()
    {
        return false;  
    }    
    
    @Override
    public boolean checkDoNotCache()
    {
       return false;  
    }
    
    @Override
    public boolean checkHasPostUserInfo()
    {
        return false;
    }
    
    @Override
    public void applyClearCache()
    {
       
    }
    
    @Override
    public void applyDoNotCache()
    {
      
    }
    
    @Override
    public void applyHasPostUserInfo()
    {
       
    }
   
    @Override
    public State state()
    {
       return null;
    }
    
    @Override
    public Qos qos()
    {
        return null;
    }
    
    ////////// begin AckMsg-specific methods //////////
    
    @Override
    public boolean checkHasText()
    {
        return false;
    }
       
    @Override
    public boolean checkHasNakCode()
    {
        return false;
    }
    
    @Override
    public void applyHasText()
    {
        
    }
    
    @Override
    public void applyHasNakCode()
    {
       
    }
    
    @Override
    public void ackId(long ackId)
    {
        
    }
    
   @Override
    public long ackId()
    {
	   return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public void nakCode(int nakCode)
    {
        
    }
    
    @Override
    public int nakCode()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public void text(Buffer text)
    {
       
    }
    
    @Override
    public Buffer text()
    {
       return null;
    }    
    
    ////////// end AckMsg-specific methods //////////
        
    ////////// begin GenericMsg-specific methods //////////
    

    @Override
    public boolean checkMessageComplete()
    {
       return false;
    }

    @Override
    public boolean checkHasSecondarySeqNum()
    {
        return false;
    }
    
    @Override
    public void applyMessageComplete()
    {
       
    }
    
    @Override
    public void applyHasSecondarySeqNum()
    {
       
    }

    @Override
    public void secondarySeqNum(long secondarySeqNum)
    {
       
    }
    
    @Override
    public long secondarySeqNum()
    {
    	return CodecReturnCodes.FAILURE;
    }
        
    ////////// end GenericMsg-specific methods //////////

    ////////// begin PostMsg-specific methods //////////

    @Override
    public boolean checkHasPostId()
    {
        return false;
    }
    
    @Override
    public boolean checkPostComplete()
    {
        return false;
    }    
    
    @Override
    public boolean checkHasPostUserRights()
    {
        return false;
    }

    @Override
    public void applyHasPostId()
    {
       
    }    

    @Override
    public void applyPostComplete()
    {
       
    }

    @Override
    public void applyHasPostUserRights()
    {
       
    }
    
    @Override
    public void postId(long postId)
    {
        
    }
    
    @Override
    public long postId()
    {
    	return CodecReturnCodes.FAILURE;
    }
       
    @Override
    public void postUserRights(int postUserRights)
    {
       
    }
    
    @Override
    public int postUserRights()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    ////////// end PostMsg-specific methods //////////    

    ////////// begin RefreshMsg-specific methods //////////
    
    @Override
    public boolean checkSolicited()
    {
        return false;
    }
    
    @Override
    public boolean checkRefreshComplete()
    {
        return false;
    }
    
    @Override
    public void applySolicited()
    {
       
    }
    
    @Override
    public void applyRefreshComplete()
    {
        
    }
    
    @Override
    public void applyHasQos()
    {
       
    }
    
    ////////// end RefreshMsg-specific methods //////////
    
    ////////// begin RequestMsg-specific methods //////////
    
    @Override
    public boolean checkHasPriority()
    {
        return false;
    }
    
    @Override
    public boolean checkStreaming()
    {
        return false;
    }
    
    @Override
    public boolean checkMsgKeyInUpdates()
    {
        return false;
    }
    
    @Override
    public boolean checkConfInfoInUpdates()
    {
        return false;
    }
    
    @Override
    public boolean checkNoRefresh()
    {
        return false;
    }
    
    @Override
    public boolean checkHasWorstQos()
    {
        return false;
    }
    
    @Override
    public boolean checkPause()
    {
        return false;
    }
    
    @Override
    public boolean checkHasView()
    {
        return false;
    }
    
    @Override
    public boolean checkHasBatch()
    {
        return false;
    }
    
    @Override
    public void applyHasPriority()
    {
       
    }
    
    @Override
    public void applyStreaming()
    {
        
    }
    
    @Override
    public void applyMsgKeyInUpdates()
    {
     
    }
    
    @Override
    public void applyConfInfoInUpdates()
    {
        
    }
    
    @Override
    public void applyNoRefresh()
    {
        
    }
    
    @Override
    public void applyHasWorstQos()
    {
        
    }
    
    @Override
    public void applyPause()
    {
        
    }
    
    @Override
    public void applyHasView()
    {
        
    }
    
    @Override
    public void applyHasBatch()
    {
       
    }
    
    @Override
    public Priority priority()
    {
        return null;
    }
    
    @Override
    public Qos worstQos()
    {
       return null;
    }
    
    ////////// end RequestMsg-specific methods //////////
    
    ////////// begin StatusMsg-specific methods //////////
    
    @Override
    public boolean checkHasGroupId()
    {
        return false;
    }
    
    @Override
    public boolean checkHasState()
    {
        return false;
    }
    
    @Override
    public void applyHasGroupId()
    {
       
    }
    
    @Override
    public void applyHasState()
    {
        
    }
    
    ////////// end StatusMsg-specific methods //////////
    
    ////////// begin UpdateMsg-specific methods //////////
    
    @Override
    public boolean checkHasConfInfo()
    {
        return false;
    }
    
    @Override
    public boolean checkDoNotConflate()
    {
        return false;
    }
    
    @Override
    public boolean checkDoNotRipple()
    {
        return false;
    }
    

    @Override
    public boolean checkDiscardable()
    {
        return false;
    }
    
    @Override
    public void applyHasConfInfo()
    {
       
    }
    
    @Override
    public void applyDoNotConflate()
    {
       
    }
    
    @Override
    public void applyDoNotRipple()
    {
        
    }

    @Override
    public void applyDiscardable()
    {
        
    }

    @Override
    public void updateType(int updateType)
    {
       
    }
    
    @Override
    public int updateType()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public void conflationCount(int conflationCount)
    {
       
    }
    
    @Override
    public int conflationCount()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public void conflationTime(int conflationTime)
    {
       
    }
    
    @Override
    public int conflationTime()
    {
    	return CodecReturnCodes.FAILURE;
    }
   
    ////////// end UpdateMsg-specific methods //////////

    @Override
    public void clear()
    {
    }

	@Override
	public void state(State state) 
	{
		
		
	}
}
