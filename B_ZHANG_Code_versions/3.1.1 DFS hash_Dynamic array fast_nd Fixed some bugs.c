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

//版本3.1.0 仅用stato作为hash index，之后再来搜索储存
//删除





//加一个，用双向链表来储存run
//3.x.x版本用dynamic array储存run



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUM_SECTION 10
#define VAL_BAN 10000
#define HASHTABLESIZE_INCREASE 10


//公共变量
char section[NUM_SECTION];                               //用于大段识别
int max;

int acc1=VAL_BAN, acc2=VAL_BAN, acc3=VAL_BAN, acc4=VAL_BAN, acc5=VAL_BAN;
int acc6=VAL_BAN, acc7=VAL_BAN, acc8=VAL_BAN, acc9=VAL_BAN, acc10=VAL_BAN;
int size_stack;

char *section_run;
int position_run;
int ban_cont=0;
int baned_index=0;
bool have_u=0;
struct matched_state_read*from_find=NULL;
struct hash_state_node *matched_at_find=NULL;


//hash struct（separate chaining）
struct hash_state_node{
    char read;
    char write;
    char direction;
    int next_state;
    struct hash_state_node *next;
};


struct hashtbl{

    struct hash_state_node **first_node_of_same_state;
    
};

struct matched_state_read{
    
    struct hash_state_node *state_read_match;
    struct matched_state_read *next_matched;
    
};

//hash variant
int hashtable_size=100;
struct hashtbl* head_of_hash;  //head_of_hash





//function
char identification_section(char *);
void read_tr(void);
void read_acc(void);
void put_numacc(int);
void all_for_run(void);
void read_run(void);
void do_run(void);
void change_write_dirction(struct hash_state_node*b);
bool judgment_acc(int);
int ban_index(int key_run,char key_read);


//functions hash
int indexhash(const int);
struct hashtbl* initializetable(void);
int find(int key_state,char key_read);
void insert(int key_state,char key_read,char write,char direction,int next_state);
void destroytable(void);

//struct and functions of stack for memory ND
struct nd_stack{
    
    int backup_for_state;
    int backup_position_run;
    struct matched_state_read* feasible_curr;
    char* backup_of_run;
    struct nd_stack* next_nd;
    int max;
};

struct nd_stack* push(int backup_for_state,
                      int backup_position_run,
                      struct matched_state_read* feasible_curr,
                      char*backup_run,
                      struct nd_stack*,
                      int max);

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
//        printf("%c",section[0]);                            //测试识别情况
        read_acc();
//        printf("\n%d %d %d %d %d %d %d %d %d %d\n",
//               acc1,acc2,acc3,acc4,acc5,acc6,acc7,acc8,acc9,acc10);                //测试用
    }
    
    if(identification_section(section)== 'm' )                 //    读入max段
    {
//        printf("%c\n",section[0]);                                 //测试用
        if(scanf("%d\n",&max))
            ;
//        printf("%d\n",max);                           //测试用
        
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


 int indexhash(const int key_state){
    
    int index;
     int old_hashtable_size=hashtable_size;
     
    index=key_state;
     
     
     
     //问题点！！！
     if(index>=hashtable_size){
         
         while (index>=hashtable_size)
             hashtable_size=hashtable_size+HASHTABLESIZE_INCREASE;
         
 //        printf("hashtable_suze=%d \n",hashtable_size);
         head_of_hash->first_node_of_same_state=realloc(head_of_hash->first_node_of_same_state,
                                                        hashtable_size*sizeof(struct hash_state_node*));
         //初始化
         for(;old_hashtable_size<hashtable_size;old_hashtable_size++)
             head_of_hash->first_node_of_same_state[old_hashtable_size]=NULL;
         
         if(head_of_hash->first_node_of_same_state==NULL){
             fprintf(stderr, "Out of space！！！！！");
         }
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
    head_of_hash->first_node_of_same_state=calloc(hashtable_size,sizeof(struct hash_state_node*));   //存疑
    if(head_of_hash->first_node_of_same_state==NULL){
        fprintf(stderr, "Out of space！！！！！");
    }
    
    return head_of_hash;
    
}


//返回串成串的指针
int find(int key_state,char key_read){
    
    
    
    struct matched_state_read *newnode=NULL,*first=NULL;     //问题：matched什么时候free
    int count_nd=0;
    struct hash_state_node *a=NULL;
    
    if (key_state==-1)
        return 0;
    
    a=head_of_hash->first_node_of_same_state[indexhash(key_state)];

    
    for (; a!=NULL; a=a->next) {
        if (a->read==key_read) {
            count_nd++;
            if (count_nd>=2){
                if (first==NULL) {
                    
                    first=calloc(1, sizeof(struct matched_state_read));
                    if(first==NULL){
                        fprintf(stderr, "Out of space！！！！！");
                    }
                    first->state_read_match=matched_at_find;
                    
                    newnode=calloc(1, sizeof(struct matched_state_read));
                    if(newnode==NULL){
                        fprintf(stderr, "Out of space！！！！！");
                    }
                    newnode->state_read_match=a;
                    first->next_matched=newnode;
                    newnode->next_matched=NULL;
                }
                else{
                    newnode=calloc(1, sizeof(struct matched_state_read));
                    newnode->state_read_match=a;
                    newnode->next_matched=first;
                    first=newnode;
                }
            }
            
            matched_at_find=a;
        }
    }
    from_find=first;
    return count_nd;
}



void insert(int key_state,char key_read,char write,char direction,int next_state){
    
     int index_hash;
    struct hash_state_node *newcell;
    
    if(key_state<0 ||
       key_read<64 ||
       key_read>123){
        fprintf(stderr, "Input error!!!");
    }
    
    
    index_hash = indexhash(key_state);
    
    newcell=calloc(1,sizeof(struct hash_state_node));
    if(newcell==NULL){
        fprintf(stderr, "Out of space！！！！！For section_run");
        
    }
    newcell->read=key_read;
    newcell->write=write;
    newcell->direction=direction;
    newcell->next_state=next_state;
    newcell->next=NULL;
    
    if(head_of_hash->first_node_of_same_state[index_hash]==NULL){
        head_of_hash->first_node_of_same_state[index_hash]=newcell;
    }
    else{
        newcell->next=head_of_hash->first_node_of_same_state[index_hash];
        head_of_hash->first_node_of_same_state[index_hash]=newcell;
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
 


//打印hash表 测试用
/*
void print_hash_for_test(void){
    
    int i;
    struct hash_state_node *p;
    
//    printf("\n\n\n\n");
    
    
    for (i=0; i<hashtable_size; i++){
        for(p=head_of_hash->first_node_of_same_state[i];
            p!=NULL;
            p=p->next)
                printf("i=%d  %c %c %c %d\n",i,p->read,p->write,p->direction,p->next_state);
    }
}
*/

void destroytable(void){
    
    int i;
    struct hash_state_node *a,*b;
    
    for (i=0; i<hashtable_size; i++) {
        if(head_of_hash->first_node_of_same_state[i]->next==NULL)
            free(head_of_hash->first_node_of_same_state[i]);
        else{
            a=head_of_hash->first_node_of_same_state[i];
            b=a;
            for (a=a->next; b!=NULL; b=a,a=a->next)
                free(b);
        }
    }
    free(head_of_hash);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                   *
 *                                                   *
 *               Function for acc                    *
 *                                                   *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
 *               Function for run                    *
 *                                                   *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */



void all_for_run(void){
    
    for(;;){
        size_stack=0;
        position_run=0;
        ban_cont=0;
        baned_index=0;
        have_u=0;
        //read_run
        read_run();
        if(section_run[0]<64 || section_run[0]>123)
            break;          //没有可读取的就退出
        do_run();
//        puts(section_run);

        free(section_run);
        section_run=NULL;
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
    struct matched_state_read *feasible=NULL,*for_free_feasible=NULL;
    struct matched_state_read *free_for_ban1=NULL,*free_for_ban2=NULL;
   // struct matched_state_read*ceshi=NULL;
    struct nd_stack*stack=NULL,*last_stack=NULL;
    int pri_max=max;
    bool acc_yes_no=0;
    char *backup_run;
    int how_much_nd=-1;
    bool pickup_from_stack=false;
   
    
    
    
    for(;;){
        
        
        /*
                printf("state=%d  %c  hownuchnd=%d  size_stack=%d   ",state_of_run,section_run[position_run],how_much_nd,size_stack);
                puts(section_run);
          */
        
        
        if (ban_index(state_of_run, section_run[position_run])!=baned_index){
            if(pickup_from_stack==false)
                how_much_nd=find(state_of_run, section_run[position_run]);
        }
        else{
            if(pickup_from_stack==true){
                if (feasible!=NULL){
                    if (feasible->next_matched!=NULL) {
                        for (free_for_ban1=feasible,free_for_ban1=free_for_ban1->next_matched;
                             feasible!=NULL;
                             feasible=free_for_ban1,free_for_ban1=free_for_ban1->next_matched) {
                            free(feasible);
                            feasible=NULL;
                        }
                    }
                    else
                        free(feasible);
                }
            }
            how_much_nd=0;
        }

        if(how_much_nd>0 || pickup_from_stack==true){
            if(how_much_nd>=2 || pickup_from_stack==true){
                
               
                
                if (pickup_from_stack==false)
                    feasible=from_find;
                
                //ban model
                if(position_run==0 &&
                   feasible->state_read_match->direction=='L' &&
                   state_of_run==feasible->state_read_match->next_state)
                {
                    ban_cont++;
                    if (ban_cont>3) {
                        have_u=1;
                        baned_index=ban_index(state_of_run, section_run[position_run]);
                        if (feasible->next_matched!=NULL) {
                            for (free_for_ban2=feasible,free_for_ban2=free_for_ban2->next_matched;
                                 feasible->next_matched!=NULL;
                                 feasible=free_for_ban2,free_for_ban2=free_for_ban2->next_matched) {
                                free(feasible);
                                feasible=NULL;
                            }
                        }
                    }
                }
                else
                    ban_cont=0;
                
                
                
                if(feasible->next_matched!=NULL){                           //有分叉路
                    backup_run=calloc(strlen(section_run)+5, sizeof(char));
                    if (backup_run==NULL) {
                        fprintf(stderr, "Out of space！！！！！For backup_run");
                    }
                    memcpy(backup_run, section_run, strlen(section_run));  //to do backup
                    stack=push(state_of_run,position_run,feasible, backup_run,stack,pri_max);
                }
                pickup_from_stack=false;
                //执行
                state_of_run=feasible->state_read_match->next_state;
                change_write_dirction(feasible->state_read_match);
                
                
                if (feasible->next_matched==NULL){
                    free(feasible);
                    feasible=NULL;
                }
        
            }
            else{                        //执行matched_at_find
                state_of_run=matched_at_find->next_state;
                change_write_dirction(matched_at_find);
                
                //ban model
                if(position_run==0 &&
                   matched_at_find->direction=='L' &&
                   state_of_run==matched_at_find->next_state)
                {
                    ban_cont++;
                    if (ban_cont>5) {
                        have_u=1;
                        baned_index=ban_index(state_of_run, section_run[position_run]);
                    }
                }
                else
                    ban_cont=0;
            }
            
            //max 判断
            pri_max--;
            if(pri_max<=0){
                state_of_run=-1;          //目的为使这个结点无效
                have_u=1;
                pri_max=max;
            }
            
            acc_yes_no=judgment_acc(state_of_run);
            if (acc_yes_no){
                printf("1\n");
                free(section_run);
                section_run=NULL;
                break;
            }
        }
        else{
            
            if(size_stack!=0){
            
                free(section_run);
                section_run=NULL;
                
                //取出备份
                last_stack=pop_pointer_last(stack);
                state_of_run=last_stack->backup_for_state;
                position_run=last_stack->backup_position_run;
                for_free_feasible=last_stack->feasible_curr;
                last_stack->feasible_curr=last_stack->feasible_curr->next_matched;
                free(for_free_feasible);
                feasible=last_stack->feasible_curr;
                section_run=last_stack->backup_of_run;
                pri_max=last_stack->max;
                
                
                /*
                        //测试
                        for (ceshi=feasible; ceshi!=NULL; ceshi=ceshi->next_matched) {
                            printf("取备份 next_state=%d write=%c  ",ceshi->state_read_match->next_state,ceshi->state_read_match->write);
                            puts(section_run);
                        }
                    */
                
                
                pickup_from_stack=true;
                free(last_stack);
                last_stack=NULL;
                size_stack--;
                
                /*
                        //测试
                        printf("取出备份 state=%d write=%c  ",state_of_run,section_run[position_run]);
                        puts(section_run);
                */
                
            }
            else{
                
                if (have_u) {
                    printf("U\n");
                    free(section_run);
                    section_run=NULL;
                    //free
                    break;
                }
                printf("0\n");
                free(section_run);
                section_run=NULL;
                //free
                break;
            }
        }
    }//for "for(;;)"
}


int ban_index(int key_run,char key_read){
    
    int banindex;
    
    banindex=key_run*1000+key_read;
    return banindex;
}



void change_write_dirction(struct hash_state_node*feasible){
    
    section_run[position_run]=feasible->write;
    if(feasible->direction=='R'){
        position_run++;
        
        if (position_run>=strlen(section_run)) {
            
            section_run=realloc(section_run, (strlen(section_run)+5)*sizeof(char));
            if (section_run==NULL){
                fprintf(stderr, "Out of space！！！！！For section_run");
                
            }
            section_run[position_run+1]='\0';
            section_run[position_run]='_';
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
                      struct matched_state_read* feasible_curr,
                      char*backup_run,
                      struct nd_stack*stack,
                      int max){
    
    struct nd_stack*new,*a;
    
    new=calloc(1,sizeof(struct nd_stack));
    if (new==NULL) {
        fprintf(stderr, "Out of space！！！！！For section_run");
        
    }
    new->backup_for_state=backup_for_state;
    new->backup_position_run=backup_position_run;
    new->feasible_curr=feasible_curr;
    new->backup_of_run=backup_run;
    new->max=max;
    new->next_nd=NULL;
    
    if(size_stack==0){
        stack=new;
    }
    else{
        a=pop_pointer_last(stack);
        a->next_nd=new;
    }
    size_stack++;
    
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


