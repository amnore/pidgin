/* pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301 USA
 */

#ifndef PIDGIN_ACCOUNT_CHOOSER_H
#define PIDGIN_ACCOUNT_CHOOSER_H
/**
 * SECTION:pidgin-account-chooser
 * @section_id: pidgin-account-chooser
 * @short_description: <filename>pidginaccountchooser.h</filename>
 * @title: Pidgin Account Chooser Widget
 */

#include "pidgin.h"

#include "account.h"

G_BEGIN_DECLS

/**
 * pidgin_account_option_menu_new:
 * @default_account: The account to select by default.
 * @show_all: Whether or not to show all accounts, or just
 *            active accounts.
 * @cb: (scope call): The callback to call when an account is selected.
 * @filter_func: (scope call): A function for checking if an account should
 *               be shown. This can be NULL.
 * @user_data: Data to pass to the callback function.
 *
 * Creates a drop-down option menu filled with accounts.
 *
 * Returns: (transfer full): The drop-down option menu.
 */
GtkWidget *pidgin_account_option_menu_new(PurpleAccount *default_account,
                                          gboolean show_all, GCallback cb,
                                          PurpleFilterAccountFunc filter_func,
                                          gpointer user_data);

/**
 * pidgin_account_option_menu_get_selected:
 * @optmenu: The drop-down option menu created by
 *        pidgin_account_option_menu_new.
 *
 * Gets the currently selected account from an account drop down box.
 *
 * Returns: (transfer none): Returns the PurpleAccount that is currently
 *          selected.
 */
PurpleAccount *pidgin_account_option_menu_get_selected(GtkWidget *optmenu);

/**
 * pidgin_account_option_menu_set_selected:
 * @optmenu: The GtkOptionMenu created by
 *        pidgin_account_option_menu_new.
 * @account: The PurpleAccount to select.
 *
 * Sets the currently selected account for an account drop down box.
 */
void pidgin_account_option_menu_set_selected(GtkWidget *optmenu,
                                             PurpleAccount *account);

G_END_DECLS

#endif /* PIDGIN_ACCOUNT_CHOOSER_H */
