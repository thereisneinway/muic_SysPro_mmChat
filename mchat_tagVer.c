#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>

#define FILE_LENGTH 0x100

struct mm_st{
  int written,tag;
  char data[BUFSIZ];
};

void quit(int signal) { exit(EXIT_SUCCESS); }

int main(int argc, char *argv[]){
  //INPUT CHECKING
  argv++;
  int index = atoi(*argv);
  if(index>2 || index<1){
    fprintf(stderr, "Incorrect input <[1,2]>\n");
    exit(EXIT_FAILURE);
  }

  int running = 1;
  
  int fd;
  void* file_memory;
  struct mm_st* mm_area;

  /* Prepare a file large enough to hold an unsigned integer. */
  fd = open ("chat_log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  lseek (fd, FILE_LENGTH+1, SEEK_SET);
  write (fd, "", 1);
  lseek (fd, 0, SEEK_SET);
  file_memory = mmap (0, FILE_LENGTH, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  mm_area = (struct mm_st *) file_memory;
  //CHILD PROCESS CREATION
  pid_t pid = fork();
  //SIGNAL EXIT HANDLING
  struct sigaction sg;
  sg.sa_handler = quit;
  sigemptyset(&sg.sa_mask);
  sg.sa_flags = 0;
  sigaction(SIGTERM, &sg, 0);


  switch(pid){
    case -1: exit(EXIT_FAILURE);
    case 0:{//WRITE
       
      printf("Write initialize %d successful\n",index);
      char buffer[BUFSIZ];
      mm_area->written = 0;
      while(running){
        /*while (mm_area->written[index]){
          usleep(rand() %4);
          printf("Waiting...\n");
        }*/
        fgets(buffer, BUFSIZ, stdin);
        strcpy(mm_area->data, buffer);
        mm_area->written = 2;
        mm_area->tag = index;
        sprintf((char*) file_memory,"%s\n", mm_area);
        if (strncmp(mm_area->data, "end chat", 8) == 0){ 
          running = 0;
          kill(getppid(), SIGTERM); 
        } 
      }
      munmap(file_memory, FILE_LENGTH);
      close (fd);
      break;
    }
    default:{//READ
      //SWITCHING DIRECTION
      if(index == 2) index = 1;
      else index = 2;
      
      printf("Read initialize %d successful\n",index);

      while(running){
        sscanf(file_memory, "%s\n", mm_area);
        if (mm_area->written && mm_area->tag == index){
          printf("Receieve data : %s", mm_area->data);
          mm_area->written = 0;
          if (strncmp(mm_area->data, "end chat", 8) == 0){
            running = 0;
            kill(0, SIGTERM);
          }
        }
        usleep(rand() %4);
      }
      close (fd);
      munmap(file_memory, FILE_LENGTH);
      
    }
  }//End switch
    

  
  exit(EXIT_SUCCESS);
}