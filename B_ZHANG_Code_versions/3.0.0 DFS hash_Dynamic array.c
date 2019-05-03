//
//  main.c
//  NTM
//
//  Created by Leo ZHANG on 24/07/2018.
//  Copyright © 2018 Leo ZHANG. All rights reserved.
//

//此project重点 1.用什么样的数据结构去储存TR
//             2.怎样管理ND

//修改方案。1.将tr存入table hash 用state 和read做映射 对nd部分用链表存（即用chaining解决冲突）
//        2.run时对nd按顺序走，走之前存position state 和 run。用一个链表储存走过的路（只存nd）
//        3.如果再遇见nd按照2的方法
//        4.在一个地方彻底走不通需要回弹时，删除走过路链表尾结点，哈希表链状往下推。如为U，记录
//        5.走到1就停，走到0，U就回弹，走完后有U就是U，没U就是零

//加一个，用双向链表来储存run




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUM_SECTION 10
#define HASHTABLE_SIZE 10000000
#define VAL_BAN 10000
#define HASH_MUL_VAL 2000


//公共变量
char section[NUM_SECTION];                               //用于大段识别
int max;

int acc1=VAL_BAN, acc2=VAL_BAN, acc3=VAL_BAN, acc4=VAL_BAN, acc5=VAL_BAN;
int acc6=VAL_BAN, acc7=VAL_BAN, acc8=VAL_BAN, acc9=VAL_BAN, acc10=VAL_BAN;
int size_stack;

char *section_run;
int position_run;


//hash struct（separate chaining）
struct hash_node_feasible_state{
    char write;
    char direction;
    int next_state;
    struct hash_node_feasible_state *next;
};


struct hashtbl{

    struct hash_node_feasible_state **node_of_feasible_state;
    
};

//hash variant hashsize=50000
struct hashtbl* head_of_hash;  //head_of_hash





//function
char identification_section(char *);
void read_tr(void);
void read_acc(void);
void put_numacc(int);
void all_for_run(void);
void read_run(void);
void do_run(void);
void change_write_dirction(struct hash_node_feasible_state*b);
bool judgment_acc(int);

//functions hash
unsigned int hash(const int,const char);
struct hashtbl* initializetable(void);
struct hash_node_feasible_state* find(int key_state,char key_read);
void insert(int key_state,char key_read,char write,char direction,int next_state);
void destroytable(void);

//struct and functions of stack for memory ND
struct nd_stack{
    
    int backup_for_state;
    int backup_position_run;
    struct hash_node_feasible_state* feasible_curr;
    char* backup_of_run;
    struct nd_stack* next_nd;
};

struct nd_stack* push(int backup_for_state,
                      int backup_position_run,
                      struct hash_node_feasible_state* feasible_curr,
                      char*backup_run,
                      struct nd_stack*);
struct nd_stack* pop_pointer_last(struct nd_stack*);





//function for test
//void print_hash_for_test(void);





//main
int main(void) {
    
    initializetable();
    if(identification_section(NULL)== 't' )                 //    读入tr段
    {
        read_tr();
//        print_hash_for_test();                              //测试用
                                                        //调试用
    }

    
    if(identification_section(section)== 'a' )                 //    读入acc段
    {
 //       printf("%c",section[0]);                            //测试识别情况
        read_acc();
 //       printf("\n%d %d %d %d %d %d %d %d %d %d\n",
 //              acc1,acc2,acc3,acc4,acc5,acc6,acc7,acc8,acc9,acc10);                //测试用
    }
    
    if(identification_section(section)== 'm' )                 //    读入max段
    {
 //       printf("%c\n",section[0]);                                 //测试用
        if(scanf("%d\n",&max))
            ;
  //      printf("%d\n",max);                           //测试用
        
    }
    
    if(identification_section(NULL)== 'r' )                 //    读入run段
    {
//        printf("\nit is run\n");                   //测试用
        all_for_run();
    }                                               //
    
    
    void destroytable(void);

    
    return 0;

    
}


//content function

/*
 识别是哪一段
    
*t---tr：La prima parte, che inizia con "tr", contiene le transizioni,
        una per linea - ogni carattere può essere separato dagli altri da spazi.
        Per es. 0 a c R 1 significa che si va dallo stato 0 allo stato 1, leggendo a e scrivendo c;
        la testina viene spostata a destra (R)
    
*a---acc：La parte successiva, che inizia con "acc", elenca gli stati di accettazione, uno per linea.
    
*m-max：Per evitare problemi di computazioni infinite, nella sezione successiva, che inizia con "max",
        viene indicato il numero di mosse massimo che si possono fare per accettare una stringa.
    
*r---run：La parte finale, che inizia con "run", è un elenco di stringhe da fornire alla macchina, una per linea.
*/

char identification_section(char *a){
    
    char str[10];
    
    if(a==NULL){
        if(scanf("%s\n",str)){
            return str[0];
        }
        else{
            fprintf(stderr, "Input Error");
            exit(EXIT_SUCCESS);
        }
    }
    else
        return a[0];
    
    
}


//读入tr，插入hash表
void read_tr(void){
    
    int a,e;
    char b,c,d;
    
    for(;;){
        if(scanf("%d %c %c %c %d\n",&a,&b,&c,&d,&e)>0)
            insert(a,b,c,d,e);
        else{
            if(scanf("%s",section)>0)
                ;
            break;
        }
    }
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                   *
 *                                                   *
 *               Function of Hash                    *
 *                                                   *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

unsigned int indexhash(const int key_state,const char key_read){
    
    int index;
    index=key_state*HASH_MUL_VAL+key_read;
    if(index>HASHTABLE_SIZE){
        fprintf(stderr, "Hash table is too small!!!");
        exit(EXIT_FAILURE);
    }
    return index;            //state乘100+read形成唯一索引
    
}


struct hashtbl* initializetable(void){
    
  
    
    //分配表空间
    head_of_hash=calloc(1, sizeof(struct hashtbl));
    if(head_of_hash==NULL){
        fprintf(stderr, "Out of space！！！！！");
        
    }
    
   //分配表内链空间
    head_of_hash->node_of_feasible_state=calloc(1,sizeof(struct hash_node_feasible_state*)*HASHTABLE_SIZE);   //存疑
    if(head_of_hash->node_of_feasible_state==NULL){
        fprintf(stderr, "Out of space！！！！！");
        
    }
    
    return head_of_hash;
    
    
    
}


struct hash_node_feasible_state* find(int key_state,char key_read){
    
    struct hash_node_feasible_state* l;
    if (key_state<0)
        return NULL;
    
    
    l=head_of_hash->node_of_feasible_state[indexhash(key_state, key_read)];

    return l;
    
}


void insert(int key_state,char key_read,char write,char direction,int next_state){
    
    int index_hash;
    struct hash_node_feasible_state *newcell;
    
    if(key_state<0 ||
       key_read<64 ||
       key_read>123){
        fprintf(stderr, "Input error!!!");
        
    }
    
    
    index_hash = indexhash(key_state, key_read);
    
    newcell=calloc(1,sizeof(struct hash_node_feasible_state));
    if(newcell==NULL){
        fprintf(stderr, "Out of space！！！！！For section_run");
        
    }
    newcell->write=write;
    newcell->direction=direction;
    newcell->next_state=next_state;
    newcell->next=NULL;
    
    if(find(key_state, key_read)==NULL){
        free(head_of_hash->node_of_feasible_state[index_hash]);
        head_of_hash->node_of_feasible_state[index_hash]=newcell;
    }
    else{
        newcell->next=head_of_hash->node_of_feasible_state[index_hash];
        head_of_hash->node_of_feasible_state[index_hash]=newcell;
    }
    
    
/*
//用于测试的打印
    printf("%d    %d %c %c %c %d\n",index_hash,
                                key_state,
                                key_read,
                                newcell->write,
                                newcell->direction,
                                newcell->next_state)
    ;
 */
    
    
}
 

/*
//打印hash表 测试用
void print_hash_for_test(void){
    
    int i;
    struct hash_node_feasible_state *p;
    
//    printf("\n\n\n\n");
    
    
    for (i=0; i<HASHTABLE_SIZE; i++){
        for(p=head_of_hash->node_of_feasible_state[i];
            p!=NULL;
            p=p->next)
                printf("i=%d   %c %c %d\n",i,p->write,p->direction,p->next_state);
    }
}
*/

void destroytable(void){
    
    int i;
    struct hash_node_feasible_state *a,*b;
    
    for (i=0; i<HASHTABLE_SIZE; i++) {
        if(head_of_hash->node_of_feasible_state[i]->next==NULL)
            free(head_of_hash->node_of_feasible_state[i]);
        else{
            a=head_of_hash->node_of_feasible_state[i];
            b=head_of_hash->node_of_feasible_state[i];
            for (a=a->next; b!=NULL; b=a,a=a->next)
                free(b);
        }
    }
    free(head_of_hash);
}




//读acc段
void read_acc(){
    
    int num_acc;
    
    for(;;){
        if(scanf("%d\n",&num_acc)<=0){
            if(scanf("%s\n",section))
                ;
            break;
        }
        else
            put_numacc(num_acc);
    }
}

void put_numacc(int a){
    
    if(acc1==VAL_BAN)
        acc1=a;
    else if(acc2==VAL_BAN)
        acc2=a;
    else if(acc3==VAL_BAN)
        acc3=a;
    else if(acc4==VAL_BAN)
        acc4=a;
    else if(acc5==VAL_BAN)
        acc5=a;
    else if(acc6==VAL_BAN)
        acc6=a;
    else if(acc7==VAL_BAN)
        acc7=a;
    else if(acc8==VAL_BAN)
        acc8=a;
    else if(acc9==VAL_BAN)
        acc9=a;
    else
        acc10=a;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                   *
 *                                                   *
 *               下面就是run 相关函数了                  *
 *                                                   *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */


void all_for_run(void){
    
    for(;;){
        size_stack=0;
        position_run=0;
        //read_run
        read_run();
        if(section_run[0]<64 || section_run[0]>123)
            break;          //没有可读取的就退出
//        puts(section_run);
        do_run();
        
        free(section_run);
    }
    free(section_run);
}


void read_run(void){

    char a;
    int i;
    
    section_run=calloc(1, sizeof(char));
    if (section_run==NULL) {
        fprintf(stderr, "Out of space！！！！！For backup_run");
    }
    
    for(i=1;;i++){
        if(scanf("%c",&a)<=0 || a=='\n')
            break;
        section_run[i-1]=a;
        section_run=realloc(section_run, (i+1)*sizeof(char));
    }
    section_run[i-1]='\0';
   
}



void do_run(void){
    
    int state_of_run=0;                            //用于表示当前状态
    struct hash_node_feasible_state*feasible=NULL;
    struct nd_stack*stack=NULL,*last_stack=NULL;
    int pri_max=max;
    bool have_u=0;
    bool acc_yes_no=0;
    char *backup_run;
    int ban_cont=0;
    int ban_index=0;
    char*for_recycle=NULL;
    char*for_recycle1=NULL;
    char*for_recycle2=NULL;
    char*for_recycle3=NULL;
    
    
    for(;;){
        
   /*
        printf("\n\n\n开始部分\n");
        puts(section_run);
        printf("State=%d\n",state_of_run);
        printf("char=%c\n",section_run[position_run]);
        printf("position=%d\n",position_run);
        printf("lentre=%lu\n\n",strlen(section_run));
    */
    
        /*
        for(i=100;i>=0;i--)
            ;
        if(i<0)
            exit(EXIT_FAILURE);
        */
        
        if(feasible==NULL){
            feasible=find(state_of_run, section_run[position_run]);
            if (indexhash(state_of_run, section_run[position_run])==ban_index) {
                feasible=NULL;
            }
        }
        if(feasible!=NULL){
            if(feasible->next!=NULL){                           //有分叉路
                
                if (for_recycle!=NULL)          //查查recycle内部有无野指针
                    backup_run=for_recycle;
                else if (for_recycle1!=NULL)
                    backup_run=for_recycle1;
                else if (for_recycle2!=NULL)
                    backup_run=for_recycle2;
                else if (for_recycle3!=NULL)
                    backup_run=for_recycle3;
                else{
                    backup_run=calloc(strlen(section_run)+10, sizeof(char));
                    if (backup_run==NULL) {
                        fprintf(stderr, "Out of space！！！！！For backup_run");
                    }
                }
                backup_run=realloc(backup_run, (strlen(section_run)+5)*sizeof(char));
                memcpy(backup_run, section_run, strlen(section_run));  //to do backup
                stack=push(state_of_run,position_run,feasible, backup_run,stack);
  
         /*
                printf("\n压栈！！！\n");
                puts(backup_run);//
                printf("State=%d\n",state_of_run);
                printf("char=%c\n",backup_run[position_run]);
                printf("position=%d\n",position_run);
                printf("lentre=%lu\n\n",strlen(section_run));
           */
                
            }
            //执行
            state_of_run=feasible->next_state;
            change_write_dirction(feasible);
            
            if(position_run==0 &&
               feasible->direction=='L' &&
                state_of_run==feasible->next_state)
            {
                ban_cont++;
                if (ban_cont>5) {
                    have_u=1;
                    ban_index=indexhash(state_of_run, section_run[position_run]);
                }
            }
            else
                ban_cont=0;
            
//            fprintf(stderr, "position_run->curr_char=%c\n",position_run->curr_char);
            
            feasible=NULL;
            pri_max--;
            
//            printf("pri_max%d\n",pri_max);
            
            if(pri_max<=0){
                state_of_run=-1;          //目的为使这个结点无效
                have_u=1;
                pri_max=max;
            }
            acc_yes_no=judgment_acc(state_of_run);
            if (acc_yes_no){
                printf("1\n");
                for_recycle1=section_run;
            //   if(realloc(section_run, 0)==NULL)
            //        ;//free
                break;
            }
        }
        else{
            if(size_stack!=0){
                
                for_recycle=section_run;
      //         if(realloc(section_run, 0)==NULL)
      //              ;//free
                
                
//                fprintf(stderr, "取出备份");
//                fprintf(stderr, "size_stack=%d\n",size_stack);
                //取出备份
                last_stack=pop_pointer_last(stack);
                state_of_run=last_stack->backup_for_state;
                position_run=last_stack->backup_position_run;
                last_stack->feasible_curr=last_stack->feasible_curr->next;
                feasible=last_stack->feasible_curr;
                section_run=last_stack->backup_of_run;
                
                
  /*
                printf("\n取回！！！\n");
                puts(section_run);
                printf("State=%d\n",state_of_run);
                printf("char=%c\n",section_run[position_run]);
                printf("position=%d\n",position_run);
                printf("lentre=%lu\n\n",strlen(section_run));
   */
                
 
                free(last_stack);
                last_stack=NULL;
                size_stack--;
            }
            else{
                if (have_u) {
                    printf("U\n");
                    for_recycle2=section_run;
                //   if(realloc(section_run, 0)==NULL)
                //        ;//free
                    break;
                }
                printf("0\n");
                for_recycle3=section_run;
            //    if(realloc(section_run, 0)==NULL)
            //        ;//free
                break;
            }
            
        }
    }
    
}

/*
 
 //测试专用
 int j=0;
 j++;
if(j>=1){
    fprintf(stderr, "进来了\n");
 
 }
 //测试专用
 
 */


void change_write_dirction(struct hash_node_feasible_state*feasible){
    
 //   fprintf(stderr, "position=%d\n",position_run);
  //  fprintf(stderr, "section_run[position_run]=%c\n",section_run[position_run]);
    
    
    section_run[position_run]=feasible->write;
    if(feasible->direction=='R'){
        position_run++;
        
        if (position_run>=strlen(section_run)) {
            
            section_run=realloc(section_run, (strlen(section_run)+5)*sizeof(char));
            
    /*
            printf("\ngo to R   ");
            puts(section_run);
            printf("position=%d\n",position_run);
            printf("lentre=%lu\n\n",strlen(section_run));
        */
      
            if (section_run==NULL){
                fprintf(stderr, "Out of space！！！！！For section_run");
                
            }
            section_run[position_run+1]='\0';
            section_run[position_run]='_';
            
            
 //           printf("reac之后   ");
 //           puts(section_run);//
        }
    }
    else if (feasible->direction=='L'){
        position_run--;
        if (position_run<0) {
            section_run=realloc(section_run, (strlen(section_run)+5)*sizeof(char));
            if (section_run==NULL){
                fprintf(stderr, "Out of space！！！！！For section_run");
            }
            memmove(section_run+1, section_run, strlen(section_run)+3);
            section_run[0]='_';
            position_run=0;
        }
    }
    else
        ;
    
    
    
}

bool judgment_acc(int acc_yes){
    
    if(acc_yes==acc1 ||
       acc_yes==acc2 ||
       acc_yes==acc3 ||
       acc_yes==acc4 ||
       acc_yes==acc5 ||
       acc_yes==acc6 ||
       acc_yes==acc7 ||
       acc_yes==acc8 ||
       acc_yes==acc9 ||
       acc_yes==acc10)
        return 1;
    return 0;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                   *
 *                                                   *
 *               function for stack                  *
 *                                                   *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */


struct nd_stack* push(int backup_for_state,
                      int backup_position_run,
                      struct hash_node_feasible_state* feasible_curr,
                      char*backup_run,
                      struct nd_stack*stack){
    
    struct nd_stack*new,*a;
    
    new=calloc(1,sizeof(struct nd_stack));
    if (new==NULL) {
        fprintf(stderr, "Out of space！！！！！For section_run");
        
    }
    new->backup_for_state=backup_for_state;
    new->backup_position_run=backup_position_run;
    new->feasible_curr=feasible_curr;
    new->backup_of_run=backup_run;
    new->next_nd=NULL;
    
    if(size_stack==0){
        stack=new;
//    fprintf(stderr, "注意这里 ！！！new->backup_for_state=%d\n",new->backup_for_state);
    }
    else{
        a=pop_pointer_last(stack);
        a->next_nd=new;
    }
    size_stack++;
    
 //   fprintf(stderr, "注意这里 ！！！stack->backup_for_state=%d\n",stack->backup_for_state);

    
    return stack;
}

struct nd_stack* pop_pointer_last(struct nd_stack*stack){
    
    struct nd_stack*a;
    int i=size_stack;
    
    a=stack;
    for (;i>1; i--)
        a=a->next_nd;
    return a;
}

