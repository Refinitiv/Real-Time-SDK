/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

class JsonBuffer {

    JsonBuffer() {}

    JsonBuffer(int startLength) {
        data = new byte[startLength];
    }

    public byte[] data;
    public int position;

    @Override
    public String toString() {

        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < position; i++)
            sb.append((char)data[i]);

        return sb.toString();
    }
}
