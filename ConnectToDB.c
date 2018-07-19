#include "LibIncludes.h"
#include "MonitorChanges.c"
#include "CreateTriggerTables.c"

int connectToDB(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, char *server, char *user, char *password, char *database);
int sendAndReceiveQuerryResults(MYSQL *dbconn, MYSQL_RES *res, MYSQL_ROW row, char *request, char *server, char *user, char *password, char *database);

int main(int argc, char *argv[])
{
    MYSQL *dbconn;  //DB connection socket
    MYSQL_RES *res; //DB query result variable
    MYSQL_ROW row;  //DB query result split into multiple rows variable

    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(300);
    user = (char *)malloc(300);
    password = (char *)malloc(300);
    database = (char *)malloc(300);
    GetLogins(server, user, password, database);

    //----Request for query----//
    char *request;
    request = (char *)malloc(1024);
    //----Request for query----//

    /*
    connectToDB - connecting to specified database
    sendAndReceiveQuerryResults - send input querry to database and get results
    */

    //https://stackoverflow.com/questions/3453168/c-program-mysql-connection
    checkForTables();
    ThreadWatch();
Input:
    //printf("Enter query request: ");
    fgets(request, 1024, stdin);
    sendAndReceiveQuerryResults(dbconn, res, row, request, server, user, password, database);
    goto Input;
    free(request);
    sleep(8);
    free(request);
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}

int sendAndReceiveQuerryResults(MYSQL *dbconn, MYSQL_RES *res, MYSQL_ROW row, char *request, char *server, char *user, char *password, char *database)
{
    dbconn = mysql_init(NULL);
    connectToDB(dbconn, res, row, server, user, password, database);
    clock_t tic = clock();
    if (dbconn == 0)
    {
        printf("Connected");
    }
    if (mysql_query(dbconn, request))
    {
        fprintf(stderr, "\n[sendAndReceiveQueryResults] Error: %s [%d]\n", mysql_error(dbconn), mysql_errno(dbconn));
        exit(1);
    }
    if ((res = mysql_use_result(dbconn)) == NULL)
    {
        fprintf(stderr, "\n[sendAndReceiveQueryResults] Error: %s [%d]\n", mysql_error(dbconn), mysql_errno(dbconn));
        exit(1);
    }

    clock_t toc = clock();
    int num_fields = mysql_num_fields(res);
    printf("Results from query:\n\n");
    int resAmount = 0;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        for (int i = 0; i < num_fields; i++)
        {
            printf("| %s |", row[i]);
            resAmount++;
        }
        printf("\n");
    }
    printf("\nResults found - %d\n", resAmount);
    printf("Query time -> %f seconds\n\n", (double)(toc - tic) / CLOCKS_PER_SEC);
    mysql_free_result(res);
    mysql_close(dbconn);
    return 0;
}
