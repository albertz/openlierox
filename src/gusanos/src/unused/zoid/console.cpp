#include "console.h"
#include "player.h"
#include "engine.h"
#include "keys.h"
#include "text.h"
#include <fstream>
#include <string>
#include <algorithm>

using std::fstream;

using std::ifstream;

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
	//talking
	int i;
	for (i=0;i<local_players;i++)
		player[local_player[i]]->talking=true;
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

void s_echolist::add_echo(const char* text)
{
  end->next=new struct msg;
	end->next->prev=end;
	end=end->next;
	end->next=NULL;
  strcpy(end->text,text);
  count++;
  time=0;
  if (count>3) remove_echo();
  con->log.create_msg(text);
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
			//it is so delete it and return 0
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

int s_bindtable::create_binding(int bind, const char* cmd_str )
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

void msg_log::create_com(const char* text)
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

void msg_log::create_msg(const char* text)
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
	//extended console
	default_height=create_variable("CON_HEIGHT",100);
	height= *default_height;
	con->scroll_back=0;
	speed=create_variable("CON_SPEED",4);
  echo_delay=create_variable("CON_ECHODELAY",200);
	bindtable.init();
  echolist.init();
};

int* console::create_variable(const char* name,int value)
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

void console::add_cmd(const char* name,void (*func)())
{
	cmd_end->next=new struct s_cmd;
	cmd_end->next->prev=cmd_end;
	cmd_end=cmd_end->next;
	cmd_end->next=NULL;
	cmd_end->func=func;
	strcpy(cmd_end->name,name);
};

struct variable* console::find_variable(const char* name)
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

struct s_cmd* console::find_cmd(const char* name)
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
	int i, chrh;
	chrh=font->chrh+1; // (+1 because there is space between lines)
	
	if (pos>0)
	{
		rectfill(where,0,0,319,pos-2,0);
		sprintf(tmp, "%s%c",textbuf,'&');
		font->draw_string(where,tmp,3,pos-chrh-1,false);
		//extended console
		currmsg=log.end;
		for (i=0;(currmsg!=log.start && i<(scroll_back*game->v_height/chrh - 1));i++) // (-1 because the last line, where you write, is not considered)
			currmsg=currmsg->prev;
		i=0;
		while (currmsg!=log.start && i < pos/chrh )
		{
			font->draw_string(where,currmsg->text,3,pos-((i+2)*chrh)-2,false);
			i++;
			currmsg=currmsg->prev;
		};
		line(where,0,pos-1,319,pos-1,makecol(120,120,120));
	};
  echolist.render(where);
  
  if(flag==2) //chat window
  {
    rectfill(where,10,70,where->w-10,70+chrh,makecol(0,0,0));
    rect(where,9,69,where->w-9,71+chrh,makecol(100,100,100));
    font->draw_string(where,textbuf,11,71,false);
  };
}

void bind()
{
	char k[15],fn_name[512],tmpstr[512];
	if (strlen(con->arg)>0)
	{
		int o;
    unsigned int t;
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

void console::parse(const char* _str)
{
  std::string str=_str;
	if (!str.empty())
	{
		unsigned int t;
		std::string var, val,tmpstr;
		struct variable* tmpvar;
		struct s_cmd *tmp_cmd;

    t=str.find_first_of(' ');
    
    //split it
    var=str;
    val.clear();
    if (t!=str.npos)
    {
    var=str.substr(0,t);
    val=str.substr(t+1);
    };
		tmpvar=con->find_variable(var.c_str());
		tmp_cmd=con->find_cmd(var.c_str());
		if (tmpvar!=NULL)
		{
			if (!val.empty())
			{
				*tmpvar->value=atoi(val.c_str());
				con->log.create_com(str.c_str());
				con->tmp_com=con->log.start;
			}
			else
			{
        char itos[64];
				tmpstr=tmpvar->name;
				tmpstr+=" IS ";
				tmpstr+=sprintf(itos,"%d",*tmpvar->value);
				con->log.create_com(str.c_str());
				con->log.create_msg(tmpstr.c_str());
				con->tmp_com=con->log.start;
			};		
		} else if (tmp_cmd!=NULL)
		{			
			if (!val.empty())
				strcpy(con->arg,val.c_str());
			else
				strcpy(con->arg,"");
			con->log.create_com(str.c_str());
			con->tmp_com=con->log.start;
			tmp_cmd->func();
		}	
		else
		{
			tmpstr="UNKNOWN COMMAND \"";
			tmpstr+=var;
			tmpstr+="\"";
			con->log.create_msg(tmpstr.c_str());
			con->tmp_com=con->log.start;
		};
	};
};

void console::parse_silent(const char* _str)
{
  std::string str=_str;
	if (!str.empty())
	{
		int t;
		std::string var, val,tmpstr;
		struct variable* tmpvar;
		struct s_cmd *tmp_cmd;
    t=str.find_first_of(' ');
    //split it
    var=str.substr(0,t);
    val=str.substr(t+1);
		tmpvar=con->find_variable(var.c_str());
		tmp_cmd=con->find_cmd(var.c_str());
		if (tmpvar!=NULL)
		{
			if (!val.empty())
			{
				*tmpvar->value=atoi(val.c_str());
			};
		} else if (tmp_cmd!=NULL)
		{
			strcpy(con->arg,"");
			if (!val.empty())
			{
				strcpy(con->arg,val.c_str());
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
	//console closed
  if (con->flag==0)
	{
		check_bindings();
  }
	//chat window
	else if (con->flag==2)
  {
    if (keypressed())
		{
			char k;
			k=readkey();
			k=toupper(k);
			if (k==8) //Backspace
				con->textbuf[strlen(con->textbuf)-1]=0;
			else if (k==13) //Enter
			{
				strcpy(con->arg,con->textbuf);
        send_msg();
				strcpy(con->textbuf,"");
        con->flag=0;
				//talking
				int i;
				for (i=0;i<local_players;i++)
					player[local_player[i]]->talking=false;
			}
			else sprintf(con->textbuf,"%s%c",con->textbuf,k);
		};
  }
	//console opened
  else if(con->flag==1)
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
		if (con->flag3 && !key[KEY_UP] && !key[KEY_DOWN])
			con->flag3=false;
		
		//extended console
		if (key[KEY_PGDN] && !con->flag4)
		{
			con->flag4=true;
			if (con->height<*con->default_height)
				con->height= *con->default_height;
			else if (con->height==*con->default_height)
				con->height= game->v_height;
			else if (con->height==game->v_height)
				con->scroll_back++;
		};
		if (key[KEY_PGUP] && !con->flag4)
		{
			con->flag4=true;
			if (scroll_back>0)
				if (key[KEY_RSHIFT])
					con->scroll_back=0;
				else
					con->scroll_back--;
			else if (con->height>*con->default_height)
				con->height= *con->default_height;
			else if (con->height==*con->default_height)
				con->height= 4*con->font->chrh;
		};
		if (con->flag4 && !key[KEY_PGDN] && !key[KEY_PGUP])
			con->flag4=false;		

		if (keypressed())
		{
			char k;
			k=readkey();
			k=toupper(k);
			if (k==8) //Backspace
				con->textbuf[strlen(con->textbuf)-1]=0;
			else if (k==13) //Enter
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
	ifstream fbuf;
	std::string tmp_str;
  tmp_str=game->mod;
  tmp_str+="/";
  tmp_str+=con->arg;
  if (!exists(tmp_str.c_str()))
	{
    tmp_str="default/";
    tmp_str+=con->arg;
  };
  fbuf.open(tmp_str.c_str());
	if (fbuf.is_open() && fbuf.good())
	{
		//...parse the file
		//fgets(tmp_str, sizeof(tmp_str), fbuf);
		while (!fbuf.eof())
		{
      getline(fbuf,tmp_str);
			//if (fgets(tmp_str, sizeof(tmp_str), fbuf)==NULL) break;
			//if (tmp_str[strlen(tmp_str)-1]=='\n') tmp_str[strlen(tmp_str)-1]='\0';
			//char *cptr = ucase(tmp_str);
      std::transform(tmp_str.begin(), tmp_str.end(), tmp_str.begin(), toupper);
			con->parse(tmp_str.c_str());
			//free(cptr);
		};
		
	};
  fbuf.close();
}; 

void console::save_log(const char* logname)
{
  fstream stream;
  struct msg *currmsg;
    
  stream.open(logname, fstream::out);
  if (!stream.is_open() || !stream.good())
  {
    fprintf(stderr, "Cannot open output file.\n");
    return;
  }

  currmsg=log.start;
  while (currmsg->next!=NULL )
  {
    currmsg=currmsg->next;
    stream.write(currmsg->text, strlen(currmsg->text));
    stream.write("\n", strlen("\n"));
  };
  stream.close(); /* close file */
  return;
};
