
// Основная программа для управления //

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "xmpp.h"   
#include "util.h"
#include "commands.h"

#define CONFIG_GROUP_NAME "losbot"
#define CONFIG_PATH "./conf.ini"

/*
 * Чтение конфигурационного файла
 */
gboolean read_config(const gchar *path) {
  GKeyFile *keyfile;
  GError *error = NULL;

  gchar **keys, *value;
  gsize *nkeys = NULL;
  int i = 0;

  keyfile = g_key_file_new();

  if(!g_key_file_load_from_file(keyfile, path, 0, &error)) {
    fprintf(stderr, "Could not be loaded config file: %s\n", error->message);
    return FALSE;
  }

  keys = g_key_file_get_keys(keyfile, CONFIG_GROUP_NAME, nkeys, &error);
  if(!keys) {
    fprintf(stderr, "Could not be read keys from config file: %s\n", error->message);
    return FALSE;
  }

  while(keys[i]) {
    value = g_key_file_get_value(keyfile, CONFIG_GROUP_NAME, keys[i], &error);
    g_hash_table_replace(config, g_strdup(keys[i]), g_strdup(value));
    fprintf(stderr, "g_hash_table_lookup(config, %s) = %s\n", 
                  keys[i], (gchar*)g_hash_table_lookup(config, keys[i]));
    i++;
  }
  fprintf(stderr, "\n");  

  g_strfreev(keys);
  return TRUE;
}

/*
 * Main
 */
int main(int argc, char **argv) {
  LmMessageHandler  *logger;
  LmConnection      *connect; 

  init_hash_tables();
  read_config(CONFIG_PATH);

  struct _jid jid;
  jid.server = (gchar*)g_hash_table_lookup(config, "server");
  jid.username = (gchar*)g_hash_table_lookup(config, "username");
  jid.password = (gchar*)g_hash_table_lookup(config, "password");
  jid.resource = (gchar*)g_hash_table_lookup(config, "resource");

  connect = jid_connect(&jid, g_main_context_default());
  if(connect == NULL)
    return -1;

  jid_init(connect, &jid);
  set_status(connect, &jid, (gchar*)g_hash_table_lookup(config, "status"), 
                        (gchar*)g_hash_table_lookup(config, "priority"));

  sleep(1);
  logger = lm_message_handler_new(handle_messages, NULL, NULL);
  lm_connection_register_message_handler(connect, logger,
                LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);

  main_loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(main_loop);
   
  return 0;

}

