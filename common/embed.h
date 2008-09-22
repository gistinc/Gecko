#ifndef __embed_h_
#define __embed_h_

#include "prtypes.h"

// XXX
typedef PRUint32 nsresult;

class MozViewListener;
class WindowCreator;

class nsIInterfaceRequestor;

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
   */
  MozApp(const char* aProfilePath = 0);

  /**
   * Destructor.
   */
  virtual ~MozApp();

  /**
   * Set a string preference.
   *
   * @param name the preference's name
   * @param value the new value to be set
   * @return 0 on success
   */
  nsresult SetCharPref(const char *name, const char *value);

  /**
   * Set a boolean preference.
   *
   * @param name the preference's name
   * @param value the new value to be set
   * @return 0 on success
   */
  nsresult SetBoolPref(const char *name, PRBool value);

  /**
   * Set an integer preference.
   *
   * @param name the preference's name
   * @param value the new value to be set
   * @return 0 on success
   */
  nsresult SetIntPref(const char *name, int value);

  /**
   * Get a string preference.
   *
   * @param name the preference's name
   * @param value the result is stored here.
   *  <b>NOTE:</b> This is allocated on the heap and it is the caller's
   *  responsibility to free it.
   * @return 0 on success
   */
  nsresult GetCharPref(const char *name, char **value);

  /**
   * Get a boolean preference.
   *
   * @param name the preference's name
   * @param value the result is stored here.
   * @return 0 on success
   */
  nsresult GetBoolPref(const char *name, PRBool *value);

  /**
   * Get an integer preference.
   *
   * @param name the preference's name
   * @param value the result is stored here.
   * @return 0 on success
   */
  nsresult GetIntPref(const char *name, int *value);

 private:
  class Private;
  Private *mPrivate;
};

/**
 * Class for viewing web content.
 * Should be attached to a native windwow/widget.
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
   * @param x position relative to native window
   * @param y position relative to native window
   * @param width size
   * @param height size
   * @param chromeFlags optional flags, e.g. those parsed in to
   *  MozViewListener::OpenWindow
   * @return 0 on success
   */
  nsresult CreateBrowser(void* aNativeWindow, PRInt32 x, PRInt32 y,
    PRInt32 width, PRInt32 height, PRUint32 chromeFlags = 0);

  /**
   * Sets location and dimension of the browser window.
   * Call this when resizing.
   *
   * @param x position relative to native window
   * @param y position relative to native window
   * @return 0 on success
   */
  nsresult SetPositionAndSize(PRInt32 x, PRInt32 y,
    PRInt32 width, PRInt32 height);

  /**
   * Load content from a specified uri.
   *
   * @param uri is treated as utf8
   * @return 0 on success
   */
  nsresult LoadURI(const char* uri);

  /**
   * Load content from memory.
   *
   * @param base_url base used for resolving links in the content
   * @param content_type mime type
   * @param data the actual content
   * @param len length in bytes
   * @return 0 on success
   */
  nsresult LoadData(const char    *base_url,
		    const char    *content_type,
		    const PRUint8 *data,
		    PRUint32       len);

  /**
   * Stops any loading or ongoing processing.
   *
   * @return 0 on success
   */
  nsresult Stop();

  /**
   * Reloads the current content from it's original source.
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
   * @param focus true to give fous, false to take it away
   * @return 0 on success
   */
  nsresult SetFocus(PRBool focus);

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
   * @param pNewListener pointer to the listener
   */
  void SetListener(MozViewListener* pNewListener);

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
   * @param parent pointer to the parent view.
   */
  void SetParentView(MozView* parent);

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
   * @param pAMozView pointer to the MozView
   */
  void SetMozView(MozView* pAMozView);

  // --- methods the embedding app can override ---
  /**
   * Informs the application to set it's title
   *
   * @param newTitle (utf8) title of the document.
   */
  virtual void SetTitle(const char* newTitle);

  /**
   * Informs the application to change it's status message
   *
   * @param newStatus (utf8)
   * @param statusType <TBD> (should tell if it's caused by
   *  hovering over a link, or by JavaScript.
   */
  virtual void StatusChanged(const char* newStatus, PRUint32 statusType);

  /**
   * Informs the application that the location of the browser
   * has changed.
   *
   * @param newLocation (utf8)
   */
  virtual void LocationChanged(const char* newLocation);

  /**
   * Called before new content is loaded to allow the application
   * to deny loading.
   *
   * @param newLocation the requested uri
   * @return true to abort the load, false to allow it.
   */
  virtual PRBool OpenURI(const char* newLocation);

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
   * @param flags various flags for the new window, see
   *  <a href="http://developer.mozilla.org/en/docs/nsIWebBrowserChrome#Constants">
   *  chromeFlags</a>. These should also be passed on to
   *  MozView::CreateBrowser
   * @return Pointer to a newly created MozView or 0 to
   *  disallow openning of the new window.
   *  <b>NOTE:</b> It is the applications responsibility
   *  to manage this pointer, i.e. free the MozView object
   *  when no longer needed.
   */
  virtual MozView* OpenWindow(PRUint32 flags);

  /**
   * Ask to change the size of the bowser, and therefore the
   * enclosing window.
   *
   * @param width in pixels
   * @param height in pixels
   */
  virtual void SizeTo(PRUint32 width, PRUint32 height);

  /**
   * Ask to set the visibility of the brower window.
   *
   * @param visible true to show, false to hide
   */
  virtual void SetVisibility(PRBool visible);

  /**
   * Make the corresponding window behave as modal.
   */
  virtual void StartModal();

  /**
   * Exit a modal window.
   *
   * @param result as returned from the dialog.
   */
  virtual void ExitModal(nsresult result);

protected:
  MozView* pMozView;
};

#endif /* __embed_h_ */

