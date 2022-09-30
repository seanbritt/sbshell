#include "shellheader.h"

/*
int main(int argc, char **argv, char **envp){

  clr();
  printf("\n\n----------dir-----------\n");
  dir(argc, argv);
  printf("\n\n----------cd-----------\n");
  cd(argc, argv);
  printf("\n\n----------envrion-----------\n");
  environ(envp);
  printf("\n\n----------echo-----------\n");
  echo(argv);
  printf("\n\n----------help-----------\n");
  help();
  printf("\n\n----------pause-----------\n");
  _pause();
  printf("\n\n----------exit-----------\n");
  quit();
  return 0;
}
 */








int cd(int argc, char **argv){

    //get the cwd
  char buf[PATH_MAX];
  getcwd(buf, sizeof(buf));


  if(argc > 2){
    printf("too many arguments to cd\n");
    printf("current directory: %s\n", buf);
    return 0;
  }
  if (argc == 1){
    printf("%s\n", buf);
    return 1;
  }else{
    if(chdir(*(argv+1))==-1){
      printf("invalid directory: %s\n", *(argv+1));
      printf("current directory: %s\n", buf);
      return 0;
    }
    
    getcwd(buf, sizeof(buf));
    printf("\t%s\n", buf);
    printf("\t%s\n", *(argv+1));
    return 1;
  }
}






void clr(){
  printf("\033[H\033[2J");
}




int dir(int argc, char **argv){
  struct dirent *dirptr;
  DIR *directory;
  char buf[PATH_MAX], *path;

  //decide if it's a valid request to dir
  if(argc > 2){
    printf("too many arguments to dir\n");
    return 0;
  }if (argc == 1){
    path = getcwd(buf, sizeof(buf));
  }else{
    path = *(argv+1);
  }

  //find out if the directory can open
  if((directory = opendir(path)) == NULL){
    printf("failed opening current directory\n");
    return 0;
  }

  
  //this reads the directory into the dirent dirptr until the end of the directory
  while((dirptr = readdir(directory)) != NULL){
      //if the name of the current sub file starts with a '.' or
    if( (dirptr->d_name)[0] == '.' ) continue;
    else{

	printf("%s\n", dirptr->d_name);

    }
  }
  closedir(directory);
  return 1;

}




void environ(char **envp){

  int i;
  char *temp;
  for(i=0; (temp = *(envp+i)) != NULL; i++){
    printf("%s\n", temp);
  }
}





void echo(char **argv){

    char *temp;
    int i;
    for(i=1; (temp = *(argv+i)) != NULL; i++){
        printf("%s ", temp);
    }
    printf("\n");

}



void help(){


    char* shell = strdup(getenv("SHELL"));
    printf("shell: <%s>\n", shell);
    int len = strlen(shell);
    *(shell + len - 7) = '\0';
    strcat(shell, "readme_doc");

    pid_t pid;
    int status = 0;
    char *command[] = {"more", shell, NULL};


    if ((pid = fork()) == -1){
        printf("broken at help");
        exit(0);
    }else if (pid == 0){
        printf("<%s>\n", *(command+1));
        printf("<%s>\n", *(command+2));
        execvp(*command, command);
        printf("borken at execing more for readme_doc");
        exit(0);
    }else{
        waitpid(pid, &status, 0);
    }

    free(shell);

    //printf("%s\n", shell);

}



void _pause(){

  char buf[1024];   
  /*
  pid_t pid;
  pid = fork();
 
		
  if (pid == -1){
    printf("fork problem\n");
  }
  else if (pid == 0){
*/
    printf("paused...enter to continue\n");
    while((read(0, buf, 1024))!= 0){	
      if(strstr(buf, "\n")){
	break;
      }
    }
    /*
  }else{
    wait(NULL);
  }
    */
}


void quit(){
  exit(0);
}
