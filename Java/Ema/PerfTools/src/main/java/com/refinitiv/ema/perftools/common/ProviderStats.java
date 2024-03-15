/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022,2024 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import com.refinitiv.ema.perftools.emajniprovperf.NIProviderPerfConfig;
import com.refinitiv.ema.perftools.emajniprovperf.NIProviderThread;
import com.refinitiv.ema.perftools.emajprovperf.IProviderPerfConfig;
import com.refinitiv.ema.perftools.emajprovperf.IProviderThread;

import java.io.PrintWriter;
import java.util.Calendar;
import java.util.TimeZone;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

public class ProviderStats {
    private final ProviderThreadStats totalStats;

    private final ValueStatistics cpuUsageStatistics;
    private final ValueStatistics memUsageStatistics;

    private BaseProviderPerfConfig config;
    private ProviderThread[] providerThreads;
    private LogFileInfo summaryFile;

    private AtomicBoolean _stop = new AtomicBoolean(true);
    private volatile boolean _exitApp;
    private CountDownLatch _loopExited = new CountDownLatch(1);
    private CountDownLatch _stopped = new CountDownLatch(1);

    public ProviderStats() {
        this.totalStats = new ProviderThreadStats();
        this.cpuUsageStatistics = new ValueStatistics();
        this.memUsageStatistics = new ValueStatistics();

    }

    public void initialize(XmlMsgData xmlMsgData, BaseProviderPerfConfig config) {
        this.config = config;
        cpuUsageStatistics.clear();
        memUsageStatistics.clear();

        this.providerThreads = new ProviderThread[config.threadCount()];

        for (int i = 0; i < config.threadCount(); i++) {
            if (config instanceof NIProviderPerfConfig) {
                providerThreads[i] = new NIProviderThread((NIProviderPerfConfig) this.config, xmlMsgData);
            } else {
                providerThreads[i] = new IProviderThread(this.config, xmlMsgData);
            }
            providerThreads[i].initialize(i);
        }

        System.out.println(config.toString());

        if (config.summaryFilename() != null && !config.summaryFilename().isEmpty()) {
            this.summaryFile = LogFileHelper.initFile(config.summaryFilename());
        }
        LogFileHelper.writeFile(summaryFile, config.toString());
    }

    public void run() {
        registerShutdownHook();
        startThreads();
        long nextTime = System.currentTimeMillis() + 1000L;
        int intervalSeconds = 0;
        int currentRuntimeSec = 0;
        int writeStatsInterval = config.writeStatsInterval();
        int runTime = config.runTime();
        boolean displayStats = config.displayStats();

        do {
            if (System.currentTimeMillis() >= nextTime) {
                nextTime += 1000;
                ++intervalSeconds;
                ++currentRuntimeSec;

                // Check if it's time to print stats
                if (intervalSeconds >= writeStatsInterval) {
                    this.collectStats(true, displayStats, currentRuntimeSec, writeStatsInterval);
                    intervalSeconds = 0;
                }
            }

            try {
                long sleepTime = nextTime - System.currentTimeMillis();
                if (sleepTime > 0) {
                    Thread.sleep(sleepTime);
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        } while (currentRuntimeSec < runTime && !_exitApp);
        System.out.printf("\nRun time of %d seconds has expired.\n\n", runTime);

        _loopExited.countDown();
        if (_stop.getAndSet(false)) {
            stop();
            _stopped.countDown();
        }
    }

    public void startThreads() {
        for (ProviderThread thread : providerThreads) {
            thread.start();
        }
    }

    public void stopThreads() {
        for (ProviderThread providerThread : providerThreads) {
            providerThread.shutdown(true);
        }

        for (ProviderThread providerThread : providerThreads) {
            int shutdownCount = 0;
            while (!providerThread.shutdownAck() && shutdownCount < 3) {
                try {
                    Thread.sleep(1000);
                    shutdownCount++;
                } catch (InterruptedException e) {
                    System.err.println("Thread.sleep(1000) failed");
                    System.exit(-1);
                }
            }
        }

        System.out.println("Shutting down.");
    }

    public void clear() {
        for (ProviderThread thread : providerThreads) {
            thread.clear();
        }
    }

    /**
     * Collects and writes provider statistics. Stats will reflect changes from the previous
     * call to this method.
     *
     * @param writeStats        - if true, writes statistics to provider stats file
     * @param displayStats      - if true, writes stats to stdout.
     * @param currentRuntimeSec - current time
     * @param timePassedSec     - time passed since last stats collection, used to calculate message rates.
     */
    public void collectStats(boolean writeStats, boolean displayStats, long currentRuntimeSec, long timePassedSec) {
        long refreshCount, itemRefreshCount, updateCount, updatePackedMsgCount, requestCount, closeCount, postCount, statusCount, genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount, outOfBuffersCount, msgSentCount, bufferSentCount;
        double processCpuLoad = ResourceUsageStats.currentProcessCpuLoad();
        double memoryUsage = ResourceUsageStats.currentMemoryUsage();
        if (timePassedSec != 0) {
            cpuUsageStatistics.update(processCpuLoad);
            memUsageStatistics.update(memoryUsage);
        }

        for (int i = 0; i < config.threadCount(); ++i) {
            ProviderThread providerThread = providerThreads[i];
            collectGenMsgStats(providerThread.providerThreadStats(), providerThread);

            ProviderThreadStats stats = providerThread.providerThreadStats();

            requestCount = stats.requestCount().getChange();
            refreshCount = stats.refreshCount().getChange();
            itemRefreshCount = stats.itemRefreshCount().getChange();
            updateCount = stats.updateCount().getChange();
            closeCount = stats.closeCount().getChange();
            postCount = stats.postCount().getChange();
            statusCount = stats.statusCount().getChange();
            genMsgSentCount = stats.genMsgSentCount().getChange();
            genMsgRecvCount = stats.genMsgRecvCount().getChange();
            latencyGenMsgSentCount = stats.latencyGenMsgSentCount().getChange();
            latencyGenMsgRecvCount = stats.intervalGenMsgLatencyStats().count();
            outOfBuffersCount = stats.outOfBuffersCount().getChange();
            updatePackedMsgCount = stats.updatePackedMsgCount().getChange();
            
            if (writeStats) {
                // Write stats to the stats file.
                Calendar rightNow = Calendar.getInstance(TimeZone.getTimeZone("GMT"));

                LogFileHelper.writeFile(providerThread.statsFile(),
                        String.format("%d-%02d-%02d %02d:%02d:%02d",
                                rightNow.get(Calendar.YEAR), rightNow.get(Calendar.MONTH), rightNow.get(Calendar.DAY_OF_MONTH),
                                rightNow.get(Calendar.HOUR_OF_DAY), rightNow.get(Calendar.MINUTE), rightNow.get(Calendar.SECOND))
                );

                if (providerThread instanceof IProviderThread) {
                    LogFileHelper.writeFile(providerThread.statsFile(), String.format(
                            ", %d, %d, %d, %d, %d, %d, %d, %d, %.1f, %.1f, %.1f, %.1f, %.2f, %.2f\n",
                            requestCount,
                            refreshCount,
                            updateCount,
                            postCount,
                            genMsgSentCount,
                            genMsgRecvCount,
                            latencyGenMsgSentCount,
                            latencyGenMsgRecvCount,
                            stats.intervalGenMsgLatencyStats().average(),
                            Math.sqrt(stats.intervalGenMsgLatencyStats().variance()),
                            ((stats.intervalGenMsgLatencyStats().count() > 0) ? stats.intervalGenMsgLatencyStats().maxValue() : 0.0),
                            ((stats.intervalGenMsgLatencyStats().count() > 0) ? stats.intervalGenMsgLatencyStats().minValue() : 0.0),
                            processCpuLoad,
                            memoryUsage));
                } else {
                    LogFileHelper.writeFile(providerThread.statsFile(), String.format(
                            ", %d, %d, %d, %.2f, %.2f\n",
                            requestCount,
                            refreshCount,
                            updateCount,
                            processCpuLoad,
                            memoryUsage));
                }
            }

            //Add the new counts to the provider's total.
            totalStats.requestCount().add(requestCount);
            totalStats.updateCount().add(updateCount);
            totalStats.refreshCount().add(refreshCount);
            totalStats.itemRefreshCount().add(itemRefreshCount);
            totalStats.closeCount().add(closeCount);
            totalStats.postCount().add(postCount);
            totalStats.statusCount().add(statusCount);
            totalStats.genMsgSentCount().add(genMsgSentCount);
            totalStats.genMsgRecvCount().add(genMsgRecvCount);
            totalStats.latencyGenMsgSentCount().add(latencyGenMsgSentCount);
            totalStats.outOfBuffersCount().add(outOfBuffersCount);
            totalStats.updatePackedMsgCount().add(updatePackedMsgCount);

            if (displayStats) {
                //Print screen stats.
                if (config.threadCount() == 1) {
                    System.out.printf("%03d: ", currentRuntimeSec);
                } else {
                    System.out.printf("%03d: Thread %d:\n  ", currentRuntimeSec, i + 1);
                }

                System.out.printf("UpdRate: %8d, CPU: %6.2f%%, Mem: %6.2fMB\n",
                        updateCount / timePassedSec,
                        processCpuLoad, memoryUsage);

                if (requestCount > 0 || refreshCount > 0) {
                    if (providerThread instanceof IProviderThread) {
                        System.out.printf("  - Received %d item requests (total: %d), sent %d images (total: %d)\n",
                                requestCount,
                                totalStats.requestCount().getTotal(),
                                refreshCount,
                                totalStats.refreshCount().getTotal()
                        );
                    } else {
                        System.out.printf("  - Sent %d images (total: %d)\n", itemRefreshCount, totalStats.itemRefreshCount().getTotal());
                    }
                }
                
                if (updatePackedMsgCount > 0 && ((updateCount / updatePackedMsgCount) > 1))
                	System.out.printf("Average update messages packed per message: %8d\n", 
                        updateCount / updatePackedMsgCount);

                if (postCount > 0) {
                    System.out.printf("  Posting: received %d, reflected %d\n", postCount, postCount);
                }

                if (genMsgRecvCount > 0 || genMsgSentCount > 0) {
                    System.out.printf("  GenMsgs: sent %d, received %d, latencies sent %d, latencies received %d\n",
                            genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);
                }

                if (stats.intervalGenMsgLatencyStats().count() > 0) {
                    stats.intervalGenMsgLatencyStats().print("  GenMsgLat(usec)", "Msgs", false);
                    stats.intervalGenMsgLatencyStats().clear();
                }

                closeCount = totalStats.closeCount().getChange();
                if (closeCount > 0) {
                    System.out.printf("  - Received %d closes.\n", closeCount);
                }

                outOfBuffersCount = totalStats.outOfBuffersCount().getChange();
                if (outOfBuffersCount > 0) {
                    System.out.printf("  - Stopped %d updates due to lack of output buffers.\n", outOfBuffersCount);
                }
            }
        }
    }

    /**
     * Prints summary stats to the summary file.
     *
     * @param fileWriter - file to write summary stats to.
     */
    public void printSummaryStats(PrintWriter fileWriter) {
        long statsTime = 0;
        long currentTime = System.nanoTime();

        if (config.threadCount() > 1) {
            totalStats.inactiveTime(providerThreads[0].providerThreadStats().inactiveTime());
            totalStats.firstGenMsgSentTime(providerThreads[0].providerThreadStats().firstGenMsgSentTime());
            totalStats.firstGenMsgRecvTime(providerThreads[0].providerThreadStats().firstGenMsgRecvTime());
            for (int i = 0; i < config.threadCount(); ++i) {
                ProviderThread providerThread = providerThreads[i];
                ProviderThreadStats stats = providerThread.providerThreadStats();
                statsTime = (stats.inactiveTime() > 0 && stats.inactiveTime() < currentTime) ? stats.inactiveTime() : currentTime;
                if (stats.inactiveTime() > 0 && stats.inactiveTime() < totalStats.inactiveTime()) {
                    totalStats.inactiveTime(stats.inactiveTime());
                }

                fileWriter.printf("\n--- THREAD %d SUMMARY ---\n\n", i + 1);

                fileWriter.printf("Overall Statistics: \n");

                if (config instanceof IProviderPerfConfig) {
                    IProviderPerfConfig ippConfig = (IProviderPerfConfig) config;


                    if (stats.firstGenMsgSentTime() != 0 && stats.firstGenMsgSentTime() < totalStats.firstGenMsgSentTime()) {
                        totalStats.firstGenMsgSentTime(stats.firstGenMsgSentTime());
                    }

                    if (stats.firstGenMsgRecvTime() != 0 && stats.firstGenMsgRecvTime() < totalStats.firstGenMsgRecvTime()) {
                        totalStats.firstGenMsgSentTime(stats.firstGenMsgRecvTime());
                    }

                    if (totalStats.genMsgLatencyStats().count() > 0) {
                        fileWriter.printf("  GenMsg latency avg (usec): %.1f\n" +
                                        "  GenMsg latency std dev (usec): %.1f\n" +
                                        "  GenMsg latency max (usec): %.1f\n" +
                                        "  GenMsg latency min (usec): %.1f\n",
                                stats.genMsgLatencyStats().average(),
                                Math.sqrt(stats.genMsgLatencyStats().variance()),
                                stats.genMsgLatencyStats().count() > 0 ? stats.genMsgLatencyStats().maxValue() : 0,
                                stats.genMsgLatencyStats().count() > 0 ? stats.genMsgLatencyStats().minValue() : 0);
                    } else {
                        fileWriter.printf("  No GenMsg latency information was received.\n");
                    }

                    if (ippConfig.genMsgsPerSec() > 0) {
                        fileWriter.printf("  GenMsgs sent: %d\n", stats.genMsgSentCount().getTotal());
                    }

                    if (stats.genMsgRecvCount().getTotal() > 0) {
                        fileWriter.printf("  GenMsgs received: %d\n", stats.genMsgRecvCount().getTotal());
                    }

                    if (ippConfig.latencyGenMsgRate() > 0) {
                        fileWriter.printf("  GenMsg latencies sent: %d\n", stats.latencyGenMsgSentCount().getTotal());
                    }

                    if (stats.genMsgLatencyStats().count() > 0) {
                        fileWriter.printf("  GenMsg latencies received: %d\n", stats.genMsgLatencyStats().count());
                    }

                    if (ippConfig.genMsgsPerSec() > 0) {
                        fileWriter.printf("  Avg GenMsg send rate: %.0f\n", stats.genMsgSentCount().getTotal() /
                                ((statsTime - stats.firstGenMsgSentTime()) / 1000000000.0));
                    }

                    if (stats.genMsgRecvCount().getTotal() > 0) {
                        fileWriter.printf("  Avg GenMsg receive rate: %.0f\n", stats.genMsgRecvCount().getTotal() /
                                ((statsTime - stats.firstGenMsgRecvTime()) / 1000000000.0));
                    }

                    if (ippConfig.latencyGenMsgRate() > 0) {
                        fileWriter.printf("  Avg GenMsg latency send rate: %.0f\n", stats.latencyGenMsgSentCount().getTotal() /
                                ((statsTime - stats.firstGenMsgSentTime()) / 1000000000.0));
                    }

                    if (stats.genMsgLatencyStats().count() > 0) {
                        fileWriter.printf("  Avg GenMsg latency receive rate: %.0f\n", stats.genMsgLatencyStats().count() /
                                ((statsTime - stats.firstGenMsgRecvTime()) / 1000000000.0));
                    }
                }

                if (config instanceof IProviderPerfConfig)  {
                    fileWriter.printf("  Image requests received: %d\n", totalStats.requestCount().getTotal());
                }

                if (config instanceof NIProviderPerfConfig) {
                	fileWriter.printf("  Images sent: %d\n", totalStats.itemRefreshCount().getTotal());
                }

                if (config.updatesPerSec() > 0) {
                    fileWriter.printf("  Updates sent: %d\n", stats.updateCount().getTotal());
                }

                if (stats.postCount().getTotal() > 0) {
                    fileWriter.printf("  Posts received: %d\n", stats.postCount().getTotal());
                    fileWriter.printf("  Posts reflected: %d\n", stats.postCount().getTotal());
                }
                
                if (stats.updatePackedMsgCount().getTotal() > 0) {
                    fileWriter.printf("  Packed Update Messages Sent: %d\n", stats.updatePackedMsgCount().getTotal());
                }
                break;
            }
        } else {
            ProviderThreadStats totalStats = providerThreads[0].providerThreadStats();
            statsTime = (totalStats.inactiveTime() != 0 && totalStats.inactiveTime() < currentTime) ? totalStats.inactiveTime() : currentTime;
        }
        fileWriter.printf("\n--- OVERALL SUMMARY ---\n\n");

        fileWriter.printf("Overall Statistics: \n");

        if (this.totalStats.genMsgLatencyStats().count() > 0) {
            fileWriter.printf("  GenMsg latency avg (usec): %.1f\n" +
                            "  GenMsg latency std dev (usec): %.1f\n" +
                            "  GenMsg latency max (usec): %.1f\n" +
                            "  GenMsg latency min (usec): %.1f\n",
                    this.totalStats.genMsgLatencyStats().average(),
                    Math.sqrt(this.totalStats.genMsgLatencyStats().variance()),
                    this.totalStats.genMsgLatencyStats().maxValue(),
                    this.totalStats.genMsgLatencyStats().minValue());
        } else {
            fileWriter.printf("  No GenMsg latency information was received.\n");
        }

        if (config instanceof IProviderPerfConfig) {
            IProviderPerfConfig ippConfig = (IProviderPerfConfig) config;
            if (ippConfig.genMsgsPerSec() > 0) {
                fileWriter.printf("  GenMsgs sent: %d\n", totalStats.genMsgSentCount().getTotal());
            }

            if (this.totalStats.genMsgRecvCount().getTotal() > 0) {
                fileWriter.printf("  GenMsgs received: %d\n", totalStats.genMsgRecvCount().getTotal());
            }

            if (ippConfig.latencyGenMsgRate() > 0) {
                fileWriter.printf("  GenMsg latencies sent: %d\n", totalStats.latencyGenMsgSentCount().getTotal());
            }

            if (totalStats.genMsgLatencyStats().count() > 0) {
                fileWriter.printf("  GenMsg latencies received: %d\n", totalStats.genMsgLatencyStats().count());
            }

            if (ippConfig.genMsgsPerSec() > 0) {
                fileWriter.printf("  Avg GenMsg send rate: %.0f\n", totalStats.genMsgSentCount().getTotal() /
                        ((statsTime - totalStats.firstGenMsgSentTime()) / 1000000000.0));
            }

            if (this.totalStats.genMsgRecvCount().getTotal() > 0) {
                fileWriter.printf("  Avg GenMsg receive rate: %.0f\n", totalStats.genMsgRecvCount().getTotal() /
                        ((statsTime - totalStats.firstGenMsgRecvTime()) / 1000000000.0));
            }

            if (ippConfig.latencyGenMsgRate() > 0) {
                fileWriter.printf("  Avg GenMsg latency send rate: %.0f\n", totalStats.latencyGenMsgSentCount().getTotal() /
                        ((statsTime - totalStats.firstGenMsgSentTime()) / 1000000000.0));
            }
        }

        if (totalStats.genMsgLatencyStats().count() > 0) {
            fileWriter.printf("  Avg GenMsg latency receive rate: %.0f\n", totalStats.genMsgLatencyStats().count() /
                    ((statsTime - totalStats.firstGenMsgRecvTime()) / 1000000000.0));
        }

        if (config instanceof IProviderPerfConfig)  {
            fileWriter.printf("  Image requests received: %d\n", totalStats.requestCount().getTotal());
        }

        if (config instanceof NIProviderPerfConfig) {
        	fileWriter.printf("  Images sent: %d\n", totalStats.itemRefreshCount().getTotal());
        }

        if (config.updatesPerSec() > 0) {
            fileWriter.printf("  Updates sent: %d\n", totalStats.updateCount().getTotal());
        }

        if (totalStats.postCount().getTotal() > 0) {
            fileWriter.printf("  Posts received: %d\n", totalStats.postCount().getTotal());
            fileWriter.printf("  Posts reflected: %d\n", totalStats.postCount().getTotal());
        }
        
        if (totalStats.updatePackedMsgCount().getTotal() > 0) {
            fileWriter.printf("  Packed Update Messages Sent: %d\n", totalStats.updatePackedMsgCount().getTotal());
            fileWriter.printf("  Average Messages Packed per Packed Message Sent: %d\n", totalStats.updateCount().getTotal() / totalStats.updatePackedMsgCount().getTotal());
        }

        if (cpuUsageStatistics.count() > 0) {
            fileWriter.printf("  CPU/Memory Samples: %d\n", cpuUsageStatistics.count());
            fileWriter.printf("  CPU Usage max (%%): %.2f\n", cpuUsageStatistics.maxValue());
            fileWriter.printf("  CPU Usage min (%%): %.2f\n", cpuUsageStatistics.minValue());
            fileWriter.printf("  CPU Usage avg (%%): %.2f\n", cpuUsageStatistics.average());
            fileWriter.printf("  Memory Usage max (MB): %.2f\n", memUsageStatistics.maxValue());
            fileWriter.printf("  Memory Usage min (MB): %.2f\n", memUsageStatistics.minValue());
            fileWriter.printf("  Memory Usage avg (MB): %.2f\n", memUsageStatistics.average());
        }

        fileWriter.printf("\n");
    }

    private void collectGenMsgStats(ProviderThreadStats threadStats, ProviderThread providerThread) {
        if (providerThread instanceof IProviderThread) {
            IProviderThread ippThread = (IProviderThread) providerThread;
            TimeRecordQueue latencyRecords = threadStats.genMsgLatencyRecords();
            while (!latencyRecords.records().isEmpty()) {
                TimeRecord record = latencyRecords.records().poll();
                double latency = (double) (record.endTime() - record.startTime()) / (double) record.ticks();

                threadStats.intervalGenMsgLatencyStats().update(latency);
                threadStats.genMsgLatencyStats().update(latency);

                if (config.threadCount() > 1) {
                    totalStats.genMsgLatencyStats().update(latency);
                }

                LogFileHelper.writeFile(ippThread.latencyFile(),
                        String.format("Gen, %d, %d, %d\n", record.startTime(), record.endTime(), (record.endTime() - record.startTime())));

                latencyRecords.pool().add(record);
            }
        }
    }


    /**
     * Prints the final stats.
     */
    public void printFinalStats() {
        PrintWriter printWriter = new PrintWriter(System.out);
        printSummaryStats(printWriter);
        printWriter.flush();
        printSummaryStats(summaryFile.writer());
        summaryFile.writer().close();
    }

    private void stop() {
        try {
            stopThreads();
            printFinalStats();
            clear();
        } finally {
            _stopped.countDown();
        }
    }

    private void registerShutdownHook() {
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            try {
                if (_stop.getAndSet(false)) {
                    _exitApp = true;
                    _loopExited.await(1000, TimeUnit.MILLISECONDS);
                    stop();
                } else {
                    _stopped.await(providerThreads.length * 5000, TimeUnit.MILLISECONDS);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }));
    }

    public ProviderThread[] providerThreads() {
        return providerThreads;
    }
}
