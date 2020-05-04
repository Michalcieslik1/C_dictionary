/*
  Implements a minimal shell.  The shell simply finds executables by
  searching the directories in the PATH environment variable.
  Specified executable are run in a child process.

  AUTHOR: PUT YOUR NAME HERE
*/

#include "bshell.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

int parsePath(char *dirs[]);
char *lookupPath(char *fname, char **dir,int num);
int parseCmd(char *cmdLine, Command *cmd);

/*
  Read PATH environment var and create an array of dirs specified by PATH.

  PRE: dirs allocated to hold MAX_ARGS pointers.
  POST: dirs contains null-terminated list of directories.
  RETURN: number of directories.

  NOTE: Caller must free dirs[0] when no longer needed.

*/
int parsePath(char *dirs[]) {
  int i, numDirs;
  char *pathEnv;
  char *thePath;
  //char *nextcharptr; /* point to next char in thePath */

  for (i = 0; i < MAX_PATHS; i++) dirs[i] = NULL;
  pathEnv = (char *) getenv("PATH");
  
  thePath = (char *)malloc(strlen(pathEnv));

  if (pathEnv == NULL) return 0; /* No path var. That's ok.*/

  /* for safety copy from pathEnv into thePath */
  strcpy(thePath,pathEnv);


//#ifdef DEBUG
  //printf("Path: %s\n",thePath);
//#endif

  /* Now parse thePath */
  /* 
     Find all substrings delimited by DELIM.  Make a dir element
     point to each substring.
     TODO: approx a dozen lines.
  */
  char* token;
  token = strtok(thePath, DELIM);
  numDirs = 0;
  
  
  while (token != NULL && MAX_PATHS){ 
    //printf("%s%i%s%s\n", "  dir[",numDirs,"] before: ", dirs[numDirs]);
    dirs[numDirs] = token;
    token = strtok(NULL, DELIM); 
    //printf("%s%i%s%s\n", "  dir[",numDirs,"] after: ", dirs[numDirs]);
    numDirs++; 
  }
  
  
  //printf("%s%i%s%s\n", "  dir[",0,"] outside the loop: ", dirs[0]);

  /* Print all dirs */
  
  
//#ifdef DEBUG
  //for (i = 0; i < numDirs; i++) {
//  printf("%s\n",dirs[i]);
  //}
//#endif
    
  return numDirs;
}


/*
  Search directories in dir to see if fname appears there.  This
  procedure is correct!

  PRE dir is valid array of directories
  PARAMS
   fname: file name
   dir: array of directories
   num: number of directories.  Must be >= 0.

  RETURNS full path to file name if found.  Otherwise, return NULL.

  NOTE: Caller must free returned pointer.
*/

char *lookupPath(char *fname, char **dir,int num) {
  char *fullName; // resultant name
  int maxlen; // max length copied or concatenated.
  int i;

  fullName = (char *) malloc(MAX_PATH_LEN);
  /* Check whether filename is an absolute path.*/
  if (fname[0] == '/') {
    strncpy(fullName,fname,MAX_PATH_LEN-1);
    if (access(fullName, F_OK) == 0) {
      return fullName;
    }
  }

  /* Look in directories of PATH.  Use access() to find file there. */
  else {
    for (i = 0; i < num; i++) {
      // create fullName
      maxlen = MAX_PATH_LEN - 1;
      strncpy(fullName,dir[i],maxlen);
      maxlen -= strlen(dir[i]);
      strncat(fullName,"/",maxlen);
      maxlen -= 1;
      strncat(fullName,fname,maxlen);
      // OK, file found; return its full name.
      if (access(fullName, F_OK) == 0) {
	return fullName;
      }
    }
  }
  fprintf(stderr,"%s: command not found\n",fname);
  free(fullName);
  return NULL;
}

/*
  Parse command line and fill the cmd structure.

  PRE 
   cmdLine contains valid string to parse.
   cmd points to valid struct.
  PST 
   cmd filled, null terminated.
  RETURNS arg count

  Note: caller must free cmd->argv[0..argc]

*/
int parseCmd(char *cmdLine, Command *cmd) {
  int argc = 0; // arg count
  char* token;
  int i = 0;

  token = strtok(cmdLine, SEP);
  while (token != NULL && argc < MAX_ARGS){    
    cmd->argv[argc] = strdup(token);
    token = strtok (NULL, SEP);
    argc++;
  }

  cmd->argv[argc] = NULL;  
  cmd->argc = argc;

#ifdef DEBUG
  //printf("CMDS (%d): ", cmd->argc);
  //for (i = 0; i < argc; i++)
    //printf("CMDS: %s",cmd->argv[i]);
  //printf("\n");
#endif
  
  return argc;
}


/*
  Runs simple shell.
*/
int main(int argc, char *argv[]) {

  //------------------------VARIABLES-------------------------
  char *dirs[MAX_PATHS]; // list of dirs in environment
  char *path;
  char currentD[512];
  int numPaths;
  char cmdline[LINE_LEN];
  Command cmd;
  int numCmds;
  int status;
  int shouldParentWait;
  numPaths = parsePath(dirs);
  int numBackground = 0;
  pid_t backgroundProcessID[10];
  char *backgroundProcessName[10];
  
//---------------------------LOOP-------------------------------
  while(1){
    
    for(int i = 0; i < 10; i++)
    {
      if (backgroundProcessID[i])
      {
        if (waitpid(backgroundProcessID[i],&status,WNOHANG) == backgroundProcessID[i]){
          for (int j = i; j < numBackground - 1; j++){
            backgroundProcessID[j] = backgroundProcessID[j+1];
            backgroundProcessName[j] = backgroundProcessName[j+1];
          }
          numBackground--;
        }
      }
    }
    
    
    for(int i = 0; i< argc;i++){ printf("%s:", getcwd(currentD,512));}
    
    //if the input provided by the user is null or just \n, handle those cases
    if(fgets(cmdline,LINE_LEN,stdin) != NULL || cmdline[0] != '\n' )
    {
      numCmds = parseCmd(cmdline, &cmd);
      
//----------------------BUILT IN COMMANDS--------------------------
      
      //if input is exit, exit the loop
      if(strcmp(cmd.argv[0], "exit") == 0 || strcmp(cmd.argv[0], "e") == 0 ) {
        break;
      }
      
      //if input is jobs, show the jobs
       else if(strcmp(cmd.argv[0], "jobs") == 0 || strcmp(cmd.argv[0], "j")== 0 ) {
         //checking the status of the jobs
         for(int i = 0; i < 10; i++)
        {
          if (backgroundProcessID[i])
          {
            if (waitpid(backgroundProcessID[i],&status,WNOHANG) == backgroundProcessID[i]){
              for (int j = i; j < numBackground - 1; j++){
                backgroundProcessID[j] = backgroundProcessID[j+1];
                backgroundProcessName[j] = backgroundProcessName[j+1];
              }
            numBackground--;
            }else if(waitpid(backgroundProcessID[i],&status,WNOHANG)== -1){
              //printf("ERROR: process %i with the id %s failed the waitpid check",(int) backgroundProcessID[i], backgroundProcessName[i]);
            }
          }
        }
        
        //printing out the active jobs
         printf("Processes in the background:\n");
         for(int i = 0; i < numBackground; i++){
           printf("ID: %i, Name: %s\n", (int)backgroundProcessID[i], backgroundProcessName[i]);
        }
      } else if(strcmp(cmd.argv[0], "kill") == 0 || strcmp(cmd.argv[0], "k")== 0  ){
        if (cmd.argv[1] == NULL){
          printf("USAGE: kill (process ID)\n");
          
        } else{
          int isID=-1;
          int matchingi = -1;
          int givenID = atoi(cmd.argv[1]);
          
          for(int i=0; i < numBackground; i++)
          {
            if (backgroundProcessID[i] == givenID){
              isID = 1;
              matchingi = i;
            }
          }
          if(isID > 0){
            kill(backgroundProcessID[matchingi],SIGTERM);
          }else{
            printf("Process with the id %s not found.\n", cmd.argv[1]);
          }
          
        }
      
      }else{
      
//----------------------------SHELL------------------------------
      
      //find the path to the specified command
      path =lookupPath(cmd.argv[0],dirs,numPaths);
      
      //printf("%s: %s\n", "---The path is", path);
      if (numBackground < 10)
      {
        shouldParentWait = strcmp(cmd.argv[cmd.argc - 1],"&");
      }else{
        printf("Too many processes in the background.\n");
        shouldParentWait = -1;
        cmd.argv[cmd.argc -1] = NULL;
      }
      
      for(int l = 0;l<cmd.argc;l++){
        //printf("TEST: checking argv from before %s\n",cmd.argv[l]);
      }
      
      //if a path is found...
      if(path != NULL){
        //...split the process into a parent and a child...
        pid_t forkVal = fork();
      
        if(forkVal < 0)
        {
          printf("ERROR: Fork Failed");
        } 
        else if(forkVal == 0){
          //printf("%s\n","------Child Process");
          if (0== shouldParentWait){ 
            cmd.argv[cmd.argc-1] = NULL; 
            setpgid(0,0);
            }
          for(int l = 0;l < cmd.argc;l++){
            //printf("TEST: Checking argv: %s\n",cmd.argv[l]);
          }
          
          //...and execute the given command.
          execv(path, cmd.argv);
          printf("%s\n", "Execv() failed.");
          break;
    
        } else{
            if(0 != shouldParentWait) { 
              while(waitpid(forkVal, &status, 0)<=0);
            }else{
              //printf("------Not waiting for child\n");
              backgroundProcessID[numBackground] = forkVal;
              backgroundProcessName[numBackground] = cmd.argv[0];
              //printf("TEST: bpName[%i], value: %s\n", numBackground,backgroundProcessName[numBackground]);
              numBackground++;
              
              for(int i = 0; i<cmd.argc; i++)
              {
                //cmd.argv[i] = NULL;
              }
            }
          }
        }
      }
    }
//--------------------------FREEING MEMORY------------------------
    }
    free(path);
    for(int i = 0; i < numCmds; i++){
      free(cmd.argv[i]);
  }
  
  free(dirs[0]);
}

