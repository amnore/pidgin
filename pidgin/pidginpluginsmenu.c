/*
 * pidgin
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include "pidginpluginsmenu.h"

#include <gplugin.h>

#include <purple.h>

struct _PidginPluginsMenu {
	GtkMenu parent;

	GtkWidget *separator;

	GSimpleActionGroup *action_group;

	GHashTable *plugin_items;
};

#define PIDGIN_PLUGINS_MENU_ACTION_PREFIX "plugins-menu"

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_plugins_menu_action_activated(GSimpleAction *simple, GVariant *parameter,
                                     gpointer data)
{
	PurplePluginAction *action = (PurplePluginAction *)data;

	if(action != NULL && action->callback != NULL) {
		action->callback(action);
	}
}

static void
pidgin_plugins_menu_add_plugin_actions(PidginPluginsMenu *menu,
                                       PurplePlugin *plugin)
{
	GPluginPluginInfo *info = NULL;
	PurplePluginActionsCb actions_cb = NULL;
	GList *actions = NULL;
	GtkWidget *submenu = NULL, *item = NULL;
	gint i = 0;

	info = gplugin_plugin_get_info(GPLUGIN_PLUGIN(plugin));

	actions_cb = purple_plugin_info_get_actions_cb(PURPLE_PLUGIN_INFO(info));
	if(actions_cb == NULL) {
		g_object_unref(G_OBJECT(info));

		return;
	}

	actions = actions_cb(plugin);
	if(actions == NULL) {
		g_object_unref(G_OBJECT(info));

		return;
	}

	submenu = gtk_menu_new();

	for(i = 0; actions != NULL; i++) {
		PurplePluginAction *action = NULL;
		GSimpleAction *gaction = NULL;
		GtkWidget *action_item = NULL;
		gchar *action_base_name = NULL;
		gchar *action_full_name = NULL;

		action = (PurplePluginAction *)actions->data;
		if(action == NULL) {
			action_item = gtk_separator_menu_item_new();
			gtk_widget_show(action_item);
			gtk_menu_shell_append(GTK_MENU_SHELL(submenu), action_item);

			actions = g_list_delete_link(actions, actions);

			continue;
		}

		if(action->label == NULL) {
			actions = g_list_delete_link(actions, actions);

			g_warn_if_reached();

			continue;
		}

		action_base_name = g_strdup_printf("%s-%d",
		                                   gplugin_plugin_info_get_id(info),
		                                   i);
		action_full_name = g_strdup_printf("%s.%s",
		                                   PIDGIN_PLUGINS_MENU_ACTION_PREFIX,
		                                   action_base_name);

		/* create the menu item with the full action name */
		action_item = gtk_menu_item_new_with_label(action->label);
		gtk_actionable_set_action_name(GTK_ACTIONABLE(action_item),
		                               action_full_name);
		gtk_widget_show(action_item);
		g_free(action_full_name);

		/* add our action item to the menu */
		gtk_menu_shell_append(GTK_MENU_SHELL(submenu), action_item);

		/* now create the gaction with the base name */
		gaction = g_simple_action_new(action_base_name, NULL);
		g_free(action_base_name);

		/* now connect to the activate signal of the action using
		 * g_signal_connect_data with a destroy notify to free the plugin action
		 * when the signal handler is removed.
		 */
		g_signal_connect_data(G_OBJECT(gaction), "activate",
		                      G_CALLBACK(pidgin_plugins_menu_action_activated),
		                      action,
		                      (GClosureNotify)purple_plugin_action_free,
		                      0);

		/* finally add the action to the action group and remove our ref */
		g_action_map_add_action(G_ACTION_MAP(menu->action_group),
		                        G_ACTION(gaction));
		g_object_unref(G_OBJECT(gaction));

		actions = g_list_delete_link(actions, actions);
	}

	item = gtk_menu_item_new_with_label(gplugin_plugin_info_get_name(info));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
	gtk_widget_show(item);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	g_hash_table_insert(menu->plugin_items,
	                    g_object_ref(G_OBJECT(plugin)),
	                    item);

	g_object_unref(G_OBJECT(info));

	/* make sure that our separator is visible */
	gtk_widget_show(menu->separator);
}

static void
pidgin_plugins_menu_remove_plugin_actions(PidginPluginsMenu *menu,
                                          PurplePlugin *plugin)
{
	GPluginPluginInfo *info = NULL;
	PurplePluginActionsCb actions_cb = NULL;
	GList *actions = NULL;
	gint i = 0;

	/* try remove the menu item from plugin from the hash table.  If we didn't
	 * remove anything, we have nothing to do so bail.
	 */
	if(!g_hash_table_remove(menu->plugin_items, plugin)) {
		return;
	}

	info = gplugin_plugin_get_info(GPLUGIN_PLUGIN(plugin));

	actions_cb = purple_plugin_info_get_actions_cb(PURPLE_PLUGIN_INFO(info));
	if(actions_cb == NULL) {
		g_object_unref(G_OBJECT(info));

		return;
	}

	actions = actions_cb(plugin);
	if(actions == NULL) {
		g_object_unref(G_OBJECT(info));

		return;
	}

	/* now walk through the actions and remove them from the action group. */
	for(i = 0; actions != NULL; i++) {
		gchar *name = NULL;

		name = g_strdup_printf("%s-%d", gplugin_plugin_info_get_id(info), i);

		g_action_map_remove_action(G_ACTION_MAP(menu->action_group), name);
		g_free(name);

		actions = g_list_delete_link(actions, actions);
	}

	g_object_unref(G_OBJECT(info));

	/* finally, if this was the last item in the list, hide the separator. */
	if(g_hash_table_size(menu->plugin_items) == 0) {
		gtk_widget_hide(menu->separator);
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_plugins_menu_plugin_loaded_cb(GObject *manager, GPluginPlugin *plugin,
                                     gpointer data)
{
	pidgin_plugins_menu_add_plugin_actions(PIDGIN_PLUGINS_MENU(data), plugin);
}

static void
pidgin_plugins_menu_plugin_unloaded_cb(GObject *manager, GPluginPlugin *plugin,
                                       gpointer data)
{
	pidgin_plugins_menu_remove_plugin_actions(PIDGIN_PLUGINS_MENU(data),
	                                          plugin);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginPluginsMenu, pidgin_plugins_menu, GTK_TYPE_MENU)

static void
pidgin_plugins_menu_init(PidginPluginsMenu *menu) {
	GPluginManager *manager = NULL;

	/* initialize our template */
	gtk_widget_init_template(GTK_WIDGET(menu));

	/* create our internal action group and assign it to ourself */
	menu->action_group = g_simple_action_group_new();
	gtk_widget_insert_action_group(GTK_WIDGET(menu),
	                               PIDGIN_PLUGINS_MENU_ACTION_PREFIX,
	                               G_ACTION_GROUP(menu->action_group));

	/* create our storage for the items */
	menu->plugin_items = g_hash_table_new_full(g_direct_hash, g_direct_equal,
	                                           g_object_unref,
	                                           (GDestroyNotify)gtk_widget_destroy);

	/* Connect to the plugin manager's signals so we can stay up to date. */
	manager = gplugin_manager_get_default();

	g_signal_connect_object(manager, "loaded-plugin",
	                        G_CALLBACK(pidgin_plugins_menu_plugin_loaded_cb),
	                        menu, 0);
	g_signal_connect_object(manager, "unloaded-plugin",
	                        G_CALLBACK(pidgin_plugins_menu_plugin_unloaded_cb),
	                        menu, 0);
};

static void
pidgin_plugins_menu_class_init(PidginPluginsMenuClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin/Plugins/menu.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginPluginsMenu,
	                                     separator);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_plugins_menu_new(void) {
	return GTK_WIDGET(g_object_new(PIDGIN_TYPE_PLUGINS_MENU, NULL));
}

