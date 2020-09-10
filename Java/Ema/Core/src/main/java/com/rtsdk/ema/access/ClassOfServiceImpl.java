package com.rtsdk.ema.access;

public class ClassOfServiceImpl implements ClassOfService 
{
	CosCommonImpl _common = new CosCommonImpl();
	CosAuthenticationImpl _authentication = new CosAuthenticationImpl();
	CosFlowControlImpl _flowControl = new CosFlowControlImpl();
	CosDataIntegrityImpl _dataIntegrity = new CosDataIntegrityImpl();
	CosGuaranteeImpl _guarantee = new CosGuaranteeImpl();
	
	@Override
	public ClassOfService clear()
	{
			_common.clear();
			_authentication.clear();
			_flowControl.clear();
			_dataIntegrity.clear();
			_guarantee.clear();

		return this;
	}

	@Override
	public ClassOfService common(CosCommon cosCommon) 
	{
		_common.maxMsgSize(cosCommon.maxMsgSize());
		return this;
	}

	@Override
	public ClassOfService authentication(CosAuthentication cosAuthentication) 
	{
		_authentication.type(cosAuthentication.type()); 
		return this;
	}

	@Override
	public ClassOfService flowControl(CosFlowControl cosFlowControl) 
	{
		_flowControl.type(cosFlowControl.type());
		_flowControl.recvWindowSize(cosFlowControl.recvWindowSize());
		return this;
	}

	@Override
	public ClassOfService dataIntegrity(CosDataIntegrity cosDataIntegrity) 
	{
		_dataIntegrity.type(cosDataIntegrity.type());
		return this;
	}

	@Override
	public ClassOfService guarantee(CosGuarantee cosGuarantee) 
	{
		_guarantee.type(cosGuarantee.type());
		_guarantee.persistenceFilePath(cosGuarantee.persistenceFilePath());
		_guarantee.persistLocally(cosGuarantee.persistedLocally());
		return this;
	}

	@Override
	public CosCommon common() 
	{
		return _common;
	}

	@Override
	public CosAuthentication authentication() 
	{
		return _authentication;
	}

	@Override
	public CosFlowControl flowControl() 
	{
		return _flowControl;
	}

	@Override
	public CosDataIntegrity dataIntegrity() 
	{
		return _dataIntegrity;
	}

	@Override
	public CosGuarantee guarantee() 
	{
		return _guarantee;
	}

}
