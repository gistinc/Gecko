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

#include "moz-web-view.h"
#include <gtk/gtk.h>

GtkWidget *g_notebook;
GtkWidget *g_entry;
GtkWidget *g_statusbar;

void
title_cb(MozWebView *view, const char *title, gpointer user_data)
{
    gtk_label_set_text(GTK_LABEL(user_data), title);
}

void
location_cb(MozWebView *view, const char *uri, gpointer user_data)
{
    gtk_entry_set_text(GTK_ENTRY(g_entry), uri);
}

void
status_cb(MozWebView *view, const char *status, gpointer user_data)
{
    gtk_statusbar_push(GTK_STATUSBAR(g_statusbar), 0, status);
}

void
activate_cb(GtkWidget *widget, gpointer user_data)
{
    const char *uri = gtk_entry_get_text(GTK_ENTRY(widget));

    int id = gtk_notebook_get_current_page(GTK_NOTEBOOK(g_notebook));
    GtkWidget *view = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), id);
    moz_web_view_load_uri(MOZ_WEB_VIEW(view), uri);
}

void
close_cb(GtkWidget *button, GtkWidget *view)
{
    int page = gtk_notebook_page_num(GTK_NOTEBOOK(g_notebook), view);
    gtk_notebook_remove_page(GTK_NOTEBOOK(g_notebook), page);
}

GtkWidget *
add_page(GtkNotebook *notebook)
{
    GtkWidget *view = moz_web_view_new();
    GtkWidget *box = gtk_hbox_new(FALSE, 5);
    GtkWidget *tab_label = gtk_label_new("Loading...");
    GtkWidget *close_button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);

    GtkWidget *close = gtk_image_new_from_stock(GTK_STOCK_CLOSE,
                                                GTK_ICON_SIZE_MENU);
    gtk_container_add(GTK_CONTAINER(close_button), close);

    gtk_box_pack_start(GTK_BOX(box), tab_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(box), close_button, FALSE, FALSE, 0);

    gtk_widget_show_all(box);
    gtk_widget_show(view);

    g_signal_connect(view, "title-changed", G_CALLBACK(title_cb), tab_label);
    g_signal_connect(view, "location-changed", G_CALLBACK(location_cb), NULL);
    g_signal_connect(view, "status-changed", G_CALLBACK(status_cb), NULL);
    g_signal_connect(close_button, "clicked", G_CALLBACK(close_cb), view);

    moz_web_view_load_uri(MOZ_WEB_VIEW(view), "http://www.google.com");

    int page = gtk_notebook_append_page(notebook, view, box);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page);

    return view;
}

void
new_tab_cb(GtkToolItem *item, void *user_data)
{
    add_page(GTK_NOTEBOOK(g_notebook));
}

int
main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete_event", gtk_main_quit, NULL);

    GtkWidget *box = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, FALSE, 0);

    g_notebook = gtk_notebook_new();

    g_entry = gtk_entry_new();
    g_signal_connect(g_entry, "activate", G_CALLBACK(activate_cb), g_notebook);

    GtkToolItem *toolitem = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(toolitem), g_entry);
    gtk_tool_item_set_expand(toolitem, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, 0);

    toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem), "New Tab");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, 1);
    g_signal_connect(toolitem, "clicked", G_CALLBACK(new_tab_cb), g_notebook);

    gtk_box_pack_start(GTK_BOX(box), g_notebook, TRUE, TRUE, 0);

    g_statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(box), g_statusbar, FALSE, FALSE, 0);

    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

    add_page(GTK_NOTEBOOK(g_notebook));

    gtk_widget_show_all(window);

    gtk_main();
}
