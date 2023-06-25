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
#include <math.h>

#define PORT 8080

struct permission
{
    char name[256];
    char pass[256];
};

void str_empty(char *str1, size_t bufsize)
{
    strncpy(str1, "", bufsize);
    if (str1[bufsize - 1] != '\0')
        str1[bufsize] = '\0';
}

int check_permission(char *uname, char *pass)
{
    FILE *file;
    char lines[128];
    struct permission client;

    file = fopen("../database/databases/user.txt", "r");
    if (file == NULL)
    {
        printf("Failed to open file\n");
        return 0;
    }

    while (fgets(lines, sizeof(lines), file) != NULL)
    {
        sscanf(lines, "Username: %[^,], Password: %[^;]", client.name, client.pass);

        int nameComparison = strcmp(client.name, uname);
        int passComparison = strcmp(client.pass, pass);

        if (nameComparison == 0 && passComparison == 0)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

void append_log(char *command, char *name)
{
    time_t times;
    struct tm *info;
    time(&times);
    info = localtime(&times);

    char write_log[256];

    FILE *file;
    char location[256];
    snprintf(location, sizeof location, "../database/log/log.log", name);
    file = fopen(location, "a");

    if (file == NULL)
    {
        printf("Error opening log file\n");
        return; // or handle the error in an appropriate way
    }

    sprintf(write_log, "%d-%.2d-%.2d %.2d:%.2d:%.2d:%s:%s;\n",
            info->tm_year + 1900, info->tm_mon + 1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, name, command);

    fputs(write_log, file);
    fclose(file);
}

int main(int argc, char *argv[])
{
    char *username = NULL;
    char *password = NULL;
    char *database = NULL;

    // Handling command line arguments
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
        else if (strcmp(argv[i], "-d") == 0)
        {
            database = argv[i + 1];
        }
    }

    int userAllowed = 0;
    int idUser = geteuid();
    char currDB[256];

    if (geteuid() == 0) // root
    {
        userAllowed = 1;
        // strncpy(argv[2], "root", strlen(argv[2]));
    }
    else
    {
        int id = geteuid();
        if (check_permission(username, password))
        {
            userAllowed = 1;
        }
        else
        {
            userAllowed = 0;
        }
        idUser = id;
    }

    if (idUser == 0) // root
    {
    }
    else if (username == NULL || password == NULL)
    {
        printf("ERR : Missing username or password.\n");
        return 0;
    }

    // SUDO
    if (userAllowed || idUser == 0)
    {
        printf("Authentication successful.\n");
    }
    else
    {
        printf("Authentication failed.\n");
        return 0;
    }

    int clientSocket, ret;
    struct sockaddr_in serverAddress;
    char buffer[32000];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        printf("SERVER : Error while connecting.\n");
        exit(1);
    }

    printf("SERVER : Client Socket is created.\n");

    memset(&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    if (ret < 0)
    {
        printf("SERVER : Error connection.\n");
        exit(1);
    }
    printf("SERVER : Connected to Server.\n");

    if (database != NULL)
    {
        printf("Entering database %s\n", database);
        snprintf(buffer, sizeof buffer, "USEDATABASE:%s:%s:%d", database, argv[2], idUser);
        send(clientSocket, buffer, strlen(buffer), 0);
    }

    while (1)
    {
        printf("Client : ");
        char input[256];
        char temp[256];
        char command[128][1024];
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

        int falseCommand = 0;
        if (strcmp(command[0], "CREATE") == 0)
        {
            if (strcmp(command[1], "USER") == 0 && strcmp(command[3], "IDENTIFIED") == 0 && strcmp(command[4], "BY") == 0)
            {
                snprintf(buffer, sizeof buffer, "CREATEUSER:%s:%s:%d", command[2], command[5], idUser);
                send(clientSocket, buffer, strlen(buffer), 0);
            }
            else if (strcmp(command[1], "DATABASE") == 0)
            {
                snprintf(buffer, sizeof buffer, "CREATEDATABASE:%s:%s:%d", command[2], argv[2], idUser);
                send(clientSocket, buffer, strlen(buffer), 0);
            }
            else if (strcmp(command[1], "TABLE") == 0)
            {
                snprintf(buffer, sizeof buffer, "CREATETABLE:%s", temp);
                send(clientSocket, buffer, strlen(buffer), 0);
            }
        }
        else if (strcmp(command[0], "GRANT") == 0 && strcmp(command[1], "PERMISSION") == 0 && strcmp(command[3], "INTO") == 0)
        {
            snprintf(buffer, sizeof buffer, "GRANTPERMISSION:%s:%s:%d", command[2], command[4], idUser);
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        else if (strcmp(command[0], "USE") == 0)
        {
            snprintf(buffer, sizeof buffer, "USEDATABASE:%s:%s:%d", command[1], argv[2], idUser);
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        else if (strcmp(command[0], "DROP") == 0)
        {
            if (strcmp(command[1], "DATABASE") == 0)
            {
                snprintf(buffer, sizeof buffer, "DROPDATABASE:%s:%s", command[2], argv[2]);
                send(clientSocket, buffer, strlen(buffer), 0);
            }
            else if (strcmp(command[1], "TABLE") == 0)
            {
                snprintf(buffer, sizeof buffer, "DROPTABLE:%s:%s", command[2], argv[2]);
                send(clientSocket, buffer, strlen(buffer), 0);
            }
            else if (strcmp(command[1], "COLUMN") == 0)
            {
                snprintf(buffer, sizeof buffer, "DROPCOLUMN:%s:%s:%s", command[2], command[4], argv[2]);
                send(clientSocket, buffer, strlen(buffer), 0);
            }
        }
        else if (strcmp(command[0], "INSERT") == 0 && strcmp(command[1], "INTO") == 0)
        {
            snprintf(buffer, sizeof buffer, "INSERT:%s", temp);
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        else if (strcmp(command[0], "UPDATE") == 0)
        {
            snprintf(buffer, sizeof buffer, "UPDATE:%s", temp);
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        else if (strcmp(command[0], "DELETE") == 0)
        {
            snprintf(buffer, sizeof buffer, "DELETE:%s", temp);
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        else if (strcmp(command[0], "SELECT") == 0)
        {
            snprintf(buffer, sizeof buffer, "SELECT:%s", temp);
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        else if (strcmp(command[0], "EXIT") != 0)
        {
            falseCommand = 1;
            char message[] = "Invalid Command";
            send(clientSocket, message, strlen(message), 0);
        }

        if (falseCommand != 1)
        {
            char sender[1024];
            if (idUser == 0)
                strcpy(sender, "root");
            else
                strcpy(sender, argv[2]);
            append_log(temp, sender);
        }

        if (strcmp(command[0], "EXIT") == 0)
        {
            send(clientSocket, command[0], strlen(command[0]), 0);
            close(clientSocket);
            printf("SERVER : Disconnected from server.\n");
            exit(1);
        }
        bzero(buffer, sizeof(buffer));
        // str_empty(buffer, sizeof buffer);
        if (recv(clientSocket, buffer, 1024, 0) < 0)
            printf("SERVER : Error in receiving data.\n");
        else
            printf("SERVER : %s\n", buffer);
    }

    return 0;
}