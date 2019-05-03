//
//  main.c
//  NTM
//
//  Created by Leo ZHANG on 24/07/2018.
//  Copyright © 2018 Leo ZHANG. All rights reserved.
//

//此project重点 1.用什么样的数据结构去储存TR
//             2.怎样管理ND






//加一个，用双向链表来储存run
//3.x.x版本用dynamic array储存run
//4.x.x版本 删除stack里的section_run备份 firsttime_build_stackt从头开始 减少内存占用

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


int* acc=NULL;
int size_stack;
char *section_run;
char *pri_run;
int position_run;



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
int hashtable_size=100;
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


//functions hash
int indexhash(const int);
struct hashtbl* initializetable(void);
struct hash_state_node* find(int key_state);
void insert(int key_state,char key_read,char write,char direction,int next_state);
void destroytable(void);

//struct and functions of stack for memory ND
struct nd_stack{
    struct hash_state_node* feasible_curr;
    struct nd_stack* next_nd;
};

struct nd_stack* push(struct hash_state_node* feasible_curr,
                    struct nd_stack*);

struct nd_stack* pop_pointer_last(struct nd_stack*);
void free_stack(struct nd_stack*);





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
 //   void destroytable(void);

    
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
    
    int i;
   
    acc=calloc(1, sizeof(int));
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
    
    for(;;){
        size_stack=0;
        position_run=0;
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
        fprintf(stderr, "Out of space！！！！！For pri_run");
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
    struct nd_stack*stack=NULL,*stack_pos=NULL,*last_stack=NULL;
    int pri_max=max;
    bool acc_yes_no=0;
    int ban_cont=0;
    int baned_index=0;
    struct hash_state_node *matched_nd_first=NULL,*match_nd_next=NULL;
    bool have_u=0;
  //  struct hash_state_node *ceshi=NULL;//test
    int pri_size_stack=0;
    bool firsttime_build_stack=true;
   
    //back for run
    pri_run=calloc(strlen(section_run)+3, sizeof(char));
    if (pri_run==NULL) {
        fprintf(stderr, "Out of space！！！！！For pri_run");
    }
    memcpy(pri_run, section_run, strlen(section_run)+1);
    
    for(;;){
        
        /*
                    printf("\nstate=%d  pri_run[position_run]=%c   size_stack=%d  max=%d  \n",
                            state_of_run,pri_run[position_run],size_stack,pri_max);
                    puts(pri_run);
        */
        
        if (ban_index(state_of_run)!=baned_index)
            matched_nd_first=find(state_of_run);
        else
            matched_nd_first=NULL;
        
        if(matched_nd_first!=NULL){
            
            match_nd_next=judgment_next_matched(matched_nd_first);
            if(match_nd_next!=NULL){                           //有分叉路
                if(firsttime_build_stack==true)
                    stack=push(matched_nd_first,stack);
                else{
                    if(pri_size_stack>0) {
                        matched_nd_first=stack_pos->feasible_curr;
                        stack_pos=stack_pos->next_nd;
                        pri_size_stack--;
                    }
                    else
                        stack=push(matched_nd_first,stack);
                }
            }
            /*
                                //测试
                                printf("\n\n执行前state=%d position_run=%d matched_nd_first->read=%c  matched_nd_first->next_state=%d  pri_run[position_run]=%c  size_stack=%d   pri_max=%d   ",
                                       state_of_run,position_run,matched_nd_first->read,matched_nd_first->next_state,pri_run[position_run],size_stack,pri_max);
                                puts(pri_run);
            
            
                                for (ceshi=matched_nd_first; ceshi!=NULL; ceshi=ceshi->next) {
                                    printf("ceshi->read=%c  ceshi->next_state=%d  \n",ceshi->read,ceshi->next_state);
                                }
                                //测试
                                int i=size_stack;
                                for (last_stack=stack; i>0; i--,last_stack=last_stack->next_nd) {
                                    printf("打印栈 %d %c %c %c %d\n",
                                           state_of_run,last_stack->feasible_curr->read,last_stack->feasible_curr->write,
                                           last_stack->feasible_curr->direction,last_stack->feasible_curr->next_state);
                                }
            */
            
            //执行matched_nd_first
            state_of_run=matched_nd_first->next_state;
            change_write_dirction(matched_nd_first);
            
            /*
                            //测试
                            printf("执行后state=%d position_run=%d matched_nd_first->read=%c  matched_nd_first->next_state=%d  pri_run[position_run]=%c  size_stack=%d   pri_max=%d   \n",
                                   state_of_run,position_run,matched_nd_first->read,matched_nd_first->next_state,pri_run[position_run],size_stack,pri_max);
                            puts(pri_run);
            */
            
            
            
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
            
            //max 判断
            pri_max--;
            if(pri_max<=0){
                state_of_run=-1;          //目的为使这个结点无效
                have_u=1;
            }
            
            //judgment_acc
            acc_yes_no=judgment_acc(state_of_run);
            if (acc_yes_no){
                printf("1\n");    //free stack stack-section...
                free(pri_run);
                pri_run=NULL;
                if(size_stack!=0){
                    free_stack(stack);
                    stack=NULL;
                }
                break;
            }
        }
        else{
            if(size_stack!=0){
                
                free(pri_run);
                pri_run=calloc(strlen(section_run)+3, sizeof(char));
                if (pri_run==NULL) {
                    fprintf(stderr, "Out of space！！！！！For pri_run");
                }
                memcpy(pri_run, section_run, strlen(section_run)+1);
              
                state_of_run=0;
                pri_max=max;
                position_run=0;
                
                //修改stack
                last_stack=pop_pointer_last(stack);
                match_nd_next=judgment_next_matched(last_stack->feasible_curr);
                if (match_nd_next!=NULL){
                    last_stack->feasible_curr=match_nd_next;
          //          printf("\n临时点位1。last_stack->feasible_curr->next_state=%d\n",last_stack->feasible_curr->next_state);
                }
                else{
                    while (match_nd_next==NULL && size_stack>0) {
                        free(last_stack);
                        last_stack=NULL;
                        size_stack--;
                        if (size_stack<=0){
                            stack=NULL;
                            stack_pos=NULL;
                            break;
                        }
                        last_stack=pop_pointer_last(stack);
                        match_nd_next=judgment_next_matched(last_stack->feasible_curr);
                    }
                    if (size_stack>0)
                        last_stack->feasible_curr=match_nd_next;
                }
                stack_pos=stack;
                pri_size_stack=size_stack;
                firsttime_build_stack=false;
                
                
                
                
                
                
            }
            if(size_stack==0){
                if (have_u) {
                    printf("U\n");
                    free(pri_run);
                    pri_run=NULL;
                    //free
                    break;
                }
                printf("0\n");
                free(pri_run);
                pri_run=NULL;
                //free
                break;
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
    
    banindex=key_run*1000+pri_run[position_run];
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



struct nd_stack* push(struct hash_state_node* feasible_curr,
                    struct nd_stack*stack){
    
    struct nd_stack*new,*a;
    
    new=calloc(1,sizeof(struct nd_stack));
    if (new==NULL) {
        fprintf(stderr, "Out of space！！！！！For section_run");
        
    }
    new->feasible_curr=feasible_curr;
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


//free
void free_stack(struct nd_stack*stack){

    struct nd_stack*a;

    
    
    for (a=stack,a=a->next_nd;
         size_stack>1;
         stack=a,a=a->next_nd,size_stack--){
        free(stack);
    }
    free(stack);
    stack=NULL;
    size_stack=0;
}

