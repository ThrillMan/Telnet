// client_handler.c
#include "client_handler.h"
#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>



char *path_generator(const char *currpath, const char *buf) {
    // Allocate enough memory for the new path
    char *str_to_ret = malloc(sizeof(char) * 256); 
    if (str_to_ret == NULL) {
        perror("malloc failed");
        return NULL;
    }

    // Remove "cd " 
    const char *no_cd = buf + 3;
    
    //absolute path
    if(!strncmp(no_cd," /",2)){
        // Create the new path string
        snprintf(str_to_ret, 256, "%s", no_cd);  // snprintf ensures no buffer overflow   
    }
    //relative path
    else{
        // Create the new path string
        snprintf(str_to_ret, 256, "%s/%s",currpath, no_cd);  // snprintf ensures no buffer overflow  
    }
    
    char test_path[512];
    snprintf(test_path, 512, "%s", str_to_ret); 
    printf("test path:%s\n",test_path);

    //test whether user inputed proper directory
    if(chdir(test_path)!=0){
        printf("Error changing the string\n");
        return "ERROR_WRONG_PATH";
    }
    else{
        //all successful, changing server directory back to previous one
        chdir(currpath);
    }
    return str_to_ret;
}

char *list_of_errors(int error){
    //return proper error associated with invalid command
    char *str_to_ret = malloc(sizeof(char) * 256);
    if (str_to_ret == NULL) {
        perror("malloc failed");
        return NULL;
    }
    switch(error){
        case 1:
            sprintf(str_to_ret,"Permission denied\n");
            break;
        case 2:
            sprintf(str_to_ret,"No such file or directory\n");
            break;
        case 127:
            sprintf(str_to_ret,"Command not found\n");
            break;
        default:
            sprintf(str_to_ret,"Some kind of error occurred\n");
            break;
    }
    return str_to_ret;
}

void* cthread(void* arg) {
    char buf[4096];
    int rc;

    char login[128];
    memset(&login,0,sizeof(login));
    struct cln* c = (struct cln*)arg;

    //creation of username ip:port
    sprintf(login, "%s:%d", inet_ntoa((struct in_addr)c->caddr.sin_addr)
    ,ntohs(c->caddr.sin_port));

    printf("new connection from:%s\n",login);

    //default path
    char path[512];
    strcpy(path,"/var/tmp");

    while(1){
    

    // Read the message from the client
    rc = read(c->cfd, buf, sizeof(buf));

    //user disconnects
    if (rc <= 0){
        printf("user disconnected\n");
        break;
    }
    buf[rc]='\0';
    printf("%s: %s\n", login,buf);

    FILE *fp;
    char file_type[400];
    memset(&file_type,0,sizeof(file_type));

    
    // Generate path if user changes directory
    if(!strncmp(buf,"cd",2)){
        char temp_path[1024];
        strcpy(temp_path,path_generator(path,buf));

        if(strcmp(temp_path,"ERROR_WRONG_PATH")==0){
            write(c->cfd, "Wrong directory\n", strlen("Wrong directory"));
            printf("wrong directory\n");
        }

        else{
            strcpy(path,temp_path);
            write(c->cfd, "Changed directory to\n", strlen("Changed directory"));
            printf("changed directory to:%s\n",path);
            
        }
        
        continue;
    }


    char command[4612];

    //generate proper cd command
    sprintf(command, "cd %s&&%s", path,buf); 

    fp = popen(command,"r");
    printf("command:%s\n",command);
    if (fp == NULL) {
        printf("Failed to run command\n");
        char error[128];
        strcpy(error,"Failed to run command\n");
        printf("%s",error);
        write(c->cfd, error, strlen(error));
        memset(&error, 0, sizeof(error));

        pclose(fp);
        memset(&buf, 0, sizeof(buf));
        continue;
    }

    memset(&buf, 0, sizeof(buf));


    while (fgets(file_type, sizeof(file_type), fp) != NULL) {
        printf("%s", file_type);
        strcat(buf,file_type);
    }
    write(c->cfd, buf, strlen(buf));

    int status = pclose(fp);
    printf("\nstatus -> %d\n", WEXITSTATUS(status));

    //means the command has not been executed
    if(WEXITSTATUS(status)>0){
        char error[128];
        strcpy(error,list_of_errors(WEXITSTATUS(status)));
        write(c->cfd, error, strlen(error));
        memset(&error, 0, sizeof(error));
    }
    memset(&buf, 0, sizeof(buf));
    }

    // Clean up and close the connection
    memset(&buf, 0, sizeof(buf));
    close(c->cfd);
    free(c);

    return 0;
}
