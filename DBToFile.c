#include "LibIncludes.h"

int writeColumnsFromDB(FILE *tempFile, MYSQL_ROW row);
int getColumnsFromDB(FILE *tempFile, char *server, char *user, char *password, char *database);
int getTablesFromDB(FILE *tempFile, char *server, char *user, char *password, char *database);
int getRowsFromDB(FILE *temp, char *server, char *user, char *password, char *database);
int getTables();
int getColumns();
int getRows();

MYSQL *DBToFileconn; //DB connection socket
MYSQL_RES *res;      //DB query result variable
MYSQL_ROW row;       //DB query result split into multiple rows variable

int getTables()
{
    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(100);
    user = (char *)malloc(100);
    password = (char *)malloc(100);
    database = (char *)malloc(100);
    //printf("gettables\n");
    GetLogins(server, user, password, database);
    FILE *temp = fopen("checkFileTables", "w");
    if (temp == NULL)
    {
        perror("Failed to open file");
        exit(1);
    }
    getTablesFromDB(temp, server, user, password, database);
    fclose(temp);
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}

int getColumns()
{
    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(100);
    user = (char *)malloc(100);
    password = (char *)malloc(100);
    database = (char *)malloc(100);
    //printf("getcolumns\n");
    GetLogins(server, user, password, database);
    FILE *temp = fopen("checkFileTables", "r");
    if (temp == NULL)
    {
        perror("Failed to open file");
        exit(1);
    }
    getColumnsFromDB(temp, server, user, password, database);
    fclose(temp);
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}

int getRows()
{
    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(100);
    user = (char *)malloc(100);
    password = (char *)malloc(100);
    database = (char *)malloc(100);
    //printf("getrows\n");
    GetLogins(server, user, password, database);
    FILE *temp = fopen("checkFileTables", "r");
    if (temp == NULL)
    {
        perror("Failed to open file");
    }
    getRowsFromDB(temp, server, user, password, database);
    fclose(temp);
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}

int getTablesFromDB(FILE *tempFile, char *server, char *user, char *password, char *database)
{
    //printf("gettablesfromdb\n");
    DBToFileconn = mysql_init(NULL);
    connectToDB(DBToFileconn, res, row, server, user, password, database);
    if (mysql_query(DBToFileconn, "show tables"))
    {
        fprintf(stderr, "\n[getTablesFromDB] Error: %s [%d]\n", mysql_error(DBToFileconn), mysql_errno(DBToFileconn));
        exit(1);
    }
    if ((res = mysql_use_result(DBToFileconn)) == NULL)
    {
        fprintf(stderr, "\n[getTablesFromDB] Error: %s [%d]\n", mysql_error(DBToFileconn), mysql_errno(DBToFileconn));
        exit(1);
    }
    int num_fields = mysql_num_fields(res);
    int resAmount = 0;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        fprintf(tempFile, "%s\n", row[0]);
    }
    mysql_close(DBToFileconn);
    mysql_free_result(res);
    return 0;
}

int getColumnsFromDB(FILE *temp, char *server, char *user, char *password, char *database)
{
    //printf("getcolumnsfromdb\n");
    FILE *tmpCol = fopen("checkFileColumns", "w");
    if (tmpCol == NULL)
    {
        perror("Failed to open file");
    }
    char *table;
    char *query;
    table = (char *)malloc(200);
    query = (char *)malloc(200);
    DBToFileconn = mysql_init(NULL);
    connectToDB(DBToFileconn, res, row, server, user, password, database);
    while (!feof(temp))
    {
        fscanf(temp, "%s", table);
        sprintf(query, "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '%s' AND TABLE_NAME = '%s'", database, table);
        if (mysql_query(DBToFileconn, query))
        {
            fprintf(stderr, "\n[getColumnsFromDB] Error: %s [%d]\n", mysql_error(DBToFileconn), mysql_errno(DBToFileconn));
            exit(1);
        }
        if ((res = mysql_use_result(DBToFileconn)) == NULL)
        {
            fprintf(stderr, "\n[getColumnsFromDB] Error: %s [%d]\n", mysql_error(DBToFileconn), mysql_errno(DBToFileconn));
            exit(1);
        }
        while ((row = mysql_fetch_row(res)) != NULL)
        {
            fprintf(tmpCol, "%s\n", row[0]);
        }
        mysql_free_result(res);
        fprintf(tmpCol, "--%s\n", table);
        memset(table, 0, strlen(table));
    }
    mysql_close(DBToFileconn);
    fclose(tmpCol);
    free(table);
    free(query);
    return 0;
}
int getRowsFromDB(FILE *temp, char *server, char *user, char *password, char *database)
{
    //printf("getrowsfromdb\n");
    FILE *tmpRow = fopen("checkFileRows", "w");
    if (tmpRow == NULL)
    {
        perror("Failed to open file");
    }
    char *table;
    char *query;
    table = (char *)malloc(200);
    query = (char *)malloc(200);
    DBToFileconn = mysql_init(NULL);
    connectToDB(DBToFileconn, res, row, server, user, password, database);
    while (!feof(temp))
    {
        fscanf(temp, "%s", table);
        if (strlen(table) != 0)
        {
            sprintf(query, "SELECT * FROM %s", table);
            if (mysql_query(DBToFileconn, query))
            {
                fprintf(stderr, "\n[getRowsFromDB] Error: %s [%d]\n", mysql_error(DBToFileconn), mysql_errno(DBToFileconn));
                exit(1);
            }
            if ((res = mysql_use_result(DBToFileconn)) == NULL)
            {
                fprintf(stderr, "\n[getRowsFromDB] Error: %s [%d]\n", mysql_error(DBToFileconn), mysql_errno(DBToFileconn));
                exit(1);
            }
            int num_fields = mysql_num_fields(res);
            while ((row = mysql_fetch_row(res)) != NULL)
            {
                for (int i = 0; i < num_fields; i++)
                {
                    if (row[i] != NULL)
                    {
                        fprintf(tmpRow, "%s ", row[i]);
                    }
                    else
                    {
                        fprintf(tmpRow, " ");
                    }
                }
                fprintf(tmpRow, "\n");
            }
            mysql_free_result(res);
            fprintf(tmpRow, "--%s\n", table);
            memset(table, 0, strlen(table));
        }
    }
    mysql_close(DBToFileconn);
    free(table);
    free(query);
    fclose(tmpRow);
    return 0;
}