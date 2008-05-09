#ifndef __embed_h_
#define __embed_h_

#include "prtypes.h"

// XXX
typedef PRUint32 nsresult;

class EmbedListener;

class MozEmbed
{
public:
    MozEmbed();
    virtual ~MozEmbed();
    nsresult CreateBrowser(void* aNativeWindow, PRInt32 x, PRInt32 y,
        PRInt32 width, PRInt32 height);
    nsresult SetPositionAndSize(PRInt32 x, PRInt32 y,
        PRInt32 width, PRInt32 height);
    nsresult LoadURI(const char* uri);
    nsresult SetFocus(PRBool focus);

    void SetListener(EmbedListener* pNewListener);
    EmbedListener* GetListener();

    void* GetNativeWindow();

private:
    class Private;
    Private *mPrivate;

    nsresult InitEmbedding();};

/**
 * This is the callback interface to the embeddig app.
 * The app can subclass this and override methods that will
 * be called as appropriate.
 *
 * Use MozEmbed::SetListener to regster the listener
 *
 * EmbedListener implements noop defaults, so the app only
 * needs to override the methods it uses.
 */
class EmbedListener
{
public:
    EmbedListener();
    virtual ~EmbedListener();
    void SetMozEmbed(MozEmbed* pAMozEmbed);

    // methods the embedding app can override
    virtual void SetTitle(const char* newTitle);

protected:
    MozEmbed* pMozEmbed;
};

#endif /* __embed_h_ */

