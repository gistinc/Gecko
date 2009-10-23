/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pelle Johnsen <pjohnsen@mozilla.com>
 *   Dave Camp <dcamp@mozilla.com>
 *   Tobias Hunger <tobias.hunger@gmail.com>
 *   Steffen Imhof <steffen.imhof@googlemail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef MOZEMBED_EMBED_H
#define MOZEMBED_EMBED_H

#include "prtypes.h"

typedef PRUint32 nsresult;

class MozViewListener;
class WindowCreator;

class nsIDOMWindow2;
class nsIDOMWindowInternal;
class nsIInterfaceRequestor;
class nsIWebNavigation;

/**
 * Provides an interface to application level functionality,
 * i.e. that is shared across all views.
 */
class MozApp
{
public:
    /**
     * Constructor
     *
     * @param aProfilePath used to set a path to where all
     *  profile data are stored. By default a subdir named
     *  mozembed of the application dir (where the executable
     *  recides). <b>NOTE:</b> Must be set before any MozView
     *  is created, otherwise the default is used.
     *  @param aEmbedPath is an optional path to the xpcom.dll
     *  and is used when we are embedded.
     */
    MozApp(const char* aProfilePath = 0, const char* aEmbedPath = 0);

    /**
     * Destructor.
     */
    virtual ~MozApp();

    /**
     * Set a string preference.
     *
     * @param aName the preference's name
     * @param aValue the new value to be set
     * @return 0 on success
     */
    nsresult SetCharPref(const char *aName, const char *aValue);

    /**
     * Set a boolean preference.
     *
     * @param aName the preference's name
     * @param aValue the new value to be set
     * @return 0 on success
     */
    nsresult SetBoolPref(const char *aName, PRBool aValue);

    /**
     * Set an integer preference.
     *
     * @param aName the preference's name
     * @param aValue the new value to be set
     * @return 0 on success
     */
    nsresult SetIntPref(const char *aName, int aValue);

    /**
     * Get a string preference.
     *
     * @param aName the preference's name
     * @param aValue the result is stored here.
     *  <b>NOTE:</b> This is allocated on the heap and it is the caller's
     *  responsibility to free it.
     * @return 0 on success
     */
    nsresult GetCharPref(const char *aName, char **aValue);

    /**
     * Get a boolean preference.
     *
     * @param aName the preference's name
     * @param aValue the result is stored here.
     * @return 0 on success
     */
    nsresult GetBoolPref(const char *aName, PRBool *aValue);

    /**
     * Get an integer preference.
     *
     * @param aName the preference's name
     * @param aValue the result is stored here.
     * @return 0 on success
     */
    nsresult GetIntPref(const char *aName, int *aValue);

 private:
    class Private;
    Private *mPrivate;
};

/**
 * Class for viewing web content.
 * Should be attached to a native window/widget.
 */
class MozView
{
public:
    /**
     * Constructor.
     */
    MozView();

    /**
     * Destructor.
     */
    virtual ~MozView();

    /**
     * Creates a new browser view attached to a native window.
     *
     * @param aNativeWindow pointer or handle to the native window
     * @param aX position relative to native window
     * @param aY position relative to native window
     * @param aWidth size
     * @param aHeight size
     * @param aChromeFlags optional flags, e.g. those parsed in to
     *  MozViewListener::OpenWindow
     * @return 0 on success
     */
    nsresult CreateBrowser(void* aNativeWindow, PRInt32 aX, PRInt32 aY,
        PRInt32 aWidth, PRInt32 aHeight, PRUint32 aChromeFlags = 0);

    /**
     * Sets location and dimension of the browser window.
     * Call this when resizing.
     *
     * @param aX position relative to native window
     * @param aY position relative to native window
     * @param aWidth size
     * @param aHeight size
     * @return 0 on success
     */
    nsresult SetPositionAndSize(PRInt32 aX, PRInt32 aY,
        PRInt32 aWidth, PRInt32 aHeight);

    /**
     * Load content from a specified uri.
     *
     * @param aUri is treated as utf8
     * @return 0 on success
     */
    nsresult LoadURI(const char* aUri);

    /**
     * Load content from memory.
     *
     * @param aBaseUrl base used for resolving links in the content
     * @param aContentType mime type
     * @param aData the actual content
     * @param aLen length in bytes
     * @return 0 on success
     */
    nsresult LoadData(const char    *aBaseUrl,
                      const char    *aContentType,
                      const PRUint8 *aData,
                      PRUint32       aLen);

    /**
     * Stops any loading or ongoing processing.
     *
     * @return 0 on success
     */
    nsresult Stop();

    /**
     * Reloads the current content from its original source.
     *
     * @return 0 on success
     */
    nsresult Reload();

    /**
     * Navigate to the previous item in the session history.
     *
     * @return 0 on success
     */
    nsresult GoBack();

    /**
     * Navigates to the next item in the session history.
     *
     * @return 0 on success
     */
    nsresult GoForward();

    /**
     * Indicates if the browser can go back.
     *
     * @return true if the browser can go back, false if not
     */
    PRBool CanGoBack();

    /**
     * Indicates if the browser can go forward.
     *
     * @return true if the browser can go forward, false if not
     */
    PRBool CanGoForward();

    /**
     * Change focus for the browser view.
     *
     * @param aFocus true to give fous, false to take it away
     * @return 0 on success
     */
    nsresult SetFocus(PRBool aFocus);

    /**
     * Make the browser view visible.
     */
    void Show();

    /**
     * Hide the browser view.
     */
    void Hide();

    /**
     * Register a listener with the browser view.
     * The listener's methods will be called by the browser
     * when various events occur.
     *
     * @param aNewListener pointer to the listener
     */
    void SetListener(MozViewListener* aNewListener);

    /**
     * Get the currently registered listener.
     *
     * @return pointer to the listener.
     */
    MozViewListener* GetListener();

    /**
     * Get parent window of the browser view.
     * I.e. the window provided by the embedding app.
     *
     * @return native pointer/handle to parent window
     */
    void* GetParentWindow();

    /**
     * Get native window of the browser view.
     * I.e. the window used by the browser itself.
     *
     * @return native pointer/handle to parent window
     */
    void *GetNativeWindow();

    /**
     * Set the parent view, i.e. the view which opened this view.
     *
     * @param aParent pointer to the parent view.
     */
    void SetParentView(MozView* aParent);

    /**
     * Get the parent view, i.e. the view which openedt this view.
     *
     * @return pointer to the parent view or null if no parent.
     */
    MozView* GetParentView();

    /**
     * Get an InterfaceRequestor for accessing the underlying XPCOM API.
     * Note: implemented using out parameter to avoid including or declaring
     * already_AddRefed
     *
     * @param aRequestor out parameter. Note: it is the callers responsibility
     *  to release this pointer.
     * @return NS_OK on success
     */
    nsresult GetInterfaceRequestor(nsIInterfaceRequestor** aRequestor);

    /**
     * Get the browser object of this view.
     *
     * @return A (void-)pointer to the browser object.
     */
    void * GetBrowser();

    /**
     * Convenience method to get the nsIDOMWindow2.
     *
     * @return A pointer to the DOMWindow interface.
     */
    nsIDOMWindow2 * GetDOMWindow();

    /**
     * Convenience method to get the nsIWebNavigation.
     *
     * @return A pointer to the webnavigation interface.
     */
    nsIWebNavigation * GetNavigation();

    /**
     * Find text in the current web page.
     * @param aSubString the (sub-)string to search for.
     * @param aCaseSensitive Shall we be case sensitive (default: no)?
     * @param aWrap Shall we wrap around at the end of the document
     *              (default: no)?
     * @param aEntireWord Does the search string need to match the entire
     *                    word (default: no)?
     * @param aBackwards Should we search backwards (default: no)?
     * @return true if the text was found and false otherwise.
     */
    bool FindText(const PRUnichar * aSubString,
                  bool aCaseSensitive = false, bool aWrap = false,
                  bool aEntireWord = false, bool aBackwards = false);

    /**
     * Evaluate JavaScript
     *
     * @param aScript the script to evaluate in utf-8
     * @return return value from JavaScript as a string in utf-8
     *  <b>NOTE:</b> this buffer must be freed by he caller using delete[]
     */
    char* EvaluateJavaScript(const char* aScript);

private:
    class Private;
    Private *mPrivate;

    friend class WindowCreator;
};

/**
 * This is the callback interface to the embedding app.
 * The app can subclass this and override methods that will
 * be called as appropriate.
 *
 * Use MozView::SetListener to regster the listener
 *
 * MozViewListener implements noop defaults, so the app only
 * needs to override the methods it uses.
 */
class MozViewListener
{
public:
    /**
     * Constructor.
     */
    MozViewListener();

    /**
     * Destructor.
     */
    virtual ~MozViewListener();

    /**
     * Set the MozView his listener is attached to.
     *
     * @param aMozView pointer to the MozView
     */
    void SetMozView(MozView* aMozView);

    // --- methods the embedding app can override ---
    /**
     * Informs the application to set its title
     *
     * @param aNewTitle (utf8) title of the document.
     */
    virtual void SetTitle(const char* aNewTitle);

    /**
     * Informs the application to change its status message
     *
     * @param aNewStatus (utf8)
     * @param aStatusType <TBD> (should tell if it's caused by
     *  hovering over a link, or by JavaScript.
     */
    virtual void StatusChanged(const char* aNewStatus, PRUint32 aStatusType);

    /**
     * Informs the application that the location of the browser
     * has changed.
     *
     * @param aNewLocation (utf8)
     */
    virtual void LocationChanged(const char* aNewLocation);

    /**
     * Called before new content is loaded to allow the application
     * to deny loading.
     *
     * @param aNewLocation the requested uri
     * @return true to abort the load, false to allow it.
     */
    virtual PRBool OpenURI(const char* aNewLocation);

    /**
     * Informs the application that the document has completed
     * loading.
     */
    virtual void DocumentLoaded();

    /**
     * Called when the browser needs to open a new window.
     * This happens e.g. if a link has a differnet target,
     * or from JavaScript window.open().
     *
     * @param aFlags various flags for the new window, see
     *  <a href="http://developer.mozilla.org/en/docs/nsIWebBrowserChrome#Constants">
     *  chromeFlags</a>. These should also be passed on to
     *  MozView::CreateBrowser
     * @return Pointer to a newly created MozView or 0 to
     *  disallow openning of the new window.
     *  <b>NOTE:</b> It is the application's responsibility
     *  to manage this pointer, i.e. free the MozView object
     *  when no longer needed.
     */
    virtual MozView* OpenWindow(PRUint32 aFlags);

    /**
     * Ask to change the size of the bowser, and therefore the
     * enclosing window.
     *
     * @param aWidth in pixels
     * @param aHeight in pixels
     */
    virtual void SizeTo(PRUint32 aWidth, PRUint32 aHeight);

    /**
     * Ask to set the visibility of the brower window.
     *
     * @param aVisible true to show, false to hide
     */
    virtual void SetVisibility(PRBool aVisible);

    /**
     * Make the corresponding window behave as modal.
     */
    virtual void StartModal();

    /**
     * Exit a modal window.
     *
     * @param aResult as returned from the dialog.
     */
    virtual void ExitModal(nsresult aResult);

    /**
     * Inform the application about console messages (JS errors and such).
     *
     * @param aMessage the new console Message.
     */
    virtual void OnConsoleMessage(const char * aMessage);

    /**
     * The focus entered or left the browser window.
     *
     * @param    aForward if PR_TRUE, the next sibling shall be focused (Tab), otherwise the
     *                    previous sibling is to be focused (Shift-Tab)
     */
    virtual void OnFocusChanged(PRBool aForward);

protected:
    MozView* mMozView;
};

#endif /* Header guard */
