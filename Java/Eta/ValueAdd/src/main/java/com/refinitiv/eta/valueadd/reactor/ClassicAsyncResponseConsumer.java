/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import org.apache.hc.core5.concurrent.FutureCallback;
import org.apache.hc.core5.http.*;
import org.apache.hc.core5.http.io.entity.ByteArrayEntity;
import org.apache.hc.core5.http.message.BasicClassicHttpResponse;
import org.apache.hc.core5.http.nio.AsyncEntityConsumer;
import org.apache.hc.core5.http.nio.CapacityChannel;
import org.apache.hc.core5.http.nio.support.AbstractAsyncResponseConsumer;
import org.apache.hc.core5.http.protocol.HttpContext;
import org.apache.hc.core5.util.ByteArrayBuffer;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Iterator;
import java.util.List;

class ClassicAsyncResponseConsumer extends AbstractAsyncResponseConsumer<ClassicHttpResponse, HttpEntity> {

    public ClassicAsyncResponseConsumer(AsyncEntityConsumer<HttpEntity> dataConsumer) {
        super(dataConsumer);
    }

    public static ClassicAsyncResponseConsumer create() {
        return new ClassicAsyncResponseConsumer(new RestAsyncEntityConsumer());
    }

    @Override
    protected ClassicHttpResponse buildResult(HttpResponse response, HttpEntity entity, ContentType contentType) {
        ClassicHttpResponse classicResponse = new BasicClassicHttpResponse(response.getCode(), response.getReasonPhrase());
        if (response.getLocale() != null) {
            classicResponse.setLocale(response.getLocale());
        }
        classicResponse.setEntity(entity);
        classicResponse.setVersion(response.getVersion());
        Iterator<Header> iter = response.headerIterator();
        while (iter.hasNext()) {
            classicResponse.addHeader(iter.next());
        }
        return classicResponse;
    }

    @Override
    public void informationResponse(HttpResponse response, HttpContext context) throws HttpException, IOException {
    }
}

class RestAsyncEntityConsumer implements AsyncEntityConsumer<HttpEntity> {

    volatile FutureCallback<HttpEntity> resultCallback;
    EntityDetails entityDetails;
    volatile ByteArrayEntity content;
    ByteArrayBuffer buffer = new ByteArrayBuffer(0);

    @Override
    public void streamStart(EntityDetails entityDetails, FutureCallback<HttpEntity> resultCallback) throws HttpException, IOException {
        this.entityDetails = entityDetails;
        this.resultCallback = resultCallback;
    }

    @Override
    public void failed(Exception cause) {
        if (resultCallback != null) {
            resultCallback.failed(cause);
        }
        releaseResources();
    }

    @Override
    public HttpEntity getContent() {
        return content;
    }

    @Override
    public void updateCapacity(CapacityChannel capacityChannel) throws IOException {
        capacityChannel.update(Integer.MAX_VALUE);
    }

    @Override
    public void consume(ByteBuffer src) throws IOException {
        if (src == null) {
            return;
        }
        if (src.hasArray()) {
            buffer.append(src.array(), src.arrayOffset() + src.position(), src.remaining());
        } else {
            while (src.hasRemaining()) {
                buffer.append(src.get());
            }
        }
    }

    @Override
    public void streamEnd(List<? extends Header> trailers) {
        if (entityDetails != null) {
            content = new ByteArrayEntity(
                    buffer.toByteArray(),
                    ContentType.parse(entityDetails.getContentType()),
                    entityDetails.getContentEncoding(),
                    entityDetails.isChunked()
            );
        }
        resultCallback.completed(content);
    }

    @Override
    public void releaseResources() {
        buffer.clear();
    }
}
