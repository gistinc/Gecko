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
 *   Anton Rogaynis <wildriding@gmail.com>
 *   Tatiana Meshkova <tanya.meshkova@gmail.com>
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

#ifndef __test_h_
#define __test_h_

#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsGridLayout>

#include "QMozView.h"
#include "QMozApp.h"

class MyTextWidget : public QGraphicsWidget
{
    Q_OBJECT

public:
    MyTextWidget(const QString& aText, QGraphicsItem* parent = 0)
     : QGraphicsWidget(parent)
     , text(aText)
    {
    }

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        if (text.isEmpty())
            return;

        painter->drawText(boundingRect(), text);
    }

private slots:
    void setText(const QString& aText)
    {
        text = aText;
        update();
    }

private:
    QString text;

};

class MyQGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    MyQGraphicsView(QGraphicsScene* scene, QWidget* parent = 0);
    ~MyQGraphicsView();

    void loadUri(const QString& uri);

protected:
    void resizeEvent(QResizeEvent* event);

private slots:
    void consoleMessage(const QString& message);

private:
    QGraphicsWidget* mForm;
    QGraphicsGridLayout* mLayout;
    QMozView* mozView;

    MyTextWidget* mTitle;
    MyTextWidget* mLocation;
    MyTextWidget* mStatus;
};

#endif /* __test_h_ */
