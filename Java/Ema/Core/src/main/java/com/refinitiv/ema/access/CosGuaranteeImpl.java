package com.refinitiv.ema.access;

public class CosGuaranteeImpl implements CosGuarantee 
{
	int _type = CosGuarantee.CosGuaranteeType.NONE;
	boolean _persistLocally = true;
	String _filePath = null;
	private OmmOutOfRangeExceptionImpl _ommOORExcept;

	@Override
	public CosGuarantee clear() 
	{
		_type = CosGuarantee.CosGuaranteeType.NONE;
		_persistLocally = true;
		_filePath = "";
		return this;
	}

	@Override
	public CosGuarantee type(int type) 
	{
		if (type < CosGuarantee.CosGuaranteeType.NONE || type > CosGuarantee.CosGuaranteeType.PERSISTENT_QUEUE)
		{
			if (_ommOORExcept == null)
				_ommOORExcept = new OmmOutOfRangeExceptionImpl();
			
			throw _ommOORExcept.message("Passed in CosGuarantee Type is out of range.");
		}
		
		_type = type;
		return this;
	}

	@Override
	public CosGuarantee persistLocally(boolean persistLocally) 
	{
		_persistLocally = persistLocally;
		return this;
	}

	@Override
	public CosGuarantee persistenceFilePath(String filePath) 
	{
		_filePath = filePath;
		return this;
	}

	@Override
	public int type() 
	{
		return _type;
	}

	@Override
	public boolean persistedLocally() 
	{
		return _persistLocally;
	}

	@Override
	public String persistenceFilePath() 
	{
		return _filePath;
	}

}
