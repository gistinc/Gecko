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

#include <QApplication>
#include <QDebug>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <nsCOMPtr.h>
#include <nsIIOService.h>
#include <nsNetUtil.h>

#include "test.h"

static nsresult ForceOnline()
{
    nsCOMPtr<nsIIOService> ioService = do_GetService(NS_IOSERVICE_CONTRACTID);
    if (!ioService)
        return NS_ERROR_FAILURE;

    ioService->SetOffline(PR_FALSE);
    return NS_OK;
}

MyQGraphicsView::MyQGraphicsView(QGraphicsScene* scene, QWidget* parent)
 : QGraphicsView(scene, parent)
{
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mLayout = new QGraphicsGridLayout;

    mTitle = new MyTextWidget("title");
    mLayout->addItem(mTitle, 0, 0, 1, 2);
    mLayout->setRowMaximumHeight(0, 20);

    mLocation = new MyTextWidget("location");
    mLayout->addItem(mLocation, 1, 0, 1, 2);
    mLayout->setRowMaximumHeight(1, 20);

    mForm = new QGraphicsWidget;
    mForm->setLayout(mLayout);
    scene->addItem(mForm);

    mozView = new QMozView(mForm);
    mLayout->addItem(mozView, 2, 0, 1, 2);

    mStatus = new MyTextWidget("status");
    mLayout->addItem(mStatus, 3, 0);
    mLayout->setRowMaximumHeight(3, 20);

    QWidget* exitButton = new QPushButton("Exit");
    mLayout->addItem(scene->addWidget(exitButton), 3, 1);
    mLayout->setColumnMaximumWidth(1, 50);

    connect(mozView, SIGNAL(locationChanged(const QString&)),
            mLocation, SLOT(setText(const QString&)));

    connect(mozView, SIGNAL(titleChanged(const QString&)),
            mTitle, SLOT(setText(const QString&)));

    connect(mozView, SIGNAL(statusChanged(const QString&)),
            mStatus, SLOT(setText(const QString&)));

    connect(mozView, SIGNAL(consoleMessage(const QString&)),
            this, SLOT(consoleMessage(const QString&)));

    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));
}

MyQGraphicsView::~MyQGraphicsView()
{
}

void MyQGraphicsView::resizeEvent(QResizeEvent* event)
{
    mForm->resize(event->size());
    QGraphicsView::resizeEvent(event);
}

void MyQGraphicsView::loadUri(const QString& uri)
{
    // ForceOnline() must be called before any network call
    // in order to get connectivity working
    ForceOnline();
    mozView->loadUri(uri);
}

void MyQGraphicsView::consoleMessage(const QString& message)
{
    qDebug() << "CONSOLE:" << message;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QGraphicsScene scene;
    MyQGraphicsView view(&scene);
    if(argc > 1)
        view.loadUri(argv[argc - 1]);
    else
        view.loadUri("http://mozilla.org");

    view.showFullScreen();

    return app.exec();
}
