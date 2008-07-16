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
 * Portions created by the Initial Developer are Copyright (C) 20007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pelle Johnsen <pjohnsen@mozilla.com>
 *   Dave Camp <dcamp@mozilla.com>
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

#include "QMozView.h"
#include "embed.h"

// used for receiving notification from MozView
class QMozViewListener : public MozViewListener
{
public:
  QMozViewListener(QMozView* aQMozView) : pQMozView(aQMozView) {}
  virtual ~QMozViewListener() {}

  void SetTitle(const char* newTitle);
  void StatusChanged(const char* newStatus, PRUint32 statusType);
  void LocationChanged(const char* newLocation);
  MozView* OpenWindow(PRUint32 flags);
  void SizeTo(PRUint32 width, PRUint32 height);

private:
  QMozView* pQMozView;
};

class QMozView::Private
{
public:
  Private(QMozView* aQMozView) : listener(aQMozView) {mozView.SetListener(&listener);}
  MozView mozView;
  QMozViewListener listener;
  QSize preferredSize;
};

void QMozViewListener::SetTitle(const char* newTitle)
{
  pQMozView->titleChanged(QString::fromUtf8(newTitle));
}

void QMozViewListener::StatusChanged(const char* newStatus, PRUint32 statusType)
{
  pQMozView->statusChanged(QString::fromUtf8(newStatus));
}

void QMozViewListener::LocationChanged(const char* newLocation)
{
  pQMozView->locationChanged(QString::fromUtf8(newLocation));
}

MozView* QMozViewListener::OpenWindow(PRUint32 flags)
{
  QMozView* newQMozView = pQMozView->openWindow(flags);
  if(!newQMozView)
    return 0;
  return &(newQMozView->mPrivate->mozView);
}

void QMozViewListener::SizeTo(PRUint32 width, PRUint32 height)
{
  pQMozView->mPrivate->preferredSize = QSize(width, height);
  pQMozView->updateGeometry();
  pQMozView->parentWidget()->adjustSize();
}

QMozView::QMozView(QWidget *parent, unsigned int flags)
: QWidget(parent)
{
  mPrivate = new Private(this);
#ifdef WIN32
  mPrivate->mozView.CreateBrowser((void*)winId(), 0, 0, 100, 100, flags);
#else
  // TODO: Hmmm what if we are not using a mozilla with Qt backend
  mPrivate->mozView.CreateBrowser(this, 0, 0, 0, 0, flags);
#endif
}

QMozView::~QMozView()
{
  delete mPrivate;
}

void QMozView::resizeEvent(QResizeEvent* event)
{
  mPrivate->mozView.SetPositionAndSize(0, 0, size().width(), size().height());
}

void QMozView::loadUri(const QString &uri)
{
  mPrivate->mozView.LoadURI(uri.toUtf8());
}

QMozView* QMozView::openWindow(unsigned int flags)
{
  return 0;
}

QSize QMozView::sizeHint() const
{
  return mPrivate->preferredSize;
}
