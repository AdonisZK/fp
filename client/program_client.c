#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

#define PORT 5050

struct permission
{
    char name[10000];
    char pass[10000];
};

void trim(char *str)
{
    // Trim leading whitespaces
    while (isspace((unsigned char)*str))
    {
        str++;
    }

    // Trim trailing whitespaces
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
    {
        end--;
    }
    *(end + 1) = '\0';
}

int checkPermission(char *uname, char *pass)
{
    FILE *file;
    char line[100];
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

        printf("Username Comparison: %d\n", nameComparison);
        printf("Password Comparison: %d\n", passComparison);

        if (nameComparison == 0 && passComparison == 0)
        {
            fclose(file);
            return 1; // Username and password match
        }
    }

    fclose(file);
    return 0; // No username and password match
}

void writeLog(char *cmd, char *name)
{
    time_t times;
    struct tm *info;
    time(&times);
    info = localtime(&times);

    char info_log[1000];

    FILE *file;
    char loc[10000];
    snprintf(loc, sizeof loc, "../database/log/log.log", name);
    file = fopen(loc, "ab");

    sprintf(info_log, "%d-%.2d-%.2d %.2d:%.2d:%.2d:%s:%s;\n", info->tm_year + 1900, info->tm_mon + 1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, name, cmd);

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

    if (username == NULL || password == NULL)
    {
        printf("Missing username or password.\n");
        return 0;
    }

    int allowed = 0;
    int id_user = geteuid();
    char used_db[1000];

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

    if (!allowed)
    {
        printf("Authentication failed. Exiting...\n");
        return 0;
    }

    //! cut

    int client_socket, ret;
    struct sockaddr_in serverAddr;
    char buff[32000];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }

    printf("[+]Client Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = connect(client_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (ret < 0)
    {
        printf("[-]Error connection.\n");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

    while (1)
    {
        printf("Client: \t");
        char input[10000];
        char temp[10000];
        char cmd[100][10000];
        char *token;
        int i = 0;
        scanf(" %[^\n]s", input);
        strcpy(temp, input);
        token = strtok(input, " ");

        while (token != NULL)
        {
            strcpy(cmd[i], token);
            i++;
            token = strtok(NULL, " ");
        }

        int falseCmd = 0;
        if (strcmp(cmd[0], "CREATE") == 0)
        {
            if (strcmp(cmd[1], "USER") == 0 && strcmp(cmd[3], "IDENTIFIED") == 0 && strcmp(cmd[4], "BY") == 0)
            {
                snprintf(buff, sizeof buff, "cUser:%s:%s:%d", cmd[2], cmd[5], id_user);
                send(client_socket, buff, strlen(buff), 0);
            }
            else if (strcmp(cmd[1], "DATABASE") == 0)
            {
                snprintf(buff, sizeof buff, "cDatabase:%s:%s:%d", cmd[2], argv[2], id_user);
                send(client_socket, buff, strlen(buff), 0);
            }
            else if (strcmp(cmd[1], "TABLE") == 0)
            {
                snprintf(buff, sizeof buff, "cTable:%s", temp);
                send(client_socket, buff, strlen(buff), 0);
            }
        }
        else if (strcmp(cmd[0], "GRANT") == 0 && strcmp(cmd[1], "PERMISSION") == 0 && strcmp(cmd[3], "INTO") == 0)
        {
            snprintf(buff, sizeof buff, "gPermission:%s:%s:%d", cmd[2], cmd[4], id_user);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(cmd[0], "USE") == 0)
        {
            snprintf(buff, sizeof buff, "uDatabase:%s:%s:%d", cmd[1], argv[2], id_user);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(cmd[0], "cekCurrentDatabase") == 0)
        {
            snprintf(buff, sizeof buff, "%s", cmd[0]);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(cmd[0], "DROP") == 0)
        {
            if (strcmp(cmd[1], "DATABASE") == 0)
            {
                snprintf(buff, sizeof buff, "dDatabase:%s:%s", cmd[2], argv[2]);
                send(client_socket, buff, strlen(buff), 0);
            }
            else if (strcmp(cmd[1], "TABLE") == 0)
            {
                snprintf(buff, sizeof buff, "dTable:%s:%s", cmd[2], argv[2]);
                send(client_socket, buff, strlen(buff), 0);
            }
            else if (strcmp(cmd[1], "COLUMN") == 0)
            {
                snprintf(buff, sizeof buff, "dColumn:%s:%s:%s", cmd[2], cmd[4], argv[2]);
                send(client_socket, buff, strlen(buff), 0);
            }
        }
        else if (strcmp(cmd[0], "INSERT") == 0 && strcmp(cmd[1], "INTO") == 0)
        {
            snprintf(buff, sizeof buff, "insert:%s", temp);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(cmd[0], "UPDATE") == 0)
        {
            snprintf(buff, sizeof buff, "update:%s", temp);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(cmd[0], "DELETE") == 0)
        {
            snprintf(buff, sizeof buff, "delete:%s", temp);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(cmd[0], "SELECT") == 0)
        {
            snprintf(buff, sizeof buff, "select:%s", temp);
            send(client_socket, buff, strlen(buff), 0);
        }
        else if (strcmp(cmd[0], ":exit") != 0)
        {
            falseCmd = 1;
            char peringatan[] = "Invalid Command";
            send(client_socket, peringatan, strlen(peringatan), 0);
        }

        if (falseCmd != 1)
        {
            char sender[10000];
            if (id_user == 0)
                strcpy(sender, "root");
            else
                strcpy(sender, argv[2]);
            writeLog(temp, sender);
        }

        if (strcmp(cmd[0], ":exit") == 0)
        {
            send(client_socket, cmd[0], strlen(cmd[0]), 0);
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