#include "LibIncludes.h"

int Start();
int columnCount(MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database, FILE *columns, FILE *rows);
int rowCount(MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database, FILE *columns, FILE *rows);
int connectToDB(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database);
int create3DStructures(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database, FILE *tables, FILE *columns, FILE *rows);
int columnAndRowWrite(MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database, FILE *columns, FILE *rows);
int getCurrentSizes(FILE *tables, FILE *columns, FILE *rows);
int GetLogins(char *server, char *user, char *password, char *database);

int Start()
{
    //printf("start\n");
    MYSQL *DefineDBconn; //DB connection socket
    MYSQL_RES *res;      //DB query result variable
    MYSQL_ROW row;       //DB query result split into multiple rows variable

    //----Database replica----//
    FILE *tables = fopen("Tables", "w");
    FILE *columns = fopen("Columns", "w");
    FILE *rows = fopen("Rows", "w");
    //----Database replica----//

    if (tables == NULL || columns == NULL || rows == NULL)
    {
        perror("Failed to open file");
        exit(1);
    }

    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(100);
    user = (char *)malloc(100);
    password = (char *)malloc(100);
    database = (char *)malloc(100);
    GetLogins(server, user, password, database);
    //----Communication port----//
    DefineDBconn = mysql_init(NULL);
    //----Communication port----//

    /*
    connectToDB - connecting to specified database
    create3Dstructures - check entire db for the table/column/row count
    */

    //https://stackoverflow.com/questions/3453168/c-program-mysql-connection

    connectToDB(DefineDBconn, res, row, server, user, password, database);
    create3DStructures(DefineDBconn, res, row, server, user, password, database, tables, columns, rows);
    mysql_close(DefineDBconn);
    fclose(tables);
    fclose(columns);
    fclose(rows);
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}

int create3DStructures(MYSQL *DefineDBconn, MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password,
                       char *database, FILE *tables, FILE *columns, FILE *rows)
{
    //printf("create3dstructures\n");
    if (mysql_query(DefineDBconn, "show tables"))
    {
        fprintf(stderr, "\n Error: %s [%d]\n", mysql_error(DefineDBconn), mysql_errno(DefineDBconn));
        exit(1);
    }
    if ((res = mysql_use_result(DefineDBconn)) == NULL)
    {
        fprintf(stderr, "\nError: %s [%d]\n", mysql_error(DefineDBconn), mysql_errno(DefineDBconn));
        exit(1);
    }
    while ((row = mysql_fetch_row(res)) != NULL)
    {

        fprintf(tables, "%s\n", row[0]);
        columnCount(res, row, server, user, password, database, columns, rows);
    }
    mysql_free_result(res);
    return 0;
}

int columnCount(MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database, FILE *columns, FILE *rows)
{
    //printf("columncount\n");
    MYSQL *DefineDBconn1;
    MYSQL_ROW rowtemp;
    DefineDBconn1 = mysql_init(NULL);
    connectToDB(DefineDBconn1, res, row, server, user, password, database);
    int resAmount = 0;
    char query1[300] = {'\0'};
    char query2[300] = {'\0'};
    sprintf(query1, "SELECT COUNT(COLUMN_NAME) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '%s' AND TABLE_NAME = '%s'", database, row[0]);
    if (mysql_query(DefineDBconn1, query1))
    {
        fprintf(stderr, "\n[columnCount] Error: %s [%d]\n", mysql_error(DefineDBconn1), mysql_errno(DefineDBconn1));
        exit(1);
    }
    if ((res = mysql_use_result(DefineDBconn1)) == NULL)
    {
        fprintf(stderr, "\n[columnCount] Error: %s [%d]\n", mysql_error(DefineDBconn1), mysql_errno(DefineDBconn1));
        exit(1);
    }
    mysql_close(DefineDBconn1);
    mysql_free_result(res);
    DefineDBconn1 = mysql_init(NULL);
    connectToDB(DefineDBconn1, res, row, server, user, password, database);
    sprintf(query2, "SELECT COUNT(*) FROM %s", row[0]);
    if (mysql_query(DefineDBconn1, query2))
    {
        fprintf(stderr, "\n[columnCount] Error: %s [%d]\n", mysql_error(DefineDBconn1), mysql_errno(DefineDBconn1));
        exit(1);
    }
    if ((res = mysql_use_result(DefineDBconn1)) == NULL)
    {
        fprintf(stderr, "\n[columnCount] Error: %s [%d]\n", mysql_error(DefineDBconn1), mysql_errno(DefineDBconn1));
        exit(1);
    }
    mysql_free_result(res);
    mysql_close(DefineDBconn1);
    columnAndRowWrite(res, row, server, user, password, database, columns, rows);
    return 0;
}

int columnAndRowWrite(MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database, FILE *columns, FILE *rows)
{
    //printf("columnandrowwrite\n");
    MYSQL *DefineDBconn1;
    MYSQL_ROW rowtemp;
    DefineDBconn1 = mysql_init(NULL);
    char query1[300] = {'\0'};
    char query2[300] = {'\0'};
    sprintf(query1, "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '%s' AND TABLE_NAME = '%s'", database, row[0]);
    connectToDB(DefineDBconn1, res, rowtemp, server, user, password, database);
    if (mysql_query(DefineDBconn1, query1))
    {
        fprintf(stderr, "\n[columnAndRowWrite] Error: %s [%d]\n", mysql_error(DefineDBconn1), mysql_errno(DefineDBconn1));
        exit(1);
    }
    if ((res = mysql_use_result(DefineDBconn1)) == NULL)
    {
        fprintf(stderr, "\n[columnAndRowWrite] Error: %s [%d]\n", mysql_error(DefineDBconn1), mysql_errno(DefineDBconn1));
        exit(1);
    }
    while ((rowtemp = mysql_fetch_row(res)) != NULL)
    {
        fprintf(columns, "%s\n", rowtemp[0]);
    }
    fprintf(columns, "--%s\n", row[0]);
    mysql_free_result(res);
    mysql_close(DefineDBconn1);
    DefineDBconn1 = mysql_init(NULL);
    //GetLogins(conn1, res, rowtemp);
    sprintf(query2, "SELECT * FROM %s", row[0]);
    connectToDB(DefineDBconn1, res, rowtemp, server, user, password, database);
    if (mysql_query(DefineDBconn1, query2))
    {
        fprintf(stderr, "\n[columnAndRowWrite] Error: %s [%d]\n", mysql_error(DefineDBconn1), mysql_errno(DefineDBconn1));
        exit(1);
    }
    if ((res = mysql_use_result(DefineDBconn1)) == NULL)
    {
        fprintf(stderr, "\n[columnAndRowWrite] Error: %s [%d]\n", mysql_error(DefineDBconn1), mysql_errno(DefineDBconn1));
        exit(1);
    }
    int num_fields = mysql_num_fields(res);
    while ((rowtemp = mysql_fetch_row(res)) != NULL)
    {
        for (int i = 0; i < num_fields; i++)
        {
            if (rowtemp[i] != NULL)
            {
                fprintf(rows, "%s ", rowtemp[i]);
            }
            else
            {
                fprintf(rows, " ");
            }
        }
        fprintf(rows, "\n");
    }
    mysql_free_result(res);
    mysql_close(DefineDBconn1);
    fprintf(rows, "--%s\n", row[0]);
    return 0;
}
int connectToDB(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database)
{
    //printf("connecttodb\n");
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        fprintf(stderr, "\n[connectToDB] Error: %s [%d]\n", mysql_error(conn), mysql_errno(conn));
        exit(1);
    }
    return 0;
}

int GetLogins(char *server, char *user, char *password, char *database)
{
    int slot = -1;
    char buff[300];
    char *s;
    int sidx = 0;
    int uidx = 0;
    int pidx = 0;
    int didx = 0;
    int foundServer = 0;
    int foundUser = 0;
    int foundPassword = 0;
    int foundDatabase = 0;
    char serv[300] = {'\0'};
    char usr[300] = {'\0'};
    char pass[300] = {'\0'};
    char db[300] = {'\0'};
    FILE *config = fopen("config", "r");
    if (config == NULL)
    {
        perror("Failed to open file");
    }
    while (!feof(config))
    {
        fgets(buff, 300, config);
        s = strstr(buff, "Server = \"");
        if (s)
        {
            foundServer = 1;
            slot = s - buff;
            slot += strlen("Server = \"");
            for (int i = slot; i < strlen(buff) - 2; i++)
            {
                serv[sidx++] = buff[i];
            }
        }
        s = strstr(buff, "User = \"");
        if (s)
        {
            foundUser = 1;
            slot = s - buff;
            slot += strlen("User = \"");
            for (int i = slot; i < strlen(buff) - 2; i++)
            {
                usr[uidx++] = buff[i];
            }
        }
        s = strstr(buff, "Password = \"");
        if (s)
        {
            foundPassword = 1;
            slot = s - buff;
            slot += strlen("Password = \"");
            for (int i = slot; i < strlen(buff) - 2; i++)
            {
                pass[pidx++] = buff[i];
            }
        }
        s = strstr(buff, "Database = \"");
        if (s)
        {
            foundDatabase = 1;
            slot = s - buff;
            slot += strlen("Database = \"");
            for (int i = slot; i < strlen(buff) - 2; i++)
            {
                db[didx++] = buff[i];
            }
        }
        memcpy(buff, "", strlen(buff));
    }
    if (foundServer == 0 || foundUser == 0 || foundPassword == 0 || foundDatabase == 0)
    {
        printf("Unable to connect to server [failed to provide valid connection details]\n");
        exit(1);
    }
    strcpy(server, serv);
    strcpy(user, usr);
    strcpy(password, pass);
    strcpy(database, db);
    fclose(config);
    return 0;
}