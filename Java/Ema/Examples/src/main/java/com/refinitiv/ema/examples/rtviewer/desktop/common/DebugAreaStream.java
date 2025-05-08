/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common;

import javafx.animation.AnimationTimer;
import javafx.concurrent.ScheduledService;
import javafx.concurrent.Task;
import javafx.concurrent.Worker;
import javafx.scene.control.TextArea;
import javafx.util.Duration;

import java.io.*;
import java.util.Objects;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class DebugAreaStream extends OutputStream {

    private static final int CONSOLE_SIZE = 0x15F90;
    private static final int HALF = 2;
    private static final String EMPTY_STRING = "";

    private final PrintStream debugPrintStream;

    private final PrintStream defaultPrintStream;

    private TextArea debugArea;

    private final StringBuilder logs = new StringBuilder(CONSOLE_SIZE);

    private final ScheduledService<Void> scheduledService;

    private final AnimationTimer animationTimer;

    private final BlockingQueue<String> completedLogs = new ArrayBlockingQueue<>(500);

    private int wasAppendedBytes;

    public DebugAreaStream() {
        this.debugPrintStream = new PrintStream(new BufferedOutputStream(this), true);
        this.defaultPrintStream = System.out;
        this.scheduledService = new ScheduledService<Void>() {
            @Override
            protected Task<Void> createTask() {
                return new FxRunnableTask(() -> writeString());
            }
        };
        this.animationTimer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                String log = completedLogs.poll();
                if (Objects.nonNull(log)) {
                    debugArea.setText(log);
                    debugArea.positionCaret(log.length());
                }
                this.stop();
            }
        };
        scheduledService.setPeriod(Duration.seconds(1L));
    }

    @Override
    public synchronized void write(int b) throws IOException {
        logs.append(b);
        wasAppendedBytes++;
    }

    public synchronized void enable(TextArea area) {
        this.debugArea = area;
        System.setOut(debugPrintStream);
        if (scheduledService.getState() == Worker.State.READY) {
            scheduledService.start();
        } else {
            scheduledService.restart();
        }
    }

    public synchronized void disable() {
        if (scheduledService.getState() != Worker.State.READY) {
            scheduledService.cancel();
        }
        wasAppendedBytes = 0;
        System.setOut(defaultPrintStream);
    }

    @Override
    public synchronized void write(byte[] b, int off, int len) throws IOException {
        logs.append(new String(b, off, len));
        wasAppendedBytes += len;
    }

    private synchronized void writeString() {
        if (wasAppendedBytes > 0 && Objects.nonNull(debugArea)) {
            try {
                final String msg = logs.toString();
                if (wasAppendedBytes > CONSOLE_SIZE) {
                    logs.setLength(0);
                } else if (logs.length() > CONSOLE_SIZE) {
                    //Delete half of filled content
                    logs.replace(0, Math.floorDiv(logs.length(), HALF), EMPTY_STRING);
                }
                completedLogs.put(msg);
                animationTimer.start();
                wasAppendedBytes = 0;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    public PrintStream getDebugPrintStream() {
        return debugPrintStream;
    }

    public PrintStream getDefaultPrintStream() {
        return defaultPrintStream;
    }

    public void clear() {
        this.disable();
        this.logs.setLength(0);
    }
}
