/*
 * purple - Jabber Protocol Plugin
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
 *
 */

#include "config.h"
#include "purple.h"
#include "jingle.h"
#include "xmlnode.h"
#include "iq.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>

#ifdef USE_VV

#include <gst/farsight/fs-candidate.h>

static gboolean
jabber_jingle_session_equal(gconstpointer a, gconstpointer b)
{
	purple_debug_info("jingle", 
					  "jabber_jingle_session_equal, comparing %s and %s\n",
					  ((JingleSession *)a)->id,
					  ((JingleSession *)b)->id);
	return !strcmp(((JingleSession *) a)->id, ((JingleSession *) b)->id);
}

static JingleSession *
jabber_jingle_session_create_internal(JabberStream *js,
									  const char *id)
{
    JingleSession *sess = g_new0(JingleSession, 1);
	sess->js = js;
		
	if (id) {
		sess->id = g_strdup(id);
	} else if (js) {
		/* init the session ID... */
		sess->id = jabber_get_next_id(js);
	}
	
	/* insert it into the hash table */
	if (!js->sessions) {
		purple_debug_info("jingle", "Creating hash table for sessions\n");
		js->sessions = g_hash_table_new(g_str_hash, g_str_equal);
	}
	purple_debug_info("jingle", "inserting session with key: %s into table\n",
					  sess->id);
	g_hash_table_insert(js->sessions, sess->id, sess);

	sess->session_started = FALSE;

	return sess;
}

JabberStream *
jabber_jingle_session_get_js(const JingleSession *sess)
{
	return sess->js;
}

JingleSession *
jabber_jingle_session_create(JabberStream *js)
{
	JingleSession *sess = jabber_jingle_session_create_internal(js, NULL);
	sess->is_initiator = TRUE;	
	return sess;
}

JingleSession *
jabber_jingle_session_create_by_id(JabberStream *js, const char *id)
{
	JingleSession *sess = jabber_jingle_session_create_internal(js, id);
	sess->is_initiator = FALSE;
	return sess;
}

const char *
jabber_jingle_session_get_id(const JingleSession *sess)
{
	return sess->id;
}

void
jabber_jingle_session_destroy(JingleSession *sess)
{
	g_hash_table_remove(sess->js->sessions, sess->id);
	g_free(sess->id);
	g_object_unref(sess->media);
	g_free(sess);
}

JingleSession *
jabber_jingle_session_find_by_id(JabberStream *js, const char *id)
{
	purple_debug_info("jingle", "find_by_id %s\n", id);
	purple_debug_info("jingle", "hash table: %p\n", js->sessions);
	purple_debug_info("jingle", "hash table size %d\n",
					  g_hash_table_size(js->sessions));
	purple_debug_info("jingle", "lookup: %p\n", g_hash_table_lookup(js->sessions, id));  
	return (JingleSession *) g_hash_table_lookup(js->sessions, id);
}

JingleSession *jabber_jingle_session_find_by_jid(JabberStream *js, const char *jid)
{
	GList *values = g_hash_table_get_values(js->sessions);
	GList *iter = values;
	gboolean use_bare = strchr(jid, '/') == NULL;

	for (; iter; iter = iter->next) {
		JingleSession *session = (JingleSession *)iter->data;
		gchar *cmp_jid = use_bare ? jabber_get_bare_jid(session->remote_jid)
					  : g_strdup(session->remote_jid);
		if (!strcmp(jid, cmp_jid)) {
			g_free(cmp_jid);
			g_list_free(values);
			return session;
		}
		g_free(cmp_jid);
	}

	g_list_free(values);
	return NULL;	
}

GList *
jabber_jingle_get_codecs(const xmlnode *description)
{
	GList *codecs = NULL;
	xmlnode *codec_element = NULL;
	const char *encoding_name,*id, *clock_rate;
	FsCodec *codec;
	
	for (codec_element = xmlnode_get_child(description, "payload-type") ;
		 codec_element ;
		 codec_element = xmlnode_get_next_twin(codec_element)) {
		encoding_name = xmlnode_get_attrib(codec_element, "name");
		id = xmlnode_get_attrib(codec_element, "id");
		clock_rate = xmlnode_get_attrib(codec_element, "clockrate");

		codec = fs_codec_new(atoi(id), encoding_name, 
				     FS_MEDIA_TYPE_AUDIO, 
				     clock_rate ? atoi(clock_rate) : 0);
		codecs = g_list_append(codecs, codec);		 
	}
	return codecs;
}

GList *
jabber_jingle_get_candidates(const xmlnode *transport)
{
	GList *candidates = NULL;
	xmlnode *candidate = NULL;
	FsCandidate *c;
	
	for (candidate = xmlnode_get_child(transport, "candidate") ;
		 candidate ;
		 candidate = xmlnode_get_next_twin(candidate)) {
		const char *type = xmlnode_get_attrib(candidate, "type");
		c = fs_candidate_new(xmlnode_get_attrib(candidate, "component"), 
							atoi(xmlnode_get_attrib(candidate, "component")),
							strcmp(type, "host") == 0 ?
								FS_CANDIDATE_TYPE_HOST :
								strcmp(type, "prflx") == 0 ?
									FS_CANDIDATE_TYPE_PRFLX :
									FS_CANDIDATE_TYPE_RELAY,
							strcmp(xmlnode_get_attrib(candidate, "protocol"),
							  "udp") == 0 ? 
				 				FS_NETWORK_PROTOCOL_UDP :
				 				FS_NETWORK_PROTOCOL_TCP,
							xmlnode_get_attrib(candidate, "ip"),
							atoi(xmlnode_get_attrib(candidate, "port")));
		candidates = g_list_append(candidates, c);
	}
	
	return candidates;
}

PurpleMedia *
jabber_jingle_session_get_media(const JingleSession *sess)
{
	return sess->media;
}

void
jabber_jingle_session_set_media(JingleSession *sess, PurpleMedia *media)
{
	sess->media = media;
}

const char *
jabber_jingle_session_get_remote_jid(const JingleSession *sess)
{
	return sess->remote_jid;
}

void
jabber_jingle_session_set_remote_jid(JingleSession *sess, 
									 const char *remote_jid)
{
	sess->remote_jid = strdup(remote_jid);
}

const char *
jabber_jingle_session_get_initiator(const JingleSession *sess)
{
	return sess->initiator;
}

void
jabber_jingle_session_set_initiator(JingleSession *sess,
									const char *initiator)
{
	sess->initiator = g_strdup(initiator);
}

gboolean
jabber_jingle_session_is_initiator(const JingleSession *sess)
{
	return sess->is_initiator;
}

static xmlnode *
jabber_jingle_session_create_jingle_element(const JingleSession *sess,
											const char *action)
{
	xmlnode *jingle = xmlnode_new("jingle");
	xmlnode_set_namespace(jingle, "urn:xmpp:tmp:jingle");
	xmlnode_set_attrib(jingle, "action", action);
	xmlnode_set_attrib(jingle, "initiator", 
					   jabber_jingle_session_get_initiator(sess));
	xmlnode_set_attrib(jingle, "responder", 
					   jabber_jingle_session_is_initiator(sess) ?
					   jabber_jingle_session_get_remote_jid(sess) :
					   jabber_jingle_session_get_initiator(sess));
	xmlnode_set_attrib(jingle, "sid", sess->id);
	
	return jingle;
}

xmlnode *
jabber_jingle_session_create_terminate(const JingleSession *sess,
									   const char *reasoncode,
									   const char *reasontext)
{
	xmlnode *jingle = 
		jabber_jingle_session_create_jingle_element(sess, "session-terminate");
	xmlnode_set_attrib(jingle, "resoncode", reasoncode);
	if (reasontext) {
		xmlnode_set_attrib(jingle, "reasontext", reasontext);
	}
	xmlnode_set_attrib(jingle, "sid", sess->id);
	
	return jingle;
}

xmlnode *
jabber_jingle_session_create_description(const JingleSession *sess)
{
    GList *codecs = purple_media_get_local_audio_codecs(sess->media);
    xmlnode *description = xmlnode_new("description");

	xmlnode_set_namespace(description, "urn:xmpp:tmp:jingle:apps:audio-rtp");
	
	/* get codecs */
	for (; codecs ; codecs = codecs->next) {
		FsCodec *codec = (FsCodec*)codecs->data;
		char id[8], clockrate[10], channels[10];
		xmlnode *payload = xmlnode_new_child(description, "payload-type");
		
		g_snprintf(id, sizeof(id), "%d", codec->id);
		g_snprintf(clockrate, sizeof(clockrate), "%d", codec->clock_rate);
		g_snprintf(channels, sizeof(channels), "%d", codec->channels);
		
		xmlnode_set_attrib(payload, "name", codec->encoding_name);
		xmlnode_set_attrib(payload, "id", id);
		xmlnode_set_attrib(payload, "clockrate", clockrate);
		xmlnode_set_attrib(payload, "channels", channels);
    }
    
    fs_codec_list_destroy(codecs);
    return description;
}
    
static guint
jabber_jingle_get_priority(guint type, guint network)
{
	switch (type) {
		case FS_CANDIDATE_TYPE_HOST:
			return network == 0 ? 4096 : network == 1 ? 2048 : 1024;
			break;
		case FS_CANDIDATE_TYPE_PRFLX:
			return 126;
			break;
		case FS_CANDIDATE_TYPE_RELAY:
			return 100;
			break;
		default:
			return 0; /* unknown type, should not happen */
	}
}

static xmlnode *
jabber_jingle_session_create_candidate_info(FsCandidate *c, FsCandidate *remote)
{
	char port[8];
	char prio[8];
	char component[8];
	xmlnode *candidate = NULL;
	
	candidate = xmlnode_new("candidate");
	
	g_snprintf(port, sizeof(port), "%d", c->port);
	g_snprintf(prio, sizeof(prio), "%d", 
		   jabber_jingle_get_priority(c->type, 0));
	g_snprintf(component, sizeof(component), "%d", c->component_id);
	
	xmlnode_set_attrib(candidate, "component", component);
	xmlnode_set_attrib(candidate, "foundation", "1"); /* what about this? */
	xmlnode_set_attrib(candidate, "generation", "0"); /* ? */
	xmlnode_set_attrib(candidate, "ip", c->ip);
	xmlnode_set_attrib(candidate, "network", "0"); /* ? */
	xmlnode_set_attrib(candidate, "port", port);
	xmlnode_set_attrib(candidate, "priority", prio); /* Is this correct? */
	xmlnode_set_attrib(candidate, "protocol",
			   c->proto == FS_NETWORK_PROTOCOL_UDP ?
			   "udp" : "tcp");
	if (c->username)
		xmlnode_set_attrib(candidate, "ufrag", c->username);
	if (c->password)
		xmlnode_set_attrib(candidate, "pwd", c->password);
	
	xmlnode_set_attrib(candidate, "type", 
			   c->type == FS_CANDIDATE_TYPE_HOST ? 
			   "host" :
			   c->type == FS_CANDIDATE_TYPE_PRFLX ? 
			   "prflx" :
		       	   c->type == FS_CANDIDATE_TYPE_RELAY ? 
			   "relay" : NULL);

	/* relay */
	if (c->type == FS_CANDIDATE_TYPE_RELAY) {
		/* set rel-addr and rel-port? How? */
	}

	if (remote) {
		char remote_port[8];
		g_snprintf(remote_port, sizeof(remote_port), "%d", remote->port);
		xmlnode_set_attrib(candidate, "rem-addr", remote->ip);
		xmlnode_set_attrib(candidate, "rem-port", remote_port);
	}

	return candidate;
}

/* split into two separate methods, one to generate session-accept
	(includes codecs) and one to generate transport-info (includes transports
	candidates) */
xmlnode *
jabber_jingle_session_create_session_accept(const JingleSession *sess)
{
	xmlnode *jingle = 
		jabber_jingle_session_create_jingle_element(sess, "session-accept");
	xmlnode *content = NULL;
	xmlnode *description = NULL;
	xmlnode *transport = NULL;
	xmlnode *candidate = NULL;
	
	
	content = xmlnode_new_child(jingle, "content");
	xmlnode_set_attrib(content, "creator", "initiator");
	xmlnode_set_attrib(content, "name", "audio-content");
	xmlnode_set_attrib(content, "profile", "RTP/AVP");
	
	description = jabber_jingle_session_create_description(sess);
	xmlnode_insert_child(content, description);

	transport = xmlnode_new_child(content, "transport");
	xmlnode_set_namespace(transport, "urn:xmpp:tmp:jingle:transports:ice-udp");
	candidate = jabber_jingle_session_create_candidate_info(
			purple_media_get_local_candidate(sess->media),
			purple_media_get_remote_candidate(sess->media));
	xmlnode_insert_child(transport, candidate);

	return jingle;
}


xmlnode *
jabber_jingle_session_create_transport_info(const JingleSession *sess)
{
	xmlnode *jingle = 
		jabber_jingle_session_create_jingle_element(sess, "transport-info");
	xmlnode *content = NULL;
	xmlnode *transport = NULL;
	GList *candidates = purple_media_get_local_audio_candidates(sess->media);

	content = xmlnode_new_child(jingle, "content");
	xmlnode_set_attrib(content, "creator", "initiator");
	xmlnode_set_attrib(content, "name", "audio-content");
	xmlnode_set_attrib(content, "profile", "RTP/AVP");
		
	transport = xmlnode_new_child(content, "transport");
	xmlnode_set_namespace(transport, "urn:xmpp:tmp:jingle:transports:ice-udp");
	
	/* get transport candidate */
	for (; candidates ; candidates = candidates->next) {
		FsCandidate *c = (FsCandidate *) candidates->data;

		if (!strcmp(c->ip, "127.0.0.1")) {
			continue;
		}

		xmlnode_insert_child(transport,
				     jabber_jingle_session_create_candidate_info(c, NULL));
	}
	fs_candidate_list_destroy(candidates);
	
	return jingle;
}

xmlnode *
jabber_jingle_session_create_content_replace(const JingleSession *sess,
					     FsCandidate *native_candidate,
					     FsCandidate *remote_candidate)
{
	xmlnode *jingle = 
		jabber_jingle_session_create_jingle_element(sess, "content-replace");
	xmlnode *content = NULL;
	xmlnode *transport = NULL;

	purple_debug_info("jingle", "creating content-modify for native candidate %s " \
			  ", remote candidate %s\n", native_candidate->candidate_id,
			  remote_candidate->candidate_id);

	content = xmlnode_new_child(jingle, "content");
	xmlnode_set_attrib(content, "creator", "initiator");
	xmlnode_set_attrib(content, "name", "audio-content");
	xmlnode_set_attrib(content, "profile", "RTP/AVP");
	
	/* get top codec from codec_intersection to put here... */
	/* later on this should probably handle changing codec */

	xmlnode_insert_child(content, jabber_jingle_session_create_description(sess));

	transport = xmlnode_new_child(content, "transport");
	xmlnode_set_namespace(transport, "urn:xmpp:tmp:jingle:transports:ice-udp");
	xmlnode_insert_child(transport,
			     jabber_jingle_session_create_candidate_info(native_candidate,
									 remote_candidate));

	purple_debug_info("jingle", "End create content modify\n");
	
	return jingle;
}

xmlnode *
jabber_jingle_session_create_content_accept(const JingleSession *sess)
{
	xmlnode *jingle = 
		jabber_jingle_session_create_jingle_element(sess, "content-accept");
	
    xmlnode *content = xmlnode_new_child(jingle, "content");
	xmlnode *description = jabber_jingle_session_create_description(sess);
    
    xmlnode_set_attrib(content, "creator", "initiator");
	xmlnode_set_attrib(content, "name", "audio-content");
	xmlnode_set_attrib(content, "profile", "RTP/AVP");
	
    xmlnode_insert_child(content, description);
    
	return jingle;
}

static void
jabber_jingle_session_send_content_accept(JingleSession *session)
{
	JabberIq *result = jabber_iq_new(jabber_jingle_session_get_js(session),
					 JABBER_IQ_SET);
	xmlnode *jingle = jabber_jingle_session_create_content_accept(session);
	xmlnode_set_attrib(result->node, "to",
			   jabber_jingle_session_get_remote_jid(session));

	xmlnode_insert_child(result->node, jingle);
	jabber_iq_send(result);
}

static void
jabber_jingle_session_send_session_accept(JingleSession *session)
{
	JabberIq *result = jabber_iq_new(jabber_jingle_session_get_js(session),
					 JABBER_IQ_SET);
	xmlnode *jingle = jabber_jingle_session_create_session_accept(session);
	xmlnode_set_attrib(result->node, "to",
			   jabber_jingle_session_get_remote_jid(session));

	xmlnode_insert_child(result->node, jingle);
	jabber_iq_send(result);
	purple_debug_info("jabber", "Sent session accept, starting stream\n");
	gst_element_set_state(purple_media_get_audio_pipeline(session->media), GST_STATE_PLAYING);

	session->session_started = TRUE;
}

static void
jabber_jingle_session_send_session_reject(JingleSession *session)
{
	JabberIq *result = jabber_iq_new(jabber_jingle_session_get_js(session),
					 JABBER_IQ_SET);
	xmlnode *jingle = jabber_jingle_session_create_terminate(session,
								 "decline", NULL);
	xmlnode_set_attrib(result->node, "to", 
			   jabber_jingle_session_get_remote_jid(session));
	xmlnode_insert_child(result->node, jingle);
	jabber_iq_send(result);
	jabber_jingle_session_destroy(session);
}

static void
jabber_jingle_session_send_session_terminate(JingleSession *session)
{
	JabberIq *result = jabber_iq_new(jabber_jingle_session_get_js(session),
					 JABBER_IQ_SET);
	xmlnode *jingle = jabber_jingle_session_create_terminate(session,
								 "no-error", NULL);
	xmlnode_set_attrib(result->node, "to",
			   jabber_jingle_session_get_remote_jid(session));
	xmlnode_insert_child(result->node, jingle);
	jabber_iq_send(result);
	jabber_jingle_session_destroy(session);
}

/* callback called when new local transport candidate(s) are available on the
	Farsight stream */
static void
jabber_jingle_session_candidates_prepared(PurpleMedia *media, JingleSession *session)
{
	if (!jabber_jingle_session_is_initiator(session)) {
		/* create transport-info package */
		JabberIq *result = jabber_iq_new(jabber_jingle_session_get_js(session),
						 JABBER_IQ_SET);
		xmlnode *jingle = jabber_jingle_session_create_transport_info(session);
		purple_debug_info("jabber", "jabber_session_candidates_prepared: %d candidates\n",
				  g_list_length(purple_media_get_local_audio_candidates(session->media)));
		xmlnode_set_attrib(result->node, "to",
				   jabber_jingle_session_get_remote_jid(session));

		xmlnode_insert_child(result->node, jingle);
		jabber_iq_send(result);
	}
}

/* callback called when a pair of transport candidates (local and remote)
	has been established */
static void
jabber_jingle_session_candidate_pair_established(PurpleMedia *media,
						 FsCandidate *native_candidate,
						 FsCandidate *remote_candidate,
						 JingleSession *session)
{	
	purple_debug_info("jabber", "jabber_candidate_pair_established called\n");
	/* if we are the initiator, we should send a content-modify message */
	if (jabber_jingle_session_is_initiator(session)) {
		JabberIq *result;
		xmlnode *jingle;
		
		purple_debug_info("jabber", "we are the initiator, let's send content-modify\n");

		result = jabber_iq_new(jabber_jingle_session_get_js(session), JABBER_IQ_SET);

		/* shall change this to a "content-replace" */
		jingle = jabber_jingle_session_create_content_replace(session,
								      native_candidate,
								      remote_candidate);
		xmlnode_set_attrib(result->node, "to",
				   jabber_jingle_session_get_remote_jid(session));
		xmlnode_insert_child(result->node, jingle);
		jabber_iq_send(result);
	}
}

static gboolean
jabber_jingle_session_initiate_media_internal(JingleSession *session,
					      const char *initiator,
					      const char *remote_jid)
{
	PurpleMedia *media = NULL;

	media = purple_media_manager_create_media(purple_media_manager_get(), 
						  session->js->gc, "fsrtpconference", remote_jid);

	if (!media) {
		purple_debug_error("jabber", "Couldn't create fsrtpconference\n");
		return FALSE;
	}

	/* this will need to be changed to "nice" once the libnice transmitter is finished */
	if (!purple_media_add_stream(media, remote_jid, PURPLE_MEDIA_AUDIO, "rawudp")) {
		purple_debug_error("jabber", "Couldn't create audio stream\n");
		purple_media_reject(media);
		return FALSE;
	}

	jabber_jingle_session_set_remote_jid(session, remote_jid);
	jabber_jingle_session_set_initiator(session, initiator);
	jabber_jingle_session_set_media(session, media);

	/* connect callbacks */
	g_signal_connect_swapped(G_OBJECT(media), "accepted", 
				 G_CALLBACK(jabber_jingle_session_send_session_accept), session);
	g_signal_connect_swapped(G_OBJECT(media), "reject", 
				 G_CALLBACK(jabber_jingle_session_send_session_reject), session);
	g_signal_connect_swapped(G_OBJECT(media), "hangup", 
				 G_CALLBACK(jabber_jingle_session_send_session_terminate), session);
	g_signal_connect(G_OBJECT(media), "candidates-prepared", 
				 G_CALLBACK(jabber_jingle_session_candidates_prepared), session);
	g_signal_connect(G_OBJECT(media), "candidate-pair", 
				 G_CALLBACK(jabber_jingle_session_candidate_pair_established), session);

	purple_media_ready(media);

	return TRUE;
}

static void
jabber_jingle_session_initiate_result_cb(JabberStream *js, xmlnode *packet, gpointer data)
{
	const char *from = xmlnode_get_attrib(packet, "from");
	JingleSession *session = jabber_jingle_session_find_by_jid(js, from);
	PurpleMedia *media = session->media;
	JabberIq *result;
	xmlnode *jingle;

	if (!strcmp(xmlnode_get_attrib(packet, "type"), "error")) {
		purple_media_got_hangup(media);
		return;
	}

	/* catch errors */
	if (xmlnode_get_child(packet, "error")) {
		purple_media_got_hangup(media);
		return;
	}

	/* create transport-info package */
	result = jabber_iq_new(jabber_jingle_session_get_js(session), JABBER_IQ_SET);
	jingle = jabber_jingle_session_create_transport_info(session);
	purple_debug_info("jabber", "jabber_session_candidates_prepared: %d candidates\n",
			  g_list_length(purple_media_get_local_audio_candidates(session->media)));
	xmlnode_set_attrib(result->node, "to",
			   jabber_jingle_session_get_remote_jid(session));

	xmlnode_insert_child(result->node, jingle);
	jabber_iq_send(result);
}

PurpleMedia *
jabber_jingle_session_initiate_media(PurpleConnection *gc, const char *who, 
				     PurpleMediaStreamType type)
{
	/* create content negotiation */
	JabberStream *js = gc->proto_data;
	JabberIq *request = jabber_iq_new(js, JABBER_IQ_SET);
	xmlnode *jingle, *content, *description, *transport;
	GList *codecs;
	JingleSession *session;
	JabberBuddy *jb;
	JabberBuddyResource *jbr;
	
	char *jid = NULL, *me = NULL;

	/* construct JID to send to */
	jb = jabber_buddy_find(js, who, FALSE);
	if (!jb) {
		purple_debug_error("jabber", "Could not find Jabber buddy\n");
		return NULL;
	}
	jbr = jabber_buddy_find_resource(jb, NULL);
	if (!jbr) {
		purple_debug_error("jabber", "Could not find buddy's resource\n");
	}

	if ((strchr(who, '/') == NULL) && jbr && (jbr->name != NULL)) {
		jid = g_strdup_printf("%s/%s", who, jbr->name);
	} else {
		jid = g_strdup(who);
	}
	
	session = jabber_jingle_session_create(js);
	/* set ourselves as initiator */
	me = g_strdup_printf("%s@%s/%s", js->user->node, js->user->domain, js->user->resource);

	if (!jabber_jingle_session_initiate_media_internal(session, me, jid)) {
		g_free(jid);
		g_free(me);
		jabber_jingle_session_destroy(session);
		return NULL;
	}

	g_free(jid);
	g_free(me);

	codecs = purple_media_get_local_audio_codecs(session->media);

	/* create request */
	
	xmlnode_set_attrib(request->node, "to", 
			   jabber_jingle_session_get_remote_jid(session));
	jingle = xmlnode_new_child(request->node, "jingle");
	xmlnode_set_namespace(jingle, "urn:xmpp:tmp:jingle");
	xmlnode_set_attrib(jingle, "action", "session-initiate");
	/* get our JID and a session id... */
	xmlnode_set_attrib(jingle, "initiator", jabber_jingle_session_get_initiator(session));
	xmlnode_set_attrib(jingle, "sid", jabber_jingle_session_get_id(session));
	
	content = xmlnode_new_child(jingle, "content");
	xmlnode_set_attrib(content, "name", "audio-content");
	xmlnode_set_attrib(content, "profile", "RTP/AVP");

	description = jabber_jingle_session_create_description(session);
	xmlnode_insert_child(content, description);

	transport = xmlnode_new_child(content, "transport");
	xmlnode_set_namespace(transport, "urn:xmpp:tmp:jingle:transports:ice-udp");

	jabber_iq_set_callback(request, jabber_jingle_session_initiate_result_cb, NULL);

	/* send request to other part */	
	jabber_iq_send(request);

	fs_codec_list_destroy(codecs);

	return session->media;
}

void
jabber_jingle_session_handle_content_replace(JabberStream *js, xmlnode *packet)
{
	xmlnode *jingle = xmlnode_get_child(packet, "jingle");
	const char *sid = xmlnode_get_attrib(jingle, "sid");
	JingleSession *session = jabber_jingle_session_find_by_id(js, sid);

	if (!jabber_jingle_session_is_initiator(session) && session->session_started) {
		JabberIq *result = jabber_iq_new(js, JABBER_IQ_RESULT);
		JabberIq *accept = jabber_iq_new(js, JABBER_IQ_SET);
		xmlnode *content_accept = NULL;

		/* send acknowledement */
		xmlnode_set_attrib(result->node, "id", xmlnode_get_attrib(packet, "id"));
		xmlnode_set_attrib(result->node, "to", xmlnode_get_attrib(packet, "from"));
		jabber_iq_send(result);

		/* send content-accept */
		content_accept = jabber_jingle_session_create_content_accept(session);
		xmlnode_set_attrib(accept->node, "id", xmlnode_get_attrib(packet, "id"));
		xmlnode_set_attrib(accept->node, "to", xmlnode_get_attrib(packet, "from"));
		xmlnode_insert_child(accept->node, content_accept);

		jabber_iq_send(accept);
	}
}

void
jabber_jingle_session_handle_session_accept(JabberStream *js, xmlnode *packet)
{
	JabberIq *result = jabber_iq_new(js, JABBER_IQ_RESULT);
	xmlnode *jingle = xmlnode_get_child(packet, "jingle");
	xmlnode *content = xmlnode_get_child(jingle, "content");
	const char *sid = xmlnode_get_attrib(jingle, "sid");
	const char *action = xmlnode_get_attrib(jingle, "action");
	JingleSession *session = jabber_jingle_session_find_by_id(js, sid);
	GList *remote_codecs = NULL;
	GList *remote_transports = NULL;
	GList *codec_intersection;
	FsCodec *top = NULL;
	xmlnode *description = NULL;
	xmlnode *transport = NULL;

	/* We should probably check validity of the incoming XML... */

	xmlnode_set_attrib(result->node, "to",
			   jabber_jingle_session_get_remote_jid(session));
	jabber_iq_set_id(result, xmlnode_get_attrib(packet, "id"));

	description = xmlnode_get_child(content, "description");
	transport = xmlnode_get_child(content, "transport");

	/* fetch codecs from remote party */
	purple_debug_info("jabber", "get codecs from session-accept\n");
	remote_codecs = jabber_jingle_get_codecs(description);
	purple_debug_info("jabber", "get transport candidates from session accept\n");
	remote_transports = jabber_jingle_get_candidates(transport);

	purple_debug_info("jabber", "Got %d codecs from responder\n",
			  g_list_length(remote_codecs));
	purple_debug_info("jabber", "Got %d transport candidates from responder\n",
			  g_list_length(remote_transports));

	purple_debug_info("jabber", "Setting remote codecs on stream\n");

	purple_media_set_remote_audio_codecs(session->media, 
					     jabber_jingle_session_get_remote_jid(session),
					     remote_codecs);

	codec_intersection = purple_media_get_negotiated_audio_codecs(session->media);
	purple_debug_info("jabber", "codec_intersection contains %d elems\n",
			  g_list_length(codec_intersection));
	/* get the top codec */
	if (g_list_length(codec_intersection) > 0) {
		top = (FsCodec *) codec_intersection->data;
		purple_debug_info("jabber", "Found a suitable codec on stream = %d\n",
				  top->id);

		/* we have found a suitable codec, but we will not start the stream
		   just yet, wait for transport negotiation to complete... */
	}
	/* if we also got transport candidates, add them to our streams
	   list of known remote candidates */
	if (g_list_length(remote_transports) > 0) {
		purple_media_add_remote_audio_candidates(session->media,
							 jabber_jingle_session_get_remote_jid(session),
							 remote_transports);
		fs_candidate_list_destroy(remote_transports);
	}
	if (g_list_length(codec_intersection) == 0 &&
			g_list_length(remote_transports)) {
		/* we didn't get any candidates and the codec intersection is empty,
		   this means this was not a content-accept message and we couldn't
		   find any suitable codecs, should return error and hang up */

	}

	g_list_free(codec_intersection);

	if (!strcmp(action, "session-accept")) {
		purple_media_got_accept(jabber_jingle_session_get_media(session));
		purple_debug_info("jabber", "Got session-accept, starting stream\n");
		gst_element_set_state(purple_media_get_audio_pipeline(session->media),
				      GST_STATE_PLAYING);
	}

	jabber_iq_send(result);

	session->session_started = TRUE;
}

void 
jabber_jingle_session_handle_session_initiate(JabberStream *js, xmlnode *packet)
{
	JingleSession *session = NULL;
	xmlnode *jingle = xmlnode_get_child(packet, "jingle");
	xmlnode *content = NULL;
	xmlnode *description = NULL;
	const char *sid = NULL;
	const char *initiator = NULL;
	GList *codecs = NULL;
	JabberIq *result = NULL;

	if (!jingle) {
		purple_debug_error("jabber", "Malformed request");
		return;
	}

	sid = xmlnode_get_attrib(jingle, "sid");
	initiator = xmlnode_get_attrib(jingle, "initiator");

	if (jabber_jingle_session_find_by_id(js, sid)) {
		/* This should only happen if you start a session with yourself */
		purple_debug_error("jabber", "Jingle session with id={%s} already exists\n", sid);
		return;
	}
	session = jabber_jingle_session_create_by_id(js, sid);

	/* init media */
	content = xmlnode_get_child(jingle, "content");
	if (!content) {
		purple_debug_error("jabber", "jingle tag must contain content tag\n");
		/* should send error here */
		return;
	}

	description = xmlnode_get_child(content, "description");

	if (!description) {
		purple_debug_error("jabber", "content tag must contain description tag\n");
		/* we should create an error iq here */
		return;
	}

	if (!jabber_jingle_session_initiate_media_internal(session, initiator, initiator)) {
		purple_debug_error("jabber", "Couldn't start media session with %s\n", initiator);
		jabber_jingle_session_destroy(session);
		/* we should create an error iq here */
		return;
	}

	codecs = jabber_jingle_get_codecs(description);

	purple_media_set_remote_audio_codecs(session->media, initiator, codecs);

	result = jabber_iq_new(js, JABBER_IQ_RESULT);
	jabber_iq_set_id(result, xmlnode_get_attrib(packet, "id"));
	xmlnode_set_attrib(result->node, "to", xmlnode_get_attrib(packet, "from"));
	jabber_iq_send(result);
}

void
jabber_jingle_session_handle_session_terminate(JabberStream *js, xmlnode *packet)
{
	JabberIq *result = jabber_iq_new(js, JABBER_IQ_SET);
	xmlnode *jingle = xmlnode_get_child(packet, "jingle");
	const char *sid = xmlnode_get_attrib(jingle, "sid");
	JingleSession *session = jabber_jingle_session_find_by_id(js, sid);

	if (!session) {
		purple_debug_error("jabber", "jabber_handle_session_terminate couldn't find session\n");
		return;
	}

	xmlnode_set_attrib(result->node, "to",
			   jabber_jingle_session_get_remote_jid(session));
	xmlnode_set_attrib(result->node, "id", xmlnode_get_attrib(packet, "id"));



	/* maybe we should look at the reasoncode to determine if it was
	   a hangup or a reject, and call different callbacks to purple_media */
	gst_element_set_state(purple_media_get_audio_pipeline(session->media), GST_STATE_NULL);

	purple_media_got_hangup(jabber_jingle_session_get_media(session));
	jabber_iq_send(result);
	jabber_jingle_session_destroy(session);
}

void
jabber_jingle_session_handle_transport_info(JabberStream *js, xmlnode *packet)
{
	JabberIq *result = jabber_iq_new(js, JABBER_IQ_RESULT);
	xmlnode *jingle = xmlnode_get_child(packet, "jingle");
	xmlnode *content = xmlnode_get_child(jingle, "content");
	xmlnode *transport = xmlnode_get_child(content, "transport");
	GList *remote_candidates = jabber_jingle_get_candidates(transport);
	const char *sid = xmlnode_get_attrib(jingle, "sid");
	JingleSession *session = jabber_jingle_session_find_by_id(js, sid);

	if (!session)
		purple_debug_error("jabber", "jabber_handle_session_candidates couldn't find session\n");

	/* send acknowledement */
	xmlnode_set_attrib(result->node, "id", xmlnode_get_attrib(packet, "id"));
	xmlnode_set_attrib(result->node, "to", xmlnode_get_attrib(packet, "from"));
	jabber_iq_send(result);

	/* add candidates to our list of remote candidates */
	if (g_list_length(remote_candidates) > 0) {
		purple_media_add_remote_audio_candidates(session->media,
							 xmlnode_get_attrib(packet, "from"),
							 remote_candidates);
		fs_candidate_list_destroy(remote_candidates);
	}
}

#endif /* USE_VV */
