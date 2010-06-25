#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>  

#define EALLOC 1

GHashTable *config;
GHashTable *rooms; 
GMainLoop *main_loop = NULL;

void* xcalloc(int nmemb, size_t size) {
  void* ptr = calloc(nmemb, size);
  if(ptr == NULL) 
    exit(EALLOC);
  return ptr;
}

gchar* skip_spaces(const gchar *str) {
  while (isspace(*str))
    ++str;
  return (gchar*)str;
}

gchar* skip_newline(const gchar *str) {
  gchar *ptr = (gchar*)str;
  while(*ptr != '\n' && *ptr != '\0')
    ++ptr;
  *ptr = '\0';
  return (gchar*)str;
}   

gboolean init_hash_tables(void) {
  config = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
  rooms = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL); 

  g_hash_table_insert(config, "server", NULL);
  g_hash_table_insert(config, "username", NULL);
  g_hash_table_insert(config, "password", NULL);
  g_hash_table_insert(config, "resource", g_strdup("P_P"));
  g_hash_table_insert(config, "muc_default_nick", NULL);
  g_hash_table_insert(config, "owner", NULL);
  g_hash_table_insert(config, "status", NULL);
  g_hash_table_insert(config, "priority", NULL);

  g_hash_table_insert(rooms, "", NULL); 

  return TRUE;
}

gboolean is_room(gchar* jid) {
  if(strstr(jid, "@conference."))
    return TRUE;
  return FALSE;
} 

gchar* rnd_string_from_file(gchar *path) {
  FILE *file;
  struct stat buf;
  long int rnd;
  gchar* str = (gchar*)xcalloc(500, sizeof(gchar));

  stat(path, &buf);

  file = fopen(path, "r");
  rnd = random() % (int)buf.st_size;
  fseek(file, rnd, SEEK_SET);
  
  fgets(str, 500, file);
  // КОСТЫЫЫЫЛЬ! FIXME
  fgets(str, 500, file);

  return skip_newline(str);
}

gchar* get_rnd_str(void) {
  gchar *str = (gchar*)xcalloc(7, sizeof(gchar));
  sprintf(str, "%d", (int)random() % 123453);
  return str;
}


