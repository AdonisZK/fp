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
    printf("name = %s  database = %s\n", name, database);
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

int updateColumn(char *table, int index, char *ganti)
{
    FILE *file, *file1;
    struct table user;
    int id, mark = 0;
    file = fopen(table, "rb");
    file1 = fopen("temp", "ab");
    int dataInput = 0;

    while (1)
    {
        fread(&user, sizeof(user), 1, file);
        if (feof(file))
            break;

        struct table tempUser;
        int iteration = 0;
        for (int i = 0; i < user.totalAttr; i++)
        {
            if (i == index && dataInput != 0)
                strcpy(tempUser.data[iteration], ganti);
            else
                strcpy(tempUser.data[iteration], user.data[i]);

            printf("%s\n", tempUser.data[iteration]);
            strcpy(tempUser.type[iteration], user.type[i]);
            printf("%s\n", tempUser.data[iteration]);
            iteration++;
        }
        tempUser.totalAttr = user.totalAttr;
        fwrite(&tempUser, sizeof(tempUser), 1, file1);
        dataInput++;
    }
    fclose(file);
    fclose(file1);
    remove(table);
    rename("temp", table);
    return 0;
}

int updateColumnWhere(char *table, int index, char *ganti, int change_index, char *where)
{
    FILE *file, *file1;
    struct table user;
    int id, mark = 0;
    file = fopen(table, "rb");
    file1 = fopen("temp", "ab");
    int dataInput = 0;

    while (1)
    {
        fread(&user, sizeof(user), 1, file);
        if (feof(file))
            break;

        struct table tempUser;
        int iteration = 0;

        for (int i = 0; i < user.totalAttr; i++)
        {
            if (i == index && dataInput != 0 && strcmp(user.data[change_index], where) == 0)
                strcpy(tempUser.data[iteration], ganti);
            else
                strcpy(tempUser.data[iteration], user.data[i]);
            printf("%s\n", tempUser.data[iteration]);
            strcpy(tempUser.type[iteration], user.type[i]);
            printf("%s\n", tempUser.data[iteration]);
            iteration++;
        }

        tempUser.totalAttr = user.totalAttr;
        fwrite(&tempUser, sizeof(tempUser), 1, file1);
        dataInput++;
    }
    fclose(file);
    fclose(file1);
    remove(table);
    rename("temp", table);
    return 0;
}

int deleteTableWhere(char *table, int index, char *column, char *where)
{
    FILE *file, *file1;
    struct table user;
    int id, mark = 0;
    file = fopen(table, "rb");
    file1 = fopen("temp", "ab");
    int dataInput = 0;

    while (1)
    {
        mark = 0;
        fread(&user, sizeof(user), 1, file);

        if (feof(file))
            break;

        struct table tempUser;
        int iteration = 0;

        for (int i = 0; i < user.totalAttr; i++)
        {
            if (i == index && dataInput != 0 && strcmp(user.data[i], where) == 0)
                mark = 1;
            strcpy(tempUser.data[iteration], user.data[i]);
            printf("%s\n", tempUser.data[iteration]);
            strcpy(tempUser.type[iteration], user.type[i]);
            printf("%s\n", tempUser.data[iteration]);
            iteration++;
        }

        tempUser.totalAttr = user.totalAttr;
        if (mark != 1)
            fwrite(&tempUser, sizeof(tempUser), 1, file1);
        dataInput++;
    }
    fclose(file);
    fclose(file1);
    remove(table);
    rename("temp", table);
    return 0;
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
                    if (strcmp(command[3], "0") == 0)
                        create_user(command[1], command[2]);
                    else
                    {
                        char message[] = "Request Denied!";
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
                    }
                }
                else if (strcmp(command[0], "GRANTPERMISSION") == 0)
                {
                    if (strcmp(command[3], "0") == 0)
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
                            bzero(buffer, sizeof(buffer));
                        }
                    }
                    else
                    {
                        char message[] = "Request Denied!";
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
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
                            bzero(buffer, sizeof(buffer));
                        }
                        else
                        {
                            strncpy(currDB, command[1], sizeof(command[1]));
                            char message[] = "Access to Database : Allowed";
                            printf("currDB = %s\n", currDB);
                            send(newSocket, message, strlen(message), 0);
                            bzero(buffer, sizeof(buffer));
                        }
                    }
                    else
                    {
                        strncpy(currDB, command[1], sizeof(command[1]));
                        char message[] = "Access to Database : Allowed";
                        printf("currDB = %s\n", currDB);
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
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
                        bzero(buffer, sizeof(buffer));
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
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }
                    else
                    {
                        char delete[2048];
                        snprintf(delete, sizeof delete, "rm -r databases/%s", command[1]);
                        system(delete);
                        char message[] = "Database Has Been Removed";
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
                    }
                }
                else if (strcmp(command[0], "DROPTABLE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        bzero(buffer, sizeof(buffer));
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
                    bzero(buffer, sizeof(buffer));
                }
                else if (strcmp(command[0], "DROPCOLUMN") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        bzero(buffer, sizeof(buffer));
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
                    printf("1 : %s\n", command[1]);
                    printf("2 : %s\n", command[2]);
                    int index = find_column(createTable, command[1]);

                    if (index == -1)
                    {
                        char message[] = "Column Not Found";
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }

                    drop_attribute(createTable, index);
                    char message[] = "Column Has Been Removed";
                    send(newSocket, message, strlen(message), 0);
                    bzero(buffer, sizeof(buffer));
                }
                else if (strcmp(command[0], "INSERT") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }

                    char query_list[128][1024];
                    char temp_cmd[2048];

                    snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                    char *currToken;
                    currToken = strtok(temp_cmd, "\'(), ");
                    int total = 0;

                    while (currToken != NULL)
                    {
                        strcpy(query_list[total], currToken);
                        total++;
                        currToken = strtok(NULL, "\'(), ");
                    }

                    char createTable[2048];
                    if (strlen(query_list[2]) > 0 && query_list[2][strlen(query_list[2]) - 1] == ';')
                    {
                        query_list[2][strlen(query_list[2]) - 1] = '\0';
                    }
                    if (strlen(currDB) > 0 && currDB[strlen(currDB) - 1] == ';')
                    {
                        currDB[strlen(currDB) - 1] = '\0';
                    }
                    snprintf(createTable, sizeof createTable, "databases/%s/%s", currDB, query_list[2]);
                    FILE *file;
                    int total_column;
                    file = fopen(createTable, "r");

                    if (file == NULL)
                    {
                        char message[] = "Table Not Found";
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }
                    else
                    {
                        struct table user;
                        fread(&user, sizeof(user), 1, file);
                        total_column = user.totalAttr;
                        fclose(file);
                    }

                    int iteration = 0;
                    int data_iteration = 3;
                    struct table column;

                    while (total > 3)
                    {
                        strcpy(column.data[iteration], query_list[data_iteration]);
                        printf("%s\n", column.data[iteration]);
                        strcpy(column.type[iteration], "string");
                        data_iteration++;
                        total = total - 1;
                        iteration++;
                    }

                    column.totalAttr = iteration;
                    if (total_column != column.totalAttr)
                    {
                        char message[] = "Input Missmatch";
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }

                    printf("iteration = %d\n", iteration);
                    FILE *file1;
                    printf("%s\n", createTable);
                    file1 = fopen(createTable, "a");
                    while (total > 3)
                    {
                        fprintf(file1, "Data: %s, Type: %s\n", query_list[data_iteration], "string");
                        data_iteration++;
                        total = total - 1;
                        iteration++;
                    }
                    fclose(file1);
                    char message[] = "Data Has Been Inserted";
                    send(newSocket, message, strlen(message), 0);
                    bzero(buffer, sizeof(buffer));
                }
                else if (strcmp(command[0], "UPDATE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }
                    char query_list[128][1024];
                    char temp_cmd[2048];
                    snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                    char *currToken;
                    currToken = strtok(temp_cmd, "\'(),= ");
                    int total = 0;
                    while (currToken != NULL)
                    {
                        strcpy(query_list[total], currToken);
                        printf("%s\n", query_list[total]);
                        total++;
                        currToken = strtok(NULL, "\'(),= ");
                    }
                    printf("total = %d\n", total);
                    char createTable[2048];
                    snprintf(createTable, sizeof createTable, "databases/%s/%s", currDB, query_list[1]);
                    if (total == 5)
                    {
                        printf("buat table = %s, kolumn = %s", createTable, query_list[3]);
                        int index = find_column(createTable, query_list[3]);
                        if (index == -1)
                        {
                            char message[] = "Column Not Found";
                            send(newSocket, message, strlen(message), 0);
                            bzero(buffer, sizeof(buffer));
                            continue;
                        }
                        printf("index = %d\n", index);
                        updateColumn(createTable, index, query_list[4]);
                    }
                    else if (total == 8)
                    {
                        printf("buat table = %s, kolumn = %s", createTable, query_list[3]);
                        int index = find_column(createTable, query_list[3]);
                        if (index == -1)
                        {
                            char message[] = "Column Not Found";
                            send(newSocket, message, strlen(message), 0);
                            bzero(buffer, sizeof(buffer));
                            continue;
                        }
                        printf("%s\n", query_list[7]);
                        int change_index = find_column(createTable, query_list[6]);
                        updateColumnWhere(createTable, index, query_list[4], change_index, query_list[7]);
                    }
                    else
                    {
                        char message[] = "Data Has Been Deleted";
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }
                    char message[] = "Data Has Been Updated";
                    send(newSocket, message, strlen(message), 0);
                    bzero(buffer, sizeof(buffer));
                }
                else if (strcmp(command[0], "DELETE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }
                    char query_list[128][1024];
                    char temp_cmd[2048];
                    snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                    char *currToken;
                    currToken = strtok(temp_cmd, "\'(),= ");
                    int total = 0;
                    while (currToken != NULL)
                    {
                        strcpy(query_list[total], currToken);
                        printf("%s\n", query_list[total]);
                        total++;
                        currToken = strtok(NULL, "\'(),= ");
                    }
                    printf("total = %d\n", total);
                    char createTable[2048];
                    snprintf(createTable, sizeof createTable, "databases/%s/%s", currDB, query_list[2]);
                    if (total == 3)
                    {
                        drop_table(createTable, query_list[2]);
                    }
                    else if (total == 6)
                    {
                        int index = find_column(createTable, query_list[4]);
                        if (index == -1)
                        {
                            char message[] = "Column Not Found";
                            send(newSocket, message, strlen(message), 0);
                            bzero(buffer, sizeof(buffer));
                            continue;
                        }
                        printf("index  = %d\n", index);
                        deleteTableWhere(createTable, index, query_list[4], query_list[5]);
                    }
                    else
                    {
                        char message[] = "Wrong input";
                        send(newSocket, message, strlen(message), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }
                    char message[] = "Data Has Been Deleted";
                    send(newSocket, message, strlen(message), 0);
                    bzero(buffer, sizeof(buffer));
                }
                else if (strcmp(command[0], "SELECT") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "Select a Database First");
                        send(newSocket, currDB, strlen(currDB), 0);
                        bzero(buffer, sizeof(buffer));
                        continue;
                    }
                    char query_list[128][1024];
                    char temp_cmd[2048];
                    snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                    char *currToken;
                    currToken = strtok(temp_cmd, "\'(),= ");
                    int total = 0;
                    while (currToken != NULL)
                    {
                        strcpy(query_list[total], currToken);
                        printf("%s\n", query_list[total]);
                        total++;
                        currToken = strtok(NULL, "\'(),= ");
                    }
                    printf("ABC\n");
                    if (total == 4)
                    {
                        char createTable[2048];
                        snprintf(createTable, sizeof createTable, "databases/%s/%s", currDB, query_list[3]);
                        printf("buat table = %s", createTable);
                        char perintahKolom[1024];
                        printf("masuk 4\n");
                        if (strcmp(query_list[1], "*") == 0)
                        {
                            FILE *file, *file1;
                            struct table user;
                            int id, mark = 0;
                            file = fopen(createTable, "rb");
                            char buffers[40000];
                            char sendDatabase[40000];
                            bzero(buffer, sizeof(buffer));
                            bzero(sendDatabase, sizeof(sendDatabase));
                            while (1)
                            {
                                char enter[] = "\n";
                                fread(&user, sizeof(user), 1, file);
                                snprintf(buffers, sizeof buffers, "\n");
                                if (feof(file))
                                {
                                    break;
                                }
                                for (int i = 0; i < user.totalAttr; i++)
                                {
                                    char padding[2048];
                                    snprintf(padding, sizeof padding, "%s\t", user.data[i]);
                                    strcat(buffers, padding);
                                }
                                strcat(sendDatabase, buffers);
                            }
                            send(newSocket, sendDatabase, strlen(sendDatabase), 0);
                            bzero(sendDatabase, sizeof(sendDatabase));
                            bzero(buffer, sizeof(buffer));
                            fclose(file);
                        }
                        else
                        {
                            int index = find_column(createTable, query_list[1]);
                            printf("%d\n", index);
                            FILE *file, *file1;
                            struct table user;
                            int id, mark = 0;
                            file = fopen(createTable, "rb");
                            char buffers[40000];
                            char sendDatabase[40000];
                            bzero(buffer, sizeof(buffer));
                            bzero(sendDatabase, sizeof(sendDatabase));
                            while (1)
                            {
                                char enter[] = "\n";
                                fread(&user, sizeof(user), 1, file);
                                snprintf(buffers, sizeof buffers, "\n");
                                if (feof(file))
                                {
                                    break;
                                }
                                for (int i = 0; i < user.totalAttr; i++)
                                {
                                    if (i == index)
                                    {
                                        char padding[2048];
                                        snprintf(padding, sizeof padding, "%s\t", user.data[i]);
                                        strcat(buffers, padding);
                                    }
                                }
                                strcat(sendDatabase, buffers);
                            }
                            printf("ini send fix\n%s\n", sendDatabase);
                            fclose(file);
                            send(newSocket, sendDatabase, strlen(sendDatabase), 0);
                            bzero(sendDatabase, sizeof(sendDatabase));
                            bzero(buffer, sizeof(buffer));
                        }
                    }
                    else if (total == 7 && strcmp(query_list[4], "WHERE") == 0)
                    {
                        char createTable[2048];
                        snprintf(createTable, sizeof createTable, "databases/%s/%s", currDB, query_list[3]);
                        printf("buat table = %s", createTable);
                        char perintahKolom[1024];
                        printf("masuk 4\n");
                        if (strcmp(query_list[1], "*") == 0)
                        {
                            FILE *file, *file1;
                            struct table user;
                            int id, mark = 0;
                            file = fopen(createTable, "rb");
                            char buffers[40000];
                            char sendDatabase[40000];
                            int index = find_column(createTable, query_list[5]);
                            printf("%d\n", index);
                            bzero(buffer, sizeof(buffer));
                            bzero(sendDatabase, sizeof(sendDatabase));
                            while (1)
                            {
                                char enter[] = "\n";
                                fread(&user, sizeof(user), 1, file);
                                snprintf(buffers, sizeof buffers, "\n");
                                if (feof(file))
                                {
                                    break;
                                }
                                for (int i = 0; i < user.totalAttr; i++)
                                {
                                    if (strcmp(user.data[index], query_list[6]) == 0)
                                    {
                                        char padding[2048];
                                        snprintf(padding, sizeof padding, "%s\t", user.data[i]);
                                        strcat(buffers, padding);
                                    }
                                }
                                strcat(sendDatabase, buffers);
                            }
                            send(newSocket, sendDatabase, strlen(sendDatabase), 0);
                            bzero(sendDatabase, sizeof(sendDatabase));
                            bzero(buffer, sizeof(buffer));
                            fclose(file);
                        }
                        else
                        {
                            int index = find_column(createTable, query_list[1]);
                            printf("%d\n", index);
                            FILE *file, *file1;
                            struct table user;
                            int id, mark = 0;
                            int change_index = find_column(createTable, query_list[5]);
                            file = fopen(createTable, "rb");
                            char buffers[40000];
                            char sendDatabase[40000];
                            bzero(buffer, sizeof(buffer));
                            bzero(sendDatabase, sizeof(sendDatabase));
                            while (1)
                            {
                                char enter[] = "\n";
                                fread(&user, sizeof(user), 1, file);
                                snprintf(buffers, sizeof buffers, "\n");
                                if (feof(file))
                                {
                                    break;
                                }
                                for (int i = 0; i < user.totalAttr; i++)
                                {
                                    if (i == index && (strcmp(user.data[change_index], query_list[6]) == 0 || strcmp(user.data[i], query_list[5]) == 0))
                                    {
                                        char padding[2048];
                                        snprintf(padding, sizeof padding, "%s\t", user.data[i]);
                                        strcat(buffers, padding);
                                    }
                                }
                                strcat(sendDatabase, buffers);
                            }
                            printf("ini send fix\n%s\n", sendDatabase);
                            fclose(file);
                            send(newSocket, sendDatabase, strlen(sendDatabase), 0);
                            bzero(sendDatabase, sizeof(sendDatabase));
                            bzero(buffer, sizeof(buffer));
                        }
                    }
                    else
                    {
                        printf("ini query 3 %s", query_list[total - 3]);
                        if (strcmp(query_list[total - 3], "WHERE") != 0)
                        {
                            char createTable[2048];
                            snprintf(createTable, sizeof createTable, "databases/%s/%s", currDB, query_list[total - 1]);
                            printf("buat table = %s", createTable);
                            printf("tanpa where");
                            int index[128];
                            int iteration = 0;
                            for (int i = 1; i < total - 2; i++)
                            {
                                index[iteration] = find_column(createTable, query_list[i]);
                                printf("%d\n", index[iteration]);
                                iteration++;
                            }
                        }
                        else if (strcmp(query_list[total - 3], "WHERE") == 0)
                        {
                            printf("dengan where");
                        }
                    }
                }
                if (strcmp(buffer, ":exit") == 0)
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(newAddress.sin_addr), ntohs(newAddress.sin_port));
                    break;
                }
                else
                {
                    printf("Client: %s\n", buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                    bzero(buffer, sizeof(buffer));
                }
            }
        }
    }

    close(newSocket);

    return 0;
}
