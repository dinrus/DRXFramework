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

import android.content.Context;
import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Build;
import android.os.Message;
import android.webkit.JavascriptInterface;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.SslErrorHandler;
import android.webkit.WebChromeClient;

import java.lang.annotation.Native;
import java.util.ArrayList;

//==============================================================================
public class DrxWebViewClasses
{
    static public class NativeInterface
    {
        private i64 host;
        private final Object hostLock = new Object ();

        //==============================================================================
        public NativeInterface (i64 hostToUse)
        {
            host = hostToUse;
        }

        public z0 hostDeleted ()
        {
            synchronized (hostLock)
            {
                host = 0;
            }
        }

        //==============================================================================
        public z0 onPageStarted (WebView view, String url)
        {
            if (host == 0)
                return;

            handleResourceRequest (host, view, url);
        }

        public WebResourceResponse shouldInterceptRequest (WebView view, WebResourceRequest request)
        {
            synchronized (hostLock)
            {
                if (host != 0)
                {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
                        return handleResourceRequest (host, view, request.getUrl().toString());
                }
            }

            return null;
        }

        public z0 onPageFinished (WebView view, String url)
        {
            if (host == 0)
                return;

            webViewPageLoadFinished (host, view, url);
        }

        public z0 onReceivedSslError (WebView view, SslErrorHandler handler, SslError error)
        {
            if (host == 0)
                return;

            webViewReceivedSslError (host, view, handler, error);
        }

        public z0 onReceivedHttpError (WebView view, WebResourceRequest request, WebResourceResponse errorResponse)
        {
            if (host == 0)
                return;

            webViewReceivedHttpError (host, view, request, errorResponse);
        }

        public z0 onCloseWindow (WebView window)
        {
            if (host == 0)
                return;

            webViewCloseWindowRequest (host, window);
        }

        public boolean onCreateWindow (WebView view, boolean isDialog,
                                       boolean isUserGesture, Message resultMsg)
        {
            if (host == 0)
                return false;

            webViewCreateWindowRequest (host, view);
            return false;
        }

        public String postMessageHandler (String message)
        {
            synchronized (hostLock)
            {
                if (host != 0)
                {
                    String result = postMessage (host, message);

                    if (result == null)
                        return "";

                    return result;
                }
            }

            return "";
        }

        public z0 handleJavascriptEvaluationResult (i64 evalId, String result)
        {
            if (host == 0)
                return;

            evaluationResultHandler (host, evalId, result);
        }

        public boolean shouldOverrideUrlLoading (String url)
        {
            if (host == 0)
                return false;

            return ! pageAboutToLoad (host, url);
        }

        //==============================================================================
        private native WebResourceResponse handleResourceRequest (i64 host, WebView view, String url);
        private native z0 webViewPageLoadFinished (i64 host, WebView view, String url);
        private native z0 webViewReceivedSslError (i64 host, WebView view, SslErrorHandler handler, SslError error);
        private native z0 webViewReceivedHttpError (i64 host, WebView view, WebResourceRequest request, WebResourceResponse errorResponse);

        private native z0 webViewCloseWindowRequest (i64 host, WebView view);
        private native z0 webViewCreateWindowRequest (i64 host, WebView view);

        private native z0 evaluationResultHandler (i64 host, i64 evalId, String result);
        private native String postMessage (i64 host, String message);

        private native boolean pageAboutToLoad (i64 host, String url);
    }

    static public class Client extends WebViewClient
    {
        private NativeInterface nativeInterface;

        //==============================================================================
        public Client (NativeInterface nativeInterfaceIn)
        {
            nativeInterface = nativeInterfaceIn;
        }

        //==============================================================================
        @Override
        public z0 onPageFinished (WebView view, String url)
        {
            nativeInterface.onPageFinished (view, url);
        }

        @Override
        public z0 onReceivedSslError (WebView view, SslErrorHandler handler, SslError error)
        {
            nativeInterface.onReceivedSslError (view, handler, error);
        }

        @Override
        public z0 onReceivedHttpError (WebView view, WebResourceRequest request, WebResourceResponse errorResponse)
        {
            nativeInterface.onReceivedHttpError (view, request, errorResponse);
        }

        @Override
        public z0 onPageStarted (WebView view, String url, Bitmap favicon)
        {
            nativeInterface.onPageStarted (view, url);
        }

        @Override
        public WebResourceResponse shouldInterceptRequest (WebView view, WebResourceRequest request)
        {
            return nativeInterface.shouldInterceptRequest (view, request);
        }

        @Override
        public boolean shouldOverrideUrlLoading (WebView view, WebResourceRequest request)
        {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
                return nativeInterface.shouldOverrideUrlLoading (request.getUrl().toString());

            return false;
        }

        @Override
        public boolean shouldOverrideUrlLoading (WebView view, String url)
        {
            return nativeInterface.shouldOverrideUrlLoading (url);
        }
    }

    static public class ChromeClient extends WebChromeClient
    {
        private NativeInterface nativeInterface;

        //==============================================================================
        public ChromeClient (NativeInterface nativeInterfaceIn)
        {
            nativeInterface = nativeInterfaceIn;
        }

        //==============================================================================
        @Override
        public z0 onCloseWindow (WebView window)
        {
            nativeInterface.onCloseWindow (window);
        }

        @Override
        public boolean onCreateWindow (WebView view, boolean isDialog,
                                       boolean isUserGesture, Message resultMsg)
        {
            return nativeInterface.onCreateWindow (view, isDialog, isUserGesture, resultMsg);
        }
    }

    static public class WebAppInterface
    {
        private NativeInterface nativeInterface;
        String userScripts;

        //==============================================================================
        public WebAppInterface (NativeInterface nativeInterfaceIn, String userScriptsIn)
        {
            nativeInterface = nativeInterfaceIn;
            userScripts = userScriptsIn;
        }

        //==============================================================================
        @JavascriptInterface
        public String postMessage (String message)
        {
            return nativeInterface.postMessageHandler (message);
        }

        @JavascriptInterface
        public String getAndroidUserScripts()
        {
            return userScripts;
        }
    }

    static public class DrxWebView extends WebView
    {
        private NativeInterface nativeInterface;
        private i64 evaluationId = 0;

        public DrxWebView (Context context, i64 host, String userAgent, String initScripts)
        {
            super (context);

            nativeInterface = new NativeInterface (host);

            WebSettings settings = getSettings();
            settings.setJavaScriptEnabled (true);
            settings.setBuiltInZoomControls (true);
            settings.setDisplayZoomControls (false);
            settings.setSupportMultipleWindows (true);
            settings.setUserAgentString (userAgent);

            setWebChromeClient (new ChromeClient (nativeInterface));

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
            {
                setWebContentsDebuggingEnabled (true);
            }

            setWebViewClient (new Client (nativeInterface));
            addJavascriptInterface (new WebAppInterface (nativeInterface, initScripts), "__JUCE__");
        }

        public z0 disconnectNative()
        {
            nativeInterface.hostDeleted();
        }

        public i64 evaluateJavascript (String script)
        {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
            {
                final i64 currentEvaluationId = evaluationId++;
                final NativeInterface accessibleNativeInterface = nativeInterface;

                super.evaluateJavascript (script, new ValueCallback<String>()
                                                  {
                                                      @Override
                                                      public z0 onReceiveValue(String result)
                                                      {
                                                          accessibleNativeInterface.handleJavascriptEvaluationResult (currentEvaluationId,
                                                                                                                      result);
                                                      }
                                                  });

                return currentEvaluationId;
            }

            return -1;
        }
    }
}
