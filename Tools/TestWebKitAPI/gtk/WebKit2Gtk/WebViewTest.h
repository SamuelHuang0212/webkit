/*
 * Copyright (C) 2011 Igalia S.L.
 * Portions Copyright (c) 2011 Motorola Mobility, Inc.  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef WebViewTest_h
#define WebViewTest_h

#include "TestMain.h"
#include <webkit2/webkit2.h>
#include <wtf/text/CString.h>

class WebViewTest: public Test {
public:
    MAKE_GLIB_TEST_FIXTURE(WebViewTest);
    WebViewTest(WebKitUserContentManager* = nullptr);
    virtual ~WebViewTest();

    virtual void loadURI(const char* uri);
    virtual void loadHtml(const char* html, const char* baseURI);
    virtual void loadPlainText(const char* plainText);
    virtual void loadRequest(WebKitURIRequest*);
    virtual void loadBytes(GBytes*, const char* mimeType, const char* encoding, const char* baseURI);
    void loadAlternateHTML(const char* html, const char* contentURI, const char* baseURI);
    void goBack();
    void goForward();
    void goToBackForwardListItem(WebKitBackForwardListItem*);

    void quitMainLoop();
    void quitMainLoopAfterProcessingPendingEvents();
    void wait(double seconds);
    void waitUntilLoadFinished();
    void waitUntilTitleChangedTo(const char* expectedTitle);
    void waitUntilTitleChanged();
    void showInWindow(GtkWindowType = GTK_WINDOW_POPUP);
    void showInWindowAndWaitUntilMapped(GtkWindowType = GTK_WINDOW_POPUP, int width = 0, int height = 0);
    void resizeView(int width, int height);
    void selectAll();
    const char* mainResourceData(size_t& mainResourceDataSize);

    bool isEditable();
    void setEditable(bool);

    void mouseMoveTo(int x, int y, unsigned mouseModifiers = 0);
    void clickMouseButton(int x, int y, unsigned button = 1, unsigned mouseModifiers = 0);
    void keyStroke(unsigned keyVal, unsigned keyModifiers = 0);

    WebKitJavascriptResult* runJavaScriptAndWaitUntilFinished(const char* javascript, GError**);
    WebKitJavascriptResult* runJavaScriptFromGResourceAndWaitUntilFinished(const char* resource, GError**);

    // Javascript result helpers.
    static char* javascriptResultToCString(WebKitJavascriptResult*);
    static double javascriptResultToNumber(WebKitJavascriptResult*);
    static bool javascriptResultToBoolean(WebKitJavascriptResult*);
    static bool javascriptResultIsNull(WebKitJavascriptResult*);
    static bool javascriptResultIsUndefined(WebKitJavascriptResult*);

    cairo_surface_t* getSnapshotAndWaitUntilReady(WebKitSnapshotRegion, WebKitSnapshotOptions);

    bool runWebProcessTest(const char* suiteName, const char* testName);

    // Prohibit overrides because this is called when the web view is created
    // in our constructor, before a derived class's vtable is ready.
    void initializeWebExtensions() override final { Test::initializeWebExtensions(); }

    WebKitWebView* m_webView;
    GMainLoop* m_mainLoop;
    CString m_activeURI;
    GtkWidget* m_parentWindow;
    CString m_expectedTitle;
    WebKitJavascriptResult* m_javascriptResult;
    GError** m_javascriptError;
    GUniquePtr<char> m_resourceData;
    size_t m_resourceDataSize;
    cairo_surface_t* m_surface;

private:
    void doMouseButtonEvent(GdkEventType, int, int, unsigned, unsigned);
};

#endif // WebViewTest_h
