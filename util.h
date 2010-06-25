#ifndef _UTIL_H
#define _UTIL_H

#define dprint(expr) fprintf(stderr, #expr " = %s\n", expr)

#define NICK_LIST "./namelist"
#define SERV_LIST "./serverlist"
#define SIMPLE_PASS "qwerty"
#define SIMPLE_RES "olololo"     

extern void* xcalloc(int nmemb, size_t size);
extern void* p_error(const char* msg, const char status); 
extern gchar* skip_spaces(const gchar *str);
extern gchar* skip_newline(const gchar *str);
extern gboolean init_hash_tables(void);
extern gboolean is_room(gchar* jid);
extern gchar* rnd_string_from_file(gchar *path);
//extern void add_rand(gchar **str);
extern gchar* get_rnd_str(void);

extern GHashTable *config;
extern GHashTable *rooms;
extern GMainLoop *main_loop;
#endif

