package com.refinitiv.eta.valueadd.common;

public class LimitedVaPool extends VaPool
{
	private int limit = -1;

	public LimitedVaPool()
	{
	}

	public LimitedVaPool(boolean useConcurrent)
	{
		super(useConcurrent);
	}

	public LimitedVaPool(boolean useConcurrent, boolean debug)
	{
		super(useConcurrent, debug);
	}

	@Override
	public void add(VaNode node)
	{
		if(checkPoolLimit(limit))
		{
			super.add(node);
		}
	}

	public void setLimit(int limit)
	{
		this.limit = limit;
	}

	private boolean checkPoolLimit(int limit)
	{
		return limit < 0 || size() < limit;
	}
}
