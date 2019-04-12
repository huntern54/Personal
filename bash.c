
// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports ten arguments
#define MAX_COMMAND_HISTORY 16  //Max command lines shown given to user


int main()
{
  //char points to cmd_str to alloc the block for the command MAX_COMMAND_SIZE
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  //allows for command arguement to hold 15 and be an array for the history
  char* history[MAX_COMMAND_HISTORY];
  history[MAX_COMMAND_HISTORY-1] = NULL;

  //counter to keep track of history storing
  int count_history = 0;

  //counter to keep track of pid storing
  int pid_count = 0;

  //array to store the previous pids
  int listpids[MAX_COMMAND_HISTORY];



  //-----------------------signal handling-------------------------------------
  sigset_t newmask;
  /*Empty out the sigset.  We can't call memset as it's
    not guaranteed to clear the set
  */
  sigemptyset( &newmask );
  /*
    Add SIGINT to the set.  This will allow us to ignore
    any ctrl-c presses
  */
  sigaddset( &newmask, SIGINT ); //blocked SIGINT
  sigaddset( &newmask, SIGTSTP ); //block SIGTSTP

  /*
    Finally, add the new signal set to our process' control block
    signal mask element.  This will block all SIGINT signals from
    reaching the process
  */
  if( sigprocmask( SIG_BLOCK, &newmask, NULL ) < 0 )
  {
    perror("sigprocmask ");
  }


//----------end of signal blocking needs to be put before the while loop so that
// way it can be blocked before 'while' loop starts

  while( 1 )
  {

    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    //makes duplicate of the original string so that way it will keep
    //original as you parse the other strings
    char *working_str  = strdup( cmd_str );


    // Tokenize the input stringswith whitespace used as the delimiter
    while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) && (token_count<MAX_NUM_ARGUMENTS))
    {

      token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);

      //----------------------------ignoring empty input for next line--------------
        if (token[0][0] == '\n')
        {
          break;
        }

        else if(strlen(token[token_count]) == 0)
        {
          memset(token[token_count],'\0',MAX_COMMAND_SIZE);
          //made so the token will not skip one token count,
          //cause while you set the null terminator to 0, it should take the NULL
          //out of the token, which will also take out the null after entering inputs
          //also taking the token count to -- to eliminate the null and then increase it after
          //to safely get a token that will not produce a null
          token_count--;
        }
        //save your commands into the history array which will print when called
        //in the child pid to print
        else if(strcmp(token[0],"history") != 0)
        {
          history[count_history] = strndup(cmd_str,strlen(cmd_str));
          count_history++;

          //will check to make sure that bounds exceed 15, if does, takes out the oldest command entered
/*        if(count_history == MAX_COMMAND_HISTORY-2)
          {
            int i;
            for(i =0; i<MAX_COMMAND_HISTORY-2; i++)
            {
              history[i] = history[i+1];
            }
            count_history = MAX_COMMAND_HISTORY-2;
          }*/
        }
        token_count++;
      }
      //NULL terminate array for exec functions
      token[token_count] = NULL;
      //---------------quit and exit function used to terminate program----------

      if(token[0] != NULL)
      {
        if(!strcmp(token[0],"quit"))
        {
          return 0;
        }

        if(!strcmp(token[0],"exit"))
        {
          return 0;
        }

      }

      //--------------------exec funtion-------------------------------
      pid_t child_pid = fork();
      int status;


      if (child_pid < 0)
      {
        //handle fork error
        perror("Child pid gone");
        return 0;
      }

      else if (child_pid == 0)
      {
        //child process
          if (strcmp(token[0],"cd") == 0)
          //if the cd is used in the child, should want to exit because the exec
          //cd needs to be used in the parent
          {
            exit(EXIT_SUCCESS);
          }
          //call onto the history function for the history commands and listpids to be printed out

          else if(!strcmp(token[0], "history" ))
          {
            int i;
            for( i = 0; i < count_history; i++)
            {
              printf("%d: %s",i, history[i]);
            }
          }

          else if(!strcmp(token[0], "listpids"))
          {
            int i, counter = 0;
            for( i = 0; i < pid_count; i++)
            {
              if (listpids[i] != 0)
              {
                printf("%d: %d \n",counter, listpids[i]);
                counter++;
              }
            }
          }

//supposed to be code that was to use the !n to run the number
/*
          if(token[0] == "!")
          {
            int number = Integer.parseInt(count_history);

            strcpy(token[0][0], number);

            //getline(token[0], strndup(working_str,strlen(working_str)));

              if(number == count_history)
              {
                execvp(count_history);
              }
          }
*/
          //handle error if command not found
          else if (execvp(token[0],token) < 0)
          {
            perror("msh: invalid option --");
          }

          //exit used to make sure fork bomb does not happen because the
          //exit is inside the while loop, causing the child to be gone
          exit(EXIT_SUCCESS);
      }

      else
      {
          //parent process
          //waits for the child process to finish, then goes back to the parent to
          //work. Also waits for the child pids to work so that way it may let the child_pid
          //finsih and then the parent will work
          waitpid(child_pid, &status, 0);
          listpids[pid_count] = child_pid;
          pid_count++;

          if (pid_count == MAX_COMMAND_HISTORY)
          {
            //handle overflow
          }

        if (token[0]!=NULL)
        {
          //if the token of [0] is called to the parent, it will change the
          //directory of the child

          if (strcmp(token[0],"cd") == 0)
          {
            //error check
            if(chdir(token[1]) != 0)
            {
            perror("error");
            }
          }
        }
      }
      //memory clean up
      free( working_str );
    }

    // when used, makes sure that a fork bomb in not possible--------------------
    return EXIT_SUCCESS;

}
