#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#include "util.h"
#include "index.h"
#include "DB.h"

void *strTrim(void *);

extern void init();
extern void *strTrim(void *);
extern int listAllDirs(void *, void *);
extern void closeDb();
extern int useDb(void *);
extern int putData(void *, void *);
extern int findData(void *, void *, void *);
extern void getRecords(void *, void *);
extern int deleteData(off_t);
extern int makeIndex(void *, void *);
extern void *strTrim(void *);

extern DbInfo dbInfo;

int main()
{
    init();

    char buff[BUFF_LEN];
    char *argu1, *argu2, *argu3;
    printf("Database start.\n> ");

    while (fgets(buff, BUFF_LEN, stdin) != NULL)
    {
        buff[BUFF_LEN - 1] = 0;
        buff[strlen(buff) - 1] = 0;

        argu1 = buff;
        argu1 = strTrim(argu1);

        if ((argu3 = strchr(argu1, ' ')))
        {
            *argu3 = 0;
            argu3++;

            argu3 = strTrim(argu3);
        }

        if ((argu2 = strchr(argu1, '.')))
        {
            *argu2 = 0;
            argu2++;

            argu2 = strTrim(argu2);
        }

        if (*argu1)
        {
            if (argu2)
            {
                if (strcmp(argu2, "put") == 0)
                {
                    if (dbInfo.properties)
                    {
                        //insert
                        int output = putData(argu1, argu3);
                        if (output > 0)
                        {
                            printf("Success\n");
                        }
                        else
                        {
                            if (output == 0)
                            {
                                fprintf(stderr, "Syntax error!\n");
                            }
                            else if (output == -1)
                            {
                                fprintf(stderr, "Write err\n");
                            }
                        }
                    }
                    else
                    {
                        printf("Choose a db first.\n");
                    }
                }
                else if (strcmp(argu2, "find") == 0 || strcmp(argu2, "delete") == 0)
                {
                    if (dbInfo.properties)
                    {
                        //select && delete
                        ArrayExt result = {.arr1 = NULL, .arr2 = NULL, .len = 0};
                        if (findData(&result, argu1, argu3) >= 0)
                        {
                            printf("Total: %d\n", result.len);
                            printf("--------------\n");

                            if (result.len != 0)
                            {
                                BlockList *head = result.arr1;
                                BlockList *current = head;
                                ArrayExt records = {.arr1 = NULL, .arr2 = NULL, .len = 0};

                                if (strcmp(argu2, "find") == 0)
                                {
                                    while (current)
                                    {
                                        getRecords(&records, current->block.data);
                                        for (int i = 0; i < records.len; i++)
                                        {
                                            printf("%s:\t%s\n", (char *)(records.arr2[2 * i]), (char *)(records.arr2[2 * i + 1]));
                                        }
                                        printf("-----------\n");

                                        current = current->next;
                                        free(records.arr2);
                                    }
                                }
                                else if (strcmp(argu2, "delete") == 0)
                                {
                                    while (current)
                                    {
                                        if (!deleteData(current->offset))
                                        {
                                            printf("Delete failed!\n");
                                        }
                                        current = current->next;
                                    }
                                }

                                //free blocklist
                                while (head)
                                {
                                    current = head;
                                    head = head->next;
                                    free(current);
                                }
                            }
                        }
                        else
                        {
                            fprintf(stderr, "Query syntax error!\n");
                        }
                    }
                    else
                    {
                        printf("Choose a db first.\n\n");
                    }
                }
                else if (strcmp(argu2, "makeIndex") == 0)
                {
                    if (dbInfo.properties)
                    {
                        //makeIndex
                        int res = makeIndex(argu1, argu3);
                        if (res >= 0)
                        {
                            printf("Data has been indexed: %d\n", res);
                        }
                        else
                        {
                            if (res == -1)
                            {
                                printf("Argument syntax error!\n");
                            }
                            else if (res == -2)
                            {
                                printf("Index has already existed.\n");
                            }
                            else
                            {
                                //index init error & create directory failed
                            }
                        }
                    }
                    else
                    {
                        printf("Choose a db first.\n\n");
                    }
                }
                else
                {
                    printf("Command not found!\n\n");
                }
            }
            else
            {
                if (strcmp(argu1, "list") == 0)
                {
                    ArrayExt result = {.arr1 = NULL, .arr2 = NULL, .len = 0};
                    if (listAllDirs(&result, "./dbs"))
                    {
                        printf("-----------\n");
                        for (int i = 0; i < result.len; i++)
                        {
                            printf("%s\n", (char *)result.arr2[i]);
                            free(result.arr2[i]);
                        }
                        printf("-----------\n");

                        free(result.arr2);
                    }
                }
                else if (strcmp(argu1, "use") == 0)
                {
                    if (argu3)
                    {
                        if (useDb(argu3))
                        {
                            printf("Switch to %s.\n\n", argu3);
                        }
                        else
                        {
                            printf("Load db failed.\n\n");
                        }
                    }
                    else
                    {
                        printf("Missing operand.\n");
                    }
                }
                else if (strcmp(argu1, "show") == 0)
                {
                    if (dbInfo.properties)
                    {
                        ArrayExt result = {.arr1 = NULL, .arr2 = NULL, .len = 0};
                        if (listAllDirs(&result, dbInfo.path))
                        {
                            printf("-----------\n");
                            for (int i = 0; i < result.len; i++)
                            {
                                printf("%s\n", (char *)result.arr2[i]);
                                free(result.arr2[i]);
                            }
                            printf("-----------\n");

                            free(result.arr2);
                        }
                    }
                    else
                    {
                        printf("Select a database first.\n");
                    }
                }
                else if (strcmp(argu1, "exit") == 0)
                {
                    if (dbInfo.properties)
                    {
                        closeDb();
                    }
                    break;
                }
                else
                {
                    printf("Command not found!\n\n");
                }
            }
        }

        printf("\n> ");
    }

    return 0;
}