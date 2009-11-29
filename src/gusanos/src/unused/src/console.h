#ifndef CONSOLE_H
#define CONSOLE_H

#include <ctype.h>
#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "keys.h"
#include "text.h"

int knametoint(char *kname);

struct s_echolist
{
  int count;
  int time;
  struct msg *start,*end;
  char text[6][255];
  void add_echo(char *text);
  void remove_echo();
  void init();
  void destroy();
  void render(BITMAP *where);
  void calc();
};

struct s_binding
{
	int key;
	char cmd_str[512];
	struct s_binding *next,*prev;
};

struct s_bindtable
{
	struct s_binding *start,*end;
	void init();
	void destroy();
	int create_binding(int bind, char* cmd_str);
	int destroy_binding(int bind);
};

struct variable
{
	int *value;
	char name[255];
	struct variable *next;
	struct variable *prev;
};

struct s_cmd
{
	void (*func)();
	char name[255];
	struct s_cmd *next;
	struct s_cmd *prev;
};

struct msg
{
	char text[255];
	struct msg *next,*prev;
	struct msg *nextcom;
	struct msg *prevcom;
};

struct msg_log
{
	struct msg *start,*end,*comend;
	void init();
	void create_msg(char* text);
	void create_com(char* text);
};

struct console
{
	int flag;
	int pos;
	int *height,*speed;
  int *echo_delay;
	bool flag2,flag3;
	char textbuf[255];
	struct msg_log log;
	struct msg *tmp_com;
	struct fnt *font;
	struct s_bindtable bindtable;
  struct s_echolist echolist;
	struct variable *start;
	struct variable *end;
	struct s_cmd *cmd_start;
	struct s_cmd *cmd_end;
	void init();
	int* create_variable(char* name, int value);
	struct variable* find_variable(char* name);
	void add_cmd(char* name, void (*func)());
	struct s_cmd* find_cmd(char* name);
	void render(BITMAP* where);
	void parse(char* str);
	void parse_silent(char* str);	
  void input();
	char arg[512];
};

extern struct console* con;

void bind();
void unbind();
void execute_config();


#endif /* CONSOLE_H */

