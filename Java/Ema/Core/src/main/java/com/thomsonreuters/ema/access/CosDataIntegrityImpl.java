package com.thomsonreuters.ema.access;

public class CosDataIntegrityImpl implements CosDataIntegrity
{
	
	int _type = CosDataIntegrity.CosDataIntegrityType.BEST_EFFORT;
	private OmmOutOfRangeExceptionImpl	_ommOORExcept;
	
	@Override
	public CosDataIntegrity clear() 
	{
		_type = CosDataIntegrity.CosDataIntegrityType.BEST_EFFORT;
		return this;
	}

	@Override
	public CosDataIntegrity type(int type) 
	{
		if (type < CosDataIntegrity.CosDataIntegrityType.BEST_EFFORT ||
				type > CosDataIntegrity.CosDataIntegrityType.RELIABLE)
		{
			if (_ommOORExcept == null)
				_ommOORExcept = new OmmOutOfRangeExceptionImpl();
			
			throw _ommOORExcept.message("Passed in CosDataIntegrity is out of range.");
		}
		
		_type= type;
		return this;
	}

	@Override
	public int type() {
		
		return _type;
	}

}
