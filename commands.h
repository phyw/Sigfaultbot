#ifndef _COMMANDS_
#define _COMMANDS_   
extern void handler_cmd(LmConnection *connect, gchar* cmd, gchar* args);
extern void *PrintHello(void *threadid);
extern void* wipe(void *tinfo);
extern void leave_all_room(gpointer key, gpointer value, gpointer connect);
#endif

