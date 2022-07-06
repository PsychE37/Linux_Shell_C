// #include"headerfiles.h"
// #include"exeggutor.c"
/* HEADERS */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<pwd.h>
#include<time.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/dir.h>
#include<sys/param.h>
#include<grp.h>
#include<time.h>
/* DEFINE */
#define true 1
#define false 0
char hostname[500],username[500],work_dir[500],home[3000],prevwd[500],lsuse[500],lsuse1[500],hst[20][100];
int histint = 0,bgno;
char curjob[2000];
pid_t id_child = -1, id_sh = -1;
struct bg
{
    int no;
    pid_t ID;  
    char job[500];
};
typedef struct bg bg;
bg bgjobs[100];
void ex(char *command);
void exec(char **a, int index,int bg);
char* readfile(char *filename, int n)
{   
    FILE* file = fopen(filename, "r");
    char line[256];
    char *info = malloc(sizeof(char) *256);
    int i = 0;
    while (fgets(line, sizeof(line), file)) 
    {
        i++;
        if(i == n)
        {
            strcpy(info, line);
        }
    }
    fclose(file);
    return info;
}
void repeat(char *arg)
{
    char *copy = arg,*a[100];
    copy = strtok(NULL, " \t\n");
    int reps = atoi(copy),index = 0,p;
    copy = strtok(NULL, " \t\n");
    while (copy != NULL)
    {
        a[index] = (char *)malloc((strlen(copy)+1) * sizeof(char));
        strcpy(a[index],copy);
        copy = strtok(NULL, " \t\n");
        index++;
    }
    a[index] = NULL;
    for (int i = 0; i < reps; i++)
    {
        pid_t chilpid = fork();
        if (chilpid == 0)
        {
            execvp(a[0],a);
            perror("repeat");
            return;
        }
        else
        {
            waitpid(chilpid,&p,WUNTRACED);
        }
    }
}
void foreg(char **a, int index)
{
    if(index != 2)
    {
        perror("Invalid Input");
        return;
    }   
    int pid, n = atoi(a[1]), status;
	if (n > bgno)
	{
        printf("Only %d jobs present\n",bgno);
        return;
	}
	pid = bgjobs[n-1].ID;
	strcpy(curjob, bgjobs[n-1].job);
	id_child = pid;
    int i = n - 1;
    for(int j = n - 1; j < bgno-1; j++)
    {
        strcpy(bgjobs[i].job, bgjobs[i+1].job);
        bgjobs[i].ID = bgjobs[i+1].ID;
    }
    bgno--;
	signal(SIGTTIN, SIG_IGN); signal(SIGTTOU, SIG_IGN); tcsetpgrp(STDIN_FILENO, id_child); 
	kill(pid, SIGCONT);
	waitpid(pid, &status, WUNTRACED);
	tcsetpgrp(STDIN_FILENO, getpgrp()); signal(SIGTTIN, SIG_DFL); signal(SIGTTOU, SIG_DFL); 
	if(WIFSTOPPED(status))
	{
		printf("%s with PID %d suspended\n", curjob, id_child);
		strcpy(bgjobs[bgno].job, curjob);
        bgjobs[bgno].ID = id_child;
        bgno++;
	}
	return;
}
void backg(char **a, int index)
{
    if(index != 2)
    {
        perror("Invalid Input");
        return;
    }   
    else
    {
        int n = atoi(a[1]);
        if (n > bgno)
        {
            printf("Only %d jobs present\n",bgno);
            return;
        }
        else
        {
            kill(bgjobs[n-1].ID, SIGCONT);   
        }
    }
}
void jobs(char *arg)
{
    char *copy = arg,*status = malloc(sizeof(char) *(40));
    int s = 0,r = 0;
    copy = strtok(NULL," \t\n"); 
    while (copy != NULL)
    {
        if (!strcmp(copy,"-s"))
        {
            s = 1;
        }
        else if (!strcmp(copy,"-r"))
        {
            r = 1;
        }
        copy = strtok(NULL,"-"); 
    }
    for (int i = 0; i < bgno-1; i++)
    {
        for (int j = i+1; j < bgno; j++)
        {
            if (strcmp(bgjobs[i].job,bgjobs[j].job) > 0)
            {
                bg swap;
                swap = bgjobs[i];
                bgjobs[i] = bgjobs[j];
                bgjobs[j] = swap;
            }   
        }   
    }
    for(int i=0; i < bgno; i++)
    {
        sprintf(status,"/proc/%d/status", bgjobs[i].ID);
        FILE *f;
        if(!(f = fopen(status, "r")))
        {
            perror("Error");
        }
        else
        {
            fclose(f);
            char *status_info = readfile(status, 3);
            char *temp = status_info;
            status_info = strtok(status_info, " :\n\t\r");
            status_info = strtok(NULL, " :\n\t\r");
            if (!s && !r)
            {
                if(status_info[0] == 'T')
                {
                    printf("[%d] Stopped %s [%d]\n",bgjobs[i].no+1, bgjobs[i].job, bgjobs[i].ID);          
                }
                else
                {
                    printf("[%d] Running %s [%d]\n",bgjobs[i].no+1, bgjobs[i].job, bgjobs[i].ID);
                }
            }
            else if (s)
            {
                if(status_info[0] == 'T')
                {
                    printf("[%d] Stopped %s [%d]\n",bgjobs[i].no+1, bgjobs[i].job, bgjobs[i].ID);          
                }
            }
            else if (r)
            {
                if(status_info[0] != 'T')
                {
                    printf("[%d] Running %s [%d]\n",bgjobs[i].no+1, bgjobs[i].job, bgjobs[i].ID);
                }
            }
        }
    }
}
void exec(char **a, int index,int bg)
{
    int status;
    pid_t pid = fork(), wid;
    a[index] = NULL;
    if (pid == 0) 
    {   
        setpgid(0, 0);
        if (execvp(a[0], a) < 0) 
        {     
            perror("Command");
            exit(1);
        }
    }
    else 
    {
        if(bg == 0)
        {
            id_child = pid;
            strcpy(curjob, a[0]);               
            for(int i = 1; i < index-1; i++)
            {
                strcat(curjob, " ");
                strcat(curjob, a[i]);
            }
            signal(SIGTTIN, SIG_IGN);
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(STDIN_FILENO, pid);
            wid = waitpid(pid, &status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, getpgrp());
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            if(WIFSTOPPED(status))
            {
                printf("%s with PID %d suspended\n", curjob, pid);
                strcpy(bgjobs[bgno].job, curjob);
                bgjobs[bgno].ID = id_child;
                bgjobs[bgno].no = bgno;
                bgno++;
            }
        }
        else
        {
            strcpy(bgjobs[bgno].job, a[0]);
            for(int i = 1; i < index-1; i++)
            {
                strcat(bgjobs[bgno].job, " ");
                strcat(bgjobs[bgno].job, a[i]);
            }
            bgjobs[bgno].ID = pid;
            bgjobs[bgno].no = bgno;
            bgno++;
            printf("[%d] %d\n", bgno, pid);
        }
    }
    return;
}
// void delay(int t)
// {
//     clock_t start_time = clock();
//     while (clock() < start_time + t)
//     {

//     }
// }
void replay(char *arg)
{
    char *copy = arg,*a[100];
    int interval,period,index = 0,flag = 0,p;
    copy = strtok(NULL, " \t\n");
    while (copy != NULL)
    {
        if (flag)
        {
            a[index] = (char *)malloc((strlen(copy)+1) * sizeof(char));
            strcpy(a[index],copy);
            index++;
        }
        if (!strcmp(copy,"-command"))
        {
            flag = 1;
        }
        if (!strcmp(copy,"-interval"))
        {
            flag = 0;
            a[index-1] = NULL;
            copy = strtok(NULL, " \t\n");
            interval = atoi(copy);
        }
        if (!strcmp(copy,"-period"))
        {
            flag = 0;
            a[index-1] = NULL;
            copy = strtok(NULL, " \t\n");
            period = atoi(copy);
        }
        copy = strtok(NULL, " \t\n");
    }
    // printf("%s\n%s\n%d\n%d\n",a[0],a[1],interval,period);
    for (int i = 1; i <= period; i++)
    {
        sleep(1);
        if (i%interval == 0)
        {
            exec(a,index,0);
        }
    }
}
void ioredirection(char *arg)
{
    int stdout = dup(STDOUT_FILENO),stdin = dup(STDIN_FILENO),status;
    char *output[2], *input[2];
    char * out_file = NULL;
    char * in_file = NULL;
    char * inp = strstr(arg, "<");
    int in = 0;
    if (inp != NULL)
    {
        in = 1;
    }
    
    int out_type = 0;
    char * out = strstr(arg, ">>");  
    if(out != NULL)
    {
        out_type = 2;
    }
    else 
    {
        out = strstr(arg, ">");
        if(out != NULL)
        {
            out_type = 1;
        }
    }
    output[0] = &arg[0];
    if(out_type)
    { 
        output[0] = strtok(arg, ">");
        output[1] = strtok(NULL, ">");
        out_file = strtok(output[1], " \t\n");
    }
    input[0] = output[0];
    if(in)
    { 
        input[0] = strtok(input[0], "<");
        input[1] = strtok(NULL, "<");
    }
    char **args = (char**)malloc(sizeof(char*) * 300);
    int no_args = 0;
    if(in)
    {
        if(input[1] == NULL)
        {
            printf("Specify file name for input\n");
            return;
        }
        in_file = strtok(input[1], " \t\n");
        // printf("%s\n",in_file);
    }
    input[0] = strtok(input[0], " \t\n");
    while (input[0] != NULL)
    {
        args[no_args]  = (char *)malloc(sizeof(char) *strlen(input[0])+10);
        strcpy(args[no_args], input[0]);
        input[0] = strtok(NULL, " \t\n");
        no_args++;
    }
    args[no_args] = NULL;
    if(out_type)
    {
        if(out_file == NULL)
        {
            printf("No output file\n");
            return;
        }
    }
    pid_t pid = fork();
    if(pid == 0)
    {
        if(in)
        {
            int fd_in = open(in_file, O_RDONLY);
            if(fd_in < 0) 
            {
                perror("I/O redirection");
                return;
            }   
            dup2(fd_in, 0);
            close(fd_in);
        }
        if(out_type)
        {
            int fd_out;
            if(out_type == 1)
            {
                fd_out = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            else if(out_type == 2)
            {
                fd_out = open(out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            if(fd_out < 0)
            {
                perror("I/O Redirection");
                return;
            }
            dup2(fd_out, 1);         
            close(fd_out);
        }
        if (execvp(args[0], args) < 0) 
        {     
            perror("Command");
            exit(EXIT_FAILURE);
        }
        dup2(stdin, 0);
        close(stdin);       
        dup2(stdout, 1);
        close(stdout);
    }        
    else 
    {
        while (wait(&status) != pid)
        {
    
        }
    }
}
void pipeline(char *arg)
{
    char *copy = arg,**a = malloc(100 * sizeof(char *));
    int index = 0;
    copy = strtok(copy,"|");
    while (copy != NULL)
    {
        a[index] = copy;
        copy = strtok (NULL, "|");
        index++;
    }
    int type=0,fd[2],file=0;
    pid_t ch;
    for(int j=0; a[j] != NULL; j++)
    {
        pipe(fd);
        ch = fork();
        if(ch == 0)
        {
            if (dup2(file,STDIN_FILENO))
            {
                perror("Duplication");
                exit(1);
            }
            if(a[j+1]!=NULL)
            {
                dup2(fd[1],1);
            }
            close(fd[0]);
            printf("%s\n",a[j]);
            ex(a[j]);
            exit(2);
        }
        else
        {
            wait(NULL);
            close(fd[1]);
            file = fd[0];
        }
    }
}
void history(char *arg)
{
    char *copy = arg;
    copy = strtok(NULL, " \t\n");
    int n = 10;
    if (copy != NULL)
    {
        n = atoi(copy);
    }
    for (int i = 0; i < n; i++)
    {
        printf("%s",(hst[(histint-n+i+20)%20]));
    } 
}
void sig(char *arg)
{
    char *copy = arg;
    copy = strtok(NULL, " \t\n");
    int process = atoi(copy);
    copy = strtok(NULL, " \t\n");
    int signal = atoi(copy);
    printf("%d\n%d\n",process,signal);
    pid_t pid;
    for (int i = 0; i < bgno; i++)
    {
        if (bgjobs[i].no = process)
        {
            pid = bgjobs[i].ID;
            break;
        }
    }
    kill(pid,signal);
}
void pinfo(char *arg)
{
    char *copy = arg;
    copy = strtok(NULL, " \t\n");
    pid_t pid;
    
    if(copy == NULL)
        pid = getpid();

    else
        pid = atoi(copy);
    
    char *exe, *status;
    sprintf(status,"/proc/%d/status", pid);
    sprintf(exe,"/proc/%d/exe", pid);

    FILE *f;

    if(!(f = fopen(status, "r")))
        printf("No such process\n");
    
    else
    {
        fclose(f);
        char *status_info = readfile(status, 3);
        char *memory_info = readfile(status, 18),*token1,*token2;
        token1 = strtok(status_info, ":\t"); 
        token1 = strtok(NULL, ":\t");
        token2 = strtok(memory_info, ":\t\r "); 
        token2 = strtok(NULL, ":\t\r");
        char exe_path[2048];
        char *p; 
        //char * p = malloc(sizeof(char) * 2048);
        printf("pid -- %d\nProcess Status -- %sVirtual Memory -- %sExecutable path -- ", pid, token1, token2); 
        int ret = readlink(exe, exe_path, 1000);
        if(ret == -1)
        {
            printf("Nil\n");
        }
        else 
        {
            exe_path[ret] = '\0'; 
            p = strstr(exe_path, home);
            if(p)
            {
                p += strlen(home);
                printf("~%s\n", p);
            }
            else printf("~%s\n", exe_path);
        }
    } 
}
int checkhidden(char *name)
{
    if (name[0] == '.')
    {
        return 1;
    }
    return 0;
}
void list(char *dir,char *file)
{
    struct stat st;
    char *path = (char *)malloc(sizeof(char) * (strlen(dir)+strlen(file)+100));
    strcpy(path, dir);                                          
    strcat(path, "/");                                
    strcat(path, file);                                     
    if (stat(path, &st))
    {
        perror("ls");
    }
    else
    {
        struct group *grp = getgrgid(st.st_gid);
        struct passwd *pass =  getpwuid(st.st_uid);
        mode_t p = st.st_mode;
        char *perms = malloc(sizeof(char)*11);
        perms[0] = (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) ? 'd' : '-';
        perms[1] = (p & S_IRUSR) ? 'r' : '-';
        perms[2] = (p & S_IWUSR) ? 'w' : '-';
        perms[3] = (p & S_IXUSR) ? 'x' : '-';
        perms[4] = (p & S_IRGRP) ? 'r' : '-';
        perms[5] = (p & S_IWGRP) ? 'w' : '-';
        perms[6] = (p & S_IXGRP) ? 'x' : '-';                                                       
        perms[7] = (p & S_IROTH) ? 'r' : '-';                                                             
        perms[8] = (p & S_IWOTH) ? 'w' : '-';                                                      
        perms[9] = (p & S_IXOTH) ? 'x' : '-';                                                       
        perms[10] = '\0';                                                                           
                                                                                                         
        char *time = (char *)malloc(sizeof(char) * 100); 
        strftime(time, 50, "%b  %d %H:%M", localtime( &st.st_mtime));
        if(perms[3] == 'x')
        {
            printf("%s\t%ld\t%s\t%s\t%ld\t %s\t%s\n", perms, st.st_nlink, pass->pw_name,grp->gr_name, st.st_size, time, file);
        }
        else if(perms[0] == 'd')
        {
            printf("%s\t%ld\t%s\t%s\t%ld\t %s\t%s\n", perms, st.st_nlink, pass->pw_name,grp->gr_name, st.st_size, time, file);
        }
        else
        {
            printf("%s\t%ld\t%s\t%s\t%ld\t %s\t%s\n", perms, st.st_nlink, pass->pw_name,grp->gr_name, st.st_size, time, file);  
        }
    }
}
void ls(char *arg)
{
    char *copy = arg;
    copy = strtok(NULL," \t\n");
    int a = 0, l = 0;
    getcwd(lsuse,sizeof(work_dir));
    while (copy != NULL)
    {
        if (!strcmp(copy,"-la")||!strcmp(copy,"-al"))
        {
            a = 1;
            l = 1;
        }
        else if (!strcmp(copy,"-l"))
        {
            l = 1;
        }
        else if (!strcmp(copy,"-a"))
        {
            a = 1;
        }
        else if (!strcmp(copy,".."))
        {
            chdir("..");
        }
        else if (!strcmp(copy,"~"))
        {
            chdir("home");
        }
        else
        {
            chdir(copy);
        }
        copy = strtok(NULL, " \n\t");
    }
    getcwd(lsuse1,sizeof(work_dir));
    struct dirent **f;
    int n = scandir(lsuse1,&f,NULL,alphasort);
    if (n >= 0)
    {
    switch (l)
    {
    case 1:
        for(int i=0; i<n; i++)
        {
            if(a == 0 && !checkhidden(f[i]->d_name) || a == 1)
            {
                list(lsuse1, f[i]->d_name);
            }
        }
        break;
            
    default:
        for(int i=0; i<n; i++) 
        { 
            if(a == 0 && !checkhidden(f[i]->d_name) || a == 1)
            {
                printf("%s\t",f[i]->d_name);  
            }
        }
        printf("\n");
        break;
    }
    }
    else
    {
        perror("ls");
    }
    chdir(lsuse);
}
void cd(char *arg)
{
    // printf("cd function\n");
    char *copy = arg,*h = getenv("HOME");
    copy = strtok(NULL, " \t\n");
    if (copy == NULL || !strcmp(copy,"~") )
    {
        getcwd(prevwd,sizeof(work_dir));
        chdir(home);
    }
    else if (!strcmp(copy,".."))
    {
        getcwd(prevwd,sizeof(work_dir));
        chdir("..");
    }
    else if (!strcmp(copy,"."))
    {
        return;
    }
    else if (!strcmp(copy,"-"))
    {
        if (strlen(prevwd) == 0)
        {
            printf("OLDPWD: not set\n");
            return;
        }
        chdir(prevwd);
        printf("%s\n",prevwd);
    }
    else
    {
        getcwd(prevwd,sizeof(work_dir));
        if (chdir(copy))
        {
            perror("cd");
        }   
    }
}
void pwd()
{
    // printf("pwd function\n");
    char *Dir = (char *)malloc(1000 * sizeof(char));
    if (getcwd(Dir,1000) != NULL)
    {
        printf("%s\n",Dir);
    }
    else
    {
        perror("pwd");
        exit(1);
    }

}
void echo(char *arg)
{
    char *copy = arg;
    copy = strtok(NULL, " \t\n");
    while (copy != NULL)
    {
        printf("%s",copy);
        copy = strtok(NULL, " \t\n");
    }
    printf("\n");
}
int redir(char *arg)
{
    if ((strstr(arg,"<") != NULL) || (strstr(arg,">") != NULL))
    {
        return 1;
    }
    return 0;
}
int pip(char *arg)
{
    if (strstr(arg,"|") != NULL)
    {
        return 1;
    }
    return 0;
}
void ex(char *command)
{
    char *original = (char *)malloc(500 * sizeof(char));
    strcpy(original, command);
    original = strtok(original, " \n\t\r");
    char **a = (char**)malloc(sizeof(char*) * 100);
    int index = 0;
    while (original != NULL)
    {
        a[index] = original;
        original = strtok(NULL, " \t\n");
        index++;
    }
    if (pip(command))
    {   
        pipeline(command);
        return;
    }
    if (redir(command))
    {
        ioredirection(command);
        return;
    }
    command = strtok(command, " \t\n");
    if (!strcmp(a[index-1],"&"))
    {
        a[index-1] = NULL;
        exec(a,index,1);
    }
    else if (!strcmp(a[0],"fg"))
    {
        foreg(a,index);
    }
    else if (!strcmp(a[0],"bg"))
    {
        backg(a,index);
    }
    else if(!strcmp(command, "cd"))
    {
        cd(command);
    }
    else if(!strcmp(command, "pwd"))
    {
        pwd();
    }
    else if(!strcmp(command, "echo"))
    {
        echo(command);
    }
    else if(!strcmp(command, "ls"))
    {
        ls(command); 
    }
    else if (!strcmp(command, "repeat"))
    {
        repeat(command);
    }
    else if(!strcmp(command, "history"))
    {
        history(command);
    }
    else if (!strcmp(command, "pinfo"))
    {
        pinfo(command);
    }
    else if (!strcmp(command, "jobs"))
    {
        jobs(command);
    }
    else if(!strcmp(command,"sig"))
    {
        sig(command);
    }
    else if (!strcmp(command, "replay"))
    {
        replay(command);
    }
    else
    {
        exec(a,index,0);
    }
}
int main()
{
    printf("\t\t\t\t\033[0;35m<< SHELL >>\033[0m\n\t\t\t\t\t\t-By Sai Pranav\n");
    gethostname(hostname,sizeof(hostname));
    getlogin_r(username,sizeof(username));
    pid_t pid;
    pid = getpid();
    char *q = malloc(sizeof(char) *(50));
    sprintf(q,"/proc/%d/exe", pid);
    int end = readlink(q, home, 1000);
    home[end] = '\0';
    for(int i = end; i >= 0; --i)
    {
        if(home[i] == '/')
        {
            home[i] = '\0';
            break;
        }
    }
    while (1)
    {
        char *Dir,*in = NULL,**comms = malloc(500 * sizeof(char *)),*y;
        ssize_t s = 0;
        if (getcwd(work_dir,sizeof(work_dir)) != NULL)
        {
            char *x;
            x = strstr(work_dir, home);
            if (!x)
            {
                Dir = work_dir;
                printf("\033[0m<\033[0;36m%s@%s\033[0m:\033[0;32m%s\033[0m>\033[0;33m",username,hostname,Dir);
            }
            else
            {
                Dir = x + strlen(home);
                printf("\033[0m<\033[0;36m%s@%s\033[0m:\033[0;32m~%s\033[0m>\033[0;33m",username,hostname,Dir);
            }
        }
        else
        {
            perror("Program");
            exit(1);
        }
        if (getline(&in,&s,stdin) < 0)
        {
            break;
        }
        if(!strcmp(in, "exit\n"))
        {
            break;
        }
        if(!strcmp(in, "\n"))
        {
            continue;
        }
        if (strcmp(hst[(histint-1 +20)%20],in))
        {
            strcpy(hst[histint],in);
            histint = (histint+1)%20;
        }
        // printf("ok\n");
        y = strtok(in, ";");
        int i;
        for (i = 0; y != NULL; i++)
        {
            comms[i] = y;
            y = strtok(NULL, ";");
        }
        // printf("ok\n");
        for (int j = 0; j < i; j++)
        {
            ex(comms[j]);
        }
        // printf("ok\n");

    }
        
}