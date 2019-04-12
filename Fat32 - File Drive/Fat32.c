/*
  Name: Hunter Nghiem
  ID:   1001275883

  Name: Nhan Lam
  ID:   1001506478

*/

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
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports ten arguments

char       BS_OEMName[8];
int16_t    BPB_BytsPerSec;
int8_t     BPB_SecPerClus;
int16_t    BPB_RsvdSecCnt;
int8_t     BPB_NumFATs;
int16_t    BPB_RootEntCnt;
char       BS_VolLab[11];
int32_t    BPB_FATSz32;
int32_t    BPB_RootClus;

int32_t    RootDirSectors = 0;
int32_t    FirstDataSector = 0;
int32_t    FirstSectorofCluster = 0;

int LBAToOffset(int32_t sector)
{
  return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

int root_address;

typedef struct __attribute__((__packed__)) DirectoryEntry{
  char      DIR_Name[11];
  uint8_t   DIR_Attr;
  uint8_t   Unused1[8];
  uint16_t  DIR_FirstClusterHigh;
  uint8_t   Unused2[4];
  uint16_t  DIR_FirstClusterLow;
  uint32_t  DIR_FileSize;
}DirectoryEntry;

DirectoryEntry directory[16];
char formatName[16][12];

FILE *root_file_system;
bool isFATOpen = false;
char * formatToken;
char* fileName;
char* ext;
char* token;
char TempName[16][12];
int address;
char readin[1];

int main()
{
  //char points to cmd_str to alloc the block for the command MAX_COMMAND_SIZE
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {

    // Print out the msh prompt
    printf ("mfs> ");

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
        token_count++;
      }
      //NULL terminate array for exec functions
      token[token_count] = NULL;
      //---------------quit and exit function used to terminate program----------
      if (token[0] == NULL)
      {
        //do nuthin
      }
      else if(token[0] != NULL)
      {
        if(!strcmp(token[0],"quit") || !strcmp(token[0] , "exit"))
        {
          return 0;
        }
        else if(!strcmp(token[0],"open") && isFATOpen == false)
        {
          root_file_system = fopen(token[1],"rb");
          isFATOpen = true;

          if(root_file_system == NULL)
          {
            perror("Error in opening the image: ");
            //fclose(root_file_system);

          }
          else
          {
            fseek(root_file_system,3,SEEK_SET);
            fread(&BS_OEMName,8,1,root_file_system);

            fseek(root_file_system,11,SEEK_SET);
            fread(&BPB_BytsPerSec,2,1,root_file_system);

            fseek(root_file_system,13,SEEK_SET);
            fread(&BPB_SecPerClus,1,1,root_file_system);

            fseek(root_file_system,14,SEEK_SET);
            fread(&BPB_RsvdSecCnt,2,1,root_file_system);

            fseek(root_file_system,16,SEEK_SET);
            fread(&BPB_NumFATs,1,1,root_file_system);

            fseek(root_file_system,17,SEEK_SET);
            fread(&BPB_RootEntCnt,2,1,root_file_system);

            fseek(root_file_system,71,SEEK_SET);
            fread(&BS_VolLab,11,1,root_file_system);

            fseek(root_file_system,36,SEEK_SET);
            fread(&BPB_FATSz32,4,1,root_file_system);

            fseek(root_file_system,44,SEEK_SET);
            fread(&BPB_RootClus,4,1,root_file_system);

            root_address = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);

            fseek(root_file_system, root_address, SEEK_SET);
            fread(directory, 32 * 16, 1, root_file_system);

            int i;
            for (i = 0; i < 16; i++)
            {
              if (directory[i].DIR_Attr == 0x01 || directory[i].DIR_Attr == 0x10 || directory[i].DIR_Attr == 0x20)
              {

                  memcpy(formatName[i], directory[i].DIR_Name, 11);
                  formatName[i][11] = '\0';
                  memcpy(TempName[i],formatName[i],12);
              }
            }
          }
        }
        else if(!strcmp(token[0],"close"))
        {
          isFATOpen = false;
          fclose(root_file_system);
          printf("Close Successful.\n");
        }
        else if (isFATOpen)
        {

          if(!strcmp(token[0],"info")&&isFATOpen)
          {
            printf("BPB_BytsPerSec: %x in hexadecimal and %d in base 10.\n", BPB_BytsPerSec, BPB_BytsPerSec);
            printf("BPB_SecPerClus: %x in hexadecimal and %d in base 10.\n", BPB_SecPerClus, BPB_SecPerClus);
            printf("BPB_RsvdSecCnt: %x in hexadecimal and %d in base 10.\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
            printf("BPB_NumFATs   : %x in hexadecimal and %d in base 10.\n", BPB_NumFATs, BPB_NumFATs);
            printf("BPB_FATSz32   : %x in hexadecimal and %d in base 10.\n", BPB_FATSz32, BPB_FATSz32);
          }

          if(!strcmp(token[0],"stat")&&isFATOpen && token[1]!=NULL)
          {
            int count;
            int i;
            for(i =0; i <16 ; i++)
            {
              fileName = strtok(TempName[i], " \t");
              ext = strtok(NULL, " \t");

              if(ext != NULL)
              {
                char text[] = ".";
                strcat(text,ext);
                //memcpy(&fileName[strlen(fileName)], ext, 3);
                strcat(fileName,text);
              }
              if(fileName != NULL)
              {
                strcpy(TempName[i],fileName);
              }
              char capitalName[16];
              strcpy (capitalName,token[1]);
              char * txt = capitalName;
              while(*txt)
              {
                *txt = toupper(*txt);
                txt++;
              }
              if(token[1] == NULL)
              {
                printf("Error: File not found.");
              }
              if(strcmp(capitalName,TempName[i]) == 0)
              {
                printf("Attribute: %d\n", directory[i].DIR_Attr);
                printf("Cluster Number: %d\n", directory[i].DIR_FirstClusterLow);
                count ++;
              }
            }
            if(count > 0)
            {
              printf("Error: FIle not found.\n");
              count = 0;
            }
          }

          if(!strcmp(token[0],"get")&&isFATOpen)
          {
            int i;
            for(i =0; i <16 ; i++)
            {
              fileName = strtok(TempName[i], " \t");
              ext = strtok(NULL, " \t");

              if(ext != NULL)
              {
                char text[] = ".";
                strcat(text,ext);
                //memcpy(&fileName[strlen(fileName)], ext, 3);
                strcat(fileName,text);
              }
              if(fileName != NULL)
              {
                strcpy(TempName[i],fileName);
              }
              char capitalName[16];
              strcpy (capitalName,token[1]);
              char * txt = capitalName;
              while(*txt)
              {
                *txt = toupper(*txt);
                txt++;
              }
              if(strcmp(capitalName,TempName[i]) == 0)
              {
                address = LBAToOffset(directory[i].DIR_FirstClusterLow);
                FILE *new_file_system2 = fopen(capitalName, "w");

                int size_of_file = directory[i].DIR_FileSize;
                fseek(root_file_system, address, SEEK_SET);

                int i;
                for ( i = 0; i < size_of_file  ; i++)
                {
                fread(&readin, 1, 1, root_file_system);
                fprintf(new_file_system2, "%c", readin[0]);
                }

                fclose(new_file_system2);
              }
            //  char user_input = (strcmp(token[1])
            }
          }

          if(!strcmp(token[0],"cd")&&isFATOpen)
          {

            int i;
            for(i =0; i <16 ; i++)
            {
              fileName = strtok(TempName[i], " \t");
              ext = strtok(NULL, " \t");

              if(ext != NULL)
              {
                char text[] = ".";
                strcat(text,ext);
                //memcpy(&fileName[strlen(fileName)], ext, 3);
                strcat(fileName,text);
              }
              if(fileName != NULL)
              {
                strcpy(TempName[i],fileName);
              }
              char capitalName[16];
              strcpy (capitalName,token[1]);
              char * txt = capitalName;
              while(*txt)
              {
                *txt = toupper(*txt);
                txt++;
              }
              if(strcmp(capitalName,TempName[i]) == 0)
              {
                int newaddress = 0;
                newaddress = LBAToOffset(directory[i].DIR_FirstClusterLow);

                fseek(root_file_system, newaddress, SEEK_SET);

                for(i =0; i < 16; i++)
                {
                  fread(directory, sizeof(DirectoryEntry), 1, root_file_system);
                }

                for (i = 0; i < 16; i++)
                {


                      memcpy(formatName[i], directory[i].DIR_Name, 11);
                      formatName[i][11] = '\0';
                      printf("%s", formatName[i]);
                      memcpy(TempName[i],formatName[i],12);

                }



              }
            }
          }

          if(!strcmp(token[0],"ls")&&isFATOpen)
          {
            int i;
            for (i = 0; i < 16; i++)
            {
              if (directory[i].DIR_Attr == 0x01 || directory[i].DIR_Attr == 0x10 || directory[i].DIR_Attr == 0x20)
              {
                  printf("%s\n", formatName[i]);
              }
            }

          }

          if(!strcmp(token[0],"read")&&isFATOpen)
          {
            int i;
            for(i =0; i <16 ; i++)
            {
              fileName = strtok(TempName[i], " \t");
              ext = strtok(NULL, " \t");

              if(ext != NULL)
              {
                char text[] = ".";
                strcat(text,ext);
                //memcpy(&fileName[strlen(fileName)], ext, 3);
                strcat(fileName,text);
              }
              if(fileName != NULL)
              {
                strcpy(TempName[i],fileName);
              }
              char capitalName[16];
              strcpy (capitalName,token[1]);
              char * txt = capitalName;
              while(*txt)
              {
                *txt = toupper(*txt);
                txt++;
              }
              if(strcmp(capitalName,TempName[i]) == 0)
              {
                address = LBAToOffset(directory[i].DIR_FirstClusterLow);
                if(token[2] == NULL)
                {
                  fseek(root_file_system, address, SEEK_SET);
                }
                if(token[2] != NULL)
                {
                  fseek(root_file_system, address+atoi(token[2]), SEEK_SET);
                }
                int i;
                for ( i = 0; i < atoi(token[3])  ; i++)
                {
                fread(&readin, 1, 1, root_file_system);
                printf("%d\n", readin[0]);
                }
              }
          }

          if(!strcmp(token[0],"volume")&&isFATOpen)
          {
            //printf(BS_OEMName);
          }
        }
      }

      free( working_str );
    }
  }
    // when used, makes sure that a fork bomb in not possible--------------------
    return EXIT_SUCCESS;

}
