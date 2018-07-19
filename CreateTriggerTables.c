#include "LibIncludes.h"
#include "CreateTriggersOnTables.c"

int foundUpdateChanges = 0;
int foundRemoveChanges = 0;
int foundInsertChanges = 0;
int removed = 0;

int checkForTables();
int checkUpdateTable(MYSQL *createtableconn, MYSQL_ROW row, char *server, char *user, char *password, char *database);
int checkRemoveTable(MYSQL *createtableconn, MYSQL_ROW row, char *server, char *user, char *password, char *database);
int checkInsertTable(MYSQL *createtableconn, MYSQL_ROW row, char *server, char *user, char *password, char *database);
int createTables(MYSQL *createtableconn, char *server, char *user, char *password, char *database);
int DropTable(MYSQL *createtableconn, MYSQL_ROW row, char *server, char *user, char *password, char *database);
int DropTriggers(MYSQL *createtableconn, char *server, char *user, char *password, char *database);
int removeTrigger(MYSQL_ROW row, char *server, char *user, char *password, char *database);

int checkForTables()
{
    //--Database connection/query/result variables--//
    MYSQL *createtableconn; //DB connection socket
    MYSQL_RES *res;         //DB query result variable
    MYSQL_ROW row;          //DB query result split into multiple rows variable
    //--Database connection/query/result variables--//

    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(300);
    user = (char *)malloc(300);
    password = (char *)malloc(300);
    database = (char *)malloc(300);
    GetLogins(server, user, password, database);
    //printf("checkfortables\n");
    DropTriggers(createtableconn, server, user, password, database);
    createtableconn = mysql_init(NULL);
    connectToDB(createtableconn, res, row, server, user, password, database);
    if (mysql_query(createtableconn, "show tables"))
    {
        fprintf(stderr, "\n[checkForTables] Error: %s [%d]\n", mysql_error(createtableconn), mysql_errno(createtableconn));
        exit(1);
    }
    if ((res = mysql_use_result(createtableconn)) == NULL)
    {
        fprintf(stderr, "\n[checkForTables] Error: %s [%d]\n", mysql_error(createtableconn), mysql_errno(createtableconn));
        exit(1);
    }
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        removed = 0;
        checkUpdateTable(createtableconn, row, server, user, password, database);
        checkRemoveTable(createtableconn, row, server, user, password, database);
        checkInsertTable(createtableconn, row, server, user, password, database);
        if (removed == 0)
        {
            checkForTriggers(row);
        }
    }
    mysql_free_result(res);

    mysql_close(createtableconn);
    createTables(createtableconn, server, user, password, database);
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}

int DropTriggers(MYSQL *createtableconn, char *server, char *user, char *password, char *database)
{
    //printf("droptrigger\n");
    char query[300];
    createtableconn = mysql_init(NULL);
    connectToDB(createtableconn, res, row, server, user, password, database);
    if (mysql_query(createtableconn, "show triggers"))
    {
        fprintf(stderr, "\n[DropTriggers] Error: %s [%d]\n", mysql_error(createtableconn), mysql_errno(createtableconn));
        exit(1);
    }
    if ((res = mysql_use_result(createtableconn)) == NULL)
    {
        fprintf(stderr, "\n[DropTriggers] Error: %s [%d]\n", mysql_error(createtableconn), mysql_errno(createtableconn));
        exit(1);
    }
    //printf("labas ");
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        removeTrigger(row, server, user, password, database);
    }
    //printf("vakaras\n");
    mysql_free_result(res);
    mysql_close(createtableconn);
    return 0;
}

int removeTrigger(MYSQL_ROW row, char *server, char *user, char *password, char *database)
{
    //printf("removetrigger\n");
    MYSQL *createtableconn1; //DB connection socket
    MYSQL_RES *res1;         //DB query result variable
    MYSQL_ROW row1;
    createtableconn1 = mysql_init(NULL);
    connectToDB(createtableconn1, res1, row1, server, user, password, database);
    char query[300];
    sprintf(query, "DROP TRIGGER %s", row[0]);
    if (mysql_query(createtableconn1, query))
    {
        fprintf(stderr, "\n[removeTrigger] Error: %s [%d]\n", mysql_error(createtableconn1), mysql_errno(createtableconn1));
        exit(1);
    }
    mysql_close(createtableconn1);
    //printf("Deleted trigger %s\n", row[0]);
    return 0;
}

int checkUpdateTable(MYSQL *createtableconn, MYSQL_ROW row, char *server, char *user, char *password, char *database)
{
    //printf("checkupdatetable\n");
    if (strcmp(row[0], "UpdateChanges") == 0)
    {
        DropTable(createtableconn, row, server, user, password, database);
        removed = 1;
    }
    return 0;
}
int checkRemoveTable(MYSQL *createtableconn, MYSQL_ROW row, char *server, char *user, char *password, char *database)
{
    //printf("checkremovetable\n");
    if (strcmp(row[0], "RemoveChanges") == 0)
    {
        DropTable(createtableconn, row, server, user, password, database);
        removed = 1;
    }
    return 0;
}
int checkInsertTable(MYSQL *createtableconn, MYSQL_ROW row, char *server, char *user, char *password, char *database)
{
    //printf("checkinserttable\n");
    if (strcmp(row[0], "InsertChanges") == 0)
    {
        DropTable(createtableconn, row, server, user, password, database);
        removed = 1;
    }
    return 0;
}

int DropTable(MYSQL *createtableconn, MYSQL_ROW row, char *server, char *user, char *password, char *database)
{
    //printf("droptable\n");
    char query[200] = {'\0'};
    sprintf(query, "DROP TABLE %s", row[0]);
    createtableconn = mysql_init(NULL);
    connectToDB(createtableconn, res, row, server, user, password, database);
    if (mysql_query(createtableconn, query))
    {
        fprintf(stderr, "\n[DropTable] Error: %s [%d]\n", mysql_error(createtableconn), mysql_errno(createtableconn));
        exit(1);
    }
    mysql_close(createtableconn);
    return 0;
}

int createTables(MYSQL *createtableconn, char *server, char *user, char *password, char *database)
{
    //printf("createtables\n");
    if (foundUpdateChanges == 0)
    {
        createtableconn = mysql_init(NULL);
        connectToDB(createtableconn, res, row, server, user, password, database);
        if (mysql_query(createtableconn, "CREATE TABLE UpdateChanges (Dat TIMESTAMP, Usr VARCHAR(100), Tbl VARCHAR(100), ID INT(100));"))
        {
            fprintf(stderr, "\n[createTables] Error: %s [%d]\n", mysql_error(createtableconn), mysql_errno(createtableconn));
            exit(1);
        }
        mysql_close(createtableconn);
    }
    if (foundRemoveChanges == 0)
    {
        createtableconn = mysql_init(NULL);
        connectToDB(createtableconn, res, row, server, user, password, database);
        if (mysql_query(createtableconn, "CREATE TABLE RemoveChanges (Dat TIMESTAMP, Usr VARCHAR(100), Tbl VARCHAR(100), ID INT(100));"))
        {
            fprintf(stderr, "\n[createTables] Error: %s [%d]\n", mysql_error(createtableconn), mysql_errno(createtableconn));
            exit(1);
        }
        mysql_close(createtableconn);
    }
    if (foundInsertChanges == 0)
    {
        createtableconn = mysql_init(NULL);
        connectToDB(createtableconn, res, row, server, user, password, database);
        if (mysql_query(createtableconn, "CREATE TABLE InsertChanges (Dat TIMESTAMP, Usr VARCHAR(100), Tbl VARCHAR(100), ID VARCHAR(100));"))
        {
            fprintf(stderr, "\n[createTables] Error: %s [%d]\n", mysql_error(createtableconn), mysql_errno(createtableconn));
            exit(1);
        }
        mysql_close(createtableconn);
    }
    return 0;
}