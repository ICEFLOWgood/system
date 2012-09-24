
#include<mysql/mysql.h>
//#include<Python.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include <locale.h>

#define HASH_TABLE_MAX_SIZE 10000//哈希数组大小；

typedef struct Infor_Struct//员工信息结构体；
{
    char myname[32];
    char abbreviation[32];
    char full[32];
    char company[32];
    char privation[32];
    char extension[32];
    char emall[32];
}INFOR;

typedef struct HashNode_Struct//定义哈希数组类型；
{
    char* sKey;
    INFOR* infor;
    struct HashNode_Struct*pNext;
}HashNode;

void display_header();
void hash_table_insert(const char* skey, INFOR* nvalue);
HashNode** hash_table_lookup(const char* skey);
int init_hash_table(char*DBName, char*DBUserName, char*DBUserPassword);
int update_hash_table(char*DBName, char*DBUserName, char*DBUserPassword);
void hash_table_release();

HashNode*hashTable[HASH_TABLE_MAX_SIZE];//定义哈希数组；
int hash_table_size;//哈希表实际大小；
int res;
MYSQL my_connection;
MYSQL_RES *res_ptr;
MYSQL_ROW sqlrow;

void hash_table_init()//初始化；
{
    hash_table_size = 0;
    memset(hashTable, 0, sizeof(HashNode*)*HASH_TABLE_MAX_SIZE);
}

unsigned int hash_table_hash_str(const char*skey)//字符串哈希算法函数；
{
    const signed char*p = (const signed char*)skey;
    unsigned int h = *p;
    if(h)
    {
        for(p+=1; *p!='\0'; p++)
        {
            h = (h<<5) - h + *p;
        }
    }
    return h;
}

        
int test()//测试函数；
{
    char DBName[] = "Telephone";
    char DBUserName[] = "root";
    char DBUserPassword[] = "IceFlow2012";
    
    init_hash_table(DBName, DBUserName, DBUserPassword);
 //   update_hash_table(DBName, DBUserName, DBUserPassword);
 /*int i;
 for (i=0; i<HASH_TABLE_MAX_SIZE; i++)
 {
	if(hashTable[i])
    {
        HashNode* pHead = hashTable[i];
        while(pHead)
        {
            printf("****%d\t%s\n", i, pHead->sKey);
            pHead = pHead->pNext;
        }
    }
}*/
   
    printf("哈希表实际大小：%d\n", hash_table_size);
    char a[256];
    printf("请输入要查询的信息：");
    scanf("%s", a);
    HashNode**ptest;
    ptest = hash_table_lookup(a);
    int i = 0;
    while((ptest[i]))
    {
        printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n", 
										((ptest[i]))->infor->myname,
                                        ((ptest[i]))->infor->abbreviation,
                                        ((ptest[i]))->infor->full,
                                        ((ptest[i]))->infor->company,
                                        ((ptest[i]))->infor->privation,
                                        ((ptest[i]))->infor->extension,
                                        ((ptest[i]))->infor->emall);
        i++;
    }
   
    return 0;
}

//free the memory of the hash table
void hash_table_release()//重载hash表数据之前，释放之前hash表资源；
{
    int i;
    for(i = 0; i < HASH_TABLE_MAX_SIZE; ++i)
    {
        if(hashTable[i])
        {
            HashNode* pHead = hashTable[i];
            while(pHead)
            {
                HashNode* pTemp = pHead;
                pHead = pHead->pNext;
                if(pTemp)
                {
                    free(pTemp->sKey);
                    free(pTemp->infor);
                    free(pTemp);
                }
            }
        }
    }
}


int update_hash_table(char*DBName, char*DBUserName, char*DBUserPassword)//重载hash表；
{
    hash_table_release();
    init_hash_table(DBName, DBUserName, DBUserPassword);
}

int init_hash_table(char*DBName, char*DBUserName, char*DBUserPassword)//初始化hash表；
{
    char mysql_server_ip[] = "172.16.20.50";
/*    char mysqlDB_name[] = "Telephone";
    char mysql_name[] = "root";
    char mysql_password[] = "IceFlow2012";
 */ 
    char*mysqlDB_name = DBName;
    char*mysql_name = DBUserName;
    char*mysql_password = DBUserPassword;
//printf("DB %s,DBu %s,DBp %s\n", mysqlDB_name,mysql_name,mysql_password);

    hash_table_init();//初始化哈希表大小和哈希数组；
    mysql_init(&my_connection);//初始化MYSQL结构；
    //数据库的连接；
    if(mysql_real_connect(&my_connection, 
                            mysql_server_ip, 
                            mysql_name,
                            mysql_password,
                            mysqlDB_name,
                            0,
                            NULL,
                            0))
    {
        printf("Connection success\n");
        res = mysql_query(&my_connection, "select * from staffs");//查询数据库信息；
        if(res)
        {
            printf("select error:%s\n", mysql_error(&my_connection));
            return -1;
        }
        else
        {
            res_ptr = mysql_store_result(&my_connection);//将检索到的数据储存到本地
            if(res_ptr)
            {
                printf("Retrieved %lu rows\n", (unsigned long)mysql_num_rows(res_ptr));//打印取回多少行；
                display_header();
                if(mysql_errno(&my_connection))
                {
                    fprintf(stderr, "Retrive error:%s\n", mysql_error(&my_connection));
                    return -1;
                }
            }
            mysql_free_result(res_ptr);//释放资源；
        }
        mysql_close(&my_connection);
        printf("Connection closed.\n");
    }
    else
    {
        fprintf(stderr, "Connection failed\n");
        if(mysql_errno(&my_connection))
        {
            fprintf(stderr,"Connection error %d:%s\n", 
                                        mysql_errno(&my_connection), //错误消息编号；
                                        mysql_error(&my_connection));//返回包含错误消息的、由NULL终结的字符串；
            return -1;
        }
    }
    return 0;
}

void display_header()//建立哈希表；
{
    unsigned int num_fields;
    unsigned int i;
    MYSQL_ROW row;
    INFOR* NewInfor;
    
    num_fields = mysql_num_fields(res_ptr);//返回结果集合中字段的数；
    while(row = mysql_fetch_row(res_ptr))//检索集合并且返回下一行；
    {
        for(i=1; i<num_fields; i++)//循环给每一行的每一个字段的信息进行hash计算，放入hash表；
        {  
            NewInfor = malloc(sizeof(INFOR));//给新的节点分配内存空间；
            memset(NewInfor, 0, sizeof(INFOR));
        
            strcpy(NewInfor->myname, row[1]?row[1]:"NULL");
            strcpy(NewInfor->abbreviation, row[2]?row[2]:"NULL");
            strcpy(NewInfor->full, row[3]?row[3]:"NULL");
            strcpy(NewInfor->company, row[4]?row[4]:"NULL");
            strcpy(NewInfor->privation, row[5]?row[5]:"NULL");
            strcpy(NewInfor->extension, row[6]?row[6]:"NULL");
            strcpy(NewInfor->emall, row[7]?row[7]:"NULL");
            
            hash_table_insert(row[i]?row[i]:"NULL", NewInfor);
// printf("line227 init_hashtable\t%s\n", row[i]?row[i]:"NULL");
        }
    }
}

void hash_table_insert(const char* skey, INFOR* nvalue)//向哈希表中插入元素；
{
    if(hash_table_size >= HASH_TABLE_MAX_SIZE)
    {
        printf("Out of hash table memory!\n");
        return;
    }
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;
    HashNode* pHead = hashTable[pos];
    while(pHead)
    {
        if(skey[0] == '\0')//空值跳过插入环节；
        {
            return;
        }
        if(strcmp(pHead->sKey, skey) == 0 && strcmp(pHead->infor->emall, nvalue->emall) == 0)//将同名不同邮箱信息放到同一个下标；
        {
            printf("%s already exists!\n", skey);
            return;
        }
        pHead = pHead->pNext;
    }
    //在hash表数组下标下建立链表；
    HashNode* pNewNode = (HashNode*)malloc(sizeof(HashNode));
    memset(pNewNode, 0, sizeof(HashNode));
    
    pNewNode->sKey = (char*)malloc(sizeof(char)*(strlen(skey)+1));//为结构体中的sKey分配空间；
    memset(pNewNode->sKey, 0, sizeof(char)*(strlen(skey)+1));
    
    strcpy(pNewNode->sKey, skey);
    pNewNode->infor = nvalue;
    pNewNode->pNext = hashTable[pos];
    hashTable[pos] = pNewNode;
    
    hash_table_size++;
}

HashNode* hn[64] = {0};//存储符合条件的节点的地址；
HashNode** hash_table_lookup(const char* skey)//查找哈希表数据；
{
    int i = 0;
	memset(hn, 0, sizeof(hn));
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;
    if(hashTable[pos])
    {
        HashNode* pHead = hashTable[pos];
        while(pHead)//查找下标相同的所有节点；
        {
            if(strcmp(skey, pHead->sKey) == 0)
            {
                hn[i++] = pHead;//将查找到的匹配的结构体指针地址，放到数组中；
                //return pHead;
            }
            pHead = pHead->pNext;
        }
        if(pHead == NULL)//如果查到链表尾部，返回数组；
        {
          return hn;
        }
    }
    return NULL;
}



