//
//  main.c
//  NTM
//
//  Created by Leo ZHANG on 24/07/2018.
//  Copyright © 2018 Leo ZHANG. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUM_STATE 3000
#define NUM_CONTENT_OF_STSTE 5                  //例如0aaS1，再加一个空字符
#define NUM_SECTION 10
#define NUM_BASE 20
#define MAX_TIMES_AT_SAME_STATE 1000
#define TIMES_TO_TRY 9999
#define MAX_TIMES_CHANGE_ROAD 100

//varie pubblic
int content[MAX_NUM_STATE][NUM_CONTENT_OF_STSTE];
int pri_content[MAX_NUM_STATE][NUM_CONTENT_OF_STSTE];   //防止无意义串捣乱
int total_num_state;                            //读入的状态总数
char section[NUM_SECTION];                               //用于大段识别
int max;
int acc1=10000,acc2=10000,acc3=10000;
int times_to_try;                           //可以去尝试的次数
int state_of_run=0;                 //用于表示当前状态
int position_run=0;                            //用于表示run串内部位置
int num_of_list=0;                             //符合条件 创建的合适链表数
int up_state=10000;                         //用于记录上一个状态
int num_mem=0;                              //用于内存分配的跟踪
int sign_pri_content=0;                         //标记是否赋值
int times_state_uni=MAX_TIMES_AT_SAME_STATE;              //为防止在同一状态呆太久而设置
int max_times_change_road=MAX_TIMES_CHANGE_ROAD;                   //最多切换路径次数

struct feasible_state{
    char write;
    char direction;
    int next_state;
    struct feasible_state *next;
};


//function
char identification_section(char *);
void read_tr(void);
void put_in_content(char *,int);
void read_acc(void);
void put_numacc(int);
void read_and_do_run(void);
void do_run(char *);
struct feasible_state* search_and_do_list(char *);
struct feasible_state* choose_a_node(struct feasible_state*);
void go_to_next_state(struct feasible_state* ,char *);
void free_list(struct feasible_state*);

void printlianggebiao(void);        //用于测试的打印

//main
int main(void) {
    
    if(identification_section(NULL)== 't' )                 //    读入tr段
    {
        read_tr();
        printlianggebiao();                                 //调试用
    }

    if(identification_section(section)== 'a' )                 //    读入acc段
    {
 //       printf("%c",section[0]);                            //测试识别情况
        read_acc();
 //       printf("\n%d %d %d\n",acc1,acc2,acc3);                //测试用
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
//         read_and_do_run();
        ;
    }                                               //
        
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
        else
            return 0;
    }
    else
        return a[0];
    
}


//读入tr，存入二维数组 content
void read_tr(void){
    
    char function_tr[20];
    
    for(total_num_state=0;total_num_state<=MAX_NUM_STATE;total_num_state++){
        
        if(scanf("%[^\n]\n",function_tr)>0){
            
            if(function_tr[0]>96){
                strcpy(section,function_tr);
                break;
            }
            put_in_content(function_tr,total_num_state);
        }
        else
            break;
    }
    
}




//放入content 对state和next state的位数作了区分
void put_in_content(char *a,int i){
    

    if(a[1]== ' '){                     //开始是1位数

        content[i][0]=a[0]-48;
                                        //注意：-48 是将字符变为数字储存
        content[i][1]=a[2];             //read
        content[i][2]=a[4];             //write
        content[i][3]=a[6];             //direction
        
        if(a[9]<48 || a[9]>57)                         //结尾为一位数 直接存
            content[i][4]=a[8]-48;          //next state
        else if (a[10]<48 || a[10]>57)                                  //结尾为两位数 算好了存
            content[i][4]=(a[8]-48)*10+(a[9]-48);
        else                                        //结尾为三位数
            content[i][4]=((a[8]-48)*100+(a[9]-48)*10+(a[10]-48));
        
    }
    else if(a[2]== ' '){                           //开始是两位数
        
        content[i][0]=(a[0]-48)*10+(a[1]-48);
        content[i][1]=a[3];             //read
        content[i][2]=a[5];             //write
        content[i][3]=a[7];             //direction
        
        if(a[10]<48 || a[10]>57 )                        //结尾为一位数 直接存
            content[i][4]=a[9]-48;              //next state
        else if(a[11]<48 || a[11]>57)                                 //结尾为两位数 算好了存
            content[i][4]=(a[9]-48)*10+(a[10]-48);
        else                                        //结尾为三位数
            content[i][4]=((a[9]-48)*100+(a[10]-48)*10+(a[11]-48));
        
        
    }
    else{                                       //开始是三位数
        content[i][0]=(a[0]-48)*100+(a[1]-48)*10+(a[2]-48);
        content[i][1]=a[4];             //read
        content[i][2]=a[6];             //write
        content[i][3]=a[8];             //direction
        
        if(a[11]<48 || a[11]>57 )                        //结尾为一位数 直接存
            content[i][4]=a[10]-48;              //next state
        else if(a[12]<48 || a[12]>57)                                 //结尾为两位数 算好了存
            content[i][4]=(a[10]-48)*10+(a[11]-48);
        else                                        //结尾为三位数
            content[i][4]=((a[10]-48)*100+(a[11]-48)*10+(a[12]-48));
        
    }
    
    
}




 
  //用于测试的打印

void printlianggebiao(void){

    int i,j;
    for (i=0; i<3000; i++) {
        for(j=0; j<5; j++){
            printf("%d " ,content[i][j]);
        }
        printf("\n");
    }
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
    
    if(acc1==10000)
        acc1=a;
    else if(acc2==10000)
        acc2=a;
    else
        acc3=a;
}




/* * * * * * * * * * * *
 *                     *
 *下面就是run 相关函数了   *
 *                     *
 *                     *
 * * * * * * * * * * * */

void read_and_do_run(void){
    
    char context_run[3000]={'_','_','_','_','_',
                            '_','_','_','_','_',
                            '_','_','_','_','_',
                            '_','_','_','_','_'};
    int i;
    
    
    for(;;){
        if(scanf("%s\n",context_run+NUM_BASE)<=0)
            break;
        
        for (i=NUM_BASE;
             context_run[i]>=65 && context_run[i]<=122;
             i++) {
            ;
        }
        
        for (; i<2999; i++){
            
            context_run[i]='_';
        }
        
 /*
  *
  *测试用灯
  *
  */
        
//        printf("\n\n\n\n");
//        puts(context_run);                  //测试
//        printf("\n\n\n\n");
        
        position_run=NUM_BASE;
        state_of_run=0;
        num_mem=0;                  //用于内存测试
        do_run(context_run);
    }
}


void do_run(char *a){               //传进来的a是run串 例如aabbaa
    
    struct feasible_state *list_feasible;
    struct feasible_state *node_choosed;
    int pri_max=max;                //为了重来
    char pri_a[3000];
    int value_nega_pos=10;                  //可越位次数
    int restar_times_for_max=200;
    
    strcpy(pri_a,a);
    memcpy(pri_content, content, sizeof(content));
    sign_pri_content=0;                 //标记是否赋值
    max_times_change_road=MAX_TIMES_CHANGE_ROAD;                   //最多切换路径次数
    up_state=10000;                         //用于记录上一个状态

    for(times_to_try=TIMES_TO_TRY;times_to_try>0;){

        
        list_feasible=search_and_do_list(pri_a);          //建立好符合此步骤的链表 ok
        
        
  
//防止在某状态上无限次数呆
        if(up_state == state_of_run){                 //如果在某节点内呆的次数过多 则切路径
            times_state_uni--;
            if(!times_state_uni){
                free_list(list_feasible);
                num_of_list=0;                  //切换路径
                times_state_uni = MAX_TIMES_AT_SAME_STATE;
                max_times_change_road--;
                if(!max_times_change_road){
                    printf("U\n");
 //                   printf("点位1 num_mem=%d\n",num_mem);
                    break;
                }
            }
        }
        else
            times_state_uni = MAX_TIMES_AT_SAME_STATE;

        
//越位
        if(position_run<0){                     //为了区别是走错路越位还是U型越位
                                                //设置一个值，错了十次以上就U
            free_list(list_feasible);
            num_of_list=0;
            value_nega_pos--;
            if(!value_nega_pos){
                printf("U\n");
//                printf("点位2 num_mem=%d\n",num_mem);
                break;
            }
        }
        
        
        
//pri_max低于特定值可以重启几次
        if(pri_max<10 && restar_times_for_max>0){
            restar_times_for_max--;
            free_list(list_feasible);
            num_of_list=0;
        }
        
        
        
//切换路径
        if(num_of_list==0){         //如找不到匹配项 则此次路径不对 可尝试次数减一
            times_to_try--;
            position_run=NUM_BASE;
            state_of_run=0;
            pri_max=max;
            strcpy(pri_a,a);
            continue;
        }
        
        node_choosed=choose_a_node(list_feasible);      //随机选一个合适的节点 ok
        up_state=state_of_run;
        go_to_next_state(node_choosed,pri_a);
        free_list(list_feasible);
        pri_max--;
   
/*
 *
 *测试用灯
 *
 */
        
//        printf("\n\n\nmax_times_change_road=%d\n",max_times_change_road);
        
//        printf("value_nega_pos=%d\n",value_nega_pos);
//        printf("pri_max=%d\n",pri_max);
        
 
        if(pri_max<0){
            printf("U\n");
 //           printf("点位3 num_mem=%d\n",num_mem);
            break;
        }
        
 
        /*(pri_a[position_run]=='_' ||
         pri_a[position_run+1]=='_')
         &&*/
 
        if(state_of_run==acc1 ||
           state_of_run==acc2 ||
           state_of_run==acc3){
            printf("1\n");
 //           printf("点位4 num_mem=%d\n",num_mem);
            break;
        }
        
  
    }
 
    if(times_to_try<=0){
        if(sign_pri_content)
            printf("U\n");
        else
            printf("0\n");
        
//        printf("点位5 num_mem=%d\n",num_mem);
    }
  
}





struct feasible_state* search_and_do_list(char *a){

    int i;                           //用于计数循环到哪一个状态了
 
    struct feasible_state *first=NULL;               //链表头节点
    struct feasible_state *new_node_state;              //链表普通节点
    
    num_of_list=0;                             //符合条件 创建的合适链表数
    
    for(i=0; i<=total_num_state;i++){
        if(state_of_run==pri_content[i][0] &&
            a[position_run]==pri_content[i][1]){
            
            
            if(pri_content[i][0]==pri_content[i][4] &&
               pri_content[i][1]==pri_content[i][2] &&
               pri_content[i][3]=='S' &&
               max_times_change_road<MAX_TIMES_CHANGE_ROAD &&
               pri_content[i][0]==up_state){                   //最多切换路径次数              //为防止在同一状态呆太久而设置){
                pri_content[i][0]=10000;
                sign_pri_content=1;
            }
            
            
            
               num_of_list++;
               new_node_state=malloc(sizeof(struct feasible_state));
               num_mem++;
            
               new_node_state->write=pri_content[i][2];
               new_node_state->direction=pri_content[i][3];
               new_node_state->next_state=pri_content[i][4];
               if(first==NULL)
                   first=new_node_state;
               else{
                   new_node_state->next=first;
                   first=new_node_state;
               }
           }
    }
    
    
//     printf("test num_of_list=%d\n",num_of_list);    //测试用
    
    return first;           //如果没有可匹配项 则return NULL
  
}





struct feasible_state* choose_a_node(struct feasible_state* a){
    
    int random_number;
    struct feasible_state* p;
    
    random_number=rand()%num_of_list;
    for (p=a; p!=NULL && random_number>0; random_number--,p=p->next) {
        ;
    }
    p->next=NULL;
    return p;
}




void go_to_next_state(struct feasible_state* a,char *b){
    
    
    b[position_run]=a->write;
    if(a->direction=='R')
        position_run++;
    else if(a->direction=='L')
        position_run--;
    
    
    state_of_run=a->next_state;
    
/*
 *
 *下面四行都是测试用
 *
 */
    
    
//    puts(b);
//    printf("position_run=%d\n",position_run);
//    printf("position_char=%c\n",b[position_run]);
//    printf("state_of_run=%d\n\n",state_of_run);
    
}


void free_list(struct feasible_state*a){
    
    struct feasible_state *prev,*cur;
    
    for (prev=NULL,cur=a;
         cur!=NULL;
         prev=cur,cur=cur->next){
        free(prev);
        num_mem--;
//        if(num_of_list>1)
//            printf("临时点位 num_mem=%d\n",num_mem);
    }
    free(prev);
}







