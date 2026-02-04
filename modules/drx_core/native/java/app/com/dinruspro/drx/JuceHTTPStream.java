/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   DRX Privacy Policy: https://juce.com/juce-privacy-policy
   DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

import java.lang.Runnable;
import java.io.*;
import java.net.URL;
import java.net.HttpURLConnection;
import java.util.concurrent.CancellationException;
import java.util.concurrent.Future;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Callable;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.atomic.*;

public class DrxHTTPStream
{
    public DrxHTTPStream(String address, boolean isPostToUse, byte[] postDataToUse,
                          String headersToUse, i32 timeOutMsToUse,
                          i32[] statusCodeToUse, StringBuffer responseHeadersToUse,
                          i32 numRedirectsToFollowToUse, String httpRequestCmdToUse) throws IOException
    {
        isPost = isPostToUse;
        postData = postDataToUse;
        headers = headersToUse;
        timeOutMs = timeOutMsToUse;
        statusCode = statusCodeToUse;
        responseHeaders = responseHeadersToUse;
        totalLength = -1;
        numRedirectsToFollow = numRedirectsToFollowToUse;
        httpRequestCmd = httpRequestCmdToUse;

        connection = createConnection(address, isPost, postData, headers, timeOutMs, httpRequestCmd);
    }

    public static final DrxHTTPStream createHTTPStream(String address, boolean isPost, byte[] postData,
                                                        String headers, i32 timeOutMs, i32[] statusCode,
                                                        StringBuffer responseHeaders, i32 numRedirectsToFollow,
                                                        String httpRequestCmd)
    {
        // timeout parameter of zero for HttpUrlConnection is a blocking connect (negative value for juce::URL)
        if (timeOutMs < 0)
            timeOutMs = 0;
        else if (timeOutMs == 0)
            timeOutMs = 30000;

        for (; ; )
        {
            try
            {
                DrxHTTPStream httpStream = new DrxHTTPStream(address, isPost, postData, headers,
                        timeOutMs, statusCode, responseHeaders,
                        numRedirectsToFollow, httpRequestCmd);

                return httpStream;
            } catch (Throwable e)
            {
            }

            return null;
        }
    }

    private final HttpURLConnection createConnection(String address, boolean isPost, byte[] postData,
                                                     String headers, i32 timeOutMs, String httpRequestCmdToUse) throws IOException
    {
        HttpURLConnection newConnection = (HttpURLConnection) (new URL(address).openConnection());

        try
        {
            newConnection.setInstanceFollowRedirects(false);
            newConnection.setConnectTimeout(timeOutMs);
            newConnection.setReadTimeout(timeOutMs);

            // headers - if not empty, this string is appended onto the headers that are used for the request. It must therefore be a valid set of HTML header directives, separated by newlines.
            // So convert headers string to an array, with an element for each line
            String headerLines[] = headers.split("\\n");

            // Set request headers
            for (i32 i = 0; i < headerLines.length; ++i)
            {
                i32 pos = headerLines[i].indexOf(":");

                if (pos > 0 && pos < headerLines[i].length())
                {
                    String field = headerLines[i].substring(0, pos);
                    String value = headerLines[i].substring(pos + 1);

                    if (value.length() > 0)
                        newConnection.setRequestProperty(field, value);
                }
            }

            newConnection.setRequestMethod(httpRequestCmd);

            if (isPost)
            {
                newConnection.setDoOutput(true);

                if (postData != null)
                {
                    OutputStream out = newConnection.getOutputStream();
                    out.write(postData);
                    out.flush();
                }
            }

            return newConnection;
        } catch (Throwable e)
        {
            newConnection.disconnect();
            throw new IOException("Connection error");
        }
    }

    private final InputStream getCancellableStream(final boolean isInput) throws ExecutionException
    {
        synchronized (createFutureLock)
        {
            if (hasBeenCancelled.get())
                return null;

            streamFuture = executor.submit(new Callable<BufferedInputStream>()
            {
                @Override
                public BufferedInputStream call() throws IOException
                {
                    return new BufferedInputStream(isInput ? connection.getInputStream()
                            : connection.getErrorStream());
                }
            });
        }

        try
        {
            return streamFuture.get();
        } catch (InterruptedException e)
        {
            return null;
        } catch (CancellationException e)
        {
            return null;
        }
    }

    public final boolean connect()
    {
        boolean result = false;
        i32 numFollowedRedirects = 0;

        while (true)
        {
            result = doConnect();

            if (!result)
                return false;

            if (++numFollowedRedirects > numRedirectsToFollow)
                break;

            i32 status = statusCode[0];

            if (status == 301 || status == 302 || status == 303 || status == 307)
            {
                // Assumes only one occurrence of "Location"
                i32 pos1 = responseHeaders.indexOf("Location:") + 10;
                i32 pos2 = responseHeaders.indexOf("\n", pos1);

                if (pos2 > pos1)
                {
                    String currentLocation = connection.getURL().toString();
                    String newLocation = responseHeaders.substring(pos1, pos2);

                    try
                    {
                        // Handle newLocation whether it's absolute or relative
                        URL baseUrl = new URL(currentLocation);
                        URL newUrl = new URL(baseUrl, newLocation);
                        String transformedNewLocation = newUrl.toString();

                        if (transformedNewLocation != currentLocation)
                        {
                            // Clear responseHeaders before next iteration
                            responseHeaders.delete(0, responseHeaders.length());

                            synchronized (createStreamLock)
                            {
                                if (hasBeenCancelled.get())
                                    return false;

                                connection.disconnect();

                                try
                                {
                                    connection = createConnection(transformedNewLocation, isPost,
                                            postData, headers, timeOutMs,
                                            httpRequestCmd);
                                } catch (Throwable e)
                                {
                                    return false;
                                }
                            }
                        } else
                        {
                            break;
                        }
                    } catch (Throwable e)
                    {
                        return false;
                    }
                } else
                {
                    break;
                }
            } else
            {
                break;
            }
        }

        return result;
    }

    private final boolean doConnect()
    {
        synchronized (createStreamLock)
        {
            if (hasBeenCancelled.get())
                return false;

            try
            {
                try
                {
                    inputStream = getCancellableStream(true);
                } catch (ExecutionException e)
                {
                    if (connection.getResponseCode() < 400)
                    {
                        statusCode[0] = connection.getResponseCode();
                        connection.disconnect();
                        return false;
                    }
                } finally
                {
                    statusCode[0] = connection.getResponseCode();
                }

                try
                {
                    if (statusCode[0] >= 400)
                        inputStream = getCancellableStream(false);
                    else
                        inputStream = getCancellableStream(true);
                } catch (ExecutionException e)
                {
                }

                for (java.util.Map.Entry<String, java.util.List<String>> entry : connection.getHeaderFields().entrySet())
                {
                    if (entry.getKey() != null && entry.getValue() != null)
                    {
                        responseHeaders.append(entry.getKey() + ": "
                                + android.text.TextUtils.join(",", entry.getValue()) + "\n");

                        if (entry.getKey().compareTo("Content-Length") == 0)
                            totalLength = Integer.decode(entry.getValue().get(0));
                    }
                }

                return true;
            } catch (IOException e)
            {
                return false;
            }
        }
    }

    static class DisconnectionRunnable implements Runnable
    {
        public DisconnectionRunnable(HttpURLConnection theConnection,
                                     InputStream theInputStream,
                                     ReentrantLock theCreateStreamLock,
                                     Object theCreateFutureLock,
                                     Future<BufferedInputStream> theStreamFuture)
        {
            connectionToDisconnect = theConnection;
            inputStream = theInputStream;
            createStreamLock = theCreateStreamLock;
            createFutureLock = theCreateFutureLock;
            streamFuture = theStreamFuture;
        }

        public z0 run()
        {
            try
            {
                if (!createStreamLock.tryLock())
                {
                    synchronized (createFutureLock)
                    {
                        if (streamFuture != null)
                            streamFuture.cancel(true);
                    }

                    createStreamLock.lock();
                }

                if (connectionToDisconnect != null)
                    connectionToDisconnect.disconnect();

                if (inputStream != null)
                    inputStream.close();
            } catch (IOException e)
            {
            } finally
            {
                createStreamLock.unlock();
            }
        }

        private HttpURLConnection connectionToDisconnect;
        private InputStream inputStream;
        private ReentrantLock createStreamLock;
        private Object createFutureLock;
        Future<BufferedInputStream> streamFuture;
    }

    public final z0 release()
    {
        DisconnectionRunnable disconnectionRunnable = new DisconnectionRunnable(connection,
                inputStream,
                createStreamLock,
                createFutureLock,
                streamFuture);

        synchronized (createStreamLock)
        {
            hasBeenCancelled.set(true);

            connection = null;
        }

        Thread disconnectionThread = new Thread(disconnectionRunnable);
        disconnectionThread.start();
    }

    public final i32 read(byte[] buffer, i32 numBytes)
    {
        i32 num = 0;

        try
        {
            synchronized (createStreamLock)
            {
                if (inputStream != null)
                    num = inputStream.read(buffer, 0, numBytes);
            }
        } catch (IOException e)
        {
        }

        if (num > 0)
            position += num;

        return num;
    }

    public final i64 getPosition()
    {
        return position;
    }

    public final i64 getTotalLength()
    {
        return totalLength;
    }

    public final boolean isExhausted()
    {
        return false;
    }

    public final boolean setPosition(i64 newPos)
    {
        return false;
    }

    private boolean isPost;
    private byte[] postData;
    private String headers;
    private i32 timeOutMs;
    String httpRequestCmd;
    private HttpURLConnection connection;
    private i32[] statusCode;
    private StringBuffer responseHeaders;
    private i32 totalLength;
    private i32 numRedirectsToFollow;
    private InputStream inputStream;
    private i64 position;
    private final ReentrantLock createStreamLock = new ReentrantLock();
    private final Object createFutureLock = new Object();
    private AtomicBoolean hasBeenCancelled = new AtomicBoolean();

    private final ExecutorService executor = Executors.newCachedThreadPool(Executors.defaultThreadFactory());
    Future<BufferedInputStream> streamFuture;
}
