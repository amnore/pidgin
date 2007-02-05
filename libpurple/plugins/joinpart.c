/**
 * gaim
 *
 * Gaim is the legal property of its developers, whose names are too numerous
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "internal.h"
#include "conversation.h"
#include "debug.h"
#include "plugin.h"
#include "version.h"

#define JOINPART_PLUGIN_ID "core-rlaager-joinpart"


/* Preferences */

/* The number of minutes before a person is considered
 * to have stopped being part of active conversation. */
#define DELAY_PREF "/plugins/core/joinpart/delay"
#define DELAY_DEFAULT 10

/* The number of people that must be in a room for this
 * plugin to have any effect */
#define THRESHOLD_PREF "/plugins/core/joinpart/threshold"
#define THRESHOLD_DEFAULT 20

struct joinpart_key
{
	GaimConversation *conv;
	char *user;
};

static guint joinpart_key_hash(const struct joinpart_key *key)
{
	g_return_val_if_fail(key != NULL, 0);

	return g_direct_hash(key->conv) + g_str_hash(key->user);
}

static gboolean joinpart_key_equal(const struct joinpart_key *a, const struct joinpart_key *b)
{
	if (a == NULL)
		return (b == NULL);
	else if (b == NULL)
		return FALSE;

	return (a->conv == b->conv) && !strcmp(a->user, b->user);
}

static void joinpart_key_destroy(struct joinpart_key *key)
{
	g_return_if_fail(key != NULL);

	g_free(key->user);
	g_free(key);
}

static gboolean should_hide_notice(GaimConversation *conv, const char *name,
                                   GHashTable *users)
{
	GaimConvChat *chat;
	int threshold;
	struct joinpart_key *key;
	time_t *last_said;

	g_return_val_if_fail(conv != NULL, FALSE);
	g_return_val_if_fail(gaim_conversation_get_type(conv) == GAIM_CONV_TYPE_CHAT, FALSE);

	/* If the room is small, don't bother. */
	chat = GAIM_CONV_CHAT(conv);
	threshold = gaim_prefs_get_int(THRESHOLD_PREF);
	if (g_list_length(gaim_conv_chat_get_users(chat)) < threshold)
		return FALSE;

	/* We always care about our buddies! */
	if (gaim_find_buddy(gaim_conversation_get_account(conv), name))
		return FALSE;

	/* Only show the notice if the user has spoken recently. */
	key = g_new(struct joinpart_key, 1);
	key->conv = conv;
	key->user = g_strdup(name);
	last_said = g_hash_table_lookup(users, key);
	if (last_said != NULL)
	{
		int delay = gaim_prefs_get_int(DELAY_PREF);
		if (delay > 0 && (*last_said + (delay * 60)) >= time(NULL))
			return FALSE;
	}

	return TRUE;
}

static gboolean chat_buddy_leaving_cb(GaimConversation *conv, const char *name,
                               const char *reason, GHashTable *users)
{
	return should_hide_notice(conv, name, users);
}

static gboolean chat_buddy_joining_cb(GaimConversation *conv, const char *name,
                                      GaimConvChatBuddyFlags flags,
                                      GHashTable *users)
{
	return should_hide_notice(conv, name, users);
}

static void received_chat_msg_cb(GaimAccount *account, char *sender,
                                 char *message, GaimConversation *conv,
                                 GaimMessageFlags flags, GHashTable *users)
{
	struct joinpart_key key;
	time_t *last_said;

	/* Most of the time, we'll already have tracked the user,
	 * so we avoid memory allocation here. */
	key.conv = conv;
	key.user = sender;
	last_said = g_hash_table_lookup(users, &key);
	if (last_said != NULL)
	{
		/* They just said something, so update the time. */
		time(last_said);
	}
	else
	{
		struct joinpart_key *key2;

		key2 = g_new(struct joinpart_key, 1);
		key2->conv = conv;
		key2->user = g_strdup(sender);

		last_said = g_new(time_t, 1);
		time(last_said);

		g_hash_table_insert(users, key2, last_said);
	}
}

static gboolean check_expire_time(struct joinpart_key *key,
                                  time_t *last_said, time_t *limit)
{
	gaim_debug_info("joinpart", "Removing key for %s/%s\n", key->conv->name, key->user);
	return (*last_said < *limit);
}

static gboolean clean_users_hash(GHashTable *users)
{
	int delay = gaim_prefs_get_int(DELAY_PREF);
	time_t limit = time(NULL) - (60 * delay);

	g_hash_table_foreach_remove(users, (GHRFunc)check_expire_time, &limit);

	return TRUE;
}

static gboolean plugin_load(GaimPlugin *plugin)
{
	void *conv_handle;
	GHashTable *users;
	guint id;
	gpointer *data;

	users = g_hash_table_new_full((GHashFunc)joinpart_key_hash,
	                              (GEqualFunc)joinpart_key_equal,
	                              (GDestroyNotify)joinpart_key_destroy,
	                              g_free);

	conv_handle = gaim_conversations_get_handle();
	gaim_signal_connect(conv_handle, "chat-buddy-joining", plugin,
	                    GAIM_CALLBACK(chat_buddy_joining_cb), users);
	gaim_signal_connect(conv_handle, "chat-buddy-leaving", plugin,
	                    GAIM_CALLBACK(chat_buddy_leaving_cb), users);
	gaim_signal_connect(conv_handle, "received-chat-msg", plugin,
	                    GAIM_CALLBACK(received_chat_msg_cb), users);

	/* Cleanup every 5 minutes */
	id = gaim_timeout_add(1000 * 60 * 5, (GSourceFunc)clean_users_hash, users);

	data = g_new(gpointer, 2);
	data[0] = users;
	data[1] = GUINT_TO_POINTER(id);
	plugin->extra = data;

	return TRUE;
}

static gboolean plugin_unload(GaimPlugin *plugin)
{
	gpointer *data = plugin->extra;

	/* Destroy the hash table. The core plugin code will
	 * disconnect the signals, and since Gaim is single-threaded,
	 * we don't have to worry one will be called after this. */
	g_hash_table_destroy((GHashTable *)data[0]);

	g_source_remove(GPOINTER_TO_UINT(data[1]));
	g_free(data);

	return TRUE;
}

static GaimPluginPrefFrame *
get_plugin_pref_frame(GaimPlugin *plugin)
{
	GaimPluginPrefFrame *frame;
	GaimPluginPref *ppref;

	g_return_val_if_fail(plugin != NULL, FALSE);

	frame = gaim_plugin_pref_frame_new();

	ppref = gaim_plugin_pref_new_with_label(_("Join/Part Hiding Configuration"));
	gaim_plugin_pref_frame_add(frame, ppref);

	ppref = gaim_plugin_pref_new_with_name_and_label(THRESHOLD_PREF,
	                                                 _("Minimum Room Size"));
	gaim_plugin_pref_set_bounds(ppref, 0, 1000);
	gaim_plugin_pref_frame_add(frame, ppref);


	ppref = gaim_plugin_pref_new_with_name_and_label(DELAY_PREF,
	                                                 _("User Inactivity Timeout (in minutes)"));
	gaim_plugin_pref_set_bounds(ppref, 0, 8 * 60); /* 8 Hours */
	gaim_plugin_pref_frame_add(frame, ppref);

	return frame;
}

static GaimPluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,   /* page_num (reserved) */
	NULL /* frame (reserved) */
};

static GaimPluginInfo info =
{
	GAIM_PLUGIN_MAGIC,
	GAIM_MAJOR_VERSION,
	GAIM_MINOR_VERSION,
	GAIM_PLUGIN_STANDARD,                             /**< type           */
	NULL,                                             /**< ui_requirement */
	0,                                                /**< flags          */
	NULL,                                             /**< dependencies   */
	GAIM_PRIORITY_DEFAULT,                            /**< priority       */

	JOINPART_PLUGIN_ID,                               /**< id             */
	N_("Join/Part Hiding"),                           /**< name           */
	VERSION,                                          /**< version        */
	                                                  /**  summary        */
	N_("Hides extraneous join/part messages."),
	                                                  /**  description    */
	N_("This plugin hides join/part messages in large "
	   "rooms, except for those users actively taking "
	   "part in a conversation."),
	"Richard Laager <rlaager@pidgin.im>",             /**< author         */
	GAIM_WEBSITE,                                     /**< homepage       */

	plugin_load,                                      /**< load           */
	plugin_unload,                                    /**< unload         */
	NULL,                                             /**< destroy        */

	NULL,                                             /**< ui_info        */
	NULL,                                             /**< extra_info     */
	&prefs_info,                                      /**< prefs_info     */
	NULL                                              /**< actions        */
};

static void
init_plugin(GaimPlugin *plugin)
{
	gaim_prefs_add_none("/plugins/core/joinpart");

	gaim_prefs_add_int(DELAY_PREF, DELAY_DEFAULT);
	gaim_prefs_add_int(THRESHOLD_PREF, THRESHOLD_DEFAULT);
}

GAIM_INIT_PLUGIN(joinpart, init_plugin, info)
