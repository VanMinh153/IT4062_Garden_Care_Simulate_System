#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#define MAX 31 // Max of username is 30 character
#define MAX2 200 // Max of number account
#define MAX3 1001 // Max of content user want to post is 1000 character

typedef struct account{
    char username[MAX];
    bool status;
} stAccount;
stAccount accounts[MAX2];
char strTime[30];

char* getTime();
int readAccount();
int clearStdin();

int main (){
    int userChoice;
    char username[MAX];
    bool hasLogin = false;
    int numAccounts = readAccount();
    FILE* log = fopen("log_20215092.txt", "w");
    if(log == NULL){
        perror("Error opening file log_20215092.txt");
        return -1;
    }
    while(1){
    //Menu
    printf("---------------------------------------------\n");
    printf("\t USER MANAGEMENT PROGRAM\n");
    printf("1. Log in\n");
    printf("2. Post message\n");
    printf("3. Logout\n");
    printf("4. exit\n");
    int intV;
    char charV[5];
    while(true){
     char strOut[] = "Your choice: ";
     write(STDOUT_FILENO, strOut, strlen(strOut));
     intV = read(STDIN_FILENO, charV, 2);
     clearStdin();
     printf(" > %s#\n", charV);
     if(charV[1]=='\n') break;
     else{
        printf("Must to choose 1 to 4\n");
     }

    }
    userChoice = charV[0]-'0';

    switch(userChoice){
     case 1:;
      while(true){
       char strOut[] = "Username: ";
       write(STDOUT_FILENO, strOut, strlen(strOut));
       intV = read(STDIN_FILENO, username, sizeof(username));
       if(username[intV-1]=='\n') break;
       else{ printf("Username have to less than %d character", MAX-1); clearStdin(); }
      }
      username[intV-1] = '\0';
      if(hasLogin == true){
       printf("You have already logged in\n");
       fprintf(log, "%s $ 1 $ %s $ -ERR\n", getTime(),username);
       break;
      }

      bool isExist = 0;
      for(int i=0; i < numAccounts; i++){
       if(strcmp(username, accounts[i].username) == 0){
        isExist = 1;
        if(accounts[i].status == 1){
         printf("Hello %s\n", username);
         hasLogin = true;
         fprintf(log, "%s $ 1 $ %s $ +OK\n", getTime(),username); break;
        }
        else{
         printf("Account is banned\n");
         fprintf(log, "%s $ 1 $ %s $ -ERR\n", getTime(),username); break;
        }
       }
      }
      if(isExist == 0){
       printf("Account is not exist\n");
       fprintf(log, "%s $ 1 $ %s $ -ERR\n", getTime(),username);
       }
      break;
     case 2:;
      char content[MAX3];
      while(true){
       char strOut[] = "Post message: ";
       write(STDOUT_FILENO, strOut, strlen(strOut));
       intV = read(STDIN_FILENO, content, sizeof(content));
       if(content[intV-1]=='\n') break;
       else{ printf("Username have to less than %d character", MAX3-1); clearStdin(); }
      }
      content[intV-1] = '\0';
      if(hasLogin == false){
        printf("You have not logged in\n");
        fprintf(log, "%s $ 2 $ %s $ -ERR\n", getTime(),content);
      }
      else{
       printf("Successful post\n");
       fprintf(log, "%s $ 2 $ %s $ +OK\n", getTime(),content);
      }
      break;
     case 3:
      if(hasLogin == true){
       printf("Successful log out\n");
       hasLogin = false;
       fprintf(log, "%s $ 3 $ $ +OK\n", getTime());
      }
      else{
       printf("You have not logged in\n");
       fprintf(log, "%s $ 3 $ $ -ERR\n", getTime());
      }
      break;
     case 4: return 0;
     default: printf("Must to choose 1 to 4\n"); break;
    }
    }
    return 0;
}

// Function
int clearStdin(){
    char charV;
    while((charV=getchar())!='\n' && charV!=EOF);
    return 0;
}
//
char* getTime() {
    time_t currentTime;
    struct tm *tmTime;
    time(&currentTime);
    tmTime = localtime(&currentTime);
    strftime(strTime, sizeof(strTime), "[%d/%m/%Y %H:%M:%S]", tmTime);
//    printf("%s",strTime);
    return strTime;
}
//
int readAccount() {
    int fd = open("account.txt", O_RDONLY);
    int intV;
    if (fd == -1) {
        perror("Error opening file account.txt");
        return -1;
    }
    char* buffer = (char*) malloc(10000);
    int numAccounts = 0;

    intV = read(fd, buffer, 9999);
    if (intV <= 0) {
        perror("Fail to read file account.txt");
        free(buffer);
        return -1;
    }
    buffer[intV] = '\0';
//    printf("\'%s\'", buffer);
    char* token = strtok(buffer, "\n");
    char status[4];
    char strV[9];
    int intV2 = snprintf(strV, sizeof(strV), "%%%ds %%%lds", MAX,sizeof(status)-1);
//    puts(strV);
//    printf(">%d", intV2);
    if( intV2 > sizeof(strV)-1 ) // NOTE: you are comparing signed and unsigned integers
        fputs("strV string length exceeded\n",stderr);
    while (token != NULL) {
        if (sscanf(token, strV, accounts[numAccounts].username, status) == 2) {
            accounts[numAccounts].status = (strcmp(status, "1") == 0);
            numAccounts++;
        } else {
            printf("Error parsing line: %s\n", token);
        }

        token = strtok(NULL, "\n");
    }
//    for (int i = 0; i < numAccounts; i++) {
//        printf("%s \t%d\n", accounts[i].username, accounts[i].status);
//    }
    free(buffer);
    close(fd);
    return numAccounts;
}
