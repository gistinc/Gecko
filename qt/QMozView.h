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

#ifndef QMOZEMBED_QMOZVIEW_H
#define QMOZEMBED_QMOZVIEW_H

#include "QMozEmbedExport.h"

#include <QtGui/QWidget>
#include <QtGui/QGraphicsWidget>

class QMozViewListener;
class nsIInterfaceRequestor;

class Q_MOZEMBED_EXPORT QMozView : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit QMozView(QGraphicsWidget *parent = 0, unsigned int flags = 0);
    virtual ~QMozView();

    void loadUri(const QString& uri);
    void getInterfaceRequestor(nsIInterfaceRequestor** aRequestor);
    QString evaluateJavaScript(const QString& script);

    virtual QSize sizeHint() const;

    bool findText(const QString & sub_string,
                  bool case_sensitive = false, bool wrap = false,
                  bool entire_word = false, bool backwards = false) const;

Q_SIGNALS:
    void locationChanged(const QString& newUri);
    void titleChanged(const QString& newTitle);
    void statusChanged(const QString& newStatus);
    void consoleMessage(const QString & message);
    void startModal();
    void exitModal();

protected:
    virtual void resizeEvent(QGraphicsSceneResizeEvent* event);

    virtual QMozView* openWindow(unsigned int flags);

private:
    class Private;
    Private * const mPrivate;

    friend class QMozViewListener;
};

#endif /* Header guard */

