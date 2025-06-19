/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

class ConversionResultsImpl implements ConversionResults {

    private int length;

    ConversionResultsImpl() {}

    @Override
    public int getLength() {
        return length;
    }

    @Override
    public void setLength(int value) {
        this.length = value;
    }
}
