/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/**
 * This package contains the ETA Value Add Cache.
 * The Value Add Payload Cache component provides a facility for storing
 * OMM containers (the data payload of OMM messages). Typical use of a 
 * payload cache is to store the current image of OMM data streams, where
 * each entry in the cache corresponds to a single data stream. The initial
 * content of a cache entry is defined by the payload of a refresh message.
 * The current (or last) value of the entry is defined by the cumulative 
 * application of all refresh and update messages applied to the cache entry 
 * container. Values are stored in and retrieved from the cache as
 * encoded OMM containers.
 */
package com.refinitiv.eta.valueadd.cache;
