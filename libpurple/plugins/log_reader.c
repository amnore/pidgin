/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>

#include <ctype.h>
#include <stdio.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

/* Where is the Windows partition mounted? */
#ifndef PURPLE_LOG_READER_WINDOWS_MOUNT_POINT
#define PURPLE_LOG_READER_WINDOWS_MOUNT_POINT "/mnt/windows"
#endif

enum name_guesses {
	NAME_GUESS_UNKNOWN,
	NAME_GUESS_ME,
	NAME_GUESS_THEM
};


/*****************************************************************************
 * Adium Logger                                                              *
 *****************************************************************************/

/* The adium logger doesn't write logs, only reads them.  This is to include
 * Adium logs in the log viewer transparently.
 */

static PurpleLogLogger *adium_logger;

enum adium_log_type {
	ADIUM_HTML,
	ADIUM_TEXT,
};

struct adium_logger_data {
	char *path;
	enum adium_log_type type;
};

static GList *adium_logger_list(PurpleLogType type, const char *sn, PurpleAccount *account)
{
	GList *list = NULL;
	const char *logdir;
	PurpleProtocol *protocol;
	char *protocol_name;
	char *temp;
	char *path;
	GDir *dir;

	g_return_val_if_fail(sn != NULL, NULL);
	g_return_val_if_fail(account != NULL, NULL);

	logdir = purple_prefs_get_string("/plugins/core/log_reader/adium/log_directory");

	/* By clearing the log directory path, this logger can be (effectively) disabled. */
	if (!logdir || !*logdir)
		return NULL;

	protocol = purple_account_get_protocol(account);
	if (!protocol)
		return NULL;

	protocol_name = g_ascii_strup(purple_protocol_get_list_icon(protocol, account, NULL), -1);

	temp = g_strdup_printf("%s.%s", protocol_name, purple_account_get_username(account));
	path = g_build_filename(logdir, temp, sn, NULL);
	g_free(temp);

	dir = g_dir_open(path, 0, NULL);
	if (dir) {
		const gchar *file;

		while ((file = g_dir_read_name(dir))) {
			if (!g_str_has_prefix(file, sn)) {
				continue;
			}
			if (g_str_has_suffix(file, ".html") ||
			    g_str_has_suffix(file, ".AdiumHTMLLog")) {
				GDateTime *dt;
				gint year, month, day, hour, minute, second;
				const char *date = file;

				date += strlen(sn) + 2;
				if (sscanf(date, "%u|%u|%u", &year, &month, &day) != 3) {

					purple_debug_error("Adium log parse",
					                   "Filename timestamp parsing error\n");
				} else {
					char *filename = g_build_filename(path, file, NULL);
					FILE *handle = g_fopen(filename, "rb");
					char contents[57];   /* XXX: This is really inflexible. */
					char *contents2;
					struct adium_logger_data *data;
					size_t rd;
					PurpleLog *log;

					if (!handle) {
						g_free(filename);
						continue;
					}

					rd = fread(contents, 1, 56, handle) == 0;
					fclose(handle);
					contents[rd] = '\0';

					/* XXX: This is fairly inflexible. */
					contents2 = contents;
					while (*contents2 && *contents2 != '>')
						contents2++;
					if (*contents2)
						contents2++;
					while (*contents2 && *contents2 != '>')
						contents2++;
					if (*contents2)
						contents2++;

					if (sscanf(contents2, "%u.%u.%u",
							&hour, &minute, &second) != 3) {

						purple_debug_error("Adium log parse",
						                   "Contents timestamp parsing error\n");
						g_free(filename);
						continue;
					}

					data = g_new0(struct adium_logger_data, 1);
					data->path = filename;
					data->type = ADIUM_HTML;

					/* XXX: Look into this later... Should we figure out a timezone? */
					dt = g_date_time_new_local(year, month, day, hour, minute, second);

					log = purple_log_new(PURPLE_LOG_IM, sn, account, NULL, dt);
					log->logger = adium_logger;
					log->logger_data = data;

					g_date_time_unref(dt);

					list = g_list_prepend(list, log);
				}
			} else if (g_str_has_suffix(file, ".adiumLog")) {
				GDateTime *dt;
				gint year, month, day, hour, minute, second;
				const char *date = file;

				date += strlen(sn) + 2;
				if (sscanf(date, "%u|%u|%u", &year, &month, &day) != 3) {

					purple_debug_error("Adium log parse",
					                   "Filename timestamp parsing error\n");
				} else {
					char *filename = g_build_filename(path, file, NULL);
					FILE *handle = g_fopen(filename, "rb");
					char contents[14];   /* XXX: This is really inflexible. */
					char *contents2;
					struct adium_logger_data *data;
					PurpleLog *log;
					size_t rd;

					if (!handle) {
						g_free(filename);
						continue;
					}

					rd = fread(contents, 1, 13, handle);
					fclose(handle);
					contents[rd] = '\0';

					contents2 = contents;
					while (*contents2 && *contents2 != '(')
						contents2++;
					if (*contents2)
						contents2++;

					if (sscanf(contents2, "%u.%u.%u", &hour, &minute, &second) != 3) {

						purple_debug_error("Adium log parse",
						                   "Contents timestamp parsing error\n");
						g_free(filename);
						continue;
					}

					data = g_new0(struct adium_logger_data, 1);
					data->path = filename;
					data->type = ADIUM_TEXT;

					/* XXX: Look into this later... Should we figure out a timezone? */
					dt = g_date_time_new_local(year, month, day, hour, minute, second);

					log = purple_log_new(PURPLE_LOG_IM, sn, account, NULL, dt);
					log->logger = adium_logger;
					log->logger_data = data;

					g_date_time_unref(dt);

					list = g_list_prepend(list, log);
				}
			}
		}
		g_dir_close(dir);
	}

	g_free(protocol_name);
	g_free(path);

	return list;
}

static char *adium_logger_read (PurpleLog *log, PurpleLogReadFlags *flags)
{
	struct adium_logger_data *data;
	GError *error = NULL;
	gchar *read = NULL;

	/* XXX: TODO: We probably want to set PURPLE_LOG_READ_NO_NEWLINE
	 * XXX: TODO: for HTML logs. */
	if (flags != NULL)
		*flags = 0;

	g_return_val_if_fail(log != NULL, g_strdup(""));

	data = log->logger_data;

	g_return_val_if_fail(data->path != NULL, g_strdup(""));

	purple_debug_info("Adium log read", "Reading %s\n", data->path);
	if (!g_file_get_contents(data->path, &read, NULL, &error)) {
		purple_debug_error("Adium log read", "Error reading log: %s\n",
					   (error && error->message) ? error->message : "Unknown error");
		if (error)
			g_error_free(error);
		return g_strdup("");
	}

	if (data->type != ADIUM_HTML) {
		char *escaped = g_markup_escape_text(read, -1);
		g_free(read);
		read = escaped;
	}

#ifdef WIN32
	/* This problem only seems to show up on Windows.
	 * The BOM is displaying as a space at the beginning of the log.
	 */
	if (g_str_has_prefix(read, "\xef\xbb\xbf")) {
		/* FIXME: This feels so wrong... */
		char *temp = g_strdup(&(read[3]));
		g_free(read);
		read = temp;
	}
#endif

	/* TODO: Apply formatting.
	 * Replace the above hack with something better, since we'll
	 * be looping over the entire log file contents anyway.
	 */

	return read;
}

static int adium_logger_size (PurpleLog *log)
{
	struct adium_logger_data *data;
	char *text;
	size_t size;

	g_return_val_if_fail(log != NULL, 0);

	data = log->logger_data;

	if (purple_prefs_get_bool("/plugins/core/log_reader/fast_sizes")) {
		GStatBuf st;

		if (!data->path || g_stat(data->path, &st))
			st.st_size = 0;

		return st.st_size;
	}

	text = adium_logger_read(log, NULL);
	size = strlen(text);
	g_free(text);

	return size;
}

static void adium_logger_finalize(PurpleLog *log)
{
	struct adium_logger_data *data;

	g_return_if_fail(log != NULL);

	data = log->logger_data;

	g_free(data->path);
	g_free(data);
}


/*****************************************************************************
 * MSN Messenger Logger                                                      *
 *****************************************************************************/

/* The msn logger doesn't write logs, only reads them.  This is to include
 * MSN Messenger message histories in the log viewer transparently.
 */

static PurpleLogLogger *msn_logger;

struct msn_logger_data {
	PurpleXmlNode *root;
	PurpleXmlNode *message;
	const char *session_id;
	int last_log;
	GString *text;
};

/* This function is really confusing.  It makes baby rlaager cry...
   In other news: "You lost a lot of blood but we found most of it."
 */
static GDateTime *
msn_logger_parse_timestamp(PurpleXmlNode *message)
{
	const char *datetime;
	GDateTime *stamp;
	const char *date;
	const char *time;
	int month;
	int day;
	int year;
	int hour;
	int min;
	int sec;
	char am_pm;
	char *str;
	GDateTime *t;
	GTimeSpan diff;

	g_return_val_if_fail(message != NULL, NULL);

	datetime = purple_xmlnode_get_attrib(message, "DateTime");
	if (!(datetime && *datetime))
	{
		purple_debug_error("MSN log timestamp parse",
		                   "Attribute missing: %s\n", "DateTime");
		return NULL;
	}

	stamp = purple_str_to_date_time(datetime, TRUE);

	date = purple_xmlnode_get_attrib(message, "Date");
	if (!(date && *date))
	{
		purple_debug_error("MSN log timestamp parse",
		                   "Attribute missing: %s\n", "Date");
		return stamp;
	}

	time = purple_xmlnode_get_attrib(message, "Time");
	if (!(time && *time))
	{
		purple_debug_error("MSN log timestamp parse",
		                   "Attribute missing: %s\n", "Time");
		return stamp;
	}

	if (sscanf(date, "%u/%u/%u", &month, &day, &year) != 3)
	{
		purple_debug_error("MSN log timestamp parse",
		                   "%s parsing error\n", "Date");
		return stamp;
	}
	else
	{
		if (month > 12)
		{
			int tmp = day;
			day = month;
			month = tmp;
		}
	}

	if (sscanf(time, "%u:%u:%u %c", &hour, &min, &sec, &am_pm) != 4)
	{
		purple_debug_error("MSN log timestamp parse",
		                   "%s parsing error\n", "Time");
		return stamp;
	}

        if (am_pm == 'P') {
                hour += 12;
        } else if (hour == 12) {
                /* 12 AM = 00 hr */
                hour = 0;
        }

	str = g_strdup_printf("%04i-%02i-%02iT%02i:%02i:%02i", year, month, day, hour, min, sec);
	t = purple_str_to_date_time(str, TRUE);

	if (g_date_time_compare(stamp, t) > 0)
		diff = g_date_time_difference(stamp, t);
	else
		diff = g_date_time_difference(t, stamp);

	if (diff > (14LL * G_TIME_SPAN_HOUR)) {
		if (day <= 12)
		{
			/* Swap day & month variables, to see if it's a non-US date. */
			g_free(str);
			str = g_strdup_printf("%04i-%02i-%02iT%02i:%02i:%02i", year, month, day, hour, min, sec);
			t = purple_str_to_date_time(str, TRUE);

			if (g_date_time_compare(stamp, t) > 0)
				diff = g_date_time_difference(stamp, t);
			else
				diff = g_date_time_difference(t, stamp);

			if (diff > (14LL * G_TIME_SPAN_HOUR)) {
				/* We got a time, it's not impossible, but
				 * the diff is too large.  Display the UTC time. */
				g_free(str);
				return stamp;
			}
			else
			{
				/* Legal time */
				/* Fall out */
			}
		}
		else
		{
			/* We got a time, it's not impossible, but
			 * the diff is too large.  Display the UTC time. */
			g_free(str);
			return stamp;
		}
	}

	/* If we got here, the time is legal with a reasonable offset.
	 * Let's find out if it's in our TZ. */
	if (purple_str_to_date_time(str, FALSE) == stamp)
	{
		g_free(str);
		return stamp;
	}
	g_free(str);

	return stamp;
}

static GList *msn_logger_list(PurpleLogType type, const char *sn, PurpleAccount *account)
{
	GList *list = NULL;
	char *username;
	PurpleBuddy *buddy;
	const char *logdir;
	const char *savedfilename = NULL;
	char *logfile;
	char *path;
	GError *error = NULL;
	gchar *contents = NULL;
	gsize length;
	PurpleXmlNode *root;
	PurpleXmlNode *message;
	const char *old_session_id = "";
	struct msn_logger_data *data = NULL;

	g_return_val_if_fail(sn != NULL, NULL);
	g_return_val_if_fail(account != NULL, NULL);

	if (!purple_strequal(purple_account_get_protocol_id(account), "prpl-msn"))
		return NULL;

	logdir = purple_prefs_get_string("/plugins/core/log_reader/msn/log_directory");

	/* By clearing the log directory path, this logger can be (effectively) disabled. */
	if (!logdir || !*logdir)
		return NULL;

	buddy = purple_blist_find_buddy(account, sn);

	if ((username = g_strdup(purple_account_get_string(
			account, "log_reader_msn_log_folder", NULL)))) {
		/* As a special case, we allow the null string to kill the parsing
		 * straight away. This would allow the user to deal with the case
		 * when two account have the same username at different domains and
		 * only one has logs stored.
		 */
		if (!*username) {
			g_free(username);
			return list;
		}
	} else {
		username = g_strdup(purple_normalize(account, purple_account_get_username(account)));
	}

	if (buddy) {
		savedfilename = purple_blist_node_get_string(PURPLE_BLIST_NODE(buddy),
		                                             "log_reader_msn_log_filename");
	}

	if (savedfilename) {
		/* As a special case, we allow the null string to kill the parsing
		 * straight away. This would allow the user to deal with the case
		 * when two buddies have the same username at different domains and
		 * only one has logs stored.
		 */
		if (!*savedfilename) {
			g_free(username);
			return list;
		}

		logfile = g_strdup(savedfilename);
	} else {
		logfile = g_strdup_printf("%s.xml", purple_normalize(account, sn));
	}

	path = g_build_filename(logdir, username, "History", logfile, NULL);

	if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
		gboolean found = FALSE;
		char *at_sign;
		GDir *dir;

		g_free(path);

		if (savedfilename) {
			/* We had a saved filename, but it doesn't exist.
			 * Returning now is the right course of action because we don't
			 * want to detect another file incorrectly.
			 */
			g_free(username);
			g_free(logfile);
			return list;
		}

		/* Perhaps we're using a new version of MSN with the weird numbered folders.
		 * I don't know how the numbers are calculated, so I'm going to attempt to
		 * find logs by pattern matching...
		 */

		at_sign = g_strrstr(username, "@");
		if (at_sign)
			*at_sign = '\0';

		dir = g_dir_open(logdir, 0, NULL);
		if (dir) {
			const gchar *name;

			while ((name = g_dir_read_name(dir))) {
				const char *c = name;

				if (!g_str_has_prefix(c, username)) {
					continue;
				}

				c += strlen(username);
				while (*c) {
					if (!g_ascii_isdigit(*c))
						break;

					c++;
				}

				path = g_build_filename(logdir, name, NULL);
				/* The !c makes sure we got to the end of the while loop above. */
				if (!*c && g_file_test(path, G_FILE_TEST_IS_DIR)) {
					char *history_path = g_build_filename(
						path,  "History", NULL);
					if (g_file_test(history_path, G_FILE_TEST_IS_DIR)) {
						purple_account_set_string(account,
							"log_reader_msn_log_folder", name);
						g_free(path);
						path = history_path;
						found = TRUE;
						break;
					}
					g_free(history_path);
				}
				g_free(path);
			}
			g_dir_close(dir);
		}
		g_free(username);

		if (!found) {
			g_free(logfile);
			return list;
		}

		/* If we've reached this point, we've found a History folder. */

		username = g_strdup(purple_normalize(account, sn));
		at_sign = g_strrstr(username, "@");
		if (at_sign)
			*at_sign = '\0';

		found = FALSE;
		dir = g_dir_open(path, 0, NULL);
		if (dir) {
			const gchar *name;

			while ((name = g_dir_read_name(dir))) {
				const char *c = name;
				gchar *full_path;

				if (!g_str_has_prefix(c, username)) {
					continue;
				}

				c += strlen(username);
				while (*c) {
					if (!g_ascii_isdigit(*c))
						break;

					c++;
				}

				full_path = g_build_filename(path, name, NULL);
				if (purple_strequal(c, ".xml") &&
				    g_file_test(full_path, G_FILE_TEST_EXISTS)) {
					found = TRUE;
					g_free(path);
					path = full_path;
					g_free(logfile);
					logfile = g_strdup(name);
					break;
				}
				else
					g_free(full_path);
			}
			g_dir_close(dir);
		}
		g_free(username);

		if (!found) {
			g_free(logfile);
			return list;
		}
	} else {
		g_free(username);
		g_free(logfile);
		logfile = NULL; /* No sense saving the obvious buddy@domain.com. */
	}

	purple_debug_info("MSN log read", "Reading %s\n", path);
	if (!g_file_get_contents(path, &contents, &length, &error)) {
		g_free(path);
		purple_debug_error("MSN log read", "Error reading log\n");
		if (error)
			g_error_free(error);
		return list;
	}
	g_free(path);

	/* Reading the file was successful...
	 * Save its name if it involves the crazy numbers. The idea here is that you could
	 * then tweak the blist.xml file by hand if need be. This would be the case if two
	 * buddies have the same username at different domains. One set of logs would get
	 * detected for both buddies.
	 */
	if (buddy && logfile) {
		PurpleBlistNode *node = PURPLE_BLIST_NODE(buddy);
		purple_blist_node_set_string(node, "log_reader_msn_log_filename", logfile);
		g_free(logfile);
	}

	root = purple_xmlnode_from_str(contents, length);
	g_free(contents);
	if (!root)
		return list;

	for (message = purple_xmlnode_get_child(root, "Message"); message;
			message = purple_xmlnode_get_next_twin(message)) {
		const char *session_id;

		session_id = purple_xmlnode_get_attrib(message, "SessionID");
		if (!session_id) {
			purple_debug_error("MSN log parse",
			                   "Error parsing message: %s\n", "SessionID missing");
			continue;
		}

		if (!purple_strequal(session_id, old_session_id)) {
			/*
			 * The session ID differs from the last message.
			 * Thus, this is the start of a new conversation.
			 */
			GDateTime *stamp;
			PurpleLog *log;

			data = g_new0(struct msn_logger_data, 1);
			data->root = root;
			data->message = message;
			data->session_id = session_id;
			data->text = NULL;
			data->last_log = FALSE;

			stamp = msn_logger_parse_timestamp(message);

			log = purple_log_new(PURPLE_LOG_IM, sn, account, NULL, stamp);
			log->logger = msn_logger;
			log->logger_data = data;

			g_date_time_unref(stamp);

			list = g_list_prepend(list, log);
		}
		old_session_id = session_id;
	}

	if (data)
		data->last_log = TRUE;

	return g_list_reverse(list);
}

static char * msn_logger_read (PurpleLog *log, PurpleLogReadFlags *flags)
{
	struct msn_logger_data *data;
	GString *text = NULL;
	PurpleXmlNode *message;

	if (flags != NULL)
		*flags = PURPLE_LOG_READ_NO_NEWLINE;
	g_return_val_if_fail(log != NULL, g_strdup(""));

	data = log->logger_data;

	if (data->text) {
		/* The GTK code which displays the logs g_free()s whatever is
		 * returned from this function. Thus, we can't reuse the str
		 * part of the GString. The only solution is to free it and
		 * start over.
		 */
		g_string_free(data->text, FALSE);
	}

	text = g_string_new("");

	if (!data->root || !data->message || !data->session_id) {
		/* Something isn't allocated correctly. */
		purple_debug_error("MSN log parse",
		                   "Error parsing message: %s\n", "Internal variables inconsistent");
		data->text = text;

		return text->str;
	}

	for (message = data->message; message;
			message = purple_xmlnode_get_next_twin(message)) {

		const char *new_session_id;
		PurpleXmlNode *text_node;
		const char *from_name = NULL;
		const char *to_name = NULL;
		PurpleXmlNode *from;
		PurpleXmlNode *to;
		enum name_guesses name_guessed = NAME_GUESS_UNKNOWN;
		const char *their_name;
		GDateTime *dt = NULL;
		char *timestamp;
		char *tmp;
		const char *style;

		new_session_id = purple_xmlnode_get_attrib(message, "SessionID");

		/* If this triggers, something is wrong with the XML. */
		if (!new_session_id) {
			purple_debug_error("MSN log parse",
			                   "Error parsing message: %s\n", "New SessionID missing");
			break;
		}

		if (!purple_strequal(new_session_id, data->session_id)) {
			/* The session ID differs from the first message.
			 * Thus, this is the start of a new conversation.
			 */
			break;
		}

		text_node = purple_xmlnode_get_child(message, "Text");
		if (!text_node)
			continue;

		from = purple_xmlnode_get_child(message, "From");
		if (from) {
			PurpleXmlNode *user = purple_xmlnode_get_child(from, "User");

			if (user) {
				from_name = purple_xmlnode_get_attrib(user, "FriendlyName");

				/* This saves a check later. */
				if (!*from_name)
					from_name = NULL;
			}
		}

		to = purple_xmlnode_get_child(message, "To");
		if (to) {
			PurpleXmlNode *user = purple_xmlnode_get_child(to, "User");
			if (user) {
				to_name = purple_xmlnode_get_attrib(user, "FriendlyName");

				/* This saves a check later. */
				if (!*to_name)
					to_name = NULL;
			}
		}

		their_name = from_name;
		if (from_name && purple_prefs_get_bool("/plugins/core/log_reader/use_name_heuristics")) {
			const char *friendly_name = purple_connection_get_display_name(purple_account_get_connection(log->account));

			if (friendly_name != NULL) {
				int friendly_name_length = strlen(friendly_name);
				const char *alias;
				int alias_length;
				PurpleBuddy *buddy = purple_blist_find_buddy(log->account, log->name);
				gboolean from_name_matches;
				gboolean to_name_matches;

				if (buddy)
					their_name = purple_buddy_get_alias(buddy);

				alias = purple_account_get_private_alias(log->account);
				if (alias) {
					alias_length = strlen(alias);
				} else {
					alias = "";
					alias_length = 0;
				}

				/* Try to guess which user is me.
				 * The first step is to determine if either of the names matches either my
				 * friendly name or alias. For this test, "match" is defined as:
				 * ^(friendly_name|alias)([^a-zA-Z0-9].*)?$
				 */
				from_name_matches =
				        (g_str_has_prefix(from_name, friendly_name) &&
				         !isalnum(*(from_name + friendly_name_length))) ||
				        (g_str_has_prefix(from_name, alias) &&
				         !isalnum(*(from_name + alias_length)));

				to_name_matches =
				        to_name != NULL &&
				        ((g_str_has_prefix(to_name, friendly_name) &&
				          !isalnum(*(to_name + friendly_name_length))) ||
				         (g_str_has_prefix(to_name, alias) &&
				          !isalnum(*(to_name + alias_length))));

				if (from_name_matches) {
					if (!to_name_matches) {
						name_guessed = NAME_GUESS_ME;
					}
				} else if (to_name_matches) {
					name_guessed = NAME_GUESS_THEM;
				} else {
					if (buddy) {
						const char *server_alias = NULL;
						char *alias = g_strdup(purple_buddy_get_alias(buddy));
						char *temp;

						/* "Truncate" the string at the first non-alphanumeric
						 * character. The idea is to relax the comparison.
						 */
						for (temp = alias; *temp ; temp++) {
							if (!isalnum(*temp)) {
								*temp = '\0';
								break;
							}
						}
						alias_length = strlen(alias);

						/* Try to guess which user is them.
						 * The first step is to determine if either of the names
						 * matches their alias. For this test, "match" is
						 * defined as: ^alias([^a-zA-Z0-9].*)?$
						 */
						from_name_matches =
						        (g_str_has_prefix(from_name, alias) &&
						         !isalnum(*(from_name + alias_length)));

						to_name_matches = to_name &&
						                  (g_str_has_prefix(to_name, alias) &&
						                   !isalnum(*(to_name + alias_length)));

						g_free(alias);

						if (from_name_matches) {
							if (!to_name_matches) {
								name_guessed = NAME_GUESS_THEM;
							}
						} else if (to_name_matches) {
							name_guessed = NAME_GUESS_ME;
						} else if ((server_alias = purple_buddy_get_server_alias(buddy))) {
							friendly_name_length =
								strlen(server_alias);

							/* Try to guess which user is them.
							 * The first step is to determine if either of
							 * the names matches their friendly name. For
							 * this test, "match" is defined as:
							 * ^friendly_name([^a-zA-Z0-9].*)?$
							 */
							from_name_matches =
							        (g_str_has_prefix(from_name,
							                          server_alias) &&
							         !isalnum(*(from_name +
							                    friendly_name_length)));

							to_name_matches =
							        to_name &&
							        ((g_str_has_prefix(to_name, server_alias) &&
							          !isalnum(*(to_name +
							                     friendly_name_length))));

							if (from_name_matches) {
								if (!to_name_matches) {
									name_guessed = NAME_GUESS_THEM;
								}
							} else if (to_name_matches) {
								name_guessed = NAME_GUESS_ME;
							}
						}
					}
				}
			}
		}

		if (name_guessed != NAME_GUESS_UNKNOWN) {
			text = g_string_append(text, "<span style=\"color: #");
			if (name_guessed == NAME_GUESS_ME)
				text = g_string_append(text, "16569E");
			else
				text = g_string_append(text, "A82F2F");
			text = g_string_append(text, ";\">");
		}

		if ((dt = msn_logger_parse_timestamp(message)) != NULL) {
			timestamp = g_date_time_format(
				dt,
				"<font size=\"2\">(%H:%M:%s)</font> ");
			text = g_string_append(text, timestamp);
			g_free(timestamp);
			g_date_time_unref(dt);
		} else {
			text = g_string_append(text,
				"<font size=\"2\">(00:00:00)</font> ");
		}

		if (from_name) {
			text = g_string_append(text, "<b>");

			if (name_guessed == NAME_GUESS_ME) {
				if (purple_account_get_private_alias(log->account))
					text = g_string_append(text, purple_account_get_private_alias(log->account));
				else
					text = g_string_append(text, purple_account_get_username(log->account));
			}
			else if (name_guessed == NAME_GUESS_THEM)
				text = g_string_append(text, their_name);
			else
				text = g_string_append(text, from_name);

			text = g_string_append(text, ":</b> ");
		}

		if (name_guessed != NAME_GUESS_UNKNOWN)
			text = g_string_append(text, "</span>");

		style = purple_xmlnode_get_attrib(text_node, "Style");

		tmp = purple_xmlnode_get_data(text_node);
		if (style && *style) {
			text = g_string_append(text, "<span style=\"");
			text = g_string_append(text, style);
			text = g_string_append(text, "\">");
			text = g_string_append(text, tmp);
			text = g_string_append(text, "</span><br>");
		} else {
			text = g_string_append(text, tmp);
			text = g_string_append(text, "<br>");
		}
		g_free(tmp);
	}

	data->text = text;

	return text->str;
}

static int msn_logger_size (PurpleLog *log)
{
	char *text;
	size_t size;

	g_return_val_if_fail(log != NULL, 0);

	if (purple_prefs_get_bool("/plugins/core/log_reader/fast_sizes"))
		return 0;

	text = msn_logger_read(log, NULL);
	size = strlen(text);
	g_free(text);

	return size;
}

static void msn_logger_finalize(PurpleLog *log)
{
	struct msn_logger_data *data;

	g_return_if_fail(log != NULL);

	data = log->logger_data;

	if (data->last_log)
		purple_xmlnode_free(data->root);

	if (data->text)
		g_string_free(data->text, FALSE);

	g_free(data);
}


/*****************************************************************************
 * Trillian Logger                                                           *
 *****************************************************************************/

/* The trillian logger doesn't write logs, only reads them.  This is to include
 * Trillian logs in the log viewer transparently.
 */

static PurpleLogLogger *trillian_logger;
static void trillian_logger_finalize(PurpleLog *log);

struct trillian_logger_data {
	char *path; /* FIXME: Change this to use PurpleStringref like log.c:old_logger_list */
	int offset;
	int length;
	char *their_nickname;
};

static GList *trillian_logger_list(PurpleLogType type, const char *sn, PurpleAccount *account)
{
	GList *list = NULL;
	const char *logdir;
	PurpleProtocol *protocol;
	char *protocol_name;
	const char *buddy_name;
	char *filename;
	char *path;
	GError *error = NULL;
	gchar *contents = NULL;
	gsize length;
	gchar *line;
	gchar *c;

	g_return_val_if_fail(sn != NULL, NULL);
	g_return_val_if_fail(account != NULL, NULL);

	logdir = purple_prefs_get_string("/plugins/core/log_reader/trillian/log_directory");

	/* By clearing the log directory path, this logger can be (effectively) disabled. */
	if (!logdir || !*logdir)
		return NULL;

	protocol = purple_account_get_protocol(account);
	if (!protocol)
		return NULL;

	protocol_name = g_ascii_strup(purple_protocol_get_list_icon(protocol, account, NULL), -1);

	buddy_name = purple_normalize(account, sn);

	filename = g_strdup_printf("%s.log", buddy_name);
	path = g_build_filename(
		logdir, protocol_name, filename, NULL);

	purple_debug_info("Trillian log list", "Reading %s\n", path);
	/* FIXME: There's really no need to read the entire file at once.
	 * See src/log.c:old_logger_list for a better approach.
	 */
	if (!g_file_get_contents(path, &contents, &length, &error)) {
		if (error) {
			g_error_free(error);
			error = NULL;
		}
		g_free(path);

		path = g_build_filename(
			logdir, protocol_name, "Query", filename, NULL);
		purple_debug_info("Trillian log list", "Reading %s\n", path);
		if (!g_file_get_contents(path, &contents, &length, &error)) {
			if (error)
				g_error_free(error);
		}
	}
	g_free(filename);

	if (contents) {
		struct trillian_logger_data *data = NULL;
		int offset = 0;
		int last_line_offset = 0;

		line = contents;
		c = contents;
		while (*c) {
			offset++;

			if (*c != '\n') {
				c++;
				continue;
			}

			*c = '\0';
			if (g_str_has_prefix(line, "Session Close ")) {
				if (data && !data->length) {
					if (!(data->length = last_line_offset - data->offset)) {
						/* This log had no data, so we remove it. */
						GList *last = g_list_last(list);

						purple_debug_info("Trillian log list",
						                  "Empty log. Offset %i\n", data->offset);

						trillian_logger_finalize((PurpleLog *)last->data);
						list = g_list_delete_link(list, last);
					}
				}
			} else if (line[0] && line[1] && line[2] &&
			           g_str_has_prefix(&line[3], "sion Start ")) {
				/* The conditional is to make sure we're not reading off
				 * the end of the string.  We don't want strlen(), as that'd
				 * have to count the whole string needlessly.
				 *
				 * The odd check here is because a Session Start at the
				 * beginning of the file can be overwritten with a UTF-8
				 * byte order mark.  Yes, it's weird.
				 */
				char *their_nickname = line;
				char *timestamp;

				if (data && !data->length)
					data->length = last_line_offset - data->offset;

				while (*their_nickname && (*their_nickname != ':'))
					their_nickname++;
				their_nickname++;

				/* This code actually has nothing to do with
				 * the timestamp YET. I'm simply using this
				 * variable for now to NUL-terminate the
				 * their_nickname string.
				 */
				timestamp = their_nickname;
				while (*timestamp && *timestamp != ')')
					timestamp++;

				if (*timestamp == ')') {
					char *month_str;
					gint year, month, day, hour, minute, second;
					GDateTime *dt;

					*timestamp = '\0';
					if (line[0] && line[1] && line[2])
						timestamp += 3;

					/* Now we start dealing with the timestamp. */

					/* Skip over the day name. */
					while (*timestamp && (*timestamp != ' '))
						timestamp++;
					*timestamp = '\0';
					timestamp++;

					/* Parse out the month. */
					month_str = timestamp;
					while (*timestamp &&  (*timestamp != ' '))
						timestamp++;
					*timestamp = '\0';
					timestamp++;

					/* Parse the day, time, and year. */
					if (sscanf(timestamp, "%u %u:%u:%u %u",
							&day, &hour,
							&minute, &second,
							&year) != 5) {

						purple_debug_error("Trillian log timestamp parse",
						                   "Session Start parsing error\n");
					} else {
						PurpleLog *log;

						month = purple_time_parse_month(month_str);

						data = g_new0(
							struct trillian_logger_data, 1);
						data->path = g_strdup(path);
						data->offset = offset;
						data->length = 0;
						data->their_nickname =
							g_strdup(their_nickname);

						/* XXX: Look into this later... Should we figure out a timezone? */
						dt = g_date_time_new_local(year, month, day, hour, minute, second);

						log = purple_log_new(PURPLE_LOG_IM,
							sn, account, NULL, dt);
						log->logger = trillian_logger;
						log->logger_data = data;

						g_date_time_unref(dt);

						list = g_list_prepend(list, log);
					}
				}
			}
			c++;
			line = c;
			last_line_offset = offset;
		}

		g_free(contents);
	}
	g_free(path);

	g_free(protocol_name);

	return g_list_reverse(list);
}

static char * trillian_logger_read (PurpleLog *log, PurpleLogReadFlags *flags)
{
	struct trillian_logger_data *data;
	char *read;
	FILE *file;
	PurpleBuddy *buddy;
	char *escaped;
	GString *formatted;
	char *c;
	const char *line;

	if (flags != NULL)
		*flags = PURPLE_LOG_READ_NO_NEWLINE;

	g_return_val_if_fail(log != NULL, g_strdup(""));

	data = log->logger_data;

	g_return_val_if_fail(data->path != NULL, g_strdup(""));
	g_return_val_if_fail(data->length > 0, g_strdup(""));
	g_return_val_if_fail(data->their_nickname != NULL, g_strdup(""));

	purple_debug_info("Trillian log read", "Reading %s\n", data->path);

	file = g_fopen(data->path, "rb");
	g_return_val_if_fail(file != NULL, g_strdup(""));

	read = g_malloc(data->length + 2);

	if (fseek(file, data->offset, SEEK_SET) != 0) {
		fclose(file);
		g_free(read);
		g_return_val_if_reached(g_strdup(""));
	}
	data->length = fread(read, 1, data->length, file);
	fclose(file);

	if (read[data->length-1] == '\n') {
		read[data->length] = '\0';
	} else {
		read[data->length] = '\n';
		read[data->length+1] = '\0';
	}

	/* Load miscellaneous data. */
	buddy = purple_blist_find_buddy(log->account, log->name);

	escaped = g_markup_escape_text(read, -1);
	g_free(read);
	read = escaped;

	/* Apply formatting... */
	formatted = g_string_sized_new(strlen(read));
	c = read;
	line = read;
	while (c)
	{
		const char *link;
		const char *footer = NULL;
		GString *temp = NULL;

		/* There's always a trailing '\n' at the end of the file (see above), so
		 * just quit out if we don't find another, because we're at the end.
		 */
		c = strchr(c, '\n');
		if (!c)
			break;

		*c = '\0';
		c++;

		/* Convert links.
		 *
		 * The format is (Link: URL)URL
		 * So, I want to find each occurance of "(Link: " and replace that chunk with:
		 * <a href="
		 * Then, replace the next ")" with:
		 * ">
		 * Then, replace the next " " (or add this if the end-of-line is reached) with:
		 * </a>
		 *
		 * As implemented, this isn't perfect, but it should cover common cases.
		 */
		while (line && (link = strstr(line, "(Link: ")))
		{
			const char *tmp = link;

			link += 7;
			if (*link)
			{
				char *end_paren;
				char *space;

				if (!(end_paren = strchr(link, ')')))
				{
					/* Something is not as we expect.  Bail out. */
					break;
				}

				if (!temp)
					temp = g_string_sized_new(strlen(line));

				g_string_append_len(temp, line, (tmp - line));

				/* Start an <a> tag. */
				g_string_append(temp, "<a href=\"");

				/* Append up to the ) */
				g_string_append_len(temp, link, end_paren - link);

				/* Finish the <a> tag. */
				g_string_append(temp, "\">");

				/* The \r is a bit of a hack to keep there from being a \r in
				 * the link text, which may not matter. */
				if ((space = strchr(end_paren, ' ')) || (space = strchr(end_paren, '\r')))
				{
					g_string_append_len(temp, end_paren + 1, space - end_paren - 1);

					/* Close the <a> tag. */
					g_string_append(temp, "</a>");

					space++;
				}
				else
				{
					/* There is no space before the end of the line. */
					g_string_append(temp, end_paren + 1);
					/* Close the <a> tag. */
					g_string_append(temp, "</a>");
				}
				line = space;
			}
			else
			{
				/* Something is not as we expect.  Bail out. */
				break;
			}
		}

		if (temp)
		{
			if (line)
				g_string_append(temp, line);
			line = temp->str;
		}

		if (line && *line == '[') {
			const char *timestamp;

			if ((timestamp = strchr(line, ']'))) {
				line++;
				/* TODO: Parse the timestamp and convert it to Purple's format. */
				g_string_append(formatted, "<font size=\"2\">(");
				g_string_append_len(formatted, line, (timestamp - line));
				g_string_append(formatted,")</font> ");
				line = timestamp + 1;
				if (line[0] && line[1])
					line++;
			}

			if (g_str_has_prefix(line, "*** ")) {
				line += (sizeof("*** ") - 1);
				g_string_append(formatted, "<b>");
				footer = "</b>";
				if (g_str_has_prefix(line, "NOTE: This user is offline.")) {
					line = _("User is offline.");
				} else if (g_str_has_prefix(
				                   line,
				                   "NOTE: Your status is currently set to ")) {

					line += (sizeof("NOTE: ") - 1);
				} else if (g_str_has_prefix(line, "Auto-response sent to ")) {
					g_string_append(formatted, _("Auto-response sent:"));
					while (*line && *line != ':')
						line++;
					if (*line)
						line++;
					g_string_append(formatted, "</b>");
					footer = NULL;
				} else if (strstr(line, " signed off ")) {
					const char *alias = NULL;

					if (buddy != NULL)
						alias = purple_buddy_get_alias(buddy);

					if (alias != NULL) {
						g_string_append_printf(formatted,
							_("%s has signed off."), alias);
					} else {
						g_string_append_printf(formatted,
							_("%s has signed off."), log->name);
					}
					line = "";
				} else if (strstr(line, " signed on ")) {
					const char *alias = NULL;

					if (buddy != NULL)
						alias = purple_buddy_get_alias(buddy);

					if (alias != NULL)
						g_string_append(formatted, alias);
					else
						g_string_append(formatted, log->name);

					line = " logged in.";
				} else if (g_str_has_prefix(line, "One or more messages may "
				                                  "have been undeliverable.")) {

					g_string_append(formatted,
						"<span style=\"color: #ff0000;\">");
					g_string_append(formatted,
						_("One or more messages may have been "
						  "undeliverable."));
					line = "";
					footer = "</span></b>";
				} else if (g_str_has_prefix(line,
				                            "You have been disconnected.")) {

					g_string_append(formatted,
						"<span style=\"color: #ff0000;\">");
					g_string_append(formatted,
						_("You were disconnected from the server."));
					line = "";
					footer = "</span></b>";
				} else if (g_str_has_prefix(
				                   line, "You are currently disconnected.")) {

					g_string_append(formatted,
						"<span style=\"color: #ff0000;\">");
					line = _("You are currently disconnected. Messages "
					         "will not be received unless you are "
					         "logged in.");
					footer = "</span></b>";
				} else if (g_str_has_prefix(line, "Your previous message has "
				                                  "not been sent.")) {

					g_string_append(formatted,
						"<span style=\"color: #ff0000;\">");

					if (g_str_has_prefix(
					            line,
					            "Your previous message has not been sent.  "
					            "Reason: Maximum length exceeded.")) {

						g_string_append(formatted,
							_("Message could not be sent because "
							  "the maximum length was exceeded."));
						line = "";
					} else {
						g_string_append(formatted,
							_("Message could not be sent."));
						line += (sizeof(
							"Your previous message "
							"has not been sent. ") - 1);
					}

					footer = "</span></b>";
				}
			} else if (g_str_has_prefix(line, data->their_nickname)) {
				if (buddy != NULL) {
					const char *alias = purple_buddy_get_alias(buddy);

					if (alias != NULL) {
						line += strlen(data->their_nickname) + 2;
						g_string_append_printf(formatted,
							"<span style=\"color: #A82F2F;\">"
							"<b>%s</b></span>: ", alias);
					}
				}
			} else {
				const char *line2 = strchr(line, ':');
				if (line2) {
					const char *acct_name;
					line2++;
					line = line2;
					acct_name = purple_account_get_private_alias(log->account);
					if (!acct_name)
						acct_name = purple_account_get_username(log->account);

					g_string_append_printf(formatted,
						"<span style=\"color: #16569E;\">"
						"<b>%s</b></span>:", acct_name);
				}
			}
		}

		if (line)
			g_string_append(formatted, line);

		line = c;
		if (temp)
			g_string_free(temp, TRUE);

		if (footer)
			g_string_append(formatted, footer);

		g_string_append(formatted, "<br>");
	}

	g_free(read);

	/* XXX: TODO: What can we do about removing \r characters?
	 * XXX: TODO: and will that allow us to avoid this
	 * XXX: TODO: g_strchomp(), or is that unrelated? */
	/* XXX: TODO: Avoid this g_strchomp() */
	return g_strchomp(g_string_free(formatted, FALSE));
}

static int trillian_logger_size (PurpleLog *log)
{
	struct trillian_logger_data *data;
	char *text;
	size_t size;

	g_return_val_if_fail(log != NULL, 0);

	data = log->logger_data;

	if (purple_prefs_get_bool("/plugins/core/log_reader/fast_sizes")) {
		return data ? data->length : 0;
	}

	text = trillian_logger_read(log, NULL);
	size = strlen(text);
	g_free(text);

	return size;
}

static void trillian_logger_finalize(PurpleLog *log)
{
	struct trillian_logger_data *data;

	g_return_if_fail(log != NULL);

	data = log->logger_data;

	g_free(data->path);
	g_free(data->their_nickname);
	g_free(data);
}

/*****************************************************************************
 * QIP Logger                                                           *
 *****************************************************************************/

/* The QIP logger doesn't write logs, only reads them.  This is to include
 * QIP logs in the log viewer transparently.
 */
#define QIP_LOG_DELIMITER "--------------------------------------"
#define QIP_LOG_IN_MESSAGE (QIP_LOG_DELIMITER "<-")
#define QIP_LOG_OUT_MESSAGE (QIP_LOG_DELIMITER ">-")
#define QIP_LOG_IN_MESSAGE_ESC (QIP_LOG_DELIMITER "&lt;-")
#define QIP_LOG_OUT_MESSAGE_ESC (QIP_LOG_DELIMITER "&gt;-")
#define QIP_LOG_TIMEOUT (G_TIME_SPAN_HOUR)

static PurpleLogLogger *qip_logger;

struct qip_logger_data {

	char *path; /* FIXME: Change this to use PurpleStringref like log.c:old_logger_list  */
	int offset;
	int length;
};

static GList *qip_logger_list(PurpleLogType type, const char *sn, PurpleAccount *account)
{
	GList *list = NULL;
	const char *logdir;
	PurpleProtocol *protocol;
	char *username;
	char *filename;
	char *path;
	char *contents;
	struct qip_logger_data *data = NULL;
	GDateTime *prev_dt = NULL;
	GDateTime *dt = NULL;
	gint year, month, day, hour, minute, second;
	gboolean main_cycle = TRUE;
	char *c;
	char *start_log;
	char *new_line = NULL;
	int offset = 0;
	GError *error;

	g_return_val_if_fail(sn != NULL, NULL);
	g_return_val_if_fail(account != NULL, NULL);

	/* QIP only supports ICQ. */
	if (!purple_strequal(purple_account_get_protocol_id(account), "prpl-icq"))
		return NULL;

	logdir = purple_prefs_get_string("/plugins/core/log_reader/qip/log_directory");

	/* By clearing the log directory path, this logger can be (effectively) disabled. */
	if (!logdir || !*logdir)
		return NULL;

	protocol = purple_account_get_protocol(account);
	if (!protocol)
		return NULL;

	username = g_strdup(purple_normalize(account, purple_account_get_username(account)));
	filename = g_strdup_printf("%s.txt", purple_normalize(account, sn));
	path = g_build_filename(logdir, username, "History", filename, NULL);
	g_free(username);
	g_free(filename);

	purple_debug_info("QIP logger", "Reading %s\n", path);

	error = NULL;
	if (!g_file_get_contents(path, &contents, NULL, &error)) {
		purple_debug_error("QIP logger",
				   "Couldn't read file %s: %s \n", path,
				   (error && error->message) ? error->message : "Unknown error");
		if (error)
			g_error_free(error);
		g_free(path);
		return list;
	}

	c = contents;
	start_log = contents;
	while (main_cycle) {

		gboolean add_new_log = FALSE;

		if (c && *c) {
			if (g_str_has_prefix(c, QIP_LOG_IN_MESSAGE) ||
			    g_str_has_prefix(c, QIP_LOG_OUT_MESSAGE)) {

				char *tmp;

				new_line = c;

				/* find EOL */
				c = strchr(c, '\n');
				if (c)
					c++;

				/* Find the last '(' character. */
				if (!c) {
					/* do nothing */
				} else if ((tmp = strchr(c, '\n')) != NULL) {
					while (*tmp && *tmp != '(') --tmp;
					c = tmp;
				} else {
					while (*c)
						c++;
					c--;
					c = g_strrstr(c, "(");
				}

				if (c != NULL) {
					const char *timestamp = ++c;

					/*  Parse the time, day, month and year  */
					if (sscanf(timestamp, "%u:%u:%u %u/%u/%u",
						&hour, &minute, &second,
						&day, &month, &year) != 6) {

						purple_debug_error("QIP logger list",
							"Parsing timestamp error\n");
					} else {
						g_date_time_unref(dt);
						dt = g_date_time_new_local(year, month, day, hour, minute, second);
						if (!prev_dt) {
							prev_dt = dt;
						} else {
							add_new_log = g_date_time_difference(dt, prev_dt) > QIP_LOG_TIMEOUT;
						}
					}
				}
			}
		} else {
			add_new_log = TRUE;
			main_cycle = FALSE;
			new_line = c;
		}

		/* adding  log */
		if (add_new_log && prev_dt) {
			PurpleLog *log;

			/* filling data */
			data = g_new0(struct qip_logger_data, 1);
			data->path = g_strdup(path);
			data->length = new_line - start_log;
			data->offset = offset;
			offset += data->length;
			purple_debug_info("QIP logger list",
				"Creating log: path = (%s); length = (%d); offset = (%d)\n",
				data->path, data->length, data->offset);

			/* XXX: Look into this later... Should we figure out a timezone? */
			dt = g_date_time_new_local(year, month, day, hour, minute, second);
			log = purple_log_new(PURPLE_LOG_IM, sn, account,
				NULL, prev_dt);

			log->logger = qip_logger;
			log->logger_data = data;

			list = g_list_prepend(list, log);

			g_date_time_unref(prev_dt);
			prev_dt = dt;
			start_log = new_line;
		}

		if (c && *c) {
			/* find EOF */
			if ((c = strchr(c, '\n')))
				c++;
		}
	}

	g_free(contents);
	g_free(path);
	return g_list_reverse(list);
}

static char *qip_logger_read(PurpleLog *log, PurpleLogReadFlags *flags)
{
	struct qip_logger_data *data;
	PurpleBuddy *buddy;
	GString *formatted;
	char *c;
	const char *line;
	gchar *contents;
	GError *error;
	char *utf8_string;
	FILE *file;

	if (flags != NULL)
		*flags = PURPLE_LOG_READ_NO_NEWLINE;

	g_return_val_if_fail(log != NULL, g_strdup(""));

	data = log->logger_data;

	g_return_val_if_fail(data->path != NULL, g_strdup(""));
	g_return_val_if_fail(data->length > 0, g_strdup(""));

	file = g_fopen(data->path, "rb");
	g_return_val_if_fail(file != NULL, g_strdup(""));

	contents = g_malloc(data->length + 2);

	if (fseek(file, data->offset, SEEK_SET) != 0) {
		fclose(file);
		g_free(contents);
		g_return_val_if_reached(g_strdup(""));
	}
	data->length = fread(contents, 1, data->length, file);
	fclose(file);

	contents[data->length] = '\n';
	contents[data->length + 1] = '\0';

	/* Convert file contents from Cp1251 to UTF-8 codeset */
	error = NULL;
	if (!(utf8_string = g_convert(contents, -1, "UTF-8", "Cp1251", NULL, NULL, &error))) {
		purple_debug_error("QIP logger",
			"Couldn't convert file %s to UTF-8: %s\n", data->path,
				   (error && error->message) ? error->message : "Unknown error");
		if (error)
			g_error_free(error);
		g_free(contents);
		return g_strdup("");
	}

	g_free(contents);
	contents = g_markup_escape_text(utf8_string, -1);
	g_free(utf8_string);

	buddy = purple_blist_find_buddy(log->account, log->name);

	/* Apply formatting... */
	formatted = g_string_sized_new(data->length + 2);
	c = contents;
	line = contents;

	while (c && *c) {
		gboolean is_in_message = FALSE;

		if (g_str_has_prefix(line, QIP_LOG_IN_MESSAGE_ESC) ||
		    g_str_has_prefix(line, QIP_LOG_OUT_MESSAGE_ESC)) {

			char *tmp;

			is_in_message = g_str_has_prefix(line, QIP_LOG_IN_MESSAGE_ESC);

			/* find EOL */
			c = strchr(c, '\n');
			if (!c)
				break;

			/* Find the last '(' character. */
			if ((tmp = strchr(c, '\n')) != NULL) {
				while (*tmp && *tmp != '(') --tmp;
				c = tmp;
			} else {
				while (*c)
					c++;
				c--;
				c = g_strrstr(c, "(");
			}

			if (c != NULL) {
				const char *timestamp = c;
				int hour;
				int min;
				int sec;

				timestamp++;

				/*  Parse the time, day, month and year */
				if (sscanf(timestamp, "%u:%u:%u",
				           &hour, &min, &sec) != 3) {
					purple_debug_error("QIP logger read",
					                   "Parsing timestamp error\n");
				} else {
					g_string_append(formatted, "<font size=\"2\">");
					/* TODO: Figure out if we can do anything more locale-independent. */
					g_string_append_printf(formatted,
						"(%u:%02u:%02u) %cM ", hour % 12,
						min, sec, (hour >= 12) ? 'P': 'A');
					g_string_append(formatted, "</font> ");

					if (is_in_message) {
						const char *alias = NULL;

						if (buddy != NULL &&
						    (alias = purple_buddy_get_alias(buddy))) {
							g_string_append_printf(formatted,
								"<span style=\"color: #A82F2F;\">"
								"<b>%s</b></span>: ", alias);
						}
					} else {
						const char *acct_name;
						acct_name = purple_account_get_private_alias(log->account);
						if (!acct_name)
							acct_name = purple_account_get_username(log->account);

						g_string_append_printf(formatted,
							"<span style=\"color: #16569E;\">"
							"<b>%s</b></span>: ", acct_name);
					}

					/* find EOF */
					c = strchr(c, '\n');
					if (c)
						c++;
					line = c;
				}
			}
		} else {
			if ((c = strchr(c, '\n')))
				*c = '\0';

			if (line[0] != '\n' && line[0] != '\r') {

				g_string_append(formatted, line);
				g_string_append(formatted, "<br>");
			}

			if (c)
				line = ++c;
		}
	}
	g_free(contents);

	/* XXX: TODO: Avoid this g_strchomp() */
	return g_strchomp(g_string_free(formatted, FALSE));
}

static int qip_logger_size (PurpleLog *log)
{
	struct qip_logger_data *data;
	char *text;
	size_t size;

	g_return_val_if_fail(log != NULL, 0);

	data = log->logger_data;

	if (purple_prefs_get_bool("/plugins/core/log_reader/fast_sizes")) {
		return data ? data->length : 0;
	}

	text = qip_logger_read(log, NULL);
	size = strlen(text);
	g_free(text);

	return size;
}

static void qip_logger_finalize(PurpleLog *log)
{
	struct qip_logger_data *data;

	g_return_if_fail(log != NULL);

	data = log->logger_data;

	g_free(data->path);
	g_free(data);
}

/*************************************************************************
 * aMSN Logger                                                           *
 *************************************************************************/

/* The aMSN logger doesn't write logs, only reads them.  This is to include
 * aMSN logs in the log viewer transparently.
 */

static PurpleLogLogger *amsn_logger;

struct amsn_logger_data {
	char *path;
	int offset;
	int length;
};

#define AMSN_LOG_CONV_START "|\"LRED[Conversation started on "
#define AMSN_LOG_CONV_END "|\"LRED[You have closed the window on "
#define AMSN_LOG_CONV_EXTRA "01 Aug 2001 00:00:00]"

static GList *amsn_logger_parse_file(char *filename, const char *sn, PurpleAccount *account)
{
	GList *list = NULL;
	GError *error;
	char *contents;
	struct amsn_logger_data *data;
	PurpleLog *log;

	purple_debug_info("aMSN logger", "Reading %s\n", filename);
	error = NULL;
	if (!g_file_get_contents(filename, &contents, NULL, &error)) {
		purple_debug_error("aMSN logger",
		                   "Couldn't read file %s: %s \n", filename,
		                   (error && error->message) ?
		                    error->message : "Unknown error");
		if (error)
			g_error_free(error);
	} else {
		char *c = contents;
		gboolean found_start = FALSE;
		char *start_log = c;
		int offset = 0;
		gint year, month, day, hour, minute, second;
		GDateTime *dt;
		while (c && *c) {
			if (g_str_has_prefix(c, AMSN_LOG_CONV_START)) {
				char month_str[4];
				if (sscanf(c + strlen(AMSN_LOG_CONV_START),
				           "%u %3s %u %u:%u:%u",
				           &day, (char*)&month_str, &year,
				           &hour, &minute, &second) != 6) {
					found_start = FALSE;
					purple_debug_error("aMSN logger",
					                   "Error parsing start date for %s\n",
					                   filename);
				} else {
					month = purple_time_parse_month(month_str);

					found_start = TRUE;
					offset = c - contents;
					start_log = c;
				}
			} else if (g_str_has_prefix(c, AMSN_LOG_CONV_END) && found_start) {
				data = g_new0(struct amsn_logger_data, 1);
				data->path = g_strdup(filename);
				data->offset = offset;
				data->length = c - start_log
					             + strlen(AMSN_LOG_CONV_END)
					             + strlen(AMSN_LOG_CONV_EXTRA);
				dt = g_date_time_new_local(year, month, day, hour, minute, second);
				log = purple_log_new(PURPLE_LOG_IM, sn, account, NULL, dt);
				log->logger = amsn_logger;
				log->logger_data = data;
				list = g_list_prepend(list, log);
				found_start = FALSE;
				g_date_time_unref(dt);

				purple_debug_info("aMSN logger",
				                  "Found log for %s:"
				                  " path = (%s),"
				                  " offset = (%d),"
				                  " length = (%d)\n",
				                  sn, data->path, data->offset, data->length);
			}
			c = strchr(c, '\n');
			if (c)
				c++;
		}

		/* I've seen the file end without the AMSN_LOG_CONV_END bit */
		if (found_start) {
			data = g_new0(struct amsn_logger_data, 1);
			data->path = g_strdup(filename);
			data->offset = offset;
			data->length = c - start_log
				             + strlen(AMSN_LOG_CONV_END)
				             + strlen(AMSN_LOG_CONV_EXTRA);
			dt = g_date_time_new_local(year, month, day, hour, minute, second);
			log = purple_log_new(PURPLE_LOG_IM, sn, account, NULL, dt);
			log->logger = amsn_logger;
			log->logger_data = data;
			list = g_list_prepend(list, log);
			g_date_time_unref(dt);

			purple_debug_info("aMSN logger",
			                  "Found log for %s:"
			                  " path = (%s),"
			                  " offset = (%d),"
			                  " length = (%d)\n",
			                  sn, data->path, data->offset, data->length);
		}
		g_free(contents);
	}

	return list;
}

/* `log_dir`/username@hotmail.com/logs/buddyname@hotmail.com.log */
/* `log_dir`/username@hotmail.com/logs/Month Year/buddyname@hotmail.com.log */
static GList *amsn_logger_list(PurpleLogType type, const char *sn, PurpleAccount *account)
{
	GList *list = NULL;
	const char *logdir;
	char *username;
	char *log_path;
	char *buddy_log;
	char *filename;
	GDir *dir;
	const char *name;

	logdir = purple_prefs_get_string("/plugins/core/log_reader/amsn/log_directory");

	/* By clearing the log directory path, this logger can be (effectively) disabled. */
	if (!logdir || !*logdir)
		return NULL;

	/* aMSN only works with MSN/WLM */
	if (!purple_strequal(purple_account_get_protocol_id(account), "prpl-msn"))
		return NULL;

	username = g_strdup(purple_normalize(account, purple_account_get_username(account)));
	buddy_log = g_strdup_printf("%s.log", purple_normalize(account, sn));
	log_path = g_build_filename(logdir, username, "logs", NULL);

	/* First check in the top-level */
	filename = g_build_filename(log_path, buddy_log, NULL);
	if (g_file_test(filename, G_FILE_TEST_EXISTS))
		list = amsn_logger_parse_file(filename, sn, account);
	else
		g_free(filename);

	/* Check in previous months */
	dir = g_dir_open(log_path, 0, NULL);
	if (dir) {
		while ((name = g_dir_read_name(dir)) != NULL) {
			filename = g_build_filename(log_path, name, buddy_log, NULL);
			if (g_file_test(filename, G_FILE_TEST_EXISTS))
				list = g_list_concat(list, amsn_logger_parse_file(filename, sn, account));
			g_free(filename);
		}
		g_dir_close(dir);
	}

	g_free(log_path);

	/* New versions use 'friendlier' directory names */
	purple_util_chrreplace(username, '@', '_');
	purple_util_chrreplace(username, '.', '_');

	log_path = g_build_filename(logdir, username, "logs", NULL);

	/* First check in the top-level */
	filename = g_build_filename(log_path, buddy_log, NULL);
	if (g_file_test(filename, G_FILE_TEST_EXISTS))
		list = g_list_concat(list, amsn_logger_parse_file(filename, sn, account));
	g_free(filename);

	/* Check in previous months */
	dir = g_dir_open(log_path, 0, NULL);
	if (dir) {
		while ((name = g_dir_read_name(dir)) != NULL) {
			filename = g_build_filename(log_path, name, buddy_log, NULL);
			if (g_file_test(filename, G_FILE_TEST_EXISTS))
				list = g_list_concat(list, amsn_logger_parse_file(filename, sn, account));
			g_free(filename);
		}
		g_dir_close(dir);
	}

	g_free(log_path);
	g_free(username);
	g_free(buddy_log);

	return list;
}

/* Really it's |"L, but the string's been escaped */
#define AMSN_LOG_FORMAT_TAG "|&quot;L"

static char *amsn_logger_read(PurpleLog *log, PurpleLogReadFlags *flags)
{
	struct amsn_logger_data *data;
	FILE *file;
	char *contents;
	char *escaped;
	GString *formatted;
	char *start;
	gboolean in_span = FALSE;

	if (flags != NULL)
		*flags = PURPLE_LOG_READ_NO_NEWLINE;

	g_return_val_if_fail(log != NULL, g_strdup(""));

	data = log->logger_data;

	g_return_val_if_fail(data->path != NULL, g_strdup(""));
	g_return_val_if_fail(data->length > 0, g_strdup(""));

	file = g_fopen(data->path, "rb");
	g_return_val_if_fail(file != NULL, g_strdup(""));

	contents = g_malloc(data->length + 2);

	if (fseek(file, data->offset, SEEK_SET) != 0) {
		fclose(file);
		g_free(contents);
		g_return_val_if_reached(g_strdup(""));
	}
	data->length = fread(contents, 1, data->length, file);
	fclose(file);

	contents[data->length] = '\n';
	contents[data->length + 1] = '\0';

	escaped = g_markup_escape_text(contents, -1);
	g_free(contents);
	contents = escaped;

	formatted = g_string_sized_new(data->length + 2);

	start = contents;
	while (start && *start) {
		char *end;
		char *old_tag;
		char *tag;
		end = strchr(start, '\n');
		if (!end)
			break;
		*end = '\0';
		if (g_str_has_prefix(start, AMSN_LOG_FORMAT_TAG) && in_span) {
			/* New format for this line */
			g_string_append(formatted, "</span><br>");
			in_span = FALSE;
		} else if (start != contents) {
			/* Continue format from previous line */
			g_string_append(formatted, "<br>");
		}
		old_tag = start;
		tag = strstr(start, AMSN_LOG_FORMAT_TAG);
		while (tag) {
			g_string_append_len(formatted, old_tag, tag - old_tag);
			tag += strlen(AMSN_LOG_FORMAT_TAG);
			if (in_span) {
				g_string_append(formatted, "</span>");
				in_span = FALSE;
			}
			if (*tag == 'C') {
				/* |"LCxxxxxx is a hex colour */
				char colour[7];
				strncpy(colour, tag + 1, 6);
				colour[6] = '\0';
				g_string_append_printf(formatted, "<span style=\"color: #%s;\">", colour);
				/* This doesn't appear to work? */
				/* g_string_append_printf(formatted, "<span style=\"color: #%6s;\">", tag + 1); */
				in_span = TRUE;
				old_tag = tag + 7; /* C + xxxxxx */
			} else {
				/* |"Lxxx is a 3-digit colour code */
				if (g_str_has_prefix(tag, "RED")) {
					g_string_append(formatted, "<span style=\"color: red;\">");
					in_span = TRUE;
				} else if (g_str_has_prefix(tag, "GRA")) {
					g_string_append(formatted, "<span style=\"color: gray;\">");
					in_span = TRUE;
				} else if (g_str_has_prefix(tag, "NOR")) {
					g_string_append(formatted, "<span style=\"color: black;\">");
					in_span = TRUE;
				} else if (g_str_has_prefix(tag, "ITA")) {
					g_string_append(formatted, "<span style=\"color: blue;\">");
					in_span = TRUE;
				} else if (g_str_has_prefix(tag, "GRE")) {
					g_string_append(formatted, "<span style=\"color: darkgreen;\">");
					in_span = TRUE;
				} else {
					purple_debug_info("aMSN logger", "Unknown colour format: %3s\n", tag);
				}
				old_tag = tag + 3;
			}
			tag = strstr(tag, AMSN_LOG_FORMAT_TAG);
		}
		g_string_append(formatted, old_tag);
		start = end + 1;
	}
	if (in_span)
		g_string_append(formatted, "</span>");

	g_free(contents);

	return g_string_free(formatted, FALSE);
}

static int amsn_logger_size(PurpleLog *log)
{
	struct amsn_logger_data *data;
	char *text;
	int size;

	g_return_val_if_fail(log != NULL, 0);

	data = log->logger_data;

	if (purple_prefs_get_bool("/plugins/core/log_reader/fast_sizes")) {
		return data ? data->length : 0;
	}

	text = amsn_logger_read(log, NULL);
	size = strlen(text);
	g_free(text);

	return size;
}

static void amsn_logger_finalize(PurpleLog *log)
{
	struct amsn_logger_data *data;

	g_return_if_fail(log != NULL);

	data = log->logger_data;
	g_free(data->path);
	g_free(data);
}

/*****************************************************************************
 * Plugin Code                                                               *
 *****************************************************************************/

static void log_reader_init_prefs(void) {
	char *path;
#ifdef _WIN32
	const gchar *reg_key;
	char *folder;
	gboolean found = FALSE;
#endif

	purple_prefs_add_none("/plugins/core/log_reader");


	/* Add general preferences. */

	purple_prefs_add_bool("/plugins/core/log_reader/fast_sizes", FALSE);
	purple_prefs_add_bool("/plugins/core/log_reader/use_name_heuristics", TRUE);


	/* Add Adium log directory preference. */
	purple_prefs_add_none("/plugins/core/log_reader/adium");

	/* Calculate default Adium log directory. */
#ifdef _WIN32
	purple_prefs_add_string("/plugins/core/log_reader/adium/log_directory", "");
#else
	path = g_build_filename(purple_home_dir(), "Library", "Application Support",
	                        "Adium 2.0", "Users", "Default", "Logs", NULL);
	purple_prefs_add_string("/plugins/core/log_reader/adium/log_directory", path);
	g_free(path);
#endif


	/* Add Fire log directory preference. */
	purple_prefs_add_none("/plugins/core/log_reader/fire");

	/* Calculate default Fire log directory. */
#ifdef _WIN32
	purple_prefs_add_string("/plugins/core/log_reader/fire/log_directory", "");
#else
	path = g_build_filename(purple_home_dir(), "Library", "Application Support",
	                        "Fire", "Sessions", NULL);
	purple_prefs_add_string("/plugins/core/log_reader/fire/log_directory", path);
	g_free(path);
#endif


	/* Add Messenger Plus! log directory preference. */
	purple_prefs_add_none("/plugins/core/log_reader/messenger_plus");

	/* Calculate default Messenger Plus! log directory. */
#ifdef _WIN32
	path = NULL;
	folder = wpurple_get_special_folder(CSIDL_PERSONAL);
	if (folder) {
		path = g_build_filename(folder, "My Chat Logs", NULL);
		g_free(folder);
	}
#else
	path = g_build_filename(PURPLE_LOG_READER_WINDOWS_MOUNT_POINT,
	                        "Documents and Settings", g_get_user_name(),
	                        "My Documents", "My Chat Logs", NULL);
#endif
	purple_prefs_add_string("/plugins/core/log_reader/messenger_plus/log_directory", path ? path : "");
	g_free(path);


	/* Add MSN Messenger log directory preference. */
	purple_prefs_add_none("/plugins/core/log_reader/msn");

	/* Calculate default MSN message history directory. */
#ifdef _WIN32
	path = NULL;
	folder = wpurple_get_special_folder(CSIDL_PERSONAL);
	if (folder) {
		path = g_build_filename(folder, "My Received Files", NULL);
		g_free(folder);
	}
#else
	path = g_build_filename(PURPLE_LOG_READER_WINDOWS_MOUNT_POINT,
	                        "Documents and Settings", g_get_user_name(),
	                        "My Documents", "My Received Files", NULL);
#endif
	purple_prefs_add_string("/plugins/core/log_reader/msn/log_directory", path ? path : "");
	g_free(path);


	/* Add Trillian log directory preference. */
	purple_prefs_add_none("/plugins/core/log_reader/trillian");

#ifdef _WIN32
	/* XXX: While a major hack, this is the most reliable way I could
	 * think of to determine the Trillian installation directory.
	 */

	path = NULL;
	folder = NULL;
	reg_key = "Trillian.SkinZip\\shell\\Add\\command\\";
	if (wpurple_reg_val_exists(HKEY_CLASSES_ROOT, reg_key, NULL))
		folder = wpurple_read_reg_string(HKEY_CLASSES_ROOT, reg_key, NULL);
	if (folder) {
		char *value = folder;
		char *temp;

		/* Break apart buffer. */
		if (*value == '"') {
			value++;
			temp = value;
			while (*temp && *temp != '"')
				temp++;
		} else {
			temp = value;
			while (*temp && *temp != ' ')
				temp++;
		}
		*temp = '\0';

		/* Set path. */
		if (g_str_has_suffix(value, "trillian.exe")) {
			value[strlen(value) - (sizeof("trillian.exe") - 1)] = '\0';
			path = g_build_filename(value, "users", "default", "talk.ini", NULL);
		}
		g_free(folder);
	}

	if (!path) {
		char *folder = wpurple_get_special_folder(CSIDL_PROGRAM_FILES);
		if (folder) {
			path = g_build_filename(folder, "Trillian",
			                        "users", "default", "talk.ini", NULL);
			g_free(folder);
		}
	}

	if (path) {
		/* Read talk.ini file to find the log directory. */
		GError *error = NULL;
		gchar *contents = NULL;

		if (g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
			purple_debug_info("Trillian talk.ini read",
				"Reading %s\n", path);
		} else {
			g_free(path);
			path = NULL;
		}

		if (path && !g_file_get_contents(path, &contents, NULL, &error)) {
			purple_debug_error("Trillian talk.ini read",
					   "Error reading talk.ini: %s\n",
					   (error && error->message) ? error->message : "Unknown error");
			if (error)
				g_error_free(error);
		} else if (contents) {
			char *cursor, *line;
			line = cursor = contents;
			while (*cursor) {
				if (*cursor == '\n') {
					*cursor = '\0';

					/* XXX: This assumes the first Directory key is under [Logging]. */
					if (g_str_has_prefix(line, "Directory=")) {
						line += (sizeof("Directory=") - 1);
						g_strchomp(line);
						purple_prefs_add_string(
							"/plugins/core/log_reader/trillian/log_directory",
							line);
						found = TRUE;
					}

					cursor++;
					line = cursor;
				} else
					cursor++;
			}
			g_free(contents);
		}
		g_free(path);
	} /* path */

	if (!found) {
		path = NULL;
		folder = wpurple_get_special_folder(CSIDL_PROGRAM_FILES);
		if (folder) {
			path = g_build_filename(folder, "Trillian", "users",
			                        "default", "logs", NULL);
			g_free(folder);
		}

		purple_prefs_add_string(
			"/plugins/core/log_reader/trillian/log_directory", path ? path : "");
		g_free(path);
	}
#else /* !defined(_WIN32) */
	/* TODO: At some point, this could attempt to parse talk.ini
	 * TODO: from the default Trillian install directory on the
	 * TODO: Windows mount point. */

	/* Calculate default Trillian log directory. */
	path = g_build_filename(PURPLE_LOG_READER_WINDOWS_MOUNT_POINT,
	                        "Program Files", "Trillian", "users",
	                        "default", "logs", NULL);
	purple_prefs_add_string(
		"/plugins/core/log_reader/trillian/log_directory", path);
	g_free(path);
#endif

	/* Add QIP log directory preference. */
	purple_prefs_add_none("/plugins/core/log_reader/qip");

	/* Calculate default QIP log directory. */
#ifdef _WIN32
	path = NULL;
	folder = wpurple_get_special_folder(CSIDL_PROGRAM_FILES);
	if (folder) {
		path = g_build_filename(folder, "QIP", "Users", NULL);
		g_free(folder);
	}
#else
	path = g_build_filename(PURPLE_LOG_READER_WINDOWS_MOUNT_POINT,
	                        "Program Files", "QIP", "Users", NULL);
#endif
	purple_prefs_add_string("/plugins/core/log_reader/qip/log_directory", path ? path : "");
	g_free(path);

	/* Add aMSN Messenger log directory preference. */
	purple_prefs_add_none("/plugins/core/log_reader/amsn");

	/* Calculate default aMSN log directory. */
#ifdef _WIN32
	path = NULL;
	folder = wpurple_get_special_folder(CSIDL_PROFILE); /* Silly aMSN, not using CSIDL_APPDATA */
	if (folder) {
		path = g_build_filename(folder, "amsn", NULL);
		g_free(folder);
	}
#else
	path = g_build_filename(purple_home_dir(), ".amsn", NULL);
#endif
	purple_prefs_add_string("/plugins/core/log_reader/amsn/log_directory", path ? path : "");
	g_free(path);
}

static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin)
{
	PurplePluginPrefFrame *frame;
	PurplePluginPref *ppref;

	g_return_val_if_fail(plugin != NULL, FALSE);

	frame = purple_plugin_pref_frame_new();


	/* Add general preferences. */

	ppref = purple_plugin_pref_new_with_label(_("General Log Reading Configuration"));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
		"/plugins/core/log_reader/fast_sizes", _("Fast size calculations"));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
		"/plugins/core/log_reader/use_name_heuristics", _("Use name heuristics"));
	purple_plugin_pref_frame_add(frame, ppref);


	/* Add Log Directory preferences. */

	ppref = purple_plugin_pref_new_with_label(_("Log Directory"));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
		"/plugins/core/log_reader/adium/log_directory", _("Adium"));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
		"/plugins/core/log_reader/qip/log_directory", _("QIP"));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
		"/plugins/core/log_reader/msn/log_directory", _("MSN Messenger"));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
		"/plugins/core/log_reader/trillian/log_directory", _("Trillian"));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
		"/plugins/core/log_reader/amsn/log_directory", _("aMSN"));
	purple_plugin_pref_frame_add(frame, ppref);

	return frame;
}

static GPluginPluginInfo *
log_reader_query(GError **error)
{
	const gchar * const authors[] = {
		"Richard Laager <rlaager@pidgin.im>",
		NULL
	};

	return purple_plugin_info_new(
		"id",             "core-log_reader",
		"name",           N_("Log Reader"),
		"version",        DISPLAY_VERSION,
		"category",       N_("Utility"),
		"summary",        N_("Includes other IM clients' logs in the log "
		                     "viewer."),
		"description",    N_("When viewing logs, this plugin will include "
		                     "logs from other IM clients. Currently, this "
		                     "includes Adium, MSN Messenger, aMSN, and "
		                     "Trillian.\n\n"
		                     "WARNING: This plugin is still alpha code and "
		                     "may crash frequently.  Use it at your own "
		                     "risk!"),
		"authors",        authors,
		"website",        PURPLE_WEBSITE,
		"abi-version",    PURPLE_ABI_VERSION,
		"pref-frame-cb",  get_plugin_pref_frame,
		NULL
	);
}

static gboolean
log_reader_load(GPluginPlugin *plugin, GError **error)
{
	g_return_val_if_fail(plugin != NULL, FALSE);

	log_reader_init_prefs();

	/* The names of IM clients are marked for translation at the request of
	   translators who wanted to transliterate them.  Many translators
	   choose to leave them alone.  Choose what's best for your language. */
	adium_logger = purple_log_logger_new("adium", _("Adium"), 6,
									   NULL,
									   NULL,
									   adium_logger_finalize,
									   adium_logger_list,
									   adium_logger_read,
									   adium_logger_size);
	purple_log_logger_add(adium_logger);

	/* The names of IM clients are marked for translation at the request of
	   translators who wanted to transliterate them.  Many translators
	   choose to leave them alone.  Choose what's best for your language. */
	qip_logger = purple_log_logger_new("qip", _("QIP"), 6,
											NULL,
											NULL,
											qip_logger_finalize,
											qip_logger_list,
											qip_logger_read,
											qip_logger_size);
	purple_log_logger_add(qip_logger);

	/* The names of IM clients are marked for translation at the request of
	   translators who wanted to transliterate them.  Many translators
	   choose to leave them alone.  Choose what's best for your language. */
	msn_logger = purple_log_logger_new("msn", _("MSN Messenger"), 6,
									 NULL,
									 NULL,
									 msn_logger_finalize,
									 msn_logger_list,
									 msn_logger_read,
									 msn_logger_size);
	purple_log_logger_add(msn_logger);

	/* The names of IM clients are marked for translation at the request of
	   translators who wanted to transliterate them.  Many translators
	   choose to leave them alone.  Choose what's best for your language. */
	trillian_logger = purple_log_logger_new("trillian", _("Trillian"), 6,
										  NULL,
										  NULL,
										  trillian_logger_finalize,
										  trillian_logger_list,
										  trillian_logger_read,
										  trillian_logger_size);
	purple_log_logger_add(trillian_logger);

	/* The names of IM clients are marked for translation at the request of
	   translators who wanted to transliterate them.  Many translators
	   choose to leave them alone.  Choose what's best for your language. */
	amsn_logger = purple_log_logger_new("amsn", _("aMSN"), 6,
									   NULL,
									   NULL,
									   amsn_logger_finalize,
									   amsn_logger_list,
									   amsn_logger_read,
									   amsn_logger_size);
	purple_log_logger_add(amsn_logger);

	return TRUE;
}

static gboolean
log_reader_unload(GPluginPlugin *plugin, GError **error)
{
	g_return_val_if_fail(plugin != NULL, FALSE);

	purple_log_logger_remove(adium_logger);
	purple_log_logger_free(adium_logger);
	adium_logger = NULL;

	purple_log_logger_remove(msn_logger);
	purple_log_logger_free(msn_logger);
	msn_logger = NULL;

	purple_log_logger_remove(trillian_logger);
	purple_log_logger_free(trillian_logger);
	trillian_logger = NULL;

	purple_log_logger_remove(qip_logger);
	purple_log_logger_free(qip_logger);
	qip_logger = NULL;

	purple_log_logger_remove(amsn_logger);
	purple_log_logger_free(amsn_logger);
	amsn_logger = NULL;

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(log_reader)
