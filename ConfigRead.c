#include "LibIncludes.h"

char *server;
char *user;
char *password;
char *database;

int GetLogins()
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
    printf("Login:\n");
    printf("Server = %s\n", serv);
    printf("User = %s\n", usr);
    printf("Password = %s\n", pass);
    printf("Database = %s\n", db);
    server = serv;
    user = usr;
    password = pass;
    database = db;
    printf("Login:\n");
    printf("Server = %s\n", server);
    printf("User = %s\n", user);
    printf("Password = %s\n", password);
    printf("Database = %s\n", database);
    return 0;
}


int GetRefresh()
{
    int slot = -1;
    int refresh = 0;
    char buff[300];
    char *s;
    FILE *config = fopen("config", "r");
    if (config == NULL)
    {
        perror("Failed to open file");
    }
    while (!feof(config))
    {
        fgets(buff, 300, config);
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
    // printf("%d << slot\n", slot + 1);
    if (slot > -1)
    {
        while (1)
        {
            // printf("char >> %c\n", buff[slot]);
            if (isdigit(buff[slot]))
            {
                refresh *= 10;
                int number = buff[slot] - '0';
                refresh += number;
                slot++;
            }
            else
            {
                printf("\n");
                break;
            }
        }
    }
    else
    {
        printf("No refresh timer set, setting default (10)");
        refresh = 10;
    }
    return refresh;
}