#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fnmatch.h>
#include <ctype.h>

#define PORT 8080

struct table
{
    int totalAttr;
    char type[128][1024];
    char data[128][1024];
};

struct permission
{
    char name[1024];
    char pass[1024];
};

struct permission_db
{
    char database[1024];
    char name[1024];
};

void str_empty(char *str1, size_t bufsize)
{
    strncpy(str1, "", bufsize);
    if (str1[bufsize - 1] != '\0')
        str1[bufsize] = '\0';
}

int is_user_exist(char *uname)
{
    FILE *file;
    char line[128];
    struct permission user;

    file = fopen("../database/databases/user.txt", "r");
    if (file == NULL)
    {
        printf("Failed to open file\n");
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        sscanf(line, "Username: %[^,]", user.name);

        if (strlen(uname) > 0 && uname[strlen(uname) - 1] == ';')
        {
            uname[strlen(uname) - 1] = '\0';
        }
        printf("%s", user.name);
        printf("%s", uname);
        int nameComparison = strcmp(user.name, uname);
        printf("Name Comparison: %d\n", nameComparison);

        if (nameComparison == 0)
        {
            fclose(file);
            return 1; // User exists
        }
    }

    fclose(file);
    return 0; // User does not exist
}

void create_user(char *name, char *pass)
{
    FILE *file;
    file = fopen("databases/user.txt", "a");

    if (file == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(file, "\nUsername: %s, Password: %s", name, pass);

    fclose(file);
}

int is_allowed_db(char *name, char *database)
{
    FILE *file;
    struct permission_db user;
    int mark = 0;
    if (strlen(database) > 0 && database[strlen(database) - 1] == ';')
    {
        database[strlen(database) - 1] = '\0';
    }
    // printf("name = %s  database = %s\n", name, database);
    file = fopen("databases/permission.txt", "r"); // Open the file in read mode
    if (file == NULL)
    {
        printf("[-] Error in opening file.\n");
        return 0;
    }
    while (fscanf(file, "%s %s", user.name, user.database) != EOF)
    {
        int nameComparison = strcmp(user.name, name);
        int passComparison = strcmp(user.database, database);

        printf("Name comparison result: %d\n", nameComparison);
        printf("Password comparison result: %d\n", passComparison);

        if (strcmp(user.name, name) == 0 && strcmp(user.database, database) == 0)
        {
            mark = 1;
            break;
        }
    }
    fclose(file);
    return mark;
}

void write_permission(char *name, char *database)
{
    struct permission_db user;

    strcpy(user.name, name);
    strcpy(user.database, database);

    printf("%s %s\n", user.name, user.database);

    char fname[] = {"databases/permission.txt"};
    FILE *file;
    file = fopen(fname, "a");
    fprintf(file, "\n%s %s", user.name, user.database);
    fclose(file);
}

int find_column(char *table, char *column)
{
    FILE *file;
    char line[128];
    struct table user;
    int id, mark = 0;
    printf("%s", table);
    file = fopen(table, "r"); // Open the file in read mode
    if (file == NULL)
    {
        printf("Error in opening file.\n");
        return -1;
    }

    int index = -1;
    int currentIndex = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        sscanf(line, "Data: %[^,], Type: %[^\n]", user.data, user.type);
        printf("data : %s", user.data);
        int nameComparison = strcmp(user.data, column);

        if (nameComparison == 0)
        {
            index = currentIndex;
            break;
        }

        currentIndex++;
    }

    if (feof(file))
    {
        printf("Reached end of file. ");
    }
    else
    {
        printf("Column '%s' found at index %d.\n", column, index);
    }

    fclose(file);
    return index;
}

int drop_attribute(char *table, int index)
{
    FILE *file, *file1;
    char line[128];
    file = fopen(table, "r");   // Open the file in read mode
    file1 = fopen("temp", "w"); // Open a temporary file in write mode

    if (file == NULL || file1 == NULL)
    {
        printf("Error in opening file.\n");
        return -1;
    }

    int currentLine = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (currentLine == 0)
        {
            fputs(line, file1);
        }
        else
        {
            if (currentLine != index)
            {
                fputs(line, file1);
            }
        }
        currentLine++;
    }

    fclose(file);
    fclose(file1);
    remove(table);         // Delete the original file
    rename("temp", table); // Rename the temporary file to the original file name

    return 0;
}

int drop_table(char *table, char *namaTable)
{
    FILE *file, *file1;
    struct table user;
    int id, mark = 0;
    file = fopen(table, "rb");
    file1 = fopen("temp", "ab");
    fread(&user, sizeof(user), 1, file);
    int index = -1;
    struct table tempUser;

    for (int i = 0; i < user.totalAttr; i++)
    {
        strcpy(tempUser.data[i], user.data[i]);
        strcpy(tempUser.type[i], user.type[i]);
    }

    tempUser.totalAttr = user.totalAttr;
    fwrite(&tempUser, sizeof(tempUser), 1, file1);
    fclose(file);
    fclose(file1);
    remove(table);
    rename("temp", table);
    return 1;
}

int main()
{
    int socketDesc, checkBind, newSocket;
    struct sockaddr_in serverAddress;
    struct sockaddr_in newAddress;

    socklen_t address_size;

    char buffer[1024];
    pid_t childpid;

    socketDesc = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDesc < 0)
    {
        printf("SERVER : Error connection.\n");
        exit(1);
    }
    printf("SERVER : Socket created.\n");

    memset(&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    checkBind = bind(socketDesc, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (checkBind < 0)
    {
        printf("SERVER : Error while binding.\n");
        exit(1);
    }
    printf("SERVER : Success bind to port.%d\n", PORT);

    if (listen(socketDesc, 10) == 0)
        printf("SERVER : Enter a Command.\n");
    else
        printf("SERVER : Error while binding.\n");

    while (1)
    {
        newSocket = accept(socketDesc, (struct sockaddr *)&newAddress, &address_size);

        if (newSocket < 0)
            exit(1);
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddress.sin_addr), ntohs(newAddress.sin_port));

        if ((childpid = fork()) == 0)
        {
            close(socketDesc);

            while (1)
            {
                recv(newSocket, buffer, 1024, 0);
                char *token;
                char temp_buff[32000];
                strcpy(temp_buff, buffer);
                char command[128][1024];
                token = strtok(temp_buff, ":");
                int i = 0;
                char currDB[1024];

                while (token != NULL)
                {
                    strcpy(command[i], token);
                    i++;
                    token = strtok(NULL, ":");
                }
                if (strcmp(command[0], "CREATEUSER") == 0)
                {
                    if (strcmp(command[3], "0") == 0) // Root
                        create_user(command[1], command[2]);
                    else
                    {
                        char message[] = "Request Denied!";
                        send(newSocket, message, strlen(message), 0);
                        str_empty(buffer, sizeof(buffer));
                    }
                }
                else if (strcmp(command[0], "GRANTPERMISSION") == 0)
                {
                    if (strcmp(command[3], "0") == 0) //root
                    {
                        int exist = is_user_exist(command[2]);
                        if (exist == 1)
                        {
                            if (strlen(command[2]) > 0 && command[2][strlen(command[2]) - 1] == ';')
                            {
                                command[2][strlen(command[2]) - 1] = '\0';
                            }
                            write_permission(command[2], command[1]);
                        }
                        else
                        {
                            char message[] = "User Not Found!";
                            send(newSocket, message, strlen(message), 0);
                            str_empty(buffer, sizeof(buffer));
                        }
                    }
                    else
                    {
                        char message[] = "Request Denied!";
                        send(newSocket, message, strlen(message), 0);
                        str_empty(buffer, sizeof(buffer));
                    }
                }
                else if (strcmp(command[0], "CREATEDATABASE") == 0)
                {
                    char location[2048];
                    if (strlen(command[1]) > 0 && command[1][strlen(command[1]) - 1] == ';')
                    {
                        command[1][strlen(command[1]) - 1] = '\0';
                    }
                    snprintf(location, sizeof location, "databases/%s", command[1]);
                    printf("location = %s, name = %s , database = %s\n", location, command[2], command[1]);
                    mkdir(location, 0777);
                    write_permission(command[2], command[1]);
                }
                else if (strcmp(command[0], "USEDATABASE") == 0)
                {
                    if (strcmp(command[3], "0") != 0)
                    {
                        int userAllowed = is_allowed_db(command[2], command[1]);
                        if (userAllowed != 1)
                        {
                            char message[] = "Request Denied!";
                            send(newSocket, message, strlen(message), 0);
                            str_empty(buffer, sizeof(buffer));
                        }
                        else
                        {
                            strncpy(currDB, command[1], sizeof(command[1]));
                            char message[] = "Access to Database : Allowed";
                            printf("currDB = %s\n", currDB);
                            send(newSocket, message, strlen(message), 0);
                            str_empty(buffer, sizeof(buffer));
                        }
                    }
                    else
                    {
                        strncpy(currDB, command[1], sizeof(command[1]));
                        char message[] = "Access to Database : Allowed";
                        printf("currDB = %s\n", currDB);
                        send(newSocket, message, strlen(message), 0);
                        str_empty(buffer, sizeof(buffer));
                    }
                }
                else if (strcmp(command[0], "CREATETABLE") == 0)
                {
                    printf("%s\n", command[1]);
                    char *currToken;

                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        str_empty(buffer, sizeof(buffer));
                    }
                    else
                    {
                        char query_list[128][1024];
                        char temp_cmd[2048];
                        snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                        currToken = strtok(temp_cmd, "(), ");
                        int total = 0;

                        while (currToken != NULL)
                        {
                            strcpy(query_list[total], currToken);
                            printf("%s\n", query_list[total]);
                            total++;
                            currToken = strtok(NULL, "(), ");
                        }

                        if (strlen(currDB) > 0 && currDB[strlen(currDB) - 1] == ';')
                        {
                            currDB[strlen(currDB) - 1] = '\0';
                        }

                        char createTable[2048];
                        if (strlen(query_list[2]) > 0 && query_list[2][strlen(query_list[2]) - 1] == ';')
                        {
                            query_list[2][strlen(query_list[2]) - 1] = '\0';
                        }
                        snprintf(createTable, sizeof createTable, "../database/databases/%s/%s", currDB, query_list[2]);
                        int iteration = 0;
                        int data_iteration = 3;
                        struct table column;

                        while (total > 3)
                        {
                            strcpy(column.data[iteration], query_list[data_iteration]);
                            printf("%s\n", column.data[iteration]);
                            strcpy(column.type[iteration], query_list[data_iteration + 1]);
                            data_iteration = data_iteration + 2;
                            total = total - 2;
                            iteration++;
                        }

                        column.totalAttr = iteration;
                        FILE *file;

                        printf("%s\n", createTable);
                        file = fopen(createTable, "w"); // Open the file in write mode
                        if (file == NULL)
                        {
                            printf("SERVER : Error while opening file.\n");
                            return 0;
                        }
                        iteration--;
                        fprintf(file, "Total Columns: %d\n", iteration--);
                        for (int i = 0; i <= iteration; i++)
                        {
                            fprintf(file, "Data: %s, Type: %s\n", column.data[i], column.type[i]);
                        }

                        fclose(file);
                    }
                }
                else if (strcmp(command[0], "DROPDATABASE") == 0)
                {
                    int userAllowed = is_allowed_db(command[2], command[1]);

                    if (userAllowed != 1)
                    {
                        char message[] = "Request Denied!";
                        send(newSocket, message, strlen(message), 0);
                        str_empty(buffer, sizeof(buffer));
                        continue;
                    }
                    else
                    {
                        char delete[2048];
                        snprintf(delete, sizeof delete, "rm -r databases/%s", command[1]);
                        system(delete);
                        char message[] = "Database Has Been Removed";
                        send(newSocket, message, strlen(message), 0);
                        str_empty(buffer, sizeof(buffer));
                    }
                }
                else if (strcmp(command[0], "DROPTABLE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        str_empty(buffer, sizeof(buffer));
                        continue;
                    }

                    char delete[2048];
                   if (strlen(command[1]) > 0)
                    {
                        command[1][strlen(command[1]) - 1] = '\0';
                    } 
                    snprintf(delete, sizeof delete, "databases/%s/%s", currDB, command[1]);
                    remove(delete);
                    char message[] = "Table Has Been Removed";
                    send(newSocket, message, strlen(message), 0);
                    str_empty(buffer, sizeof(buffer));
                }
                else if (strcmp(command[0], "DROPCOLUMN") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        str_empty(buffer, sizeof(buffer));
                        continue;
                    }

                    if (strlen(command[2]) > 0 && command[2][strlen(command[2]) - 1] == ';')
                    {
                        command[2][strlen(command[2]) - 1] = '\0';
                    }
                    if (strlen(currDB) > 0 && currDB[strlen(currDB) - 1] == ';')
                    {
                        currDB[strlen(currDB) - 1] = '\0';
                    }

                    char createTable[2048];
                    snprintf(createTable, sizeof createTable, "databases/%s/%s", currDB, command[2]);
                    // printf("1 : %s\n", command[1]);
                    // printf("2 : %s\n", command[2]);
                    int index = find_column(createTable, command[1]);

                    if (index == -1)
                    {
                        char message[] = "Column Not Found";
                        send(newSocket, message, strlen(message), 0);
                        str_empty(buffer, sizeof(buffer));
                        continue;
                    }

                    drop_attribute(createTable, index);
                    char message[] = "Column Has Been Removed";
                    send(newSocket, message, strlen(message), 0);
                    str_empty(buffer, sizeof(buffer));
                }
                else if (strcmp(command[0], "INSERT") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        str_empty(buffer, sizeof(buffer));
                        continue;
                    }
                }
                else if (strcmp(command[0], "UPDATE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        str_empty(buffer, sizeof(buffer));
                        continue;
                    }
                }
                else if (strcmp(command[0], "DELETE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        str_empty(buffer, sizeof(buffer));
                        continue;
                    }
                }
                else if (strcmp(command[0], "SELECT") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        str_empty(buffer, sizeof(buffer));
                        continue;
                    }
                }
                if (strcmp(buffer, "EXIT") == 0)
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(newAddress.sin_addr), ntohs(newAddress.sin_port));
                    break;
                }
                else
                {
                    printf("Client: %s\n", buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                    str_empty(buffer, sizeof(buffer));
                }
            }
        }
    }

    close(newSocket);

    return 0;
}
