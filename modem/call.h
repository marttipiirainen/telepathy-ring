/*
 * modem/call.h - Client for Ofono VoiceCalls
 *
 * Copyright (C) 2007 Nokia Corporation
 *   @author Pekka Pessi <first.surname@nokia.com>
 *   @author Lassi Syrjala <first.surname@nokia.com>
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

#ifndef _MODEM_CALL_H_
#define _MODEM_CALL_H_

#include <glib-object.h>

#include <modem/request.h>
#include <modem/oface.h>

G_BEGIN_DECLS

typedef struct _ModemCallService ModemCallService;
typedef struct _ModemCallServiceClass ModemCallServiceClass;
typedef struct _ModemCallServicePrivate ModemCallServicePrivate;

struct _ModemCallServiceClass
{
  ModemOfaceClass parent_class;
};

struct _ModemCallService
{
  ModemOface parent;
  ModemCallServicePrivate *priv;
};

GType modem_call_service_get_type (void);

/* TYPE MACROS */
#define MODEM_TYPE_CALL_SERVICE \
  (modem_call_service_get_type ())
#define MODEM_CALL_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
      MODEM_TYPE_CALL_SERVICE, ModemCallService))
#define MODEM_CALL_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
      MODEM_TYPE_CALL_SERVICE, ModemCallServiceClass))
#define MODEM_IS_CALL_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
      MODEM_TYPE_CALL_SERVICE))
#define MODEM_IS_CALL_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MODEM_TYPE_CALL_SERVICE))
#define MODEM_CALL_SERVICE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
      MODEM_TYPE_CALL_SERVICE, ModemCallServiceClass))

typedef struct _ModemCallAgent ModemCallAgent;
typedef struct _ModemCallAgentClass ModemCallAgentClass;
typedef struct _ModemCallAgentPrivate ModemCallAgentPrivate;

struct _ModemCallAgentClass
{
  ModemOfaceClass parent_class;
};

struct _ModemCallAgent
{
  ModemOface parent;
  ModemCallAgentPrivate *priv;
};

GType modem_call_agent_get_type (void);

/* TYPE MACROS */
#define MODEM_TYPE_CALL_AGENT (modem_call_agent_get_type ())
#define MODEM_CALL_AGENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
      MODEM_TYPE_CALL_AGENT, ModemCallAgent))
#define MODEM_CALL_AGENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
      MODEM_TYPE_CALL_AGENT, ModemCallAgentClass))
#define MODEM_IS_CALL_AGENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
      MODEM_TYPE_CALL_AGENT))
#define MODEM_IS_CALL_AGENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MODEM_TYPE_CALL_AGENT))
#define MODEM_CALL_AGENT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
      MODEM_TYPE_CALL_AGENT, ModemCallAgentClass))

typedef struct _ModemCall ModemCall;
typedef struct _ModemCallClass ModemCallClass;
typedef struct _ModemCallPrivate ModemCallPrivate;

typedef enum _ModemCallState ModemCallState;
typedef enum _ModemCallCauseType ModemCallCauseType;
typedef enum _ModemClirOverride ModemClirOverride;

struct _ModemCallClass {
  ModemOfaceClass parent_class;
};

struct _ModemCall {
  ModemOface parent;
  ModemCallPrivate *priv;
};

GType modem_call_get_type (void);

/* TYPE MACROS */
#define MODEM_TYPE_CALL \
  (modem_call_get_type ())
#define MODEM_CALL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MODEM_TYPE_CALL, ModemCall))
#define MODEM_CALL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MODEM_TYPE_CALL, ModemCallClass))
#define MODEM_IS_CALL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MODEM_TYPE_CALL))
#define MODEM_IS_CALL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MODEM_TYPE_CALL))
#define MODEM_CALL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MODEM_TYPE_CALL, ModemCallClass))

/* ------------------------------------------------------------------------- */

#define MODEM_OFACE_CALL_MANAGER "org.ofono.VoiceCallManager"
#define MODEM_OFACE_CALL         "org.ofono.VoiceCall"
#define MODEM_OFACE_CALL_AGENT   "org.ofono.VoiceCallAgent"

/**
 * Call properties
 */

/** ModemCall state */
enum _ModemCallState
{
  MODEM_CALL_STATE_INVALID = 0,
  MODEM_CALL_STATE_DIALING,
  MODEM_CALL_STATE_ALERTING,
  MODEM_CALL_STATE_INCOMING,
  MODEM_CALL_STATE_WAITING,
  MODEM_CALL_STATE_ACTIVE,
  MODEM_CALL_STATE_HELD,
  MODEM_CALL_STATE_DISCONNECTED
};

enum _ModemCallCauseType
{
  MODEM_CALL_CAUSE_TYPE_UNKNOWN,
  MODEM_CALL_CAUSE_TYPE_NETWORK,
  MODEM_CALL_CAUSE_TYPE_LOCAL,
  MODEM_CALL_CAUSE_TYPE_REMOTE
};

/** Dial flags */
enum _ModemClirOverride
{
  MODEM_CLIR_OVERRIDE_DEFAULT = 0,
  MODEM_CLIR_OVERRIDE_ENABLED,
  MODEM_CLIR_OVERRIDE_DISABLED
};

enum
{
  MODEM_MAX_CALLS = 7
};

char const *modem_call_service_property_name_by_ofono_name (char const *);

char const *modem_call_get_state_name (int state);
GError *modem_call_new_error (guint causetype, guint cause,
    char const *prefixed);

void modem_call_service_resume (ModemCallService *);

/* Validate addresses */
gboolean modem_call_is_valid_address (char const *address);
gboolean modem_call_validate_address (char const *address, GError **error);

void modem_call_split_address (char const *address,
  char **return_address,
  char **return_dialstring,
  ModemClirOverride *return_clir);

int modem_call_event_tone (guint state, guint causetype, guint cause);

int modem_call_error_tone (GError *);

char const * const *modem_call_get_emergency_numbers (ModemCallService *self);
char const *modem_call_get_valid_emergency_urn (char const *urn);
char const *modem_call_get_emergency_service (ModemCallService*, char const*);

typedef void ModemCallServiceReply (ModemCallService *,
  ModemRequest *,
  GError *error,
  gpointer user_data);

ModemCall *modem_call_service_get_call (ModemCallService *, char const *);
ModemCall **modem_call_service_get_calls (ModemCallService *);

typedef void ModemCallRequestDialReply (ModemCallService *,
  ModemRequest *,
  ModemCall *,
  GError *error,
  gpointer user_data);

ModemRequest *modem_call_request_dial (ModemCallService *self,
  char const *destination,
  ModemClirOverride clir,
  ModemCallRequestDialReply *callback,
  gpointer user_data);

ModemRequest *modem_call_request_conference (ModemCallService *,
  ModemCallServiceReply *callback,
  gpointer user_data);

ModemRequest *modem_call_request_hangup_conference (ModemCallService *,
  ModemCallServiceReply *callback,
  gpointer user_data);

ModemRequest *modem_call_service_swap_calls (ModemCallService *self,
  ModemCallServiceReply callback,
  gpointer user_data);

char const *modem_call_get_name (ModemCall const *);
char const *modem_call_get_path (ModemCall const *);
gboolean modem_call_has_path (ModemCall const *, char const *object_path);

ModemCallState modem_call_get_state (ModemCall const *);
ModemCallState modem_call_state_from_ofono_state (char const *state);

gboolean modem_call_try_set_handler (ModemCall *, gpointer);
void modem_call_set_handler (ModemCall *, gpointer);
gpointer modem_call_get_handler (ModemCall *);

gboolean modem_call_is_member (ModemCall const *);
gboolean modem_call_is_originating (ModemCall const *);
gboolean modem_call_is_terminating (ModemCall const *);

gboolean modem_call_is_active (ModemCall const *);
gboolean modem_call_is_held (ModemCall const *);

typedef void ModemCallReply (ModemCall *,
  ModemRequest *,
  GError *,
  gpointer user_data);

ModemRequest *modem_call_request_answer (ModemCall*,
  ModemCallReply *,
  gpointer user_data);
ModemRequest *modem_call_request_release (ModemCall*,
  ModemCallReply *,
  gpointer user_data);

ModemRequest *modem_call_send_dtmf (ModemCall *self,
  char const *dialstring,
  ModemCallReply *callback,
  gpointer user_data);
ModemRequest *modem_call_start_dtmf (ModemCall *self,
  char tone,
  ModemCallReply *callback,
  gpointer user_data);
ModemRequest *modem_call_stop_dtmf (ModemCall *self,
  ModemCallReply *callback,
  gpointer user_data);

gboolean modem_call_can_join (ModemCall const *);

ModemRequest *modem_call_request_hold (ModemCall *, int hold,
  ModemCallReply *,
  gpointer user_data);

ModemRequest *modem_call_request_split (ModemCall *,
  ModemCallReply *,
  gpointer user_data);

G_END_DECLS

#endif /* #ifndef _MODEM_CALL_H_ */
