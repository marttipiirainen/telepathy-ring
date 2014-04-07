/*
 * modem/call-agent.c - Interface towards oFono VoiceCallAgent
 *
 * Copyright (C) 2014 Jolla Ltd
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define MODEM_DEBUG_FLAG MODEM_LOG_CALL

#include "src/ring-media-channel.h"

#include "modem/debug.h"
#include "modem/call.h"
#include "modem/ofono.h"
#include "modem/oface.h"
#include "modem/errors.h"
#include "modem/tones.h"
#include "modem/request-private.h"

#include <dbus/dbus-glib-lowlevel.h>

#include <string.h>
#include <errno.h>

/* ---------------------------------------------------------------------- */

G_DEFINE_TYPE (ModemCallAgent, modem_call_agent, MODEM_TYPE_OFACE);

#define VOICECALL_AGENT_PATH  "/voicecallagent"

struct _ModemCallAgentPrivate
{
  gboolean agentServiceRegistered;
  DBusGProxy *agentProxy;
  ModemTones *tones;
  guint playing_tone;
};

/* ---------------------------------------------------------------------- */

#define RETURN_IF_NOT_VALID(self)           \
  g_return_if_fail (self != NULL            \
                      && modem_oface_is_connected (MODEM_OFACE (self)))

#define RETURN_VAL_IF_NOT_VALID(self, val)  \
  g_return_val_if_fail (self != NULL        \
                      && modem_oface_is_connected (MODEM_OFACE (self)), (val))

#define RETURN_NULL_IF_NOT_VALID(self) RETURN_VAL_IF_NOT_VALID (self, NULL)

/* ---------------------------------------------------------------------- */

static void
modem_call_agent_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (modem_call_agent_parent_class)->constructed)
    G_OBJECT_CLASS (modem_call_agent_parent_class)->constructed (object);
}

static void
modem_call_agent_init (ModemCallAgent *self)
{
  DEBUG ("enter");

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
      MODEM_TYPE_CALL_AGENT, ModemCallAgentPrivate);
      
  self->priv->agentProxy =
    dbus_g_proxy_new_for_name(dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL),
      MODEM_OFACE_CALL_AGENT,
      VOICECALL_AGENT_PATH,
      MODEM_OFACE_CALL_AGENT);

  self->priv->agentServiceRegistered = FALSE;
  self->priv->tones = 0;
  self->priv->playing_tone = 0;
}

static void
modem_call_agent_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (modem_call_agent_parent_class)->dispose)
    G_OBJECT_CLASS (modem_call_agent_parent_class)->dispose (object);
}

static void
modem_call_agent_finalize (GObject *object)
{
  G_OBJECT_CLASS (modem_call_agent_parent_class)->finalize (object);
}

/* ---------------------------------------------------------------------- */
/* ModemCallAgent interface */



static void
call_agent_play_cb(ModemTones *tones,
  guint source,
  gpointer _self)
{
  ModemCallAgent *self;
  DEBUG("play tone cb");

  if (_self) {
	  /* TODO check if needed */
	  self = MODEM_CALL_AGENT(_self);
	  g_object_unref(self);
  }
}


void
ringback_tone (DBusConnection *conn,
                                  DBusMessage *msg, void *data)
{
  gboolean playTone;
  DBusMessageIter iter;
  ModemCallAgent *self = data;
  DEBUG ("###piiramar### got it");

  dbus_message_iter_init(msg, &iter);
  if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_BOOLEAN)
		DEBUG ("Invalid arguments received");

  dbus_message_iter_get_basic(&iter, &playTone);

  DEBUG ("%d", (int) playTone);

  /* Todo: Should we check here the call state first before
   * just playing the tone? If we would be able to get call
   * state from call-service e.g.
   * if (state == MODEM_CALL_STATE_ALERTING && strcmp(type, "outgoing") == 0)
   */

  if (!self->priv->tones) {
	  /* Lazy init (this is a rare feature, after all) */
	  gpointer tones_object = g_object_new( MODEM_TYPE_TONES,
			  NULL );
	  DEBUG("### tones_object=%p", tones_object);
	  self->priv->tones = MODEM_TONES(tones_object);
	  DEBUG("### tones=%p", self->priv->tones);
  }

  if (!self->priv->tones) {
	  DEBUG("Cannot play tones.");
	  return;
  }


  if (playTone == TRUE)
    {
    DEBUG("Play local tone");

    if (!modem_tones_is_playing(self->priv->tones, self->priv->playing_tone)){
    	self->priv->playing_tone = modem_tones_start_full(self->priv->tones,
    			TONES_EVENT_RINGING, 0, 0,
    			call_agent_play_cb,
    	        g_object_ref(self));
    	DEBUG("playing, source = %d", self->priv->playing_tone);
    }

    /*ring_media_channel_play_tone(RING_MEDIA_CHANNEL(self),
      TONES_EVENT_RINGING, 0, 0);*/
    }
  else {
  	DEBUG("playing, source = %d", self->priv->playing_tone);
	modem_tones_stop(self->priv->tones, self->priv->playing_tone);
    /*ring_media_channel_stop_playing(RING_MEDIA_CHANNEL(self), TRUE);*/
  }

}

static DBusHandlerResult call_agent_generic_dbus_message(DBusConnection *conn,
						DBusMessage *msg)
{
	DBusMessage *reply;

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_connection_send(conn, reply, NULL);
	dbus_message_unref(reply);

	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult call_agent_dbus_message_handler(DBusConnection *conn,
				DBusMessage *msg, void *user_data)
{
	const char *method = dbus_message_get_member(msg);
	const char *iface = dbus_message_get_interface(msg);

	/*if ((strcmp("Introspect", method) == 0) &&
		(strcmp("org.freedesktop.DBus.Introspectable", iface) == 0))
		return introspect(conn, msg);*/

	if (strcmp(MODEM_OFACE_CALL_AGENT, iface) != 0)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (strcmp("Release", method) == 0) {
		DEBUG("release");
		/* TODO unregister etc. */
		return call_agent_generic_dbus_message(conn, msg);
	}
	else if (strcmp("RingbackTone", method) == 0) {
		DEBUG("ringback");
		ringback_tone(conn, msg, user_data);
		return call_agent_generic_dbus_message(conn, msg);
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void call_agent_unregister(DBusConnection *connection, void *user_data)
{
	DEBUG("");
}


static DBusObjectPathVTable call_agent_table = {
		.unregister_function	= call_agent_unregister,
		.message_function	= call_agent_dbus_message_handler,
};


/* ---------------------------------------------------------------------- */

static void
modem_call_agent_connect (ModemOface *_self)
{
  DEBUG ("(%p): enter", _self);

  ModemCallAgent *self = MODEM_CALL_AGENT (_self);
  ModemCallAgentPrivate *priv = self->priv;

  if (!priv->agentServiceRegistered)
    {
    DBusGProxy *proxy = modem_oface_dbus_proxy (_self);
    gboolean ret;
    ModemRequest *request;

    DEBUG ("###piiramar### agent proxy=%p path=%s bus=%s interface=%s"
    		, self->priv->agentProxy, dbus_g_proxy_get_path (self->priv->agentProxy), dbus_g_proxy_get_bus_name(self->priv->agentProxy), dbus_g_proxy_get_interface (self->priv->agentProxy));
    DEBUG ("###piiramar### proxy=%p path=%s bus=%s interface=%s"
    		, proxy, dbus_g_proxy_get_path (proxy), dbus_g_proxy_get_bus_name(proxy), dbus_g_proxy_get_interface (proxy));

    /* Register call agent in order to receive requests for playing local tones
    */
    dbus_g_proxy_call_no_reply(proxy, "RegisterVoicecallAgent",
    		DBUS_TYPE_G_OBJECT_PATH, dbus_g_proxy_get_path(self->priv->agentProxy),
    		G_TYPE_INVALID);
    /* TODO error case */

    DEBUG ("###piiramar### agent reg done");

    dbus_connection_register_object_path(
    		dbus_bus_get (DBUS_BUS_SYSTEM, NULL),
    		VOICECALL_AGENT_PATH,
    		&call_agent_table,
    		_self);
    /* TODO error case */

    DEBUG ("###piiramar### object reg done");

    priv->agentServiceRegistered = TRUE;
    }
}


void
modem_call_agent_disconnect (ModemOface *_self)
{
  DEBUG ("(%p): enter", _self);

  ModemCallAgent *self = MODEM_CALL_AGENT (_self);
  ModemCallAgentPrivate *priv = self->priv;
  DBusGProxy *proxy = modem_oface_dbus_proxy (_self);
  gboolean ret;

  /* Unregister Voicecall Agent if we are registered */
  if (priv->agentServiceRegistered)
	dbus_connection_unregister_object_path(
			dbus_bus_get (DBUS_BUS_SYSTEM, NULL),
			VOICECALL_AGENT_PATH);
  /* TODO error case */
  ret = dbus_g_proxy_call (proxy, "UnregisterVoiceCallManagerAgent", NULL,
    		DBUS_TYPE_G_OBJECT_PATH, dbus_g_proxy_get_path(self->priv->agentProxy),
            G_TYPE_INVALID,
            G_TYPE_INVALID);

  /* TODO error case */
  priv->agentServiceRegistered = FALSE;
}

/* ---------------------------------------------------------------------- */

void
modem_call_agent_class_init (ModemCallAgentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModemOfaceClass *oface_class = MODEM_OFACE_CLASS (klass);

  DEBUG ("enter");

  g_type_class_add_private (klass, sizeof (ModemCallAgentPrivate));

  object_class->constructed = modem_call_agent_constructed;
  object_class->dispose = modem_call_agent_dispose;
  object_class->finalize = modem_call_agent_finalize;

  oface_class->ofono_interface = MODEM_OFACE_CALL_MANAGER;
  oface_class->connect = modem_call_agent_connect;
  oface_class->disconnect = modem_call_agent_disconnect;

  DEBUG ("leave");
}

/* Todo: This was not compiling yet */
/*void
Release (DBusConnection *conn, DBusMessage *msg, void *data)
{
  DEBUG ("Voicecall agent released");
  struct ModemCallAgent *self = data;

  ModemCallAgent *self = MODEM_CALL_AGENT (_self);
  ModemCallAgentPrivate *priv = self->priv;

  priv->agentServiceRegistered = FALSE;
}*/

