#include "LibIncludes.h"

MYSQL *tablestructconn; //DB connection socket
MYSQL_RES *res;         //DB query result variable
MYSQL_ROW row;          //DB query result split into multiple rows variable

int getTableStructs(FILE *file);
int getColumnStats(MYSQL_ROW row, FILE *file, char *server, char *user, char *password, char *database);
int getTableStruct();

int createTableStructures()
{
    //printf("createtablestructs");
    FILE *file = fopen("TableStruct", "w");
    if (file == NULL)
    {
        perror("Failed to open file");
    }
    getTableStructs(file);
    fclose(file);
    return 0;
}

int getTableStruct()
{
    //printf("gettablestruct");
    FILE *file = fopen("checkFileTableStruct", "w");
    if (file == NULL)
    {
        perror("Failed to open file");
    }
    getTableStructs(file);
    fclose(file);
    return 0;
}

int getTableStructs(FILE *file)
{
    //printf("gettablestructs");
    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(300);
    user = (char *)malloc(300);
    password = (char *)malloc(300);
    database = (char *)malloc(300);
    GetLogins(server, user, password, database);
    tablestructconn = mysql_init(NULL);
    connectToDB(tablestructconn, res, row, server, user, password, database);
    if (mysql_query(tablestructconn, "show tables"))
    {
        fprintf(stderr, "\n[getTableStructs] Error: %s [%d]\n", mysql_error(tablestructconn), mysql_errno(tablestructconn));
        exit(1);
    }
    if ((res = mysql_use_result(tablestructconn)) == NULL)
    {
        fprintf(stderr, "\n[getTableStructs] Error: %s [%d]\n", mysql_error(tablestructconn), mysql_errno(tablestructconn));
        exit(1);
    }
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        getColumnStats(row, file, server, user, password, database);
    }
    mysql_free_result(res);
    if (tablestructconn == NULL)
    {
        puts("Fuck off");
    }
    mysql_close(tablestructconn);
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}

int getColumnStats(MYSQL_ROW row, FILE *file, char *server, char *user, char *password, char *database)
{
    //printf("getcolumnstats");
    MYSQL *tablestructconn1; //DB connection socket
    MYSQL_RES *res1;         //DB query result variable
    MYSQL_ROW row1;
    tablestructconn1 = mysql_init(NULL);
    char query[200] = {'\0'};
    sprintf(query, "SHOW COLUMNS FROM %s", row[0]);
    connectToDB(tablestructconn1, res1, row1, server, user, password, database);
    if (mysql_query(tablestructconn1, query))
    {
        fprintf(stderr, "\n[getColumnStats] Error: %s [%d]\n", mysql_error(tablestructconn1), mysql_errno(tablestructconn1));
        exit(1);
    }
    if ((res1 = mysql_use_result(tablestructconn1)) == NULL)
    {
        fprintf(stderr, "\n[getColumnStats] Error: %s [%d]\n", mysql_error(tablestructconn1), mysql_errno(tablestructconn1));
        exit(1);
    }
    int num_fields = mysql_num_fields(res1);
    while ((row1 = mysql_fetch_row(res1)) != NULL)
    {
        for (int i = 0; i < num_fields; i++)
        {
            if (row1[i] != NULL)
            {
                fprintf(file, "%s ", row1[i]);
            }
            else
            {
                fprintf(file, " ");
            }
        }
        fprintf(file, "\n");
    }
    mysql_free_result(res1);
    mysql_close(tablestructconn1);
    fprintf(file, "--%s\n", row[0]);
    return 0;
}