
// Работа с xmpp //

#include <loudmouth/loudmouth.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "xmpp.h"
#include "util.h"
#include "commands.h"

/*
 * Регистрация нового jid
 */

gboolean jid_reg(struct _jid *jid, GMainContext *context) {
  LmConnection  *reg_connect;
  LmMessage     *m, *reply;
  LmMessageNode *query, *node;
  GError        *error = NULL;
  
  // Проверяем context
  if(!context)
    context = g_main_context_default();
  
  // Подключаемся
  reg_connect = lm_connection_new_with_context(jid->server, context);
  // Настраиваем ssl
/*  if(jid->use_ssl) {
    LmSSL *ssl;

    // Проверяем поддержку ssl
    if(!lm_ssl_is_supported()) {
      g_print("Your Loudmouth doesn't support SSL. Reinstall loudmouth.\n");
      jid->use_ssl = FALSE;
    }

    g_print("Configure ssl\n");
    ssl = lm_ssl_new(NULL, LM_SSL_RESPONSE_CONTINUE, NULL, NULL);
    lm_connection_set_ssl(reg_connect, ssl);
    lm_ssl_unref(ssl);

    lm_connection_set_port(reg_connect, LM_CONNECTION_DEFAULT_PORT_SSL);

  }
*/
  // Проверяем коннект
  if(!lm_connection_open_and_block(reg_connect, &error)) {
    fprintf(stderr, "Failed to open connection: %s\n", error->message);
    return FALSE;
  }

  // Определяем сообщение
  m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ,
                                    LM_MESSAGE_SUB_TYPE_SET);

  // Составляем запрос
  query = lm_message_node_add_child(m->node, "query", NULL);
  lm_message_node_set_attributes(query, "xmlns", "jabber:iq:register", NULL);
  lm_message_node_add_child(query, "username", jid->username);
  lm_message_node_add_child(query, "password", jid->password);

  // Отпревляем сообщение и ждём ответ
  reply = lm_connection_send_with_reply_and_block(reg_connect, m, &error);

  if(!reply) {
    fprintf(stderr, "Failed to send registration request on server \"%s\":\n %s\n",
             jid->server, error->message);
    return FALSE;
  } 
  
  //Закрываем соединение
  lm_connection_close(reg_connect, NULL);
  lm_connection_unref(reg_connect);    
  
  // Проверяем ответ
  switch(lm_message_get_sub_type(reply)) {
    case LM_MESSAGE_SUB_TYPE_RESULT:
      g_print("Succeeded in register account '%s@%s'\n",
                                   jid->username, jid->server);
      break;

    case LM_MESSAGE_SUB_TYPE_ERROR:
    default:
      g_print("Failed to register account '%s@%s' due to: ",
                        jid->username, jid->server);

      node = lm_message_node_find_child(reply->node, "error");
      if(node)
        g_print("%s\n", lm_message_node_get_value (node));
      else
        g_print("Unknown error\n");

      return FALSE;
      break;
  } 
  
  return TRUE;
}

/*
 * Вход в конферецию
 */
gboolean join_room(LmConnection *connect,
              const gchar* room_name, const gchar* nick) {

  LmMessage       *m;
  LmMessageNode   *query;
  gchar           *room_jid;
  GError          *error = NULL;

  room_jid = (gchar*)xcalloc(strlen(room_name)+strlen(nick)+2, sizeof(gchar));
  strcat(room_jid, room_name);
  strcat(room_jid, "/");
  strcat(room_jid, nick);
  
  // Создаём новое сообщение запросом на вход
  m = lm_message_new(room_jid, LM_MESSAGE_TYPE_PRESENCE);
  query = lm_message_node_add_child(m->node, "x", NULL);
  lm_message_node_set_attributes(query, "xmlns",
                      "http://jabber.org/protocol/muc", NULL);
  // Заходим...
  if(!lm_connection_send(connect, m, &error)) {
      g_print("Failed join to '%s' due to: %s\n",
                room_jid, error->message);
      free(room_jid); //FIXME
      return FALSE;
  }
  
  fprintf(stderr, "Join to %s as %s\n", room_name, nick);
  
  free(room_jid);
  lm_message_unref(m);
  return TRUE;
}

/*
 * Выход из конференции
 */
gboolean leave_room(LmConnection *connect,
            const gchar* room_name, const gchar* nick, const gchar* leave_status) {
  LmMessage *m;
  gchar     *room_jid;
  GError    *error = NULL;

  room_jid = (gchar*)xcalloc(strlen(room_name)+strlen(nick)+2, sizeof(gchar));
  strcat(room_jid, room_name);
  strcat(room_jid, "/");
  strcat(room_jid, nick);
  
  // Создаём и отправляем сообщение о выходе
  m = lm_message_new_with_sub_type(room_jid, LM_MESSAGE_TYPE_PRESENCE, 
                                    LM_MESSAGE_SUB_TYPE_UNAVAILABLE);
  free(room_jid);

  if(leave_status)
    lm_message_node_add_child(m->node, "status", leave_status);

  if(!lm_connection_send(connect, m, &error)) {
      g_print("Failed leave to '%s' due to: %s\n",
                room_name, error->message);
      return FALSE;
  }
  fprintf(stderr, "Leave to %s\n", room_name);

  lm_message_unref(m);
  return TRUE;
}

/*
 * Отправка сообщения в конференцию
 */
gboolean room_message(LmConnection *connect, 
                const gchar *room_name, const gchar *text) {

  LmMessage *m;
  GError    *error = NULL;

  // Создаём сообщение
  m = lm_message_new_with_sub_type(room_name, LM_MESSAGE_TYPE_MESSAGE,
                                      LM_MESSAGE_SUB_TYPE_GROUPCHAT);
  dprint(room_name);
  dprint(text);
  lm_message_node_add_child(m->node, "body", text);

  if(!lm_connection_send(connect, m, &error)) {
      g_print("Failed sent message to '%s' due to: %s\n",
                room_name, error->message);
      return FALSE;
  }

  printf("Send message to %s\n", room_name);
  lm_message_unref(m);
  return TRUE;
}

/*
 * Отправка сообщения на другой jid
 */
gboolean jid_message(LmConnection *connect, 
                    const gchar* to_jid, const gchar* text) {
  LmMessage *m;
  GError    *error = NULL;
  // Создаём сообщение
  m = lm_message_new_with_sub_type(to_jid, LM_MESSAGE_TYPE_MESSAGE,
                                      LM_MESSAGE_SUB_TYPE_CHAT);

  lm_message_node_add_child(m->node, "body", text);

  if(!lm_connection_send(connect, m, &error)) {
      g_print ("Failed sent message to '%s' due to: %s\n",
                to_jid, error->message);
      return FALSE;
  }

  printf("Send message to %s\n", to_jid);
  lm_message_unref(m);
  return TRUE;
}

/*
 * Соеденение с сервером и аутентификация
 */
LmConnection* jid_connect(const struct _jid *jid, GMainContext *context) {
  LmConnection  *new_connect;
  GError        *error = NULL;
  
  // Проверяем context
  if(!context)
    context = g_main_context_default();

  // Создаём соединение и пытаемся соедениться
  new_connect = lm_connection_new_with_context(jid->server, 
                                          context);
  if(!lm_connection_open_and_block(new_connect, &error)) {
    g_print ("Couldn't open connection to '%s':\n%s\n",
            jid->server, error->message);
    return NULL;
  }
  // Авторизируемся
  if(!lm_connection_authenticate_and_block(new_connect, jid->username,
                               jid->password, jid->resource, &error)) {
    g_print("Couldn't authenticate with '%s' '%s':\n%s\n",
             jid->username, jid->password, error->message);
    return NULL;
  }

  printf("Established connect with %s\n", jid->server);
  return new_connect;
}

/*
 * Инициализация 
 */
gboolean jid_init(LmConnection *connect, const struct _jid *jid) {
  LmMessage       *m;
  LmMessageNode   *query, *storage;
  GError          *error = NULL;

  // <iq type='get'><query xmlns='jabber:iq:roster'/></iq>
  m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ,
                        LM_MESSAGE_SUB_TYPE_GET);  
  query = lm_message_node_add_child(m->node, "query", NULL);
  lm_message_node_set_attributes(query, "xmlns", "jabber:iq:roster", NULL);

  if(!lm_connection_send(connect, m, &error)) {
      g_print ("Failed sent query 'xmlns=jabber:iq:roster', due to: %s\n",
                error->message);
      return FALSE;
  }
  
  // <iq type='get'><query xmlns='jabber:iq:private'><storage xmlns='storage:bookmarks'/></query></iq>
  m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ,
                        LM_MESSAGE_SUB_TYPE_GET);  
  query = lm_message_node_add_child(m->node, "query", NULL);
  storage = lm_message_node_add_child(m->node, "storage", NULL);

  lm_message_node_set_attributes(query, "xmlns", "jabber:iq:private", NULL);
  lm_message_node_set_attributes(storage, "xmlns", "storage:bookmarks", NULL);

  if(!lm_connection_send(connect, m, &error)) {
      g_print ("Failed sent query 'jabber:iq:private', due to: %s\n",
                error->message);
      return FALSE;
  }

  // <iq type='get'><query xmlns='jabber:iq:private'><storage xmlns='storage:rosternotes'/></query></iq>
  m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ,
                        LM_MESSAGE_SUB_TYPE_GET);
  query = lm_message_node_add_child(m->node, "query", NULL);
  storage = lm_message_node_add_child(m->node, "storage", NULL);

  lm_message_node_set_attributes(query, "xmlns", "jabber:iq:private", NULL);
  lm_message_node_set_attributes(storage, "xmlns", "storage:rosternotes", NULL);

  if(!lm_connection_send(connect, m, &error)) {
      g_print ("Failed sent second query 'jabber:iq:private', due to: %s\n",
                error->message);
      return FALSE;
  }
  
  lm_message_unref(m);
  return TRUE;
}

/*
 * Установка статуса и приоритета
 */
gboolean set_status(LmConnection *connect, const struct _jid *jid,
                    const gchar *status, const gchar *priority) {
  LmMessage *m;
  GError    *error = NULL;

  // Создаём и отправляем сообщение
  m = lm_message_new(NULL, LM_MESSAGE_TYPE_PRESENCE); 
  lm_message_node_add_child(m->node, "status", status);
  lm_message_node_add_child(m->node, "priority", priority);

  if(!lm_connection_send(connect, m, &error)) {
      g_print ("Failed set status and priority, due to: %s\n",
                error->message);
      return FALSE;
  }
  lm_message_unref(m); 
  return TRUE;
}

/*
 * Получение значения одного из child
 */
gchar* lm_message_node_get_child_value(LmMessageNode *node,
                                        const gchar *child) {
  LmMessageNode *tmp;
  tmp = lm_message_node_find_child(node, child);
  if(tmp)
    return (gchar*)lm_message_node_get_value(tmp);
  else return NULL;
}

/*
 * Получение аттрибута одного из нодов
 */
gchar* lm_message_node_get_child_attribute(LmMessageNode *node,
                        const gchar *child, const gchar *attribute) {
  LmMessageNode *tmp;
  tmp = lm_message_node_find_child(node, child);
  if(tmp)
    return (gchar*)lm_message_node_get_attribute(tmp, attribute);
  else return NULL;
}
/*
 * Обработчик входящих сообщений
 */
LmHandlerResult handle_messages(LmMessageHandler *handler,
                  LmConnection *connect, LmMessage *m, gpointer data) {
  printf("\nОбрабатываю событие LM_MESSAGE_TYPE_MESSAGE\n");

  const gchar *from, *to, *body, *xmlns;
        gchar  *str, *sstr = NULL;
  
  LmMessageSubType mstype;

  xmlns = lm_message_node_get_child_attribute(m->node, "x", "xmlns");
  if(!xmlns)
    xmlns = "";
  if(!strstr(xmlns, "delay")) {

    from = lm_message_node_get_attribute(m->node, "from");
    to = lm_message_node_get_attribute(m->node, "to");
    body = lm_message_node_get_child_value(m->node, "body");

    mstype = lm_message_get_sub_type(m);

    if(body) {
      str = g_strdup(body);
      sstr = g_strdup(strtok(str, " "));
    }

    if(sstr) {

      printf("Incoming message\n\tfrom: %s\n\tto: %s\n\tbody: %s\n",
              from, to, body);

      handler_cmd(connect, sstr, strchr(body, ' '));
    }
    
    free(str);
    free(sstr);
  }
  
  usleep(100000);
  lm_message_unref(m);
  return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

