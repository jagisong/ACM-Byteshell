//Implementation of basic shell in C programming.
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h> // for inbuild chdir(),fork(),exec(),pid_t functions
#include <stdlib.h> //for malloc(),realloc(),free(),exit(),execvp(),EXIT_SUCCESS,EXIT_FAILURE
#include <stdio.h>  // for FILE 
#include <string.h>
#include <stdbool.h> //boolean implementation

//here are functions declaration that i am going to implement 

int my_help(char **prms);
int my_cd(char **prms);
int my_mapfile(char **prms);
int my_echo(char **prms);
int my_history(char **prms);
int my_exit(char **prms);


//Several terms variable, macros and all that i have accessed globally

#define BUFFER_SIZE 4096 
char **strings;
char *echo_string;
int total = 0;


///List of builtin commands that i have implementd , followed by their corresponding functions.

char *builtin_str[] = {
  "help",
  "cd",
  "mapfile",
  "echo",
  "history",
  "exit",
};


//The provided code declares an array of function pointers named builtin_func. 
//Each element of this array is expected to be a function that takes a char** argument and returns an int.
// However, the array is initialized to an empty set of braces {}

int (*builtin_func[]) (char **) = {
  &my_help,
  &my_cd,
  &my_mapfile,
  &my_echo,
  &my_history,
  &my_exit
};


int my_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}
//respective builtin funcations body(implementations starts)

//--help 
int my_help(char **prms)
{
  int i;
  printf("Type command  name and arguments, and hit enter.\n");
  printf("The following built commands are availabe:\n");
  for (i = 0; i < my_num_builtins(); i++) 
  {
    printf("  %s\n", builtin_str[i]);
  }
  printf("Use the 'man' followed by  command e.g. 'man ls' for information on other commands.\n");
  return 1;
}

//cd -- change directory

int my_cd(char **prms)
{
  if (prms[1] == NULL) 
  {
    fprintf(stderr, "my: expected argument to \"cd\"\n");
  } 
  else 
  {
    if(chdir(prms[1]) != 0) 
    {
      perror("my");
    }
  }
  return 1;
}

//mapfile --It reads whole_strs from the standard input(file) into an array variable , 
//in our case it reads files as well as display on terminal 

#define MAX_whole_strS 1000
#define MAX_whole_str_LENGTH 100

int my_mapfile(char **prms)
{
    char* filename = prms[1];
    // printf("%s\n",filename);
    //FILE macro to declare the file pointer variable. 
    FILE* file = fopen(filename, "r"); 
    /*to open  a file in C,'fopen()' function is used with the filename 
    along with the required access modes.*/
    if (file == NULL) 
    {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    char* whole_strs[MAX_whole_strS];
    int whole_strCount = 0;
    char whole_str[MAX_whole_str_LENGTH];

    while (fgets(whole_str, sizeof(whole_str), file) != NULL)
    {
        if (whole_strCount >= MAX_whole_strS) {
            printf("Maximum number of whole_strs exceeded!\n");
            break;
        }
        size_t whole_strLength = strlen(whole_str);
        if (whole_str[whole_strLength - 1] == '\n') 
        {
            whole_str[whole_strLength - 1] = '\0'; // Remove the newwhole_str character
        }
        whole_strs[whole_strCount] = strdup(whole_str);//to create duplicate
        whole_strCount++;
    }

    fclose(file);//to close respective file.

    printf("%d\n",whole_strCount);

    for (int i = 0; i < whole_strCount; i++) 
    {
        printf("%s\n", whole_strs[i]);
        free(whole_strs[i]);
    }
    return 1;
}

// echo -->to display whole_str of text that has been passed as an argument

int my_echo(char **prms)
{
    int i = 0, x=0;
    bool f = false;
    int length=strlen(echo_string);
    for(int i=0;i<length;i++)
    {
        if(i>0 && echo_string[i]=='e' && echo_string[i-1]=='-')
        {
            x=i,f=true;break;
        }
    }
    if(!f)
    {
        i=0;
        while(echo_string[i]!='o') i++;
        i++;
        while(echo_string[i]==' ') i++;
        while(echo_string[i]!='\0')
        {
            if(echo_string[i]=='"')
            {
                i++;continue;
            }
            printf("%c",echo_string[i]);
            i++;
        }
        // printf("%s\n","without e executing");
        // printf("\n");
    }
    else
    {
        // i=0;
        i=x+1;
        // while(echo_string[i]!='o') i++;
        // i++;
        while(i<length && echo_string[i]==' ') i++;
        while (echo_string[i] != '\0') 
        {
            if(echo_string[i]=='"')
            {
                i++;continue;
            }
            if (echo_string[i] == '\\') 
            {
                switch (echo_string[i + 1]) 
                {
                    case 'n': printf("\n"); break;
                    case 't': printf("\t"); break;
                    case 'r': printf("\r"); break;
                    case 'b': printf("\b"); break;
                    case 'a': printf("\a"); break;
                    case 'v': printf("\v"); break;
                    case 'f': printf("\f"); break;
                    case '\\': printf("\\"); break;
                    // case '0': printf("\0"); break;
                    default: printf("\\%c", echo_string[i + 1]); break;
                }
                i++;
            } 
            else
            {
                putchar(echo_string[i]);
            }
            i++;
        }
    }
    putchar('\n');
    // printf("%s","our echo working");
    return 1;
}

//history -> to display whatever commands we have executed including history

int my_history(char **prms)
{
    printf("\nResulting History of commands:\n\n");
    for (int i = 0; i < total; i++)
    {
        // printf("strings[%d] = %s\n", i, strings[i]);
        printf("%d %s \n",i+1,strings[i]);
    }
    printf("\n");
    return 1;
}
//exit --> to terminate our program

int my_exit(char **prms)
{
    for (int i = 0; i < total; i++)
    {
        free(strings[i]);
    }
    free(strings);
    free(echo_string);
    return 0;
}

//Basic loop of shell - 1.read a whole_str 2.parse a whole_str(split to command and arguments) 3. execute 

//launch() function

int my_launch(char **prms)
{
  pid_t pid;//data type representing a process ID

  int status;

  pid = fork();//fork()-->system call to create new process

  if (pid == 0) 
  {
    // Child process
    if (execvp(prms[0], prms) == -1) 
    {
        //perror() function displays the description of the error 
        //that corresponds to the error code stored in the system variable errno.
        perror("Eroor:");
    }
    exit(EXIT_FAILURE);
  } 
  else if (pid < 0) 
  {
    // Error forking
    perror("Error in creating process");
  } 
  else 
  {
    // Parent process
    do
    {
        waitpid(pid, &status, WUNTRACED);
        //Wait for a specific child process to end
        //waitpid() suspends the calling process until the system gets status information on the child
    } 
    while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

// execute() function

int my_execute(char **prms)
{
    int i;

    if (prms[0] == NULL) {
        // An empty command was entered.
        printf("Please enter valid command");
        return 1;
    }
    for (i = 0; i < my_num_builtins(); i++){
        if (strcmp(prms[0], builtin_str[i]) == 0){
            //zero means it is found
            return (*builtin_func[i])(prms);
        }
    }
    return my_launch(prms);
}

// parsing -split whole_str to executalbe commands and arguments

#define my_TOK_BUFSIZE 64
char **my_split_whole_str(char *whole_str)
{
  int bufsize = my_TOK_BUFSIZE, idx = 0;
  char *seperateds = malloc(bufsize * sizeof(char));
  char *seperated, **seperateds_backup;
  const char delim[2]=" ";//delimiter

  if (!seperateds) {
    fprintf(stderr, "my: allocation error\n");
    exit(EXIT_FAILURE);
  }

  seperated = strtok(whole_str,delim);
  while (seperated != NULL)
  {
    seperateds[idx] = seperated;
    idx++;
    if (idx >= bufsize) {
      bufsize += my_TOK_BUFSIZE;
      seperateds_backup = seperateds;
      seperateds = realloc(seperateds, bufsize * sizeof(char*));//reallocation of memory
      if (!seperateds) {
		    free(seperateds_backup);
        fprintf(stderr, "my: Allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    seperated = strtok(NULL, delim);
  }
  seperateds[idx] = NULL;
  return seperateds;
}

// read a whole_str() 

char *my_read_whole_str(void)
{
    #define my_RL_BUFSIZE 2048
    int bufsize = my_RL_BUFSIZE;
    int idx = 0;
    char *buffer = malloc(sizeof(char) * bufsize);// will contain input
    int c;
    if (!buffer){
    fprintf(stderr, "my: allocation error\n");
    exit(EXIT_FAILURE);
    }

    while(true) 
    {
        // Read a character
        c = getchar();
        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } 
        else if (c == '\n') 
        {
            buffer[idx] = '\0';
            //implementation for history -start
            {
                total=total+1;
                strings = realloc(strings,total * sizeof(char *));
                strings[total-1] = malloc(idx * sizeof(char));
                strcpy(strings[total-1], buffer);
            }
            //implemention for history -ends
            //implementation of echo command-start
            {
                echo_string=realloc(echo_string,(strlen(buffer)+1)sizeof(char));
                strcpy(echo_string,buffer);
            }
            //implementation of echo command-ends

            return buffer;
        } 
        else {
            buffer[idx] = c;
        }
        

        idx++;
        // If we have exceeded the buffer, we have to reallocate it.
        if (idx >= bufsize) {
            bufsize += my_RL_BUFSIZE ;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "my: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

//basic loop

void my_loop(void)
{
    char *whole_str;//will store input
    char **prms;//will contain parsed data 
    int status;

    do{
        // enter commands
        printf(">");
        whole_str = my_read_whole_str();
        prms = my_split_whole_str(whole_str);
        status = my_execute(prms);

        free(whole_str);
        free(prms);
    } while (status);
}

int main(int argc, char **argv)
{
  my_loop();
  return EXIT_SUCCESS;
}