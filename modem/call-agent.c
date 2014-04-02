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
//#include "gdbus.h" // Todo: This was not compiling

#include <string.h>
#include <errno.h>

/* ---------------------------------------------------------------------- */

G_DEFINE_TYPE (ModemCallAgent, modem_call_agent, MODEM_TYPE_OFACE);

/* Todo: not sure what path should be like */
#define VOICECALL_UNSOLICITED_AGENT_PATH  "/voicecallagent"

struct _ModemCallAgentPrivate
{
  gboolean agentServiceRegistered;
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
      
  /* Todo: I don't know if we would need some initialisations for
   * the interface and proxy different when acting as agent */

  /* e.g. If we would provide interface */
  /*self->priv->proxy =
    dbus_g_proxy_new_for_name(dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL),
      "Telepathy-ring.VoicecallAgent",
      "/Telepathy-ring/VoicecallAgent",
      "Telepathy-ring.VoicecallAgent");*/

  self->priv->agentServiceRegistered = FALSE;
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

    /* Todo: How path should be actually given? Here an example from simkit */
    //  agent_path = VOICECALL_UNSOLICITED_AGENT_PATH;
    //  #define STK_UNSOLICITED_AGENT_PATH  "/simkitagent"
    //  QDBusObjectPath stkAgentPath(STK_UNSOLICITED_AGENT_PATH);
    //  QDBusPendingReply<> stkRegisterCall = stkIf->RegisterAgent(stkAgentPath);


    /* And this is an example how PushNotification calls the agent */
    // pn = dbus.Interface(bus.get_object('org.ofono', path),
    //        'org.ofono.PushNotification')

    /* Register call agent in order to receive requests for playing local tones 
     * Todo: Here it fails now!!! Should be checked what is still missing/how
     * to call agent interface via proxy */
    ret = dbus_g_proxy_call (proxy, "RegisterVoiceCallManagerAgent", NULL,
                              G_TYPE_OBJECT, VOICECALL_UNSOLICITED_AGENT_PATH, 
                              G_TYPE_INVALID, 
                              G_TYPE_INVALID);

    /* Maybe it would go something like this: */
    // request = modem_request (MODEM_CALL (self),
    //      proxy, "RegisterVoiceCallManagerAgent", 
    //      NULL, NULL, NULL, NULL, G_TYPE_INVALID);
      
    /* Todo: Check also the error value if we need more information about fail */
    if (ret)
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
    ret = dbus_g_proxy_call (proxy, "UnregisterVoiceCallManagerAgent", NULL,
                                G_TYPE_OBJECT, VOICECALL_UNSOLICITED_AGENT_PATH, 
                                G_TYPE_INVALID, 
                                G_TYPE_INVALID);
                                
  /* Todo: Do we even need to care or should we just use dbus_g_proxy_send? */
  if (ret)
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

  oface_class->ofono_interface = MODEM_OFACE_CALL_AGENT;
  oface_class->connect = modem_call_agent_connect;
  oface_class->disconnect = modem_call_agent_disconnect;

  DEBUG ("leave");
}

/* ---------------------------------------------------------------------- */
/* ModemCallAgent interface */

/* Todo: This is completely in phase still. Don't understand how the oFono
 * is able to call the method here via path. Should we introduce methods
 * as I commented below? */
void
RingbackTone (DBusConnection *conn,
                                  DBusMessage *msg, void *data)
{
  gboolean *playTone;
  DBusMessageIter iter;
  struct ModemCallAgent *self = data;

  if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_BOOLEAN)
		DEBUG ("Invalid arguments received");

  dbus_message_iter_get_basic(&iter, &playTone);

  DEBUG ("%i", (int) playTone);

  /* Todo: Should we check here the call state first before
   * just playing the tone? If we would be able to get call
   * state from call-service e.g.
   * if (state == MODEM_CALL_STATE_ALERTING && strcmp(type, "outgoing") == 0) 
   */

  if (*playTone == TRUE)
    {
    DEBUG("Play local tone");
    // Play local alert tone here
    ring_media_channel_play_tone(RING_MEDIA_CHANNEL(self),
      TONES_EVENT_RINGING, 0, 0);
    }
  else
    ring_media_channel_stop_playing(RING_MEDIA_CHANNEL(self), TRUE);

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

/*static const GDBusMethodTable call_agent_methods[] = {
	{ GDBUS_METHOD("RingbackTone", GDBUS_ARGS({ "playTone", "a{b}" }), NULL,
			modem_call_agent_ringback_tone) },
	{ GDBUS_METHOD("Release", NULL, NULL, modem_call_agent_release) },
	{ }
};*/
