//
//  main.c
//  NTM
//
//  Created by Leo ZHANG on 24/07/2018.
//  Copyright © 2018 Leo ZHANG. All rights reserved.
//


//加一个，用双向链表来储存run
//3.x.x版本用dynamic array储存run
//4.x.x版本 删除stack里的section_run备份 firsttime_build_stackt从头开始 减少内存占用
//5.x.x版本 设定一个阈值 大于阈值的nd使用备份模式 小于阈值的nd使用restart模式 通过调整阈值在exec时间和memory占用之间达到静态平衡
//改进计划 1.重新审视整个程序 查找为什么时间相对全储存增加了 2.检查循环重构 3.改进压缩算法  4.思考是否可以将THRESHOLD_RESTART_BACKUP动态化

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HASH_TABLE_SIZE 100
#define HASHTABLESIZE_INCREASE 10
#define THRESHOLD_RESTART_BACKUP 10000 //0-全备份 ∞-不备份

//marco for judgment infinite loops
#define THRESHOLD_FOR_JUDG_INFINITE_LOOP 10000  //max大于此阈值才判断无限循环
#define MAXNUMS_OF_LOOP_PART 10  //最大循环结


//invariant public
char section[10];                               //用于大段识别
int max;
int* acc=NULL;
int size_stack;
char *section_run=NULL;
char *pri_run=NULL;
int position_run;
int max_times_for_loop; //超过此次数 就算是无限循环了



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

//hash variant
int hashtable_size=HASH_TABLE_SIZE;
struct hashtbl* head_of_hash;  //head_of_hash

//function
char identification_section(char *);
void read_tr(void);
void read_acc(void);
void all_for_run(void);
void read_run(void);
void do_run(void);
void change_write_dirction(struct hash_state_node*b);
bool judgment_acc(int);
int ban_index(int key_run);
struct hash_state_node* judgment_next_matched(struct hash_state_node*a);
int* compress_run(void);
char* decompression(int*);


//functions hash
int indexhash(const int);
struct hashtbl* initializetable(void);
struct hash_state_node* find(int key_state);
void insert(int key_state,char key_read,char write,char direction,int next_state);
void destroytable(void);

//struct and functions of stack for memory ND
struct nd_stack_for_restart{
    struct hash_state_node* feasible_curr;
    struct nd_stack_for_restart* next_nd;
};

//struct and functions of stack for memory ND
struct nd_stack_for_backup{
    
    int backup_for_state;
    int backup_position_run;
    struct hash_state_node* feasible_curr;
    int* backup_of_run;
    struct nd_stack_for_backup* next_nd;
    int nd_max;
};

//functions for stacks
struct nd_stack_for_restart* push_for_restart(struct hash_state_node* feasible_curr,
                    struct nd_stack_for_restart*);

struct nd_stack_for_restart* pop_pointer_last_for_restart(struct nd_stack_for_restart*);
void free_stack_restart(struct nd_stack_for_restart*);


struct nd_stack_for_backup* push_for_backup(int backup_for_state,
                                            struct hash_state_node* feasible_curr,
                                            int*backup_run,
                                            struct nd_stack_for_backup*,
                                            int push_max);

struct nd_stack_for_backup* pop_pointer_last_for_backup(struct nd_stack_for_backup*);
void free_stack_backup(struct nd_stack_for_backup*);

//function for test
//void print_hash_for_test(void);

//main
int main(void) {
    
    char* pri_section=NULL;
    
    initializetable();
    if(identification_section(NULL)== 't' )                 //    读入tr段
    {
        read_tr();
 //       print_hash_for_test();                              //测试用
                                                        //调试用
    }

    pri_section=section;
    if(identification_section(pri_section)== 'a' )                 //    读入acc段
    {
 //       printf("%c\n",section[0]);                            //测试识别情况
        read_acc();
       
        /*
                int i;
                for (i=0; acc[i]!='\0'; i++) {
                    printf("acc=%d\n",acc[i]);
                }
         */
        
    }
    
    if(identification_section(NULL)== 'm' )                 //    读入max段
    {
//        printf("%c\n",section[0]);                                 //测试用
        if(scanf("%d\n",&max))
            ;
//        printf("%d\n",max);                           //测试用
        
    }
    
    if(identification_section(NULL)== 'r' )                 //    读入run段
    {
  //      printf("\nit is run\n");                   //测试用
        all_for_run();
    }                                               //
    
    free(acc);
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
            exit(EXIT_FAILURE);
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
    head_of_hash=malloc(sizeof(struct hashtbl));
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
struct hash_state_node* find(int key_state){
    
  
    struct hash_state_node *a=NULL;
    
    if (key_state<0)  //when key_state=-1
        return false;
    
    a=head_of_hash->first_node_of_same_state[indexhash(key_state)];

    
    for (; a!=NULL; a=a->next) {
        if (a->read==pri_run[position_run])
            return a;
    }
    
    return NULL;
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
    
    newcell=malloc(sizeof(struct hash_state_node));
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
    free(head_of_hash->first_node_of_same_state);
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
    
    int i;
   
    acc=malloc(sizeof(int));
    if (acc==NULL) {
        fprintf(stderr, "Out of space！！！！！For pri_run");
    }
    
    for(i=1;;i++){
        if(scanf("%d\n",&acc[i-1])<=0)
            break;
        acc=realloc(acc, (i+1)*sizeof(int));
    }
    acc[i-1]='\0';
    
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                   *
 *                                                   *
 *               Function for run                    *
 *                                                   *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */



void all_for_run(void){
    
    max_times_for_loop=max/2500;
    
    for(;;){
        size_stack=0;
        position_run=0;
        //read_run
        read_run();
        if(section_run[0]<64 || section_run[0]>123)
            break;          //没有可读取的就退出
        do_run();
//        puts(section_run);
    }
    free(pri_run);
    free(section_run);
}

void read_run(void){

    char a;
    int i;
    unsigned long count_size;
    
    if(section_run==NULL) {
        count_size=10;
        section_run=malloc(count_size*sizeof(char));
        if (section_run==NULL) {
            fprintf(stderr, "Out of space！！！！！For pri_run");
        }
    }
    else
        count_size=strlen(section_run);
    
    for(i=1;;i++){
        if(scanf("%c",&a)<=0 || a=='\n')
            break;
        if (i>=count_size) {
            count_size=count_size+5;
            section_run=realloc(section_run, count_size*sizeof(char));
            if (section_run==NULL) {
                fprintf(stderr, "Out of space！！！！！For pri_run");
            }
        }
        section_run[i-1]=a;
    }
    section_run[i-1]='\0';
   
}


//最大函数
void do_run(void){
    
    
    int state_of_run=0;                            //用于表示当前状态
    struct nd_stack_for_restart*stack_for_restart=NULL,*stack_for_restart_pos=NULL,*last_stack_for_restart=NULL;
    struct nd_stack_for_backup*stack_for_backup=NULL,*last_stack_for_backup=NULL;
    int pri_max=max;
    bool acc_yes_no=0;
    int *backup_run;
    bool pickup_from_stack=false;
    int ban_cont=0;
    int baned_index=0;
    struct hash_state_node *matched_nd_first=NULL,*match_nd_next=NULL;
    bool have_u=0;
    
    // struct hash_state_node *ceshi=NULL;//test
    int pri_size_stack=0;
    bool firsttime_build_stack=true;
    bool infinite_loop=false;
    
    //for judgment infinite loops
    int size_of_array_loop=0; //包括了空字符
    int*array_loop=NULL;
    int i=0;
    bool refill=false;
    bool only_one_time=false;
    int times_of_loop=0;
    
    //back for run
    if(pri_run!=NULL){
        if (strlen(pri_run)+2<=strlen(section_run))
            pri_run=realloc(pri_run, (strlen(section_run)+3)*sizeof(char));
    }
    else{
        pri_run=malloc((strlen(section_run)+3)*sizeof(char));
        if (pri_run==NULL) {
            fprintf(stderr, "Out of space！！！！！For pri_run");
        }
    }
    memcpy(pri_run, section_run, strlen(section_run)+1);
    
    for(;;){
        
        /*
                //测试
                 printf("state=%d  pri_run[position_run]=%c   size_stack=%d  max=%d  times_of_loop=%d size_of_array_loop=%d  ",
                 state_of_run,pri_run[position_run],size_stack,pri_max,times_of_loop,size_of_array_loop);
                 puts(pri_run);
        */
        
        
        
        if (ban_index(state_of_run)!=baned_index){
            if(pickup_from_stack==false)
                matched_nd_first=find(state_of_run);
        }
        else
            matched_nd_first=NULL;
        
        if(matched_nd_first!=NULL || pickup_from_stack==true){
            match_nd_next=judgment_next_matched(matched_nd_first);
            if(match_nd_next!=NULL){                           //有分叉路
                if (size_stack<THRESHOLD_RESTART_BACKUP) {
                    if(firsttime_build_stack==true)
                        stack_for_restart=push_for_restart(matched_nd_first,stack_for_restart);
                    else{
                        if(pri_size_stack>0) {
                            matched_nd_first=stack_for_restart_pos->feasible_curr;
                            stack_for_restart_pos=stack_for_restart_pos->next_nd;
                            pri_size_stack--;
                        }
                        else
                            stack_for_restart=push_for_restart(matched_nd_first,stack_for_restart);
                    }
                }
                else{                           //exec for backup  size_stack>=THRESHOLD_RESTART_BACKUP
                    backup_run=compress_run();
                    stack_for_backup=push_for_backup(state_of_run,match_nd_next, backup_run,stack_for_backup,pri_max);
                }
            }
            
            pickup_from_stack=false;
            //执行matched_nd_first
            state_of_run=matched_nd_first->next_state;
            change_write_dirction(matched_nd_first);
            
            //ban model
            if(position_run==0 &&
                matched_nd_first->direction=='L' &&
                state_of_run==matched_nd_first->next_state)
            {
                ban_cont++;
                if (ban_cont>3) {
                    have_u=1;
                    baned_index=ban_index(state_of_run);
                }
            }
            else
                ban_cont=0;
            
            
            //无限循环判断 judgment infinite loops
            if(max>THRESHOLD_FOR_JUDG_INFINITE_LOOP && pri_max<=max*9999/10000){   //严格控制判断情况 提高运行效率
                if(refill==false){
                    if (array_loop==NULL) {
                        array_loop=malloc(2*sizeof(int));
                        if (array_loop==NULL)
                            fprintf(stderr, "Out of space！！！！！For section_run");
                        size_of_array_loop=2;
                        array_loop[0]=-2;
                        array_loop[1]=-2;
                    }
                    if (array_loop[0]==-2)
                        array_loop[0]=state_of_run;
                    else if(array_loop[0]!=state_of_run){
                        array_loop=realloc(array_loop, (++size_of_array_loop)*sizeof(int));
                        if (array_loop==NULL)
                            fprintf(stderr, "Out of space！！！！！For section_run");
                        array_loop[size_of_array_loop-1]=-2;
                        array_loop[size_of_array_loop-2]=state_of_run;
                        if (size_of_array_loop>MAXNUMS_OF_LOOP_PART) {
                            free(array_loop);
                            array_loop=NULL;
                            i=0;
                            refill=false;
                            only_one_time=false;
                            times_of_loop=0;
                        }
                    }
                    else{
                        if (array_loop[i]==-2){
                            i=0;
                            times_of_loop++;
                            if (times_of_loop>=max_times_for_loop)
                                pri_max=0;
                        }
                        if(array_loop[i++]!=state_of_run){   //周期内不一样
                            times_of_loop=0;
                            i=0;
                            size_of_array_loop++;
                            if (size_of_array_loop>MAXNUMS_OF_LOOP_PART) {
                                free(array_loop);
                                array_loop=NULL;
                            }
                            else{
                                refill=true;
                                only_one_time=true;
                            }
                        }
                    }
                }
                else{
                    if(only_one_time==true){
                        array_loop=realloc(array_loop, (++size_of_array_loop)*sizeof(int));
                        if (array_loop==NULL)
                            fprintf(stderr, "Out of space！！！！！For section_run");
                        array_loop[size_of_array_loop-1]=-2;
                        only_one_time=false;
                    }
                    if(i<size_of_array_loop-1)
                        array_loop[i++]=state_of_run;
                    else{
                        refill=false;
                        i=0;
                    }
                    
                }
            }
            
            
            
            
            //max 判断
            pri_max--;
            if(pri_max<=0){
                state_of_run=-1;          //目的为使这个结点无效
                have_u=1;
                infinite_loop=false;
                if (array_loop!=NULL) {
                    free(array_loop);
                    array_loop=NULL;
                    i=0;
                    refill=false;
                    only_one_time=false;
                    times_of_loop=0;
                }
            }
            
            //judgment_acc
            acc_yes_no=judgment_acc(state_of_run);
            if (acc_yes_no){
                printf("1\n");    //free stack stack-section...
                if (array_loop!=NULL) {
                    free(array_loop);
                    array_loop=NULL;
                    i=0;
                    refill=false;
                    only_one_time=false;
                    times_of_loop=0;
                }
                if (size_stack<=THRESHOLD_RESTART_BACKUP) {
                    if(size_stack!=0){
                        free_stack_restart(stack_for_restart);
                        stack_for_restart=NULL;
                    }
                }
                else{
                    free_stack_backup(stack_for_backup);
                    stack_for_backup=NULL;
                    if(THRESHOLD_RESTART_BACKUP!=0){
                        free_stack_restart(stack_for_restart);
                        stack_for_restart=NULL;
                    }
                }
                break;
            }
        }
        else{
            if (size_stack<=THRESHOLD_RESTART_BACKUP) {
                if(size_stack!=0){
                    //No realloc,for save times
                    memcpy(pri_run, section_run, strlen(section_run)+1);
                    state_of_run=0;
                    pri_max=max;
                    position_run=0;
                    //循环结还原
                    if (array_loop!=NULL) {
                        free(array_loop);
                        array_loop=NULL;
                        i=0;
                        refill=false;
                        only_one_time=false;
                        times_of_loop=0;
                    }
                    //修改stack
                    last_stack_for_restart=pop_pointer_last_for_restart(stack_for_restart);
                    match_nd_next=judgment_next_matched(last_stack_for_restart->feasible_curr);
                    if (match_nd_next!=NULL){
                        last_stack_for_restart->feasible_curr=match_nd_next;
                    }
                    else{
                        while (match_nd_next==NULL && size_stack>0) {
                            free(last_stack_for_restart);
                            last_stack_for_restart=NULL;
                            size_stack--;
                            if (size_stack<=0){
                                stack_for_restart=NULL;
                                stack_for_restart_pos=NULL;
                                break;
                            }
                            last_stack_for_restart=pop_pointer_last_for_restart(stack_for_restart);
                            match_nd_next=judgment_next_matched(last_stack_for_restart->feasible_curr);
                        }
                        if (size_stack>0)
                            last_stack_for_restart->feasible_curr=match_nd_next;
                    }
                        stack_for_restart_pos=stack_for_restart;
                        pri_size_stack=size_stack;
                        firsttime_build_stack=false;
                }
                if(size_stack==0){
                    if (array_loop!=NULL) {
                        free(array_loop);
                        array_loop=NULL;
                        i=0;
                        refill=false;
                        only_one_time=false;
                        times_of_loop=0;
                    }
                    if (have_u) {
                        printf("U\n");
                        break;
                    }
                    printf("0\n");
                    break;
                }
            }
            else{    //for backup size_stack>THRESHOLD_RESTART_BACKUP
                
                if (array_loop!=NULL) {
                    free(array_loop);
                    array_loop=NULL;
                    i=0;
                    refill=false;
                    only_one_time=false;
                    times_of_loop=0;
                }
                
                free(pri_run);
                
                //取出备份
                last_stack_for_backup=pop_pointer_last_for_backup(stack_for_backup);
                state_of_run=last_stack_for_backup->backup_for_state;
                position_run=last_stack_for_backup->backup_position_run;
                pri_run=decompression(last_stack_for_backup->backup_of_run);
                pri_max=last_stack_for_backup->nd_max;
                matched_nd_first=last_stack_for_backup->feasible_curr;
                free(last_stack_for_backup->backup_of_run);

                pickup_from_stack=true;
                free(last_stack_for_backup);
                last_stack_for_backup=NULL;
                size_stack--;
            }
        }
    }//for "for(;;)"
}


struct hash_state_node* judgment_next_matched(struct hash_state_node*a){
    
    char run_char=a->read;
    for (a=a->next; a!=NULL; a=a->next) {
        if(a->read==run_char)
            return a;
    }
    return NULL;
}


int ban_index(int key_run){
    
    int banindex;
    
    banindex=key_run*1000+pri_run[position_run]; //pay attention to pri_run
    return banindex;
}



void change_write_dirction(struct hash_state_node*feasible){
    
    pri_run[position_run]=feasible->write;
    if(feasible->direction=='R'){
        position_run++;
        
        if (position_run>=strlen(pri_run)) {
            
            pri_run=realloc(pri_run, (strlen(pri_run)+5)*sizeof(char));
            if (pri_run==NULL){
                fprintf(stderr, "Out of space！！！！！For section_run");
                
            }
            pri_run[position_run+1]='\0';
            pri_run[position_run]='_';
        }
    }
    else if (feasible->direction=='L'){
        position_run--;
        if (position_run<0) {
            pri_run=realloc(pri_run, (strlen(pri_run)+5)*sizeof(char));
            if (pri_run==NULL){
                fprintf(stderr, "Out of space！！！！！For section_run");
            }
            memmove(pri_run+1, pri_run, strlen(pri_run)+3);
            pri_run[0]='_';
            position_run=0;
        }
    }
    else
        ;
}

bool judgment_acc(int acc_yes){
    
    int i;
    
    for (i=0;acc[i]!='\0';i++) {
        if(acc[i]==acc_yes)
            return true;
    }
    return false;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                   *
 *                                                   *
 *               function for stack                  *
 *                                                   *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */


//for restart
struct nd_stack_for_restart* push_for_restart(struct hash_state_node* feasible_curr,
                    struct nd_stack_for_restart*stack){
    
    struct nd_stack_for_restart*new,*a;
    
    new=malloc(sizeof(struct nd_stack_for_restart));
    if (new==NULL) {
        fprintf(stderr, "Out of space！！！！！For section_run");
    }
    new->feasible_curr=feasible_curr;
    new->next_nd=NULL;
    
    if(size_stack==0){
        stack=new;
    }
    else{
        a=pop_pointer_last_for_restart(stack);
        a->next_nd=new;
    }
    size_stack++;
    
    return stack;
}


struct nd_stack_for_restart* pop_pointer_last_for_restart(struct nd_stack_for_restart*stack){
    
    struct nd_stack_for_restart*a;
    int i=size_stack;
    
    a=stack;
    for (;i>1; i--)
        a=a->next_nd;
    return a;
}


//free
void free_stack_restart(struct nd_stack_for_restart*stack){

    struct nd_stack_for_restart*a;

    
    
    for (a=stack,a=a->next_nd;
         size_stack>1;
         stack=a,a=a->next_nd,size_stack--){
        free(stack);
    }
    free(stack);
    stack=NULL;
    size_stack=0;
}

//for backup
struct nd_stack_for_backup* push_for_backup(int backup_for_state,
                                            struct hash_state_node* feasible_curr,
                                            int*backup_run,
                                            struct nd_stack_for_backup*stack,
                                            int push_max){
    
    struct nd_stack_for_backup*new,*a;
    
    new=malloc(sizeof(struct nd_stack_for_backup));
    if (new==NULL) {
        fprintf(stderr, "Out of space！！！！！For section_run");
        
    }
    new->backup_for_state=backup_for_state;
    new->backup_position_run=position_run;
    new->feasible_curr=feasible_curr;
    new->backup_of_run=backup_run;
    new->nd_max=push_max;
    new->next_nd=NULL;
    
    if(size_stack==THRESHOLD_RESTART_BACKUP){
        stack=new;
    }
    else{
        a=pop_pointer_last_for_backup(stack);
        a->next_nd=new;
    }
    size_stack++;
    
    return stack;
}


struct nd_stack_for_backup* pop_pointer_last_for_backup(struct nd_stack_for_backup*stack){
    
    struct nd_stack_for_backup*a;
    int i=size_stack-THRESHOLD_RESTART_BACKUP;
    
    a=stack;
    for (;i>1; i--)
    a=a->next_nd;
    return a;
}


//free
void free_stack_backup(struct nd_stack_for_backup*stack){
    
    struct nd_stack_for_backup*a;
    int pri_size_stack=size_stack-THRESHOLD_RESTART_BACKUP;
    
    for (a=stack,a=a->next_nd;
         pri_size_stack>1;
         stack=a,a=a->next_nd,pri_size_stack--){
        free(stack->backup_of_run);
        free(stack);
    }
    free(stack->backup_of_run);
    free(stack);
    stack=NULL;
    size_stack=THRESHOLD_RESTART_BACKUP;
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                   *
 *                                                   *
 *               Function for compress               *
 *                                                   *
 *                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

int* compress_run(void){
    
    int i,j=0;
    int x;
    int*a;
    int count_size=7;
    
    
    a=malloc(count_size*sizeof(int));
    if (a==NULL) {
        fprintf(stderr, "Out of space！！！！！For backup_run");
    }
    for(i=0;;i=i+2){
        if (i+2>=count_size) {
            count_size=count_size+7;
            a=realloc(a, count_size*sizeof(int));
            if (a==NULL) {
                fprintf(stderr, "Out of space！！！！！For backup_run");
            }
        }
        for (x=j; pri_run[j]!='\0'&& pri_run[x]==pri_run[j] ; j++) {
            ;
        }
        a[i]=(int)pri_run[x];
        a[i+1]=j-x;
        if (pri_run[j]=='\0') {
            a[i+2]=(int)'\0';
            break;
        }
    }
    return a;
}


char* decompression(int*a){
    
    int i,j=0,k;
    char*b;
    unsigned long count_size=strlen(pri_run);
    
    b=pri_run;                          //重构
    for (i=0; a[i]!=(int)'\0';i=i+2){
        for (k=a[i+1];k>0 ;k--,j++) {
            if (j+1>=count_size){
                count_size=count_size+7;
                b=realloc(b, count_size*sizeof(char));
                if (b==NULL) {
                    fprintf(stderr, "Out of space！！！！！For backup_run");
                    
                }
            }
            b[j]=(char)a[i];
        }
    }
    b[j]='\0';
    
    /*
     //测试
     printf("count_size=%d \n",count_size);
     for(i=0;b[i]!='\0' && i<count_size;i++){
     printf("%c",b[i]);
     }
     printf("\n");
     if(b[i]!='\0'){
     printf("内部count_size=%d \n",count_size);
     printf("wwttff!!!!@!@\n");
     }
     */
    
    return b;
    
}

