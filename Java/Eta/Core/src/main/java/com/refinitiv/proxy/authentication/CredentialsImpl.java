/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.proxy.authentication;

import java.util.Map;
import java.util.HashMap;

class CredentialsImpl implements ICredentials, Cloneable
{
    private final Map<String, String> _credentials = new HashMap<String, String>();

    /**
     * Instantiates a new credentials impl.
     */
    protected CredentialsImpl()
    {
    }

    @Override
    public void set(String name, String value)
    {
        if (name != null)
        {
            _credentials.put(name, value);
        }
    }

    @Override
    public String get(String name)
    {
        String value;

        if (name != null)
        {
            value = _credentials.get(name);
        }
        else
        {
            value = null;
        }

        return value;
    }

    @Override
    public boolean isSet(String name)
    {
        return name != null && _credentials.containsKey(name);
    }
	
    @Override
    public Object clone() throws CloneNotSupportedException
    {
        CredentialsImpl result = (CredentialsImpl)super.clone();

        for (String name : _credentials.keySet())
        {
            result._credentials.put(name, _credentials.get(name));
        }

        return result;
    }
}
