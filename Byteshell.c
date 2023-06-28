
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h> 

int CMD_cd(char **arguments);
int CMD_help(char **arguments);
int CMD_exit(char **arguments);
int CMD_history(char **arguments);
int CMD_mapfile(char **arguments);
int CMD_echo(char **arguments);

#define BUFFER_SIZE 4096

char **strings;
char *str;
int total = 0;

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "echo",
    "history",
    "mapfile",
};

int (*builtin_func[])(char **) = {
    &CMD_cd,
    &CMD_help,
    &CMD_exit,
    &CMD_echo,
    &CMD_history,
    &CMD_mapfile,
};

int CMD_num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}
int CMD_cd(char **arguments)
{
  if (arguments[1] == NULL)
  {
    fprintf(stderr, "CMD: expected argument to \"cd\"\n");
  }
  else
  {
    if (chdir(arguments[1]) != 0)
    {
      perror("CMD");
    }
  }
  return 1;
}
int CMD_help(char **arguments)
{
  int i;
  printf("Enter the command  to use\n");
  printf("These are the builtin commands\n");

  for (i = 0; i < CMD_num_builtins(); i++)
  {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int CMD_exit(char **arguments)
{
  for (int i = 0; i < total; i++)
  {
    free(strings[i]);
  }
  free(strings);
  free(str);
  return 0;
}
int CMD_history(char **arguments)
{
  printf("\nHistory:\n\n");
  for (int i = 0; i < total; i++)
  {
    printf("%d %s \n", i + 1, strings[i]);
  }
  printf("\n");

  return 1;
}
#define MAX_LINES 1000
#define MAX_LINE_LENGTH 100
int CMD_mapfile(char **arguments)
{
  char *filename = arguments[3];
  printf("%s\n", filename);
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("Error opening file: %s\n", filename);
    return 1;
  }
  char *lines[MAX_LINES];
  int lineCount = 0;

  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), file) != NULL)
  {
    if (lineCount >= MAX_LINES)
    {
      printf("Maximum number of lines exceed!\n");
      break;
    }
    size_t lineLength = strlen(line);
    if (line[lineLength - 1] == '\n')
    {
      line[lineLength - 1] = '\0'; 
    }
    lines[lineCount] = strdup(line);
    lineCount++;
  }
  fclose(file);
  char *output = malloc(sizeof(char) * MAX_LINE_LENGTH * lineCount);
  if (output == NULL)
  {
    printf("Memory allocation failed!\n");
    return 1;
  }

  output[0] = '\0';

  for (int i = 0; i < lineCount; i++)
  {
    strcat(output, lines[i]);
    strcat(output, "\n");
    free(lines[i]);
  }

  char command[256];
  snprintf(command, sizeof(command), "echo '%s'", output);
  system(command);
  free(output);

  return 1;
}

int CMD_echo(char **arguments)
{
  int i = 0, x = 0;
  bool f = false;
  int length = strlen(str);
  for (int i = 0; i < length; i++)
  {
    if (i > 0 && str[i] == 'e' && str[i - 1] == '-')
    {
      x = i, f = true;
      break;
    }
  }
  if (!f)
  {
    i = 0;
    while (str[i] != 'o')
      i++;
    i++;
    while (str[i] == ' ')
      i++;
    while (str[i] != '\0')
    {
      if (str[i] == '"')
      {
        i++;
        continue;
      }
      printf("%c", str[i]);
      i++;
    }
  }
  else
  {
    i = x + 1;
    while (i < length && str[i] == ' ')
      i++;
    while (str[i] != '\0')
    {
      if (str[i] == '"')
      {
        i++;
        continue;
      }
      if (str[i] == '\\')
      {

        if (str[i + 1] == 'n')
        {
          printf("\n");
        }
        else if (str[i + 1] == 't')
        {
          printf("\t");
        }
        else if (str[i + 1] == 'r')
        {
          printf("\r");
        }
        else if (str[i + 1] == 'a')
        {
          printf("\a");
        }
        else if (str[i + 1] == 'b')
        {
          printf("\b");
        }
        else if (str[i + 1] == 'v')
        {
          printf("\v");
        }
        else if (str[i + 1] == 'f')
        {
          printf("\f");
        }
        else if (str[i + 1] == '\\')
        {
          printf("\\");
        }
        else
        {
          printf("\\%c", str[i + 1]);
        }
        i++;
      }
      else
      {
        putchar(str[i]);
      }
      i++;
    }
  }
  putchar('\n');
  return 1;
}
int CMD_launch(char **arguments)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0)
  {
    if (execvp(arguments[0], arguments) == -1)
    {
      perror("CMD");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    perror("error");
  }
  else
  {
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int CMD_execute(char **arguments)
{
  int i;

  if (arguments[0] == NULL)
  {
    return 1;
  }

  for (i = 0; i < CMD_num_builtins(); i++)
  {
    if (strcmp(arguments[0], builtin_str[i]) == 0)
    {
      return (*builtin_func[i])(arguments);
    }
  }
  return CMD_launch(arguments);
}

char *CMD_read_line(void)
{
#define CMD_RL_buffer_size 1024
  int buffer_size = CMD_RL_buffer_size;
  int pos = 0;
  char *buffer = malloc(sizeof(char) * buffer_size);
  int c;

  if (!buffer)
  {
    fprintf(stderr, "CMD: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    c = getchar();

    if (c == EOF)
    {
      exit(EXIT_SUCCESS);
    }
    else if (c == '\n')
    {
      buffer[pos] = '\0';

      {
        str = realloc(str, (strlen(buffer) + 1) * sizeof(char *));
        strcpy(str, buffer);
      }
      total = total + 1;
      strings = realloc(strings, total * sizeof(char *));
      strings[total - 1] = malloc(pos * sizeof(char));
      strcpy(strings[total - 1], buffer);
      return buffer;
    }
    else
    {
      buffer[pos] = c;
    }
    pos++;

    if (pos >= buffer_size)
    {
      buffer_size += CMD_RL_buffer_size;
      buffer = realloc(buffer, buffer_size);
      if (!buffer)
      {
        fprintf(stderr, "CMD: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define CMD_TOK_buffer_size 64
#define CMD_TOK_DELIM " \t\r\n\a"
char **CMD_split_line(char *line)
{
  int buffer_size = CMD_TOK_buffer_size, pos = 0;
  char **single_strs = malloc(buffer_size * sizeof(char *));
  char *single_str, **single_strs_backup;

  if (!single_strs)
  {
    fprintf(stderr, "CMD: allocation error\n");
    exit(EXIT_FAILURE);
  }

  single_str = strtok(line, CMD_TOK_DELIM);
  while (single_str != NULL)
  {
    single_strs[pos] = single_str;
    pos++;
    if (pos >= buffer_size)
    {
      buffer_size += CMD_TOK_buffer_size;
      single_strs_backup = single_strs;
      single_strs = realloc(single_strs, buffer_size * sizeof(char *));
      if (!single_strs)
      {
        free(single_strs_backup);
        fprintf(stderr, "CMD: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    single_str = strtok(NULL, CMD_TOK_DELIM);
  }
  single_strs[pos] = NULL;
  return single_strs;
}

void CMD_loop(void)
{
  char *line;
  char **arguments;
  int status;

  do
  {
    printf("> ");
    line = CMD_read_line();
    arguments = CMD_split_line(line);
    status = CMD_execute(arguments);

    free(line);
    free(arguments);
  } while (status);
}

int main(int argc, char **argv)
{
  CMD_loop();
  return EXIT_SUCCESS;
}
