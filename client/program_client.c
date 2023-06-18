#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5050

struct permission
{
    char name[256];
    char pass[256];
};

int checkPermission(char *uname, char *pass)
{
    FILE *file;
    char line[128];
    struct permission client;

    file = fopen("../database/databases/user.txt", "r");
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        sscanf(line, "Username: %[^,], Password: %[^;]", client.name, client.pass);

        int nameComparison = strcmp(client.name, uname);
        int passComparison = strcmp(client.pass, pass);

        if (nameComparison == 0 && passComparison == 0)
        {
            fclose(file);
            return 1; // Username and password match
        }
    }

    fclose(file);
    return 0; // No username and password match
}

void Log(char *command, char *name)
{
    time_t times;
    struct tm *info;
    time(&times);
    info = localtime(&times);

    char info_log[256];

    FILE *file;
    char loc[256];
    snprintf(loc, sizeof loc, "../database/log/log.log", name);
    file = fopen(loc, "ab");

    sprintf(info_log, "%d-%.2d-%.2d %.2d:%.2d:%.2d:%s:%s;\n", info->tm_year + 1900, info->tm_mon + 1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, name, command);

    fputs(info_log, file);
    fclose(file);
}

int main(int argc, char *argv[])
{
    char *username = NULL;
    char *password = NULL;

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-u") == 0)
        {
            username = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            password = argv[i + 1];
        }
    }

    int allowed = 0;
    int id_user = geteuid();
    char used_db[256];

    if (geteuid() == 0)
        allowed = 1;
    else
    {
        int id = geteuid();
        if (checkPermission(username, password))
        {
            allowed = 1;
        }
        else
        {
            allowed = 0;
        }
        id_user = id;
    }

    if (id_user == 0)
    {
    }
    else if (username == NULL || password == NULL)
    {
        printf("Missing username or password.\n");
        return 0;
    }

    // Check if running with sudo
    if (allowed || id_user == 0)
    {
        printf("Authentication successful\n");
        // Continue with the rest of the program
    }
    else
    {
        printf("Authentication failed. Exiting...\n");
        return 0;
    }

    int client_socket, ret;
    struct sockaddr_in serverAddr;
    char buff[32000];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        printf("[-] Error in connection.\n");
        exit(1);
    }

    printf("[+] Client Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = connect(client_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (ret < 0)
    {
        printf("[-] Error connection.\n");
        exit(1);
    }
    printf("[+] Connected to Server.\n");

    while (1)
    {
        printf("Client: \t");
        char input[256];
        char temp[256];
        char command[100][1000];
        char *token;
        int i = 0;
        scanf(" %[^\n]s", input);
        strcpy(temp, input);
        token = strtok(input, " ");

        while (token != NULL)
        {
            strcpy(command[i], token);
            i++;
            token = strtok(NULL, " ");
        }

        int falseCmd = 0;
        if (strcmp(command[0], "CREATE") == 0)
        {
            if (strcmp(command[1], "USER") == 0 && strcmp(command[3], "IDENTIFIED") == 0 && strcmp(command[4], "BY") == 0)
            {
                snprintf(buff, sizeof buff, "CREATEUSER:%s:%s:%d", command[2], command[5], id_user);
                send(client_socket, buff, strlen(buff), 0);
            }
            else if (strcmp(command[1], "DATABASE") == 0)
            {
                snprintf(buff, sizeof buff, "CREATEDATABASE:%s:%s:%d", command[2], argv[2], id_user);
                send(client_socket, buff, strlen(buff), 0);
            }
            else if (strcmp(command[1], "TABLE") == 0)
            {
                snprintf(buff, sizeof buff, "CREATETABLE:%s", temp);
                send(client_socket, buff, strlen(buff), 0);
            }
        }
        else if (strcmp(command[0], "GRANT") == 0 && strcmp(command[1], "PERMISSION") == 0 && strcmp(command[3], "INTO") == 0)
        {
            snprintf(buff, sizeof buff, "GRANTPERMISSION:%s:%s:%d", command[2], command[4], id_user);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(command[0], "USE") == 0)
        {
            snprintf(buff, sizeof buff, "USEDATABASE:%s:%s:%d", command[1], argv[2], id_user);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(command[0], "DROP") == 0)
        {
            if (strcmp(command[1], "DATABASE") == 0)
            {
                snprintf(buff, sizeof buff, "DROPDATABASE:%s:%s", command[2], argv[2]);
                send(client_socket, buff, strlen(buff), 0);
            }
            else if (strcmp(command[1], "TABLE") == 0)
            {
                snprintf(buff, sizeof buff, "DROPTABLE:%s:%s", command[2], argv[2]);
                send(client_socket, buff, strlen(buff), 0);
            }
            else if (strcmp(command[1], "COLUMN") == 0)
            {
                snprintf(buff, sizeof buff, "DROPCOLUMN:%s:%s:%s", command[2], command[4], argv[2]);
                send(client_socket, buff, strlen(buff), 0);
            }
        }
        else if (strcmp(command[0], "INSERT") == 0 && strcmp(command[1], "INTO") == 0)
        {
            snprintf(buff, sizeof buff, "INSERT:%s", temp);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(command[0], "UPDATE") == 0)
        {
            snprintf(buff, sizeof buff, "UPDATE:%s", temp);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(command[0], "DELETE") == 0)
        {
            snprintf(buff, sizeof buff, "DELETE:%s", temp);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(command[0], "SELECT") == 0)
        {
            snprintf(buff, sizeof buff, "SELEECT:%s", temp);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(command[0], ":exit") != 0)
        {
            falseCmd = 1;
            char peringatan[] = "Invalid Command";
            send(client_socket, peringatan, strlen(peringatan), 0);
        }

        if (falseCmd != 1)
        {
            char sender[1000];
            if (id_user == 0)
                strcpy(sender, "root");
            else
                strcpy(sender, argv[2]);
            Log(temp, sender);
        }

        if (strcmp(command[0], ":exit") == 0)
        {
            send(client_socket, command[0], strlen(command[0]), 0);
            close(client_socket);
            printf("[-]Disconnected from server.\n");
            exit(1);
        }
        bzero(buff, sizeof(buff));
        if (recv(client_socket, buff, 1024, 0) < 0)
            printf("[-]Error in receiving data.\n");
        else
            printf("Server: \t%s\n", buff);
    }

    return 0;
}