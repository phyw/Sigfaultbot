#ifndef _XMPP_
#define _XMPP_ 

#include <loudmouth/loudmouth.h>
#include <stdlib.h>
#include <stdio.h>


struct _jid {
  gchar *server;
  gchar *username;
  gchar *password;
  gchar *resource;
  gboolean use_ssl;
};

extern gboolean jid_reg(struct _jid *jid, GMainContext *context);
extern gchar* get_room_jid(const gchar* room_name, const gchar* nick);
extern gboolean join_room(LmConnection *connect, 
                  const gchar* room_name, const gchar* nick);
extern gboolean leave_room(LmConnection *connect, const gchar* room_name,
                    const gchar* nick, const gchar* leave_status);
extern gboolean room_message(LmConnection *connect, const gchar* room_name, const gchar* text);
extern gboolean jid_message(LmConnection *connect, const gchar* to_jid, const gchar* text);
extern LmConnection* jid_connect(const struct _jid *jid, GMainContext *context);
extern gboolean jid_init(LmConnection *connect, const struct _jid *jid);
extern gboolean set_status(LmConnection *connect, const struct _jid *jid,
                            const gchar *status, const gchar *priority);
extern LmHandlerResult handle_messages (LmMessageHandler *handler,
          LmConnection *connect, LmMessage *m, gpointer data);
#endif

