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

#define PORT 5050

struct table
{
    int tot_column;
    char type[100][1000];
    char data[100][1000];
};

struct permission
{
    char name[1000];
    char pass[1000];
};

struct permission_db
{
    char database[1000];
    char name[1000];
};

int isUserExist(char *uname)
{
    FILE *file;
    char line[128];
    struct permission user;

    file = fopen("../database/databases/user.txt", "r");
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        sscanf(line, "Username: %[^,]", user.name);

        strcat(user.name, ";");
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

void createUser(char *name, char *pass)
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

int isAllowedDb(char *name, char *database)
{
    FILE *file;
    struct permission_db user;
    int mark = 0;
    if (strlen(database) > 0)
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

void insertPermission(char *name, char *database)
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

int findColumn(char *table, char *column)
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

int deleteColumn(char *table, int index)
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
        // if (currentLine == 0)
        // {
        //     // Write the total columns line to the temporary file
        //     fputs(line, file1);
        // }
        // else
        // {
        // Skip the line if it corresponds to the specified index
        if (currentLine != index)
        {
            fputs(line, file1);
        }
        // }

        currentLine++;
    }

    fclose(file);
    fclose(file1);
    remove(table);         // Delete the original file
    rename("temp", table); // Rename the temporary file to the original file name

    return 0;
}

int deleteTable(char *table, char *namaTable)
{
    FILE *file, *file1;
    struct table user;
    int id, mark = 0;
    file = fopen(table, "rb");
    file1 = fopen("temp", "ab");
    fread(&user, sizeof(user), 1, file);
    int index = -1;
    struct table temp_user;

    for (int i = 0; i < user.tot_column; i++)
    {
        strcpy(temp_user.data[i], user.data[i]);
        strcpy(temp_user.type[i], user.type[i]);
    }

    temp_user.tot_column = user.tot_column;
    fwrite(&temp_user, sizeof(temp_user), 1, file1);
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
    int data_take = 0;

    while (1)
    {
        fread(&user, sizeof(user), 1, file);
        if (feof(file))
            break;

        struct table temp_user;
        int iteration = 0;
        for (int i = 0; i < user.tot_column; i++)
        {
            if (i == index && data_take != 0)
                strcpy(temp_user.data[iteration], ganti);
            else
                strcpy(temp_user.data[iteration], user.data[i]);

            printf("%s\n", temp_user.data[iteration]);
            strcpy(temp_user.type[iteration], user.type[i]);
            printf("%s\n", temp_user.data[iteration]);
            iteration++;
        }
        temp_user.tot_column = user.tot_column;
        fwrite(&temp_user, sizeof(temp_user), 1, file1);
        data_take++;
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
    int data_take = 0;

    while (1)
    {
        fread(&user, sizeof(user), 1, file);
        if (feof(file))
            break;

        struct table temp_user;
        int iteration = 0;

        for (int i = 0; i < user.tot_column; i++)
        {
            if (i == index && data_take != 0 && strcmp(user.data[change_index], where) == 0)
                strcpy(temp_user.data[iteration], ganti);
            else
                strcpy(temp_user.data[iteration], user.data[i]);
            printf("%s\n", temp_user.data[iteration]);
            strcpy(temp_user.type[iteration], user.type[i]);
            printf("%s\n", temp_user.data[iteration]);
            iteration++;
        }

        temp_user.tot_column = user.tot_column;
        fwrite(&temp_user, sizeof(temp_user), 1, file1);
        data_take++;
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
    int data_take = 0;

    while (1)
    {
        mark = 0;
        fread(&user, sizeof(user), 1, file);

        if (feof(file))
            break;

        struct table temp_user;
        int iteration = 0;

        for (int i = 0; i < user.tot_column; i++)
        {
            if (i == index && data_take != 0 && strcmp(user.data[i], where) == 0)
                mark = 1;
            strcpy(temp_user.data[iteration], user.data[i]);
            printf("%s\n", temp_user.data[iteration]);
            strcpy(temp_user.type[iteration], user.type[i]);
            printf("%s\n", temp_user.data[iteration]);
            iteration++;
        }

        temp_user.tot_column = user.tot_column;
        if (mark != 1)
            fwrite(&temp_user, sizeof(temp_user), 1, file1);
        data_take++;
    }
    fclose(file);
    fclose(file1);
    remove(table);
    rename("temp", table);
    return 0;
}

void writelog(char *command, char *name)
{
    time_t times;
    struct tm *info;
    time(&times);
    info = localtime(&times);

    char info_log[1000];

    FILE *file;
    file = fopen("logUser.log", "ab");

    sprintf(info_log, "%d-%.2d-%.2d %.2d:%.2d:%.2d::%s::%s\n", info->tm_year + 1900, info->tm_mon + 1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, name, command);

    fputs(info_log, file);
    fclose(file);
    return;
}

int main()
{

    int sockfd, ret;
    struct sockaddr_in serverAddr;

    int new_socket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;

    char buff[1024];
    pid_t childpid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[-]Error connection.\n");
        exit(1);
    }
    printf("[+]Socket created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("[-] Error in binding.\n");
        exit(1);
    }
    printf("[+] Bind to port %d\n", PORT);

    if (listen(sockfd, 10) == 0)
        printf("[+] Listening....\n");
    else
        printf("[-] Error in binding.\n");

    while (1)
    {
        new_socket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);

        if (new_socket < 0)
            exit(1);
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        if ((childpid = fork()) == 0)
        {
            close(sockfd);

            while (1)
            {
                recv(new_socket, buff, 1024, 0);
                char *token;
                char temp_buff[32000];
                strcpy(temp_buff, buff);
                char command[100][1000];
                token = strtok(temp_buff, ":");
                int i = 0;
                char currDB[1000];

                while (token != NULL)
                {
                    strcpy(command[i], token);
                    i++;
                    token = strtok(NULL, ":");
                }
                if (strcmp(command[0], "CREATEUSER") == 0)
                {
                    if (strcmp(command[3], "0") == 0)
                        createUser(command[1], command[2]);
                    else
                    {
                        char warning[] = "Not Allowed: Not have permission";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                    }
                }
                else if (strcmp(command[0], "GRANTPERMISSION") == 0)
                {
                    if (strcmp(command[3], "0") == 0)
                    {
                        int exist = isUserExist(command[2]);
                        if (exist == 1)
                        {
                            if (strlen(command[2]) > 0)
                            {
                                command[2][strlen(command[2]) - 1] = '\0';
                            }
                            insertPermission(command[2], command[1]);
                        }
                        else
                        {
                            char warning[] = "User Not Found";
                            send(new_socket, warning, strlen(warning), 0);
                            bzero(buff, sizeof(buff));
                        }
                    }
                    else
                    {
                        char warning[] = "Not Allowed: Not have permission";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                    }
                }
                else if (strcmp(command[0], "CREATEDATABASE") == 0)
                {
                    char loc[2000];
                    if (strlen(command[1]) > 0)
                    {
                        command[1][strlen(command[1]) - 1] = '\0';
                    }
                    snprintf(loc, sizeof loc, "databases/%s", command[1]);
                    printf("location = %s, name = %s , database = %s\n", loc, command[2], command[1]);
                    mkdir(loc, 0777);
                    insertPermission(command[2], command[1]);
                }
                else if (strcmp(command[0], "USEDATABASE") == 0)
                {
                    if (strcmp(command[3], "0") != 0)
                    {
                        int userAllowed = isAllowedDb(command[2], command[1]);
                        printf("1 : %s", command[2]);
                        printf("2 : %s", command[1]);
                        if (userAllowed != 1)
                        {
                            char warning[] = "Access_database : You're Not Allowed";
                            send(new_socket, warning, strlen(warning), 0);
                            bzero(buff, sizeof(buff));
                        }
                        else
                        {
                            strncpy(currDB, command[1], sizeof(command[1]));
                            char warning[] = "Access_database : Allowed";
                            printf("currDB = %s\n", currDB);
                            send(new_socket, warning, strlen(warning), 0);
                            bzero(buff, sizeof(buff));
                        }
                    }
                }
                else if (strcmp(command[0], "CREATETABLE") == 0)
                {
                    printf("%s\n", command[1]);
                    char *toks;

                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "You're not selecting database yet");
                        send(new_socket, currDB, strlen(currDB), 0);
                        bzero(buff, sizeof(buff));
                    }
                    else
                    {
                        char query_list[100][1000];
                        char temp_cmd[2000];
                        snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                        toks = strtok(temp_cmd, "(), ");
                        int total = 0;

                        while (toks != NULL)
                        {
                            strcpy(query_list[total], toks);
                            printf("%s\n", query_list[total]);
                            total++;
                            toks = strtok(NULL, "(), ");
                        }

                        char create_table[2000];
                        if (strlen(query_list[2]) > 0)
                        {
                            query_list[2][strlen(query_list[2]) - 1] = '\0';
                        }
                        snprintf(create_table, sizeof create_table, "../database/databases/%s/%s", currDB, query_list[2]);
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

                        column.tot_column = iteration;
                        FILE *file;
                        printf("%s\n", create_table);
                        file = fopen(create_table, "w"); // Open the file in write mode
                        if (file == NULL)
                        {
                            printf("[-] Error in opening file.\n");
                            return 0;
                        }
                        iteration--;
                        // fprintf(file, "Total Columns: %d\n", iteration--);
                        for (int i = 0; i < iteration; i++)
                        {
                            fprintf(file, "Data: %s, Type: %s\n", column.data[i], column.type[i]);
                        }

                        fclose(file);
                    }
                }
                else if (strcmp(command[0], "DROPDATABASE") == 0)
                {
                    int userAllowed = isAllowedDb(command[2], command[1]);

                    if (userAllowed != 1)
                    {
                        char warning[] = "Access_database : Not Allowed, no permission";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }
                    else
                    {
                        char delete[2000];
                        snprintf(delete, sizeof delete, "rm -r databases/%s", command[1]);
                        system(delete);
                        char warning[] = "Database Has Been Removed";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                    }
                }
                else if (strcmp(command[0], "DROPTABLE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "You're not selecting database yet");
                        send(new_socket, currDB, strlen(currDB), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }

                    char delete[2000];
                    if (strlen(command[1]) > 0)
                    {
                        command[1][strlen(command[1]) - 1] = '\0';
                    }
                    snprintf(delete, sizeof delete, "databases/%s/%s", currDB, command[1]);
                    remove(delete);
                    char warning[] = "Table Has Been Removed";
                    send(new_socket, warning, strlen(warning), 0);
                    bzero(buff, sizeof(buff));
                }
                else if (strcmp(command[0], "DROPCOLUMN") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "You're not selecting database yet");
                        send(new_socket, currDB, strlen(currDB), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }

                    if (strlen(command[2]) > 0)
                    {
                        command[2][strlen(command[2]) - 1] = '\0';
                    }

                    char create_table[2000];
                    snprintf(create_table, sizeof create_table, "databases/%s/%s", currDB, command[2]);
                    printf("1 : %s\n", command[1]);
                    printf("2 : %s\n", command[2]);
                    int index = findColumn(create_table, command[1]);

                    if (index == -1)
                    {
                        char warning[] = "Column Not Found";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }

                    deleteColumn(create_table, index);
                    char warning[] = "Column Has Been Removed";
                    send(new_socket, warning, strlen(warning), 0);
                    bzero(buff, sizeof(buff));
                }
                else if (strcmp(command[0], "INSERT") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "You're not selecting database yet");
                        send(new_socket, currDB, strlen(currDB), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }

                    char query_list[100][1000];
                    char temp_cmd[2000];
                    snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                    char *toks;
                    toks = strtok(temp_cmd, "\'(), ");
                    int total = 0;

                    while (toks != NULL)
                    {
                        strcpy(query_list[total], toks);
                        total++;
                        toks = strtok(NULL, "\'(), ");
                    }

                    char create_table[2000];
                    snprintf(create_table, sizeof create_table, "databases/%s/%s", currDB, query_list[2]);
                    FILE *file;
                    int total_column;
                    file = fopen(create_table, "r");

                    if (file == NULL)
                    {
                        char warning[] = "TABLE NOT FOUND";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }
                    else
                    {
                        struct table user;
                        fread(&user, sizeof(user), 1, file);
                        total_column = user.tot_column;
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

                    column.tot_column = iteration;
                    if (total_column != column.tot_column)
                    {
                        char warning[] = "YOUR INPUT NOT MATCH THE COLUMN";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }

                    printf("iteration = %d\n", iteration);
                    FILE *file1;
                    printf("%s\n", create_table);
                    file1 = fopen(create_table, "ab");
                    fwrite(&column, sizeof(column), 1, file1);
                    fclose(file1);
                    char warning[] = "Data Has Been Inserted";
                    send(new_socket, warning, strlen(warning), 0);
                    bzero(buff, sizeof(buff));
                }
                else if (strcmp(command[0], "UPDATE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "You're not selecting database yet");
                        send(new_socket, currDB, strlen(currDB), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }
                    char query_list[100][1000];
                    char temp_cmd[2000];
                    snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                    char *toks;
                    toks = strtok(temp_cmd, "\'(),= ");
                    int total = 0;
                    while (toks != NULL)
                    {
                        strcpy(query_list[total], toks);
                        printf("%s\n", query_list[total]);
                        total++;
                        toks = strtok(NULL, "\'(),= ");
                    }
                    printf("total = %d\n", total);
                    char create_table[2000];
                    snprintf(create_table, sizeof create_table, "databases/%s/%s", currDB, query_list[1]);
                    if (total == 5)
                    {
                        printf("buat table = %s, kolumn = %s", create_table, query_list[3]);
                        int index = findColumn(create_table, query_list[3]);
                        if (index == -1)
                        {
                            char warning[] = "Column Not Found";
                            send(new_socket, warning, strlen(warning), 0);
                            bzero(buff, sizeof(buff));
                            continue;
                        }
                        printf("index = %d\n", index);
                        updateColumn(create_table, index, query_list[4]);
                    }
                    else if (total == 8)
                    {
                        printf("buat table = %s, kolumn = %s", create_table, query_list[3]);
                        int index = findColumn(create_table, query_list[3]);
                        if (index == -1)
                        {
                            char warning[] = "Column Not Found";
                            send(new_socket, warning, strlen(warning), 0);
                            bzero(buff, sizeof(buff));
                            continue;
                        }
                        printf("%s\n", query_list[7]);
                        int change_index = findColumn(create_table, query_list[6]);
                        updateColumnWhere(create_table, index, query_list[4], change_index, query_list[7]);
                    }
                    else
                    {
                        char warning[] = "Data Has Been Deleted";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }
                    char warning[] = "Data Has Been Updated";
                    send(new_socket, warning, strlen(warning), 0);
                    bzero(buff, sizeof(buff));
                }
                else if (strcmp(command[0], "DELETE") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "You're not selecting database yet");
                        send(new_socket, currDB, strlen(currDB), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }
                    char query_list[100][1000];
                    char temp_cmd[2000];
                    snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                    char *toks;
                    toks = strtok(temp_cmd, "\'(),= ");
                    int total = 0;
                    while (toks != NULL)
                    {
                        strcpy(query_list[total], toks);
                        printf("%s\n", query_list[total]);
                        total++;
                        toks = strtok(NULL, "\'(),= ");
                    }
                    printf("total = %d\n", total);
                    char create_table[2000];
                    snprintf(create_table, sizeof create_table, "databases/%s/%s", currDB, query_list[2]);
                    if (total == 3)
                    {
                        deleteTable(create_table, query_list[2]);
                    }
                    else if (total == 6)
                    {
                        int index = findColumn(create_table, query_list[4]);
                        if (index == -1)
                        {
                            char warning[] = "Column Not Found";
                            send(new_socket, warning, strlen(warning), 0);
                            bzero(buff, sizeof(buff));
                            continue;
                        }
                        printf("index  = %d\n", index);
                        deleteTableWhere(create_table, index, query_list[4], query_list[5]);
                    }
                    else
                    {
                        char warning[] = "Wrong input";
                        send(new_socket, warning, strlen(warning), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }
                    char warning[] = "Data Has Been Deleted";
                    send(new_socket, warning, strlen(warning), 0);
                    bzero(buff, sizeof(buff));
                }
                else if (strcmp(command[0], "SELECT") == 0)
                {
                    if (currDB[0] == '\0')
                    {
                        strcpy(currDB, "You're not selecting database yet");
                        send(new_socket, currDB, strlen(currDB), 0);
                        bzero(buff, sizeof(buff));
                        continue;
                    }
                    char query_list[100][1000];
                    char temp_cmd[2000];
                    snprintf(temp_cmd, sizeof temp_cmd, "%s", command[1]);
                    char *toks;
                    toks = strtok(temp_cmd, "\'(),= ");
                    int total = 0;
                    while (toks != NULL)
                    {
                        strcpy(query_list[total], toks);
                        printf("%s\n", query_list[total]);
                        total++;
                        toks = strtok(NULL, "\'(),= ");
                    }
                    printf("ABC\n");
                    if (total == 4)
                    {
                        char create_table[2000];
                        snprintf(create_table, sizeof create_table, "databases/%s/%s", currDB, query_list[3]);
                        printf("buat table = %s", create_table);
                        char perintahKolom[1000];
                        printf("masuk 4\n");
                        if (strcmp(query_list[1], "*") == 0)
                        {
                            FILE *file, *file1;
                            struct table user;
                            int id, mark = 0;
                            file = fopen(create_table, "rb");
                            char buffers[40000];
                            char sendDatabase[40000];
                            bzero(buff, sizeof(buff));
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
                                for (int i = 0; i < user.tot_column; i++)
                                {
                                    char padding[2000];
                                    snprintf(padding, sizeof padding, "%s\t", user.data[i]);
                                    strcat(buffers, padding);
                                }
                                strcat(sendDatabase, buffers);
                            }
                            send(new_socket, sendDatabase, strlen(sendDatabase), 0);
                            bzero(sendDatabase, sizeof(sendDatabase));
                            bzero(buff, sizeof(buff));
                            fclose(file);
                        }
                        else
                        {
                            int index = findColumn(create_table, query_list[1]);
                            printf("%d\n", index);
                            FILE *file, *file1;
                            struct table user;
                            int id, mark = 0;
                            file = fopen(create_table, "rb");
                            char buffers[40000];
                            char sendDatabase[40000];
                            bzero(buff, sizeof(buff));
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
                                for (int i = 0; i < user.tot_column; i++)
                                {
                                    if (i == index)
                                    {
                                        char padding[2000];
                                        snprintf(padding, sizeof padding, "%s\t", user.data[i]);
                                        strcat(buffers, padding);
                                    }
                                }
                                strcat(sendDatabase, buffers);
                            }
                            printf("ini send fix\n%s\n", sendDatabase);
                            fclose(file);
                            send(new_socket, sendDatabase, strlen(sendDatabase), 0);
                            bzero(sendDatabase, sizeof(sendDatabase));
                            bzero(buff, sizeof(buff));
                        }
                    }
                    else if (total == 7 && strcmp(query_list[4], "WHERE") == 0)
                    {
                        char create_table[2000];
                        snprintf(create_table, sizeof create_table, "databases/%s/%s", currDB, query_list[3]);
                        printf("buat table = %s", create_table);
                        char perintahKolom[1000];
                        printf("masuk 4\n");
                        if (strcmp(query_list[1], "*") == 0)
                        {
                            FILE *file, *file1;
                            struct table user;
                            int id, mark = 0;
                            file = fopen(create_table, "rb");
                            char buffers[40000];
                            char sendDatabase[40000];
                            int index = findColumn(create_table, query_list[5]);
                            printf("%d\n", index);
                            bzero(buff, sizeof(buff));
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
                                for (int i = 0; i < user.tot_column; i++)
                                {
                                    if (strcmp(user.data[index], query_list[6]) == 0)
                                    {
                                        char padding[2000];
                                        snprintf(padding, sizeof padding, "%s\t", user.data[i]);
                                        strcat(buffers, padding);
                                    }
                                }
                                strcat(sendDatabase, buffers);
                            }
                            send(new_socket, sendDatabase, strlen(sendDatabase), 0);
                            bzero(sendDatabase, sizeof(sendDatabase));
                            bzero(buff, sizeof(buff));
                            fclose(file);
                        }
                        else
                        {
                            int index = findColumn(create_table, query_list[1]);
                            printf("%d\n", index);
                            FILE *file, *file1;
                            struct table user;
                            int id, mark = 0;
                            int change_index = findColumn(create_table, query_list[5]);
                            file = fopen(create_table, "rb");
                            char buffers[40000];
                            char sendDatabase[40000];
                            bzero(buff, sizeof(buff));
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
                                for (int i = 0; i < user.tot_column; i++)
                                {
                                    if (i == index && (strcmp(user.data[change_index], query_list[6]) == 0 || strcmp(user.data[i], query_list[5]) == 0))
                                    {
                                        char padding[2000];
                                        snprintf(padding, sizeof padding, "%s\t", user.data[i]);
                                        strcat(buffers, padding);
                                    }
                                }
                                strcat(sendDatabase, buffers);
                            }
                            printf("ini send fix\n%s\n", sendDatabase);
                            fclose(file);
                            send(new_socket, sendDatabase, strlen(sendDatabase), 0);
                            bzero(sendDatabase, sizeof(sendDatabase));
                            bzero(buff, sizeof(buff));
                        }
                    }
                    else
                    {
                        printf("ini query 3 %s", query_list[total - 3]);
                        if (strcmp(query_list[total - 3], "WHERE") != 0)
                        {
                            char create_table[2000];
                            snprintf(create_table, sizeof create_table, "databases/%s/%s", currDB, query_list[total - 1]);
                            printf("buat table = %s", create_table);
                            printf("tanpa where");
                            int index[100];
                            int iteration = 0;
                            for (int i = 1; i < total - 2; i++)
                            {
                                index[iteration] = findColumn(create_table, query_list[i]);
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
                else if (strcmp(command[0], "log") == 0)
                {
                    writelog(command[1], command[2]);
                    char warning[] = "\n";
                    send(new_socket, warning, strlen(warning), 0);
                    bzero(buff, sizeof(buff));
                }
                if (strcmp(buff, ":exit") == 0)
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                }
                else
                {
                    printf("Client: %s\n", buff);
                    send(new_socket, buff, strlen(buff), 0);
                    bzero(buff, sizeof(buff));
                }
            }
        }
    }

    close(new_socket);

    return 0;
}
