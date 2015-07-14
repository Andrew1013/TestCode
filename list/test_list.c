#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <libgen.h>
#include <unistd.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


#include "avct_list.h"

typedef struct _sTestListNode{
    struct list_head list;
    int data;
}sTestListNode;

sTestListNode  TestListNode;


unsigned int GetRandUint()  //Do not link pm lib
{
    FILE *randomData=NULL;
    unsigned int myRandomInteger=0;
    size_t tmp_read;

    randomData = fopen("/dev/urandom", "r");
    if (randomData)
    {
        tmp_read = fread(&myRandomInteger, sizeof(myRandomInteger), 1, randomData);
        fclose(randomData);
    }

    return myRandomInteger;
}

static void PrintHelp(char *prog_name)
{
    printf("usage:\n");
    printf("    %s [options] - <n|d>\n", prog_name);
    printf("options:\n");
    printf("    -n, --number            number of list\n");
    printf("    -d, --delay             delay interval in seconds for removing item from list\n");
    printf("\n");
}
static int ListCreateRandom(struct list_head* header, int iMax)
{
    int             iCounter=0;
    sTestListNode   *pTmpNode;
    struct list_head *pos, *next;

    iCounter=0;
    while(iCounter < iMax)
    {
        pTmpNode=(sTestListNode *)malloc(sizeof(sTestListNode));        
        if(pTmpNode)
        {
            pTmpNode->data = iCounter;

            if (GetRandUint()%2==0) {
                avct_list_add(&pTmpNode->list, header);
            } else {
                avct_list_add_tail(&pTmpNode->list, header);
            }
        }
        else
        {
            avct_list_for_each(pos, next, header)
            {
                pTmpNode = avct_list_entry(pos, sTestListNode, list);

                avct_list_del(pos, header);
                free(pTmpNode);
            }
            printf("sTestListNode malloc fail\n");
            return EXIT_FAILURE;
        }
        iCounter ++;
    }
    return EXIT_SUCCESS;
}

static void ListPrint(struct list_head* header)
{
    sTestListNode   *pTmpNode;
    struct list_head *pos, *next;

    printf("List ");
    avct_list_for_each(pos, next, header)
    {
        pTmpNode = avct_list_entry(pos, sTestListNode, list);
        printf("--> %d", pTmpNode->data);
    }
    printf("\n");

    return;
}

int main(int argc, char **argv)
{
    int iNum=10;
    int iDelay=0;

    int c;
    int iCounter=0;

    int *iArray=NULL;
    sTestListNode   *pTmpNode;
    struct list_head *pos, *next;

    struct option long_options[] =
    {
        {"number",  required_argument, NULL, 'n'},
        {"delay",   required_argument, NULL, 'd'},
        {0, 0, 0, 0}
    };

    while((c = getopt_long_only(argc, argv, "n:d:", long_options, NULL)) != -1)
    {
        switch (c)
        {
            case 'n':
                iNum =atoi(optarg);
                break;
            case 'd':
                iDelay = atoi(optarg);
                break;
            default:
                /* '?' */
                PrintHelp(basename(argv[0]));
                return EXIT_FAILURE;
        }
    }
    //printf("%s:%d\n", __FUNCTION__, __LINE__);

    AVCT_INIT_LIST_HEAD(&TestListNode.list);

    if(ListCreateRandom(&TestListNode.list,iNum) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    iArray=malloc(iNum*sizeof(int));
    if(!iArray)
    {
        printf("iArray malloc fail\n");
        return EXIT_FAILURE;
    }

    printf("Array ");
    iCounter=0;
    avct_list_for_each(pos, next, &TestListNode.list) //delete list node
    {
        pTmpNode = avct_list_entry(pos, sTestListNode, list);
        iArray[iCounter++]= pTmpNode->data;
        printf("--> %d", pTmpNode->data);
        avct_list_del(pos, &TestListNode.list);
        free(pTmpNode);
    }
    printf("\n\n");

    if(ListCreateRandom(&TestListNode.list,iNum) != EXIT_SUCCESS)
    {
        free(iArray);
        return EXIT_FAILURE;
    }

    for(iCounter=0; iCounter<iNum; iCounter++)
    {        
        ListPrint(&TestListNode.list);
        printf("Remove %d from list\n", iArray[iCounter]);
        if (iDelay)
            sleep(iDelay);

        avct_list_for_each(pos, next, &TestListNode.list)
        {
            pTmpNode = avct_list_entry(pos, sTestListNode, list);
            if (pTmpNode->data == iArray[iCounter])
            {
                avct_list_del(pos, &TestListNode.list);
                free(pTmpNode);
                break;
            }
        }
    }

    printf("Completed\n");
    return EXIT_SUCCESS;
}


