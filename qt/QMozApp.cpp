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
 *   Tobias Hunger <tobias.hunger@gmail.com>
 *   Steffen Imhof <steffen.imhof@googlemail.com>
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

#include "QMozApp.h"
#include "embed.h"
#include <stdlib.h>

class QMozApp::Private
{
public:
    Private(const char* aProfilePath = 0) : mozApp(aProfilePath) {}
    MozApp mozApp;
};

QMozApp::QMozApp(const QString& profilePath) :
    mPrivate(new Private(profilePath.toUtf8().data()))
{
}

QMozApp::~QMozApp()
{
    delete mPrivate;
}

QString QMozApp::stringPref(const QString& name) const
{
    char* value = 0;
    mPrivate->mozApp.GetCharPref(name.toUtf8(), &value);
    if (!value) { return QString(); }

    QString result(QString::fromUtf8(value,-1));
    free(value); // value is malloc'ed, so this is right!
    return result;
}

void QMozApp::setStringPref(const QString& name, const QString& value)
{
    mPrivate->mozApp.SetCharPref(name.toUtf8(), value.toUtf8());
}

int QMozApp::intPref(const QString& name) const
{
    int result = 0;
    mPrivate->mozApp.GetIntPref(name.toUtf8(), &result);
    return result;
}

void QMozApp::setIntPref(const QString& name, int value)
{
    mPrivate->mozApp.SetIntPref(name.toUtf8(), value);
}

bool QMozApp::boolPref(const QString& name) const
{
    PRBool result = 0;
    mPrivate->mozApp.GetBoolPref(name.toUtf8(), &result);
    return result;
}

void QMozApp::setBoolPref(const QString& name, bool value)
{
    mPrivate->mozApp.SetBoolPref(name.toUtf8(), value);
}
