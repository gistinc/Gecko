/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Pioneer Research Center USA, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; version 2.1 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __ContentListener__
#define __ContentListener__

#include "nsIURIContentListener.h"
#include "nsWeakReference.h"
#include "nsIWebNavigation.h"
#include "embed.h"

class ContentListener : public nsIURIContentListener,
			public nsSupportsWeakReference
{
 public:

  ContentListener(MozView *aOwner, nsIWebNavigation *aNavigation);
  virtual ~ContentListener();

  NS_DECL_ISUPPORTS

  NS_DECL_NSIURICONTENTLISTENER

 private:

  MozView *mOwner;
  nsCOMPtr<nsIWebNavigation> mNavigation;
};

#endif /* __ContentListener__ */
