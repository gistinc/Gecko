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
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QUrl>
#include <QDebug>

#include "test.h"
#include "QMozApp.h"

MyQMozView::MyQMozView(QWidget *parent, unsigned int flags)
: QMozView(parent, flags)
{}

QMozView* MyQMozView::openWindow(unsigned int flags)
{
    MyBrowser* newBrowser = new MyBrowser(0, flags);
    newBrowser->resize(400, 400);
    newBrowser->show();
    newBrowser->setAttribute(Qt::WA_DeleteOnClose);
    return newBrowser->getQMozView();
}

MyBrowser::MyBrowser(QWidget *parent, unsigned int flags)
: QDialog(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    location = new QLineEdit(this);
    layout->addWidget(location);

    mozView = new MyQMozView(this, flags);
    layout->addWidget(mozView, 1);

    status = new QLabel(this);
    layout->addWidget(status);

    connect(mozView, SIGNAL(locationChanged(const QString&)),
            location, SLOT(setText(const QString&)));

    connect(mozView, SIGNAL(titleChanged(const QString&)),
            this, SLOT(setWindowTitle(const QString&)));

    connect(mozView, SIGNAL(statusChanged(const QString&)),
            status, SLOT(setText(const QString&)));

    connect(mozView, SIGNAL(startModal()),
            this, SLOT(startModal()));

    connect(mozView, SIGNAL(exitModal()),
            this, SLOT(exitModal()));

    connect(location, SIGNAL(returnPressed()),
            this, SLOT(go()));

}

void MyBrowser::loadUri(const QString& uri)
{
    location->setText(uri);
    mozView->loadUri(uri);
}

void MyBrowser::go()
{
    mozView->loadUri(location->text());
}

void MyBrowser::startModal()
{
    hide();
    exec();
}

void MyBrowser::exitModal()
{
    done(0);
    // have to delete mozView now to avoid JS context assertions
    delete mozView;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MyBrowser window;

    window.resize(400, 400);
    window.show();

    if(argc > 1)
        window.loadUri(argv[argc - 1]);
    else
        window.loadUri("http://mozilla.org");

    return app.exec();
}
