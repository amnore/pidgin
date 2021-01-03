/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include "purplecredentialprovider.h"

typedef struct {
	gchar *id;
	gchar *name;
} PurpleCredentialProviderPrivate;

enum {
	PROP_0,
	PROP_ID,
	PROP_NAME,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PurpleCredentialProvider,
                                    purple_credential_provider, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_credential_provider_set_id(PurpleCredentialProvider *provider,
                                  const gchar *id)
{
	PurpleCredentialProviderPrivate *priv = NULL;

	priv = purple_credential_provider_get_instance_private(provider);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(provider), properties[PROP_ID]);
}

static void
purple_credential_provider_set_name(PurpleCredentialProvider *provider,
                                    const gchar *name)
{
	PurpleCredentialProviderPrivate *priv = NULL;

	priv = purple_credential_provider_get_instance_private(provider);

	g_free(priv->name);
	priv->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(provider), properties[PROP_NAME]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_credential_provider_get_property(GObject *obj, guint param_id,
                                        GValue *value, GParamSpec *pspec)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value,
			                   purple_credential_provider_get_id(provider));
			break;
		case PROP_NAME:
			g_value_set_string(value,
			                   purple_credential_provider_get_name(provider));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_credential_provider_set_property(GObject *obj, guint param_id,
                                        const GValue *value, GParamSpec *pspec)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);

	switch(param_id) {
		case PROP_ID:
			purple_credential_provider_set_id(provider,
			                                  g_value_get_string(value));
			break;
		case PROP_NAME:
			purple_credential_provider_set_name(provider,
			                                    g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_credential_provider_finalize(GObject *obj) {
	PurpleCredentialProvider *provider = NULL;
	PurpleCredentialProviderPrivate *priv = NULL;

	provider = PURPLE_CREDENTIAL_PROVIDER(obj);
	priv = purple_credential_provider_get_instance_private(provider);

	g_clear_pointer(&priv->id, g_free);
	g_clear_pointer(&priv->name, g_free);

	G_OBJECT_CLASS(purple_credential_provider_parent_class)->finalize(obj);
}

static void
purple_credential_provider_init(PurpleCredentialProvider *provider) {
}

static void
purple_credential_provider_class_init(PurpleCredentialProviderClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_credential_provider_get_property;
	obj_class->set_property = purple_credential_provider_set_property;
	obj_class->finalize = purple_credential_provider_finalize;

	/**
	 * PurpleCredentialProvider::id:
	 *
	 * The ID of the provider.  Used for preferences and other things that need
	 * to address it.
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id", "The identifier of the provider",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	/**
	 * PurpleCredentialProvider::name:
	 *
	 * The name of the provider which will be displayed to the user.
	 */
	properties[PROP_NAME] = g_param_spec_string(
		"name", "name", "The name of the provider",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
const gchar *
purple_credential_provider_get_id(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);

	priv = purple_credential_provider_get_instance_private(provider);

	return priv->id;
}

const gchar *
purple_credential_provider_get_name(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);

	priv = purple_credential_provider_get_instance_private(provider);

	return priv->name;
}

gboolean
purple_credential_provider_is_valid(PurpleCredentialProvider *provider,
                                    GError **error)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);

	if(purple_credential_provider_get_id(provider) == NULL) {
		g_set_error_literal(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 0,
		                    "provider has no id");

		return FALSE;
	}

	if(purple_credential_provider_get_name(provider) == NULL) {
		g_set_error_literal(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 1,
		                    "provider has no name");

		return FALSE;
	}

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);

	if(klass->read_password_async == NULL || klass->read_password_finish == NULL) {
		g_set_error_literal(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 2,
		                    "provider can not read passwords");

		return FALSE;
	}

	if(klass->write_password_async == NULL || klass->write_password_finish == NULL) {
		g_set_error_literal(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 3,
		                    "provider can not write passwords");

		return FALSE;
	}

	return TRUE;
}

void
purple_credential_provider_read_password_async(PurpleCredentialProvider *provider,
                                               PurpleAccount *account,
                                               GCancellable *cancellable,
                                               GAsyncReadyCallback callback,
                                               gpointer data)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->read_password_async) {
		klass->read_password_async(provider, account, cancellable, callback,
		                           data);
	}
}

gchar *
purple_credential_provider_read_password_finish(PurpleCredentialProvider *provider,
                                                GAsyncResult *result,
                                                GError **error)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->read_password_finish) {
		return klass->read_password_finish(provider, result, error);
	}

	return NULL;
}

void
purple_credential_provider_write_password_async(PurpleCredentialProvider *provider,
                                                PurpleAccount *account,
                                                const gchar *password,
                                                GCancellable *cancellable,
                                                GAsyncReadyCallback callback,
                                                gpointer data)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->write_password_async) {
		klass->write_password_async(provider, account, password, cancellable,
		                            callback, data);
	}
}

gboolean
purple_credential_provider_write_password_finish(PurpleCredentialProvider *provider,
                                                 GAsyncResult *result,
                                                 GError **error)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->write_password_finish) {
		return klass->write_password_finish(provider, result, error);
	}

	return FALSE;
}

void
purple_credential_provider_clear_password_async(PurpleCredentialProvider *provider,
                                                PurpleAccount *account,
                                                GCancellable *cancellable,
                                                GAsyncReadyCallback callback,
                                                gpointer data)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->clear_password_async) {
		klass->clear_password_async(provider, account, cancellable, callback,
		                            data);
	}
}

gboolean
purple_credential_provider_clear_password_finish(PurpleCredentialProvider *provider,
                                                 GAsyncResult *result,
                                                 GError **error)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->clear_password_finish) {
		return klass->clear_password_finish(provider, result, error);
	}

	return FALSE;
}

void
purple_credential_provider_close(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->close) {
		klass->close(provider);
	}
}

PurpleRequestFields *
purple_credential_provider_read_settings(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->read_settings) {
		return klass->read_settings(provider);
	}

	return NULL;
}

gboolean
purple_credential_provider_write_settings(PurpleCredentialProvider *provider,
                                          PurpleRequestFields *fields)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(fields != NULL, FALSE);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->write_settings) {
		return klass->write_settings(provider, fields);
	}

	return FALSE;
}
