#include "LibIncludes.h"
#include "DefineDBSize.c"
#include "DBToFile.c"
#include "CheckTableStructures.c"

void *checkForChange(void *args);
int createReplica();
FILE *OpenCMD(char *cmd);
int Reset(char *array);
int CloseCMD(FILE *cmd);
int CheckTables(char buffer1[128], char buffer2[128], char *cksum1, char *cksum2, char *differ);
int CheckColumns(char buffer1[128], char buffer2[128], char *cksum1, char *cksum2, char *differ);
int CheckRows(char buffer1[128], char buffer2[128], char *cksum1, char *cksum2, char *differ);
int ApplyChanges(char *main, char *temp);
int Monitor(char *file1, char *file2);
int GetRefresh();
int recheckTriggers();
int getCurrentConnections(char file[]);
int checkForTriggers(MYSQL_ROW row);
int printLogins();

int ThreadWatch()
{
    //printf("threadwatch\n");
    createReplica();
    return 0;
}

//---Creates general DB replica, which is used for comparing---//
int createReplica()
{
    //printf("createreplica\n");
    Start();
    createTableStructures();
    getCurrentConnections("Connections");
    printLogins();
    printf("\n");
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, checkForChange, NULL);
    return 0;
}
//---Creates general DB replica, which is used for comparing---//

void *checkForChange(void *args)
{
    //printf("checkforchange\n");
    int i = 0;
    while (1)
    {
        recheckTriggers();
        int resetTime = GetRefresh();
        //------Checks if theres any difference in table section------//
        getTables();
        char *file1 = "Tables";
        char *file2 = "checkFileTables";
        Monitor(file1, file2);

        //------Checks if theres any difference in table section------//

        //------Checks if theres any difference in column section------//
        getColumns();
        file1 = "Columns";
        file2 = "checkFileColumns";
        Monitor(file1, file2);

        //------Checks if theres any difference in column section------//

        //------Checks if theres any difference in row section------//
        getRows();
        file1 = "Rows";
        file2 = "checkFileRows";
        Monitor(file1, file2);

        //------Checks if theres any difference in row section------//

        //------Checks if theres any table altering------//
        getTableStruct();
        file1 = "TableStruct";
        file2 = "checkFileTableStruct";
        Monitor(file1, file2);

        //------Checks if theres any table altering------//

        //------Shows current connections------//
        getCurrentConnections("checkFileConnections");
        file1 = "Connections";
        file2 = "checkFileConnections";
        Monitor(file1, file2);

        //------Shows current connections------//
        sleep(resetTime);
    }
    pthread_exit(0);
}

int printLogins()
{
    char differ[300] = {'\0'};
    FILE *connected = fopen("Connections", "r");
    if (connected == NULL)
    {
        perror("Failed to open file");
    }
    int nr = 1;
    printf("Current connetions: \n");
    while (fgets(differ, sizeof(differ), connected))
    {
        printf("%s", differ);
        nr++;
    }
    return 0;
}

int getCurrentConnections(char file[])
{
    FILE *connected = fopen(file, "w");
    if (connected == NULL)
    {
        perror("Failed to open file");
    }
    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(300);
    user = (char *)malloc(300);
    password = (char *)malloc(300);
    database = (char *)malloc(300);
    GetLogins(server, user, password, database);
    MYSQL *connectionconn;
    MYSQL_RES *connres;
    MYSQL_ROW rows;
    connectionconn = mysql_init(NULL);
    connectToDB(connectionconn, connres, rows, server, user, password, database);
    if (mysql_query(connectionconn, "show processlist"))
    {
        fprintf(stderr, "\n[getCurrentConnections] Error: %s [%d]\n", mysql_error(connectionconn), mysql_errno(connectionconn));
        exit(1);
    }
    if ((connres = mysql_use_result(connectionconn)) == NULL)
    {
        fprintf(stderr, "\n[getCurrentConnections] Error: %s [%d]\n", mysql_error(connectionconn), mysql_errno(connectionconn));
        exit(1);
    }
    int num_fields = mysql_num_fields(connres);
    //printf("fields >> %d\n", num_fields);
    int nr = 1;
    while ((rows = mysql_fetch_row(connres)) != NULL)
    {
        fprintf(connected, "%d. ", nr);
        for (int i = 0; i < num_fields; i++)
        {
            fprintf(connected, "%s ", rows[i]);
        }
        nr++;
        fprintf(connected, "\n");
    }
    mysql_free_result(connres);
    mysql_close(connectionconn);
    free(server);
    free(user);
    free(password);
    free(database);
    fclose(connected);
    return 0;
}

int Monitor(char *file1, char *file2)
{
    //printf("monitor\n");
    char cmd1[40] = {'\0'};
    char cmd2[40] = {'\0'};
    char cmd3[40] = {'\0'};
    char buffer1[300];
    char buffer2[300];
    char differ[400];
    char line[401] = {'\0'};
    char *cksum1;
    char *cksum2;
    char *s;
    int slot;
    int connectionChange = 0;
    int into = 0;
    sprintf(cmd1, "cksum %s", file1);
    sprintf(cmd2, "cksum %s", file2);
    FILE *cksumcmd1 = OpenCMD(cmd1);
    FILE *cksumcmd2 = OpenCMD(cmd2);
    fgets(buffer1, sizeof(buffer1), cksumcmd1);
    cksum1 = strtok(buffer1, " ");
    fgets(buffer2, sizeof(buffer2), cksumcmd2);
    cksum2 = strtok(buffer2, " ");
    //printf("Current cksum - %s | %s - Original cksum\n", cksum1, cksum2);
    CloseCMD(cksumcmd1);
    CloseCMD(cksumcmd2);
    if (strcmp(cksum1, cksum2) != 0)
    {
        Reset(cksum1);
        Reset(cksum2);
        sprintf(cmd3, "diff -c0 %s %s", file1, file2);
        FILE *diffcmd = OpenCMD(cmd3);
        if (strcmp(file1, "Connections") != 0)
        {
            printf("Detected changes in %s: \n\n", file1);
            printf("\n");
        }
        while (fgets(differ, sizeof(differ), diffcmd))
        {
            //printf("differ >> %s\n", differ);
            strcpy(line, differ);
            if (differ[0] == '+' && differ[1] == ' ')
            {
                if (strcmp(file1, "Connections") != 0)
                {
                    printf("Added: ");
                }
                else
                {
                    printf("New connection: ");
                }
                printf("%s", line);
                printf("\n");
            }
            s = strstr(differ, "- ");
            if (differ[0] == '-' && differ[1] == ' ')
            {
                if (strcmp(file1, "Connections") != 0)
                {
                    printf("Removed: ");
                }
                else
                {
                    printf("Disconnected: ");
                }
                printf("%s", line);
                printf("\n");
            }
            if (differ[0] == '!' && differ[1] == ' ')
            {
                if (strcmp(file1, "Connections") != 0)
                {
                    if (into == 0)
                    {
                        printf("Modified row: ");
                        printf("%s", line);
                        printf("\n");
                        into = 1;
                    }
                    else
                    {
                        printf("Modified into: ");
                        printf("%s", line);
                        printf("\n");
                        into = 0;
                    }
                }
            }
            s = strstr(differ, ">");
            if (s)
            {
                if (strcmp(file1, "Connections") != 0)
                {
                    printf("Added: ");
                }
                else
                {
                    printf("New connection: ");
                    connectionChange = 1;
                }
                slot = s - differ;
                slot += strlen("> ");
                for (int i = slot; i < strlen(line); i++)
                {
                    printf("%c", line[i]);
                }
                printf("\n");
            }
            s = strstr(differ, "<");
            if (s)
            {
                if (strcmp(file1, "Connections") != 0)
                {
                    printf("Removed: ");
                }
                else
                {
                    printf("Disconnected: ");
                    connectionChange = 1;
                }
                printf("%s\n", line);
            }
            if (strcmp(file1, "Connections") != 0)
            {
                s = strstr(differ, "|");
                if (s)
                {
                    printf("Modified: ");
                    printf("%s", line);
                    printf("\n");
                }
            }
        }
        if (strcmp(file1, "Connections") != 0)
        {
            ApplyChanges(file1, file2);
        }
        if (strcmp(file1, "Connections") == 0 && connectionChange == 1)
        {
            ApplyChanges(file1, file2);
        }
        connectionChange = 0;
    }
    return 0;
}

int ApplyChanges(char *main, char *temp)
{
    //printf("applychanges\n");
    FILE *mFile = fopen(main, "w");
    FILE *tFile = fopen(temp, "r");
    char c = fgetc(tFile);
    while (c != EOF)
    {
        fputc(c, mFile);
        c = fgetc(tFile);
    }
    printf("Changes copied to main checkFile\n\n");
    fclose(mFile);
    fclose(tFile);
    return 0;
}

int Reset(char *array)
{
    //printf("reset\n");
    memcpy(array, "", strlen(array));
    return 0;
}

int CloseCMD(FILE *cmd)
{
    //printf("closecmd\n");
    if (pclose(cmd))
    {
        printf("Unable to close command [not found or exited with error status]\n");
        exit(1);
    }
    return 0;
}

FILE *OpenCMD(char cmd[])
{
    //printf("opencmd\n");
    FILE *CMD = popen(cmd, "r");
    if (CMD == NULL)
    {
        perror("Failed to open command");
        exit(1);
    }
    return CMD;
}

int GetRefresh()
{
    //printf("getrefresh");
    int slot = -1;
    int refresh = 0;
    char buff[200];
    char *s;
    FILE *config = fopen("config", "r");
    if (config == NULL)
    {
        perror("Failed to open file");
    }
    while (!feof(config))
    {
        fgets(buff, 200, config);
        //printf("BUFF >>> %s\n", buff);
        s = strstr(buff, "Refresh = ");
        if (s)
        {
            slot = s - buff;
            break;
        }
    }
    // printf("%d << slot\n", slot);
    slot = slot + strlen("Refresh = ");
    //printf("%d << slot\n", slot + 1);
    if (slot > -1)
    {
        while (1)
        {
            if (isdigit(buff[slot]))
            {
                refresh *= 10;
                int number = buff[slot] - '0';
                refresh += number;
                slot++;
            }
            else
            {

                break;
            }
        }
    }
    else
    {
        printf("No refresh timer set, setting default (10)");
        refresh = 10;
    }
    fclose(config);
    return refresh;
}

int recheckTriggers()
{
    MYSQL *monitorconn;
    //printf("recheckTriggers\n");
    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(300);
    user = (char *)malloc(300);
    password = (char *)malloc(300);
    database = (char *)malloc(300);
    GetLogins(server, user, password, database);
    monitorconn = mysql_init(NULL);
    connectToDB(monitorconn, res, row, server, user, password, database);
    if (mysql_query(monitorconn, "show tables"))
    {
        fprintf(stderr, "\n[checkForTables] Error: %s [%d]\n", mysql_error(monitorconn), mysql_errno(monitorconn));
        exit(1);
    }
    if ((res = mysql_use_result(monitorconn)) == NULL)
    {
        fprintf(stderr, "\n[checkForTables] Error: %s [%d]\n", mysql_error(monitorconn), mysql_errno(monitorconn));
        exit(1);
    }
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        if (strcmp(row[0], "InsertChanges") != 0 && strcmp(row[0], "RemoveChanges") != 0 && strcmp(row[0], "UpdateChanges") != 0)
        {
            checkForTriggers(row);
        }
    }
    mysql_close(monitorconn);
    mysql_free_result(res);
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}