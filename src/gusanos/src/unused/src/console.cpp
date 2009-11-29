#include "console.h"

struct console* con; 

int knametoint(char *kname)
{
	int i;
	for (i=0;i<116;i++)
	{
		if (strcmp(kname,key_names[i])==0)
		return (i+1);
	};
	return (-1);
};
	
void chat()
{
  con->flag=2;
  strcpy(con->textbuf,"");
  clear_keybuf();
};

void s_echolist::init()
{
  start=new struct msg;
	end=start;
	start->next=NULL;
	start->prev=NULL;
  count=0;
  time=0;
};

void s_echolist::add_echo(char* text)
{
  end->next=new struct msg;
	end->next->prev=end;
	end=end->next;
	end->next=NULL;
  strcpy(end->text,text);
  count++;
  time=0;
  if (count>3) remove_echo();
};

void s_echolist::remove_echo()
{
	struct msg *curr;
	curr=start->next;
  time=0;
  if(curr)
  {
    curr->prev->next=curr->next;
    if(curr!=end) curr->next->prev=curr->prev;
    else end=curr->prev;
    delete curr;
    count--;
  };
};

void s_echolist::render(BITMAP* where)
{
	struct msg *currmsg;
	int i;
  i=1;
  currmsg=end;
  while (currmsg!=start)
  {
    con->font->draw_string(where,currmsg->text,3,where->h-1-(i*6),true);
    i++;
    currmsg=currmsg->prev;
  };
};


void s_echolist::calc()
{
  time++;
  if (time>*con->echo_delay) remove_echo();
}

void s_bindtable::init()
{
	start=new struct s_binding;
	end=start;
	start->next=NULL;
	start->prev=NULL;
};

int s_bindtable::destroy_binding(int bind)
{
	struct s_binding *curr;
	curr=start;
	while (curr->next!=NULL)
	{
		curr=curr->next;
		//is this the binding?
		if (curr->key==bind)
		{
			//it is so delet it and return 0
			curr->prev->next=curr->next;
			if(curr!=end) curr->next->prev=curr->prev;
			else end=curr->prev;
			free(curr);
			return 0;
		};
	};
	//the key was not found so return 1
	return 1;
};

int s_bindtable::create_binding(int bind, char* cmd_str )
{
	destroy_binding(bind);
	end->next=new struct s_binding;
	end->next->prev=end;
	end=end->next;
	end->next=NULL;
	end->key=bind;
	strcpy(end->cmd_str,cmd_str);
	return 0;
};

void msg_log::init()
{
	start=new struct msg;
	end=start;
	comend=start;
	end->prev=NULL;
	end->next=NULL;
	comend->prevcom=NULL;
	comend->nextcom=NULL;
};

void msg_log::create_com(char* text)
{
	end->next=new struct msg;
	end->next->prev=end;
	end=end->next;
	end->next=NULL;
	comend->nextcom=end;
	comend->nextcom->prevcom=comend;
	comend=comend->nextcom;
	comend->nextcom=NULL;
	strcpy(end->text,text);
};

void msg_log::create_msg(char* text)
{
	end->next=new struct msg;
	end->next->prev=end;
	end=end->next;
	end->next=NULL;
	strcpy(end->text,text);
};

void console::init()
{
	start=new struct variable;
	end=start;
	end->next=NULL;
	end->prev=NULL;
	cmd_start=new struct s_cmd;
	cmd_end=cmd_start;
	cmd_end->next=NULL;
	cmd_end->prev=NULL;
	log.init();
	height=create_variable("CON_HEIGHT",100);
	speed=create_variable("CON_SPEED",4);
  echo_delay=create_variable("CON_ECHODELAY",200);
  add_cmd("CHAT", chat);
	bindtable.init();
  echolist.init();
};

int* console::create_variable(char* name,int value)
{
  struct variable* v=find_variable(name);
  if(v!=NULL) return v->value;
	end->next=new struct variable;
	end->next->prev=end;
	end=end->next;
	end->next=NULL;
	end->value=new int;
	*end->value=value;
	strcpy(end->name,name);
	return end->value;
};

void console::add_cmd(char* name,void (*func)())
{
	cmd_end->next=new struct s_cmd;
	cmd_end->next->prev=cmd_end;
	cmd_end=cmd_end->next;
	cmd_end->next=NULL;
	cmd_end->func=func;
	strcpy(cmd_end->name,name);
};

struct variable* console::find_variable(char* name)
{
	struct variable *curr;
	
	curr=start;
	
	// allegro_message(name);
	
	while (curr->next!=NULL)
	{
		curr=curr->next;
		if (strcmp(curr->name,name)==0)
			return curr;
	};
	// allegro_message("NOT found");
	return NULL;
};

struct s_cmd* console::find_cmd(char* name)
{
	struct s_cmd *curr;
	
	curr=cmd_start;
	
	// allegro_message(name);
	
	while (curr->next!=NULL)
	{
		curr=curr->next;
		if (strcmp(curr->name,name)==0)
			return curr;
	};
	// allegro_message("NOT found");
	return NULL;
};

void console::render(BITMAP* where)
{
	char tmp[255];
	struct msg *currmsg;
	int i;
	
	if (pos>0)
	{
		rectfill(where,0,0,319,pos-2,0);
		sprintf(tmp, "%s%c",textbuf,'&');
		font->draw_string(where,tmp,3,pos-8,false);
		i=0;
		currmsg=log.end;
		while (currmsg!=log.start && i < pos/7 )
		{
			font->draw_string(where,currmsg->text,3,pos-16-(i*7),false);
			i++;
			currmsg=currmsg->prev;
		};
		line(where,0,pos-1,319,pos-1,makecol(120,120,120));
	};
  echolist.render(where);
  
  if(flag==2)
  {
    rectfill(where,10,70,where->w-10,77,makecol(0,0,0));
    rect(where,9,69,where->w-9,78,makecol(100,100,100));
    font->draw_string(where,textbuf,11,71,false);
  };
}

void bind()
{
	char k[15],fn_name[512],tmpstr[512];
	if (strlen(con->arg)>0)
	{
		struct s_cmd *tmp_cmd;
		int t,o;
		t=0;
		while (con->arg[t]!=' ' && t<strlen(con->arg)) t++;
		strcpy(k,strmid(con->arg,0,t));
		o=knametoint(k);
		
		if (o!=-1)
		{
			strcpy(fn_name,"");
			if (t<strlen(con->arg))
			{
				strcpy(fn_name,strmid(con->arg,t+1,strlen(con->arg)-t-1));

				con->bindtable.create_binding(o,fn_name);
				return;
			}else
			{
				struct s_binding *curr;
				curr=con->bindtable.start;
				while (curr->next!=NULL)
				{
					curr=curr->next;
					if (curr->key==o) 
					{
						sprintf(tmpstr,"\"%s\" = \"%s\"",k,curr->cmd_str);
						con->log.create_msg(tmpstr);
						return;
					};
				};
				sprintf(tmpstr,"\"%s\" IS UNBOUND",k);
				con->log.create_msg(tmpstr);
				return;
			};
		};
		sprintf(tmpstr,"\"%s\" IS NOT A VALID KEY",k);
		con->log.create_msg(tmpstr);
		return;
	};
	con->log.create_msg("BIND <KEY> [COMMAND] : ATTACH A COMMAND TO A KEY");
};

void unbind()
{
	if (strlen(con->arg)>0)
	{
		int o;
		o=knametoint(con->arg);
		if (o!=-1)
		{
			con->bindtable.destroy_binding(o);
			return;
		};
	};	
};

void console::parse(char* str)
{
	if (strlen(str)!=0)
	{
		unsigned int t;
		char *var, *val,tmpstr[255];
		struct variable* tmpvar;
		struct s_cmd *tmp_cmd;
		t=0;
		while (str[t]!=' ' && t<strlen(str)) t++;
		var=strmid(str,0,t);		
		tmpvar=con->find_variable(var);
		tmp_cmd=con->find_cmd(var);
		if (tmpvar!=NULL)
		{
			if (t<strlen(str))
			{
				val=strmid(str,t+1,strlen(str)-t-1);
				*tmpvar->value=atoi(val);
				con->log.create_com(str);
				con->tmp_com=con->log.start;
			}
			else
			{
				strcpy(tmpstr,tmpvar->name);
				strcat(tmpstr," IS ");
				sprintf(tmpstr,"%s%d",tmpstr,*tmpvar->value);
				con->log.create_com(str);
				con->log.create_msg(tmpstr);
				con->tmp_com=con->log.start;
			};		
		} else if (tmp_cmd!=NULL)
		{
			strcpy(con->arg,"");
			if (t<strlen(str))
			{
				strcpy(con->arg,strmid(str,t+1,strlen(str)-t-1));
			};
			con->log.create_com(str);
			con->tmp_com=con->log.start;
			tmp_cmd->func();
		}	
		else
		{
			strcpy(tmpstr,"UNKNOWN COMMAND \"");
			strcat(tmpstr,var);
			strcat(tmpstr,"\"");
			con->log.create_msg(tmpstr);
			con->tmp_com=con->log.start;
		};
	};
};

void console::parse_silent(char* str)
{
	if (strlen(str)!=0)
	{
		unsigned int t;
		char *var, *val,tmpstr[255];
		struct variable* tmpvar;
		struct s_cmd *tmp_cmd;
		t=0;
		while (str[t]!=' ' && t<strlen(str)) t++;
		var=strmid(str,0,t);		
		tmpvar=con->find_variable(var);
		tmp_cmd=con->find_cmd(var);
		if (tmpvar!=NULL)
		{
			if (t<strlen(str))
			{
				val=strmid(str,t+1,strlen(str)-t-1);
				*tmpvar->value=atoi(val);
			};
		} else if (tmp_cmd!=NULL)
		{
			strcpy(con->arg,"");
			if (t<strlen(str))
			{
				strcpy(con->arg,strmid(str,t+1,strlen(str)-t-1));
			};
			tmp_cmd->func();
		}	
	}; 
};

void check_bindings()
{
	struct s_binding *curr;
	curr=con->bindtable.start;
	while (curr->next!=NULL)
	{
		curr=curr->next;
		if (key[curr->key]) 
		{
			con->parse_silent(curr->cmd_str);
		};
	};
};

void console::input()
{
  if (con->flag==0)
	{
		check_bindings();
  };
  if (con->flag==2)
  {
    if (keypressed())
		{
			char k;
			k=readkey();
			k=toupper(k);
			if (k==8) con->textbuf[strlen(con->textbuf)-1]=0;
			else if (k==13)
			{
				strcpy(con->arg,con->textbuf);
        send_msg();
				strcpy(con->textbuf,"");
        con->flag=0;
			}
			else sprintf(con->textbuf,"%s%c",con->textbuf,k);
		};
  };
  if(con->flag==1)
	{
		if (key[KEY_UP] && !con->flag3)
		{
			con->flag3=true;
			if (con->tmp_com!=con->log.start)
			{
				if (con->tmp_com->prevcom!=con->log.start)
				{
					con->tmp_com=con->tmp_com->prevcom;
					strcpy(con->textbuf,con->tmp_com->text);
				};
			}
			else
			{
				if (con->log.start!=con->log.comend)
				{
					con->tmp_com=con->log.comend;
					strcpy(con->textbuf,con->tmp_com->text);
				};
			};
		};
		
		if (!key[KEY_UP] && !key[KEY_DOWN] && con->flag3)
		{
			con->flag3=false;
		};
		
		if (key[KEY_DOWN] && !con->flag3)
		{
			con->flag3=true;
			if (con->tmp_com->nextcom!=NULL)
			{
				if (con->tmp_com!=con->log.start)
				{
					con->tmp_com=con->tmp_com->nextcom;
					strcpy(con->textbuf,con->tmp_com->text);
				};
			}
			else
			{
				con->tmp_com=con->log.start;
				strcpy(con->textbuf,"");
			};
			
		};
		
		
		if (keypressed())
		{
			char k;
			k=readkey();
			k=toupper(k);
			if (k==8) con->textbuf[strlen(con->textbuf)-1]=0;
			else if (k==13)
			{
				con->parse(con->textbuf);
				strcpy(con->textbuf,"");
			}
			else sprintf(con->textbuf,"%s%c",con->textbuf,k);
		};
	};
};

void execute_config()
{
	FILE *fbuf;
	char tmp_str[1024];
	sprintf(tmp_str,"%s%s%s",game->mod,"/",con->arg);
	fbuf=fopen(tmp_str,"rt");
  if (fbuf==NULL)
	{
    sprintf(tmp_str,"%s%s%s","default","/",con->arg);
    fbuf=fopen(tmp_str,"rt");
  };
	if (fbuf!=NULL)
	{
		//...parse the file
		//fgets(tmp_str, sizeof(tmp_str), fbuf);
		while (!feof(fbuf))
		{
			if (fgets(tmp_str, sizeof(tmp_str), fbuf)==NULL) break;
			if (tmp_str[strlen(tmp_str)-1]=='\n') tmp_str[strlen(tmp_str)-1]='\0';
			con->parse(ucase(tmp_str));
		};
		fclose(fbuf);
	};
}; 
