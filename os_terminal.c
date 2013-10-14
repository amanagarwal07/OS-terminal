/*Created by Aman Agarwal
    This is a user defines terminal which handles the following commands
    a)cd 
    b)pid of a process
    c)ls
    d)quit
    e)ps:list of all processes
    f)works with commands involving pipes
    g)grep
    h)hist:history of command used
    i)redirection
    j)background processes eg.emacs &
    */





#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<signal.h>
#include<wait.h>
#include<sys/stat.h>
#include<fcntl.h>
void history();
void background(char**);
void execute(char **);
void prompt();
char q[100];
int t=0;
char *p;
char r[100];
char path[100];
typedef struct processes{
	int pid;
	char s[100];
	int flag;
}ptr;
ptr data[100];
void history()
{
	int i;
	data[t].pid=getpid();
	strcpy(data[t].s,"hist");
	data[t++].flag=0;
	for(i=0;i<t;i++)
		printf("%d.%s\n",(i+1),data[i].s);
}
void prompt()
{

	printf("<%s@%s:%s>",p,q,r);
}
void childHandler(int signum)
{
	pid_t pid;
	pid=waitpid(WAIT_ANY,NULL,WNOHANG);
	int i;
	if(pid!=-1)
	{
		for(i=0;i<t;i++)
		{
			if(data[i].pid==pid && data[i].flag==1)
			{
				printf("\n%s %d exited normally\n",data[i].s,data[i].pid);
				fflush(stdout);
				prompt();
			}
			break;
		}
	}
	signal(SIGCHLD,childHandler);
	return;
}
void parse(char *line,char **arg)
{
	while (*line != '\0') {      
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0';    
		*arg++ = line;   
		while (*line != '\0' && *line != ' ' && 
				*line != '\t' && *line != '\n') 
			line++;    		}
	*arg = '\0'; 
}              

void  execute(char **arg)
{
	pid_t  pid;
	int    status;
	pid=fork();
	data[t].pid=pid;
	strcpy(data[t].s,arg[0]);
	data[t++].flag=0;
	if ((pid) < 0) {      			                                          printf("*** ERROR: forking child process failed\n");
		exit(1);
	}
	else if (pid == 0) { 
		if(execvp(*arg, arg) < 0) 
		{//perror();    
			//		signal(SIGCHLD,childHandler);
			printf("command dne\n");
			exit(1);
		}
	}
	else { 
		wait(NULL);
	}
	return;
}
void pipes(char **arg)
{
	int i,pi=0,n;
	pi=10;
	int fd[2*pi];
	for(i=0;i<2*pi;i+=2)
	{
		pipe(fd+i);
	}
	int k=0,j=0,flag=0,l=0;
	char *s[64],*str[64];
	char st[1000];
	for(i=0;arg[i]!=NULL;i++)
	{
		if(flag==0&&(strcmp(arg[i],"|")!=0)&&(strcmp(arg[i],"<")!=0)&&(strcmp(arg[i],">")!=0))
		{
			str[j++]=strdup(arg[i]);
		}
		if((arg[i+1][0]=='>'|| arg[i+1][0]=='|') && flag==0)
		{
			//	printf("%s %s %s\n",str[0],str[1],str[2]);
			flag=1;
			*(str+j)=NULL;
			/*if(strcmp(str[0],"hist")==0)
			  {
			  dup2(fd[1],1);
			  close(fd[0]);
			  for(l=0;l<t;l++)
			  printf("%d.%s\n",(l+1),data[l].s);
			  close(fd[1]);
			  continue;
			  }*/
			if(strcmp(str[0],"hist")==0)
			{
				data[t].pid=getpid();
				strcpy(data[t].s,str[0]);
				data[t++].flag=0;
				FILE *hist;
				hist=fopen("hist","w");
				for(l=0;l<t;l++)
				{
					fprintf(hist,"%s\n",data[l].s);
				}
				fclose(hist);
				//close(fd[0]);
				//hist=fopen("hist","r");
				FILE *hist1;
				hist1=fopen("hist","r");
				n=read(fileno(hist1),st,1000);
				write(fd[1],st,n);
				fclose(hist1);
			}
			else
			{
				pid_t pid=fork();
				data[t].pid=pid;
				strcpy(data[t].s,str[0]);
				data[t++].flag=0;
				if(pid==0)
				{
					dup2(fd[1],1);
					close(fd[0]);
					execvp(*str,str);
				}
				else 
				{
					wait(NULL);

					/*	if(arg[i+1]==NULL)
						{
						n=read(fd[0],st,64);
						write(STDOUT_FILENO,st,n);
						break;
						}*/

				}}
		}
		else if(arg[i][0]=='<')
		{
			str[1]=NULL;
			flag=1;
			strcpy(st,arg[i+1]);
			FILE* check=fopen(arg[i+1],"r");
			if(check)
				fclose(check);
			else
			{
				printf("FILE %s does not exist\n",arg[i+1]);
				break;
			}
			if(fork()==0)
			{
				FILE* f=fopen(st,"r");
				dup2(fileno(f),0);
				close(fd[0]);
				dup2(fd[1],1);
				//	arg[i+1]=NULL;
				//j=0;
				execvp(*str,str);
				_exit(0);
			}
			else
			{
				//	j=0;
				//     close(fd[1]);
				wait(NULL);
				if(arg[i+2]==NULL)
				{
					n=read(fd[0],st,1000);
					write(STDOUT_FILENO,st,n);
					break;
				}
				i++;
			}
		}

		else if(strcmp(arg[i],"|")==0)
		{
			flag=1;
			j=0;
			int m=i+1;;
			while(*(arg+m)!=NULL&&(strcmp(arg[m],"|")!=0)&&(strcmp(arg[m],"<")!=0)&&(strcmp(arg[m],">")!=0))
			{
				str[j++]=strdup(arg[m++]);
				// printf("%s\n",str[j++]);
			}
			*(str+j)=NULL;
			i=m-1;
			//s[0]=strdup(arg[i+1]);
			//s[1]=NULL;
			k=k+2;
			pid_t pid=fork();
			data[t].pid=pid;
			strcpy(data[t].s,arg[i]);
			data[t++].flag=0;
			if(pid==0)
			{
				//	printf("%s\n-----",s[0]);
				dup2(fd[k+1],1);
				close(fd[k]);
				dup2(fd[k-2],0);
				close(fd[k-1]);
				execvp(*str,str);
			}
			else
			{
				close(fd[k-1]);
				wait(NULL);
				if(arg[i+1]==NULL)
				{
					int n=read(fd[k],s,64);
					write(STDOUT_FILENO,s,n);
					//	break;
				}
				//i++;
			}
		}
		else if(arg[i][0]=='>')
		{
			flag=1;
			FILE* fp=fopen(arg[i+1],"w");
			close(fd[k+1]);
			int n=read(fd[k],s,64);			
			write(fileno(fp),s,n);
			fclose(fp);
			i++;
		}}
	return;
}



int redirect(char **arg)
{
	int i;
	for(i=0;arg[i]!='\0';i++)
	{
		if(arg[i][0]=='<' || arg[i][0]=='>' || arg[i][0]=='|')
			return 1;
	} 
	return 0;
}
int main()
{
	signal(SIGCHLD, childHandler);
	signal(SIGINT,SIG_IGN);
	getcwd(path,100);
	strcpy(r,"~");
	p=getenv("USER");
	int n=gethostname(q,100);
	//printf("\n<%s@%s:%s> ",p,q,r);
	int i;
	for(i=0;i<100;i++)
		data[i].flag=0;
	char line[1024];
	char *arg[64];
	char *str[64];
	pid_t pid;
	int status;
	while(1)
	{
		//pid = waitpid(-1, &status, WNOHANG);
		//if (pid > 0)
		//printf("exited\n");            
		prompt();
		gets(line);
		parse(line,arg);
		int len=0;
		while(arg[len]!=NULL)
			len++;
		//printf("%s\n",arg[0]);
		int x=redirect(arg);
		if(x==1)
		{
			/*	char *s1;
				s1=(char*)malloc(sizeof(char)*200);
				strcpy(s1,arg[0]);			
				int z;
				for(z=1;arg[z]!=NULL;z++)
				{
				char *s0=(char*)malloc(sizeof(char)*(strlen(s1)+strlen(arg[z])+1));
				strcpy(s0,s1);
				strcat(s0,arg[z]);
				strcpy(s1,s0);
				}
				data[t].pid=getpid();
				strcpy(data[t].s,s1);
				data[t++].flag=0;*/	
			pipes(arg);
		}
		else if(arg[0]=='\0')
			continue;
		else if((strcmp(arg[0],"cd")==0 && len==1)||(strcmp(arg[0],"cd")==0 && strcmp(arg[1],"~")==0))
		{
			strcpy(r,"~");
			getcwd(path,100);
			chdir(path);

		}
		else if(strcmp(arg[0],"cd")==0 && len!=1)
		{
			int flag=0;
			if(strcmp(arg[1],"..")==0)
			{
				int len=strlen(r);
				int len1=strlen(path);
				int z;
				for(z=len-1;z>=0;z--)
					if(r[z]=='/')
					{
						r[z]='\0';
						flag=1;
						break;
					}
				for(z=len1-1;z>=0;z--)
					if(path[z]=='/')
					{
						path[z]='\0';
						break;
					}

			}
			if(flag==0)
			{
				strcat(r,"/");
				strcat(r,arg[1]);
				strcat(path,"/");
				strcat(path,arg[1]);
			}
			chdir(path);

		}
		else if((strcmp(arg[0],"pid"))==0 && arg[1]=='\0')
		{
			printf("command name: ./a.out process id: %d\n",getpid());
		}
		else if(strcmp(arg[0],"quit")==0)
			exit(0);
		else if(strcmp(arg[0],"hist")==0)
		{
			history();
		}
		else if(arg[0][0]=='!')
		{
			data[t].pid=getpid();
			strcpy(data[t].s,arg[0]);
			data[t++].flag=0;
			char x=arg[0][strlen(arg[0])-1];
			int y=(int)(x)-48;
			//printf("%s\n",data[y-1].s);
			str[0]=strdup(data[y-1].s);
			str[1]=NULL;
			execute(str);
		}
		else if(arg[0][0]=='h')
		{
			data[t].pid=getpid();
			strcpy(data[t].s,arg[0]);
			data[t++].flag=0;
			char x=arg[0][strlen(arg[0])-1];
			int y=(int)(x)-48;
			for(i=t-y;i<t;i++)
				printf("%d.%s\n",data[i].pid,data[i].s);
		}
		else if(arg[1]!=NULL && arg[1][0]=='&')
		{
			arg[1]='\0';
			// background(arg);
			strcpy(data[t].s,arg[0]);
			data[t].flag=1;
			pid_t pid3;
			pid3=fork();
			if(pid3==-1)
				printf("fork failed");
			else if(pid3==0)
			{
				//	setsid();
				execvp(*arg,arg);
			}
			else
			{
				data[t++].pid=pid3;
				printf("command %s pid %d\n",arg[0],pid3);}

		}
		/*else if(strcmp(arg[0],"cd")==0)
		  {
		  chdir(arg[1]);
		  }*/
		else if(strcmp(arg[0],"pid")==0)
		{
			if(strcmp(arg[1],"all")==0)
			{
				printf("List of all processes spawned from this shell:\n");
				for(i=0;i<t;i++)
					printf("command name: %s process id : %d\n",data[i].s,data[i].pid);
			}
			else if(strcmp(arg[1],"current")==0)
			{
				printf("List of currently executing processes spawned from this shell:\n");
				for(i=0;i<t;i++)
					if(data[i].flag==1)
						printf("command name: %s process id: %d\n",data[i].s,data[i].pid);
			}
		}
		else
		{
			//  signal(SIGCHLD,SIG_IGN);
			execute(arg);
		}
	}
	return 0;
}
