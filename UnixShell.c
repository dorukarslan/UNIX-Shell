
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUILTINSIZE 3
#define ERROR write(STDERR_FILENO, "An error has occurred \n", strlen("An error has occurred \n"));
int errorFlag = 0;
int parralelFlag = 0;

char pathArr[30][240];
char cwd[128];

int init(int argSize, char **argv);
int checkCommand(char **args);

int main(int argc, char **argv)
{
  int argCheck = argc;

  if(argc > 2){
    ERROR;
    exit(1);
  }

  int result = init(argCheck,argv);

  return EXIT_SUCCESS;
}
typedef struct
{
  int (*funcPtr)(char **Userarg);
  char *name;
} builtInStruct;

builtInStruct builtInArr[BUILTINSIZE];


char *readInput(void)
{
  size_t bufsize = 0; 
  char *line = NULL;

  if (getline(&line, &bufsize, stdin) == -1)
  {
    if (feof(stdin))
    {
      exit(EXIT_SUCCESS);
    }
    else
    {
      ERROR;
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

char * checkInput(char * str){

  char *newStr = (char *)malloc(strlen(str)*2);


  int tempCounter = 0;
  for(int i= 0; i<strlen(str); i++){
    
    
    if(i == 0 ){
      newStr[tempCounter]= str[i];
      tempCounter++;
      continue;
    }

    if(str[i] != '&' && str[i] != '>'){

      newStr[tempCounter] = str[i];
      tempCounter ++;
    }
    else{
      if (str[i-1] != ' ')
      {
        
        newStr[tempCounter] = ' ';
        tempCounter ++;
       
      }
      newStr[tempCounter] = str[i];
      tempCounter ++;
      if(str[i+1] != ' '){
        newStr[tempCounter] = ' ';
        tempCounter ++;
      }
      
    }
  }
 return newStr;




}





void redirectOut(char *outputFile)
{
  if (outputFile == NULL)
  {
    ERROR;
    errorFlag = 1;
  }
  else
  {
    int fd = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    dup2(fd, 1);
    close(fd);
  }
}
char **splitCommand(char *str)
{
   
    
  int stringSize = 120;
  char **CommandArr = (char **)malloc(stringSize * sizeof(char *));
  int counter = 0;
  char *token;

   if (str[0] == '&')
    {
       
      CommandArr[0] = NULL;
        return CommandArr;
    }

  token = strtok(str, " \n\a\r\t");

  while (token != NULL)
  {
    
    
    


    if (*token == '>')
    {
      if (counter == 0)
      {
        ERROR;
        break; // Means no command given
      }
      token = strtok(NULL, " \n\a\r\t");
      char *outputFile = token;
      
      token = strtok(NULL, " \n\a\r\t");
      
      if (token != NULL ) // Means more than one outputfile given
      {
        if(*token == '&'){
            redirectOut(outputFile);

            parralelFlag = 1;
            CommandArr[counter] = NULL;
            checkCommand(CommandArr);
            counter = 0;
            memset(CommandArr, 0, stringSize * sizeof(char *));
        }
        else {
          errorFlag = 1;
          ERROR;
          break;
        }
       
      }
      else
      {
        redirectOut(outputFile);
      }
    }

    else if (*token == '&')
    {

      if (counter == 0)
      {

        ERROR;
        errorFlag = 1;
        
      }
      else
      {
        parralelFlag = 1;
        CommandArr[counter] = NULL;
        checkCommand(CommandArr);
        counter = 0;
        memset(CommandArr, 0, stringSize * sizeof(char *));
      }
    }

    else
    {
     
      CommandArr[counter] = token;
      counter++;
    }

    if (counter >= stringSize) // If command exceeds the array
    {
      stringSize += 256;
      CommandArr = (char **)realloc(CommandArr, stringSize * sizeof(char *));
    }

    token = strtok(NULL, " \n\a\r\t");
  }
  CommandArr[counter] = NULL;

  return CommandArr;
}

int createProcess(char **args)
{
  pid_t pid, wpid;
  int status;
  if (!errorFlag && args[0] != NULL)
  {

    pid = fork();
    if (pid == 0)
    {
      // Child process
      int fd;

      char *pathCommand;

      for (int i = 0; i < sizeof pathArr / sizeof pathArr[0]; i++)
      {
        if (pathArr[i][0] != '\0')
        {

          char *command = pathArr[i];

          char buf[128];

          if (!strstr(pathArr[i], cwd))

          {
            if (strcmp(pathArr[i], "/bin") != 0)
            {

              strcpy(buf, "./");
              strcat(buf, pathArr[i]);
            }
            else
            {
              strcpy(buf, pathArr[i]);
            }
          }
          strcat(buf, "/");
          strcat(buf, args[0]);

          //     printf("current %s \n", buf);
          if (access(buf, R_OK) != -1) // Means founded path is accessible and can be used as an argument.
          {

            //    printf("inside %s \n", buf);
            pathCommand = buf;

            if (strstr(args[0], ".sh"))
            {

              pathCommand = "/bin/bash";

              char **newPath = (char **)malloc(32 * sizeof(char *));
              newPath[0] = args[0];
              newPath[1] = buf;
              args = newPath;
              // printf("Control  - > %s  - %s   - %s \n ", args[0], args[1], pathCommand);
            }
            break;
          }
        }
      }
      if (execv(pathCommand, args) == -1)
      {

        ERROR;
      }
      exit(EXIT_FAILURE);
    }

    else if (pid < 0)
    {
      // Error forking
      perror("Error occered");
    }
    else
    {
      // Parent process
      do
      {

        if (!parralelFlag)
        {
          wpid = waitpid(pid, &status, WUNTRACED);
        }
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    redirectOut("/dev/tty");

    return 1;
  }
}

/*
Built in functions must be considered seperatly. Consider the case when user wants to perform cd command.
If we run it in the child process, child process working diretory will be changed and eventually it will be terminated.
However, that is not our purpose. We need to perform built in commands in shell itself (Parent process).
Hence, we consider the built in commands above seperatly via checkCommand function.
Otherwise, createProcess method will be called to perform user's intended command in a new process.
*/

// To identify wheter the given User args are valid, if so,to check is it shell (built in) command or not
int checkCommand(char **args)
{

  if (args[0] == NULL)
  {
    return 1;
  }
  for (int i = 0; i < BUILTINSIZE; i++)
  {
    
    if (strcmp(args[0], builtInArr[i].name) == 0) // To compare the names of the command
    {
      return (builtInArr[i].funcPtr)(args); // If so, call the function pointer
    }
  }

  return createProcess(args); // If not execute the User command in a nex process
}

// ------------------------- Built in Commands -------------------------
int cdCommand(char **Userarg)
{
  if (Userarg[1] == NULL) // arg[0] - Command arg[1] -> path, means that  Means no path given
  {
    ERROR;
  }
  else
  {

    if (chdir(Userarg[1]) != 0) // To check whetere the cd operation is successfull
    {
      ERROR;

    }
  }
  return 1;
}

int pathCommand(char **userArg)
{
  char tempArr[30][240] = {0};
  int counter = 1; // To skip first command
  while (userArg[counter] != NULL)
  {
    strcpy(tempArr[counter - 1], userArg[counter]);
    counter++;
  }

  memcpy(pathArr, tempArr, sizeof pathArr);

  return 1;
}

int exitCommand(char **Userarg)
{
  char *argTocComp = (char *)"";

  int counter = 0;
  while (Userarg[counter] != NULL)
  {

    counter++;
  }

  if (counter > 1)
  {
    ERROR;
    return -1;
  }

  return 0;
}

// ------------------------- Built in Commands -------------------------
int init(int argSize, char** argv)
{
  if (getcwd(cwd, sizeof(cwd)) == NULL)
  { 
    ERROR;
  }
 
  strcpy(pathArr[0], "/bin");
  char *line;
  char **args;
  int status;
  builtInStruct cd;
  builtInStruct exit;
  builtInStruct path;
  cd.name = (char *)"cd";
  cd.funcPtr = &cdCommand;
  exit.name = (char *)"exit";
  exit.funcPtr = &exitCommand;
  path.name = (char *)"path";
  path.funcPtr = &pathCommand;
  builtInArr[0] = cd;
  builtInArr[1] = exit;
  builtInArr[2] = path;
 


  if (argSize == 2)
  {
    FILE * file = fopen(argv[1], "r");
    char * parseFile = NULL;
    size_t fileSize = 0;
    if(file == NULL){
      ERROR;
      return 1;
    }
    else {
    if (NULL != file) {
    fseek (file, 0, SEEK_END);
    int size = ftell(file);
    if (0 == size) {
        ERROR;
        return 1;
    }
    fseek(file, 0, SEEK_SET);

}
    }

    while (getline(&parseFile,&fileSize,file) != -1)
    {
      errorFlag = 0;
      parralelFlag = 0;
      fflush(stdout);

     parseFile = checkInput(parseFile);
      args = splitCommand(parseFile);
      parralelFlag = 0;


      if (args[0] != NULL)
      {
          status = checkCommand(args);
      }
      

      if (status != 1)
      {
         break;
      }
        parralelFlag = 0;

      
    }
    
  }
  else {
  do
  {
    parralelFlag = 0;

    printf("UnixShell>");

    errorFlag = 0;
    parralelFlag = 0;
    fflush(stdout);
    line = readInput();
    line = checkInput(line);
    args = splitCommand(line);
    parralelFlag = 0;
    if (args[0] != NULL)
    {
        status = checkCommand(args);
    }
    

    free(line);
    free(args);

  } while (status);
  }
  return 0;
  
}