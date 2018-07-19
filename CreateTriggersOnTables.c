#include "LibIncludes.h"

int checkTrigger(MYSQL_ROW row);
int InsertTrigger(MYSQL *triggertableconn1, char insert_trigger[300], char *server, char *user, char *password, char *database, MYSQL_ROW row);
int UpdateTrigger(MYSQL *triggertableconn1, char insert_trigger[300], char *server, char *user, char *password, char *database, MYSQL_ROW row);
int RemoveTrigger(MYSQL *triggertableconn1, char insert_trigger[300], char *server, char *user, char *password, char *database, MYSQL_ROW row);
int checkForMissingTriggers(MYSQL *triggertableconn1, char insert_trigger[300], char update_trigger[300], char remove_trigger[300], int foundinsert, int foundremove, int foundupdate, char *server, char *user, char *password, char *database, MYSQL_ROW row);

int checkForTriggers(MYSQL_ROW row)
{
    char *server;
    char *user;
    char *password;
    char *database;
    server = (char *)malloc(300);
    user = (char *)malloc(300);
    password = (char *)malloc(300);
    database = (char *)malloc(300);
    GetLogins(server, user, password, database);
    //printf("checkfortriggers\n");
    MYSQL *triggertableconn1; //DB connection socket
    MYSQL_RES *trres;         //DB query result variable
    MYSQL_ROW trrow;          //DB query result split into multiple rows variable
    //----Communication port----//
    char insert_trigger[300] = {'\0'};
    char update_trigger[300] = {'\0'};
    char remove_trigger[300] = {'\0'};
    int foundinsert = 0;
    int foundupdate = 0;
    int foundremove = 0;
    sprintf(insert_trigger, "after_insert_%s", row[0]);
    sprintf(update_trigger, "after_update_%s", row[0]);
    sprintf(remove_trigger, "after_remove_%s", row[0]);
    triggertableconn1 = mysql_init(NULL);
    connectToDB(triggertableconn1, trres, trrow, server, user, password, database);
    if (mysql_query(triggertableconn1, "show triggers"))
    {
        fprintf(stderr, "\n[checkForTriggers] Error: %s [%d]\n", mysql_error(triggertableconn1), mysql_errno(triggertableconn1));
        exit(1);
    }
    if ((trres = mysql_use_result(triggertableconn1)) == NULL)
    {
        fprintf(stderr, "\n[checkForTriggers] Error: %s [%d]\n", mysql_error(triggertableconn1), mysql_errno(triggertableconn1));
        exit(1);
    }
    int num_fields = mysql_num_fields(trres);
    int resAmount = 0;
    while ((trrow = mysql_fetch_row(trres)) != NULL)
    {
        //printf("trigger >> %s\n", trrow[0]);
        if (strcmp(insert_trigger, trrow[0]) == 0)
        {
            foundinsert = 1;
        }
        if (strcmp(update_trigger, trrow[0]) == 0)
        {
            foundupdate = 1;
        }
        if (strcmp(remove_trigger, trrow[0]) == 0)
        {
            foundremove = 1;
        }
    }
    mysql_free_result(trres);
    mysql_close(triggertableconn1);
    checkForMissingTriggers(triggertableconn1, insert_trigger, update_trigger, remove_trigger, foundinsert, foundremove, foundupdate, server, user, password, database, row);
    return 0;
}

int checkForMissingTriggers(MYSQL *triggertableconn1, char insert_trigger[300], char update_trigger[300], char remove_trigger[300], int foundinsert, int foundremove, int foundupdate, char *server, char *user, char *password, char *database, MYSQL_ROW row)
{
    //printf("checkformissingtriggers\n");
    if (foundinsert == 0)
    {
        InsertTrigger(triggertableconn1, insert_trigger, server, user, password, database, row);
    }
    if (foundupdate == 0)
    {
        UpdateTrigger(triggertableconn1, update_trigger, server, user, password, database, row);
    }
    if (foundremove == 0)
    {
        RemoveTrigger(triggertableconn1, remove_trigger, server, user, password, database, row);
    }
    free(server);
    free(user);
    free(password);
    free(database);
    return 0;
}

int InsertTrigger(MYSQL *triggertableconn1, char insert_trigger[300], char *server, char *user, char *password, char *database, MYSQL_ROW row)
{
    //printf("inserttrigger\n");
    triggertableconn1 = mysql_init(NULL);
    connectToDB(triggertableconn1, res, row, server, user, password, database);
    char query[500];
    sprintf(query, "CREATE TRIGGER %s AFTER INSERT ON %s FOR EACH ROW INSERT INTO InsertChanges(Dat,Usr,Tbl,ID) VALUES(NOW(), (select user()), \"%s\", NEW.id);", insert_trigger, row[0], row[0]);
    if (mysql_query(triggertableconn1, query))
    {
        fprintf(stderr, "\n[InsertTrigger] Error: %s [%d]\n", mysql_error(triggertableconn1), mysql_errno(triggertableconn1));
        exit(1);
    }
    mysql_close(triggertableconn1);
    return 0;
}
int UpdateTrigger(MYSQL *triggertableconn1, char update_trigger[300], char *server, char *user, char *password, char *database, MYSQL_ROW row)
{
    triggertableconn1 = mysql_init(NULL);
    connectToDB(triggertableconn1, res, row, server, user, password, database);
    char query[500];
    sprintf(query, "CREATE TRIGGER %s AFTER UPDATE ON %s FOR EACH ROW INSERT INTO UpdateChanges(Dat,Usr,Tbl,ID) VALUES(NOW(), (select user()), \"%s\", id);", update_trigger, row[0], row[0]);
    if (mysql_query(triggertableconn1, query))
    {
        fprintf(stderr, "\n[UpdateTrigger] Error: %s [%d]\n", mysql_error(triggertableconn1), mysql_errno(triggertableconn1));
        exit(1);
    }
    mysql_close(triggertableconn1);
    return 0;
}
int RemoveTrigger(MYSQL *triggertableconn1, char remove_trigger[300], char *server, char *user, char *password, char *database, MYSQL_ROW row)
{
    triggertableconn1 = mysql_init(NULL);
    connectToDB(triggertableconn1, res, row, server, user, password, database);
    char query[500];
    sprintf(query, "CREATE TRIGGER %s AFTER DELETE ON %s FOR EACH ROW INSERT INTO RemoveChanges(Dat,Usr,Tbl,ID) VALUES(NOW(), (select user()), \"%s\", OLD.id);", remove_trigger, row[0], row[0]);
    if (mysql_query(triggertableconn1, query))
    {
        fprintf(stderr, "\n[RemoveTrigger] Error: %s [%d]\n", mysql_error(triggertableconn1), mysql_errno(triggertableconn1));
        exit(1);
    };
    mysql_close(triggertableconn1);
    return 0;
}
