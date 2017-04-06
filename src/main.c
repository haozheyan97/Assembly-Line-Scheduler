#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/wait.h>
#define NUM_ALS 3
#define MAX_DAY 60
#define MAX_ORDER 200
#define NUM_PRODUCT 5

struct Order
{
    int oid;
    int startDate;
    int dueDate;
    int type;
    int quantity;
    int remaining;
};

typedef struct Order Order;
Order order[MAX_ORDER+5];


struct Product    // A product contains two attributes: name and equipment needed.
{
    char name[16];
    int equipment;
};

typedef struct Product Product;
Product product[NUM_PRODUCT+3];


int NUM_order;
int ALS[NUM_ALS+3][MAX_DAY+5];
int reject_NUM;
int rejectList[MAX_ORDER+5];
int is_reject[MAX_ORDER+5]; // used to
int alg_used;

char* alg[]={"ALGORITHM","FCFS","EDF","SDF","ADV"};



 int MAX(int x,int y)
 {
    return x>y?x:y;
 }

 int MIN(int x,int y)
 {
    return x<y?x:y;
 }


/* Initalizion
 * Set all of data to 0 which means unused
 */
void init()
{
    NUM_order=0;
    alg_used=0;
    memset(ALS,0,sizeof(ALS));
    memset(order,0,sizeof(order));
    memset(product,0,sizeof(product));
}

int cmpFCFS(const void *a, const void *b){
	Order* aa = (Order*)a;
	Order* bb = (Order*)b;
	return (aa->oid - bb->oid);
}

int cmpEDF(const void *a, const void *b){
	Order* aa = (Order*)a;
	Order* bb = (Order*)b;
	return (aa->dueDate - bb->dueDate);
}

int cmpSDF(const void *a, const void *b){
	Order* aa = (Order*)a;
	Order* bb = (Order*)b;
	return (aa->startDate - bb->startDate);
}

int cmpADV(const void *a, const void *b){
	Order* aa = (Order*)a;
	int costa = aa->quantity/1000;
	Order* bb = (Order*)b;
	int costb = bb->quantity/1000;
	return ((aa->dueDate-costa) - (bb->dueDate-costb));
}

/* addOrder via stdin
 * In this function, 4 argument will be read
  */
void addOrder()
{
    getchar();
    int oid,startDate,dueDate;
    char pro[16];
    int quantity;
    scanf("R%d D%d D%d %s %d",&oid,&startDate,&dueDate,pro,&quantity);

    int type=0;
    int i;
    for(i=1;i<=NUM_PRODUCT;i++)
        if(strcmp(pro,product[i].name)==0)
            {
                type=i;
                break;
            }

    NUM_order++;
    order[NUM_order]=(Order){oid,startDate,dueDate,type,quantity,0};
}

/* Check the arrangement conflict with other AL
 * 1 for conflict and 0 for not.
*/

int checkConflict(int start,int end,int equipment)
{
    int day;
    for(day=start;day<=end;day++)
        if((ALS[1][day] & ALS[2][day]) || (ALS[1][day] & ALS[3][day]) || (ALS[2][day] & ALS[3][day]))return 1;

    return 0;
}

/* Add product Configuration inorder to make arragement of assembly line
 * the only argument is filename
 */

void addProductConfiguration(char *filename)
{
    FILE *fp;
    fp=fopen(filename,"a+");

    if(fp==NULL)
    {
        printf("FILE DOES NOT EXIST!\n");
        return;
    }


    int no=1;
    while(no<=5 && fscanf(fp,"%s",product[no].name)!=EOF)
    {
        while(1){
            char c=fgetc(fp);
            if(c!=32){no++;break;}
            char equ_no[16];
            fscanf(fp,"%s",equ_no);

            int last=strlen(equ_no)-1;
            if(equ_no[last]==',')
            {
                int eid=equ_no[last-1]-48;
                product[no].equipment+=(1<<(eid-1));
            }
            else
            {
                int eid=equ_no[last]-48;
                product[no].equipment+=(1<<(eid-1));
                fgetc(fp);
                no++;
                break;
            }


        }

    }

    fclose(fp);
}



/* Main function of ALS.
 * Given an argument to descripe the algorithm used
 * Then Ordered the orders by the algorithm and use similar decision to deivide them into different time and assembly line
 * For some special algorithm, use another ALS.
 */
int runALS(int argc, char *algorithm)
{
    memset(is_reject,0,sizeof(is_reject));
    memset(ALS,0,sizeof(ALS));

    int alg_num=0;
    qsort(order+1,NUM_order,sizeof(Order),cmpFCFS);


    if(argc==0 || strcmp(algorithm, "-FCFS") ==0 )
    {
        alg_num=1;
        printf("FCFS is used for ALS.\n");
    }

    else if(strcmp(algorithm, "-EDF") == 0)
    {
        alg_num=2;
        printf("EDF is used for ALS.\n");
        qsort(order+1,NUM_order,sizeof(Order),cmpEDF);
    }

    else if(strcmp(algorithm, "-SDF") == 0)
    {
        alg_num=3;
        printf("SDF is used for ALS.\n");
        qsort(order+1,NUM_order,sizeof(Order),cmpSDF);
    }

    else if(strcmp(algorithm, "-ADV") == 0)
    {
        alg_num=4;
        printf("ADV is used for ALS.\n");
        qsort(order+1,NUM_order,sizeof(Order),cmpADV);
    }

    else
    {
        alg_num=1;
        printf("FCFS is used for ALS.\n");
    }



    int i;
    for(i=1;i<=NUM_order;i++)order[i].remaining=order[i].quantity;



    int now=1;
    int ALS_day[4];
    ALS_day[1]=ALS_day[2]=ALS_day[3]=1;

    while(now<=NUM_order)
    {
        if(is_reject[now])continue;
        int choose_als=0,choose_day=0;
        int cost=order[now].remaining/1000;
        int als;
        for(als=1;als<=3;als++)
            {
                int startOrder=MAX(ALS_day[als],order[now].startDate);
                if(startOrder + cost - 1 > order[now].dueDate)continue;
                else
                {
                    int day;
                    for(day=startOrder;day + cost - 1 <= order[now].dueDate;day++)
                        if(!checkConflict(day,day+cost-1,order[now].type))
                            {
                                choose_als=als;
                                choose_day=day;
                                break;
                            }
                }

                if(choose_als!=0)break;
            }

        if(choose_als==0)is_reject[now]=1;
        else
        {
            int day=choose_day,t=cost;
            while(t--)
            {
                ALS[choose_als][day]=now;
                day++;
            }

            ALS_day[choose_als]=choose_day+cost;

            order[now].remaining-=cost*1000;
        }

        now++;
    }

    return alg_num;
}



void addBatchOrder(char *filename)
{
    FILE *fp;
    fp=fopen(filename,"r");
    if(fp==NULL)
    {
        printf("FILE DOES NOT EXIST!\n");
        return;
    }

    int oid,startDate,dueDate;
    char pro[16];
    int quantity;

    char buffer[128];

    while(!feof(fp) && fgets(buffer,128,fp))
    {
        if(buffer[0]<=32)break;
        sscanf(buffer,"R%d D%d D%d %s %d",&oid,&startDate,&dueDate,pro,&quantity);
        int type=0;
        int i;
        for(i=1;i<=NUM_PRODUCT;i++)
            if(strcmp(pro,product[i].name)==0)
            {
                type=i;
                break;
            }

        NUM_order++;
        order[NUM_order]=(Order){oid,startDate,dueDate,type,quantity,0};
    }

    fclose(fp);
}

/* print schedule -- Use pipe to get data from the child process.
 * Format output is also used.
 */

int printSchedule(int pipe_fd, int argc, char *filename)
{
    char buffer[1024];
    read(pipe_fd, buffer, sizeof(buffer));

    qsort(order+1,NUM_order,sizeof(Order),cmpFCFS);

    int als,day;

    int prev=0,j=0;
    for(als=1;als<=NUM_ALS;als++)
        for(day=1;day<=MAX_DAY;day++)
        {
            while(buffer[j]!=' ')j++;
            buffer[j]=0;
            ALS[als][day]=atoi(buffer+prev);
            j++;
            prev=j;
        }

    int alg_num = atoi(buffer+prev);

    if(alg_num==2){qsort(order+1,NUM_order,sizeof(Order),cmpEDF);}
    if(alg_num==3){qsort(order+1,NUM_order,sizeof(Order),cmpSDF);}
    if(alg_num==4){qsort(order+1,NUM_order,sizeof(Order),cmpADV);}

    FILE *fp;

    fp=fopen(filename,"w");

    int work_day;
    int total_order;
    if(argc>0 && fp!=NULL)
    {
        for(als=1;als<=NUM_ALS;als++)
        {
            fprintf(fp, "Assembly Line %d\n",als);

            fprintf(fp,"Algorithm:  %s\n", alg[alg_num]);
            fprintf(fp,"Start Date: D001\n");
            fprintf(fp,"End Date:   D060\n");

            fprintf(fp,"  Order Number      Start Date      End Date      Due Date      Quantity Requested      Quantity Produced\n");


            int task=0,last=1;
            work_day = total_order = 0;
            for(day=1;day<=MAX_DAY;day++){
                if(ALS[als][day]!=0)
                {
                    work_day++;
                    if(ALS[als][day]!=ALS[als][day-1])total_order++;
                }
                if(ALS[als][day]!=task)
                {
                    if(task!=0)
                    {
                        fprintf(fp,"     R%03d              D%03d            D%03d         D%03d              %-25d%d\n",order[task].oid,last,day-1,order[task].dueDate,order[task].quantity,order[task].quantity);
                        task=ALS[als][day];
                        last=day;
                        continue;}
                    if(task==0){task=ALS[als][day];last=day;continue;}
                }
            }


            fprintf(fp,"\n");
            fprintf(fp,"%-30s: %d days\n","Total number of working days: ", MAX_DAY);
            fprintf(fp,"%-30s: %d orders\n","Order accepted:",total_order);
            fprintf(fp,"%-30s: %d days\n","Day not in Use", MAX_DAY-work_day);
            fprintf(fp,"%-30s: %d days\n","Day in Use", work_day);
            fprintf(fp,"%-30s: %.1lf%%\n","Utilization", work_day * 100.0 / MAX_DAY);

            fprintf(fp,"\n");

        }

        printf("Result has been written to %s successfully!\n",filename);
        fclose(fp);
    }



    return alg_num;
}

/* A function to count the orders in each assembly line
 * Given argument is the status of assemblylines
 * The result will be store in the array out[3][2];
 * It can also generate the refuse list
 */
void stat(int (*out)[3], int (*stat)[MAX_DAY+5], int *rejectlist, int num_order)
{

    int als,day,i,j;

    for(i=1;i<=3;i++)
        for(j=1;j<=2;j++)
            out[i][j]=0;

    for(i=1;i<=num_order;i++)
        rejectlist[i]=1;

    for(als=1;als<=NUM_ALS;als++)
        for(day=1;day<=MAX_DAY;day++)
        {
            int oid=stat[als][day];
            if(oid!=0)
            {
                out[als][2]++;
                if(rejectlist[oid]==1)
                {
                    rejectlist[oid]=0;
                    out[als][1]++;
                }
            }
        }
}

void printReport(int alg_num,char *filename)
{
    FILE *fp;

    fp=fopen(filename,"w");

    if(fp==NULL)
    {
        printf("FILE DOES NOT EXIST!\n");
        return;
    }

    if(alg_num==0)
    {
        fprintf(fp,"No algorithm used. Please use command 'runALS' first.\n");
        return;
    }

    /* Part 1
     *  Summery of Schdules
     *  Contains number of orders and working days of each assembly line, with in the Utilization.
     */

    fprintf(fp,"***Summary of Schedules***\n\n");

    fprintf(fp, "Algorithm used: %s\n\n",alg[alg_num]);

    int out[4][3];

    stat(out, ALS, is_reject, NUM_order);


    fprintf(fp,"There are %d order seheduled in total.  Details are as follows:\n\n", out[1][1]+out[2][1]+out[3][1]);
    fprintf(fp,"Assembly Line | Order Accepted | Working Day | Utilization\n");
    fprintf(fp,"========================================================================\n");

    int als;
    for(als=1;als<=NUM_ALS;als++)
        fprintf(fp,"Line_%-11d %-16d %-14d %.1lf\n",als,out[als][1],out[als][2],out[als][2]*100.0/MAX_DAY);

    fprintf(fp,"\n");

    /* Part 2
     * PERFORMANCE OF ALL ASSEMBLY LINE
     * Contains Utilization of avrage and total working days.
     */

    fprintf(fp,"***PERFORMANCE***\n\n");

    int total_day = out[1][2]+out[2][2]+out[3][2],total_order = out[1][1]+out[2][1]+out[3][1];
    double avg_day = total_day * 1.0 / 3;
    double util = total_day * 100.0 / (MAX_DAY * 3);
    fprintf(fp,"%-54s %.1lf DAYS\n", "AVERAGE OF WORKING DAYS FOR THE 3 ASSEMBLY LINES:",avg_day);
    fprintf(fp,"%-54s %.1lf %%\n", "AVERAGE OF UTILIZATION:",util);
    fprintf(fp,"\n");
    fprintf(fp,"%-54s %.1lf DAYS\n", "TOTAL WORKING DAYS OF THE 3 ASSEMBLY LINES:",total_day*1.0);
    fprintf(fp,"%-54s %.1lf %%\n","UTILIZATION OF THE 3 ASSEMBLY LINES", util);
    fprintf(fp,"\n");


    /* Part 3
     * Order reject list
     * output the reject list one by one
     */

    fprintf(fp,"***Order Rejected List***\n");
    fprintf(fp,"\n");
    fprintf(fp,"TOTAL NUMBER OF ORDER RECEIVED:    %d\n",NUM_order);
    fprintf(fp,"\n");
    fprintf(fp," - ORDER ACCEPTED:   %d\n",total_order);
    fprintf(fp," - ORDER REJECTED:   %d\n",NUM_order - total_order);
    fprintf(fp,"\n");
    fprintf(fp,"REJECTED ORDER LIST\n");
    fprintf(fp,"===============================\n");

    int i;
    int reject_num=0;
    for(i=1;i<=NUM_order;i++)
    {
        if(is_reject[i])
        {
            fprintf(fp,"R%04d D%03d D%03d %s %d\n",order[i].oid,order[i].startDate,order[i].dueDate,product[order[i].type].name,order[i].quantity);
            reject_num++;
        }
    }

    fprintf(fp,"\n");
    fprintf(fp,"There are %d orders rejected.\n",reject_num);

    printf("The report is successfully written to %s.\n",filename);

    fclose(fp);

}



/* View order list
 * use of debug
 */
void viewOrder()
{
    int i;
    for(i=1;i<=NUM_order;i++)
        printf("R%04d D%03d D%03d %s %d\n",order[i].oid,order[i].startDate,order[i].dueDate,product[order[i].type].name,order[i].quantity);

}

void viewALS()
{
    int i;
    for(i=1;i<=MAX_DAY;i++)
        printf("DAY%d %d %d %d\n",i,ALS[1][i],ALS[2][i],ALS[3][i]);
}


int main()
{
    init();
    addProductConfiguration("product_configuration.txt");
    printf("\t~~WELCOME TO ALS~~\n");

    while(1)
    {
        printf("Please enter:\n> ");

        char op[32];

        scanf("%s",op);

        if(strcmp(op, "addOrder") == 0)   // addOrder command
        {
            addOrder();
        }

        else if(strcmp(op, "addBatchOrder") == 0)   // addOrder command
        {
            getchar();
            char filename[64];
            scanf("%s",filename);
            printf("%s\n",filename);
            addBatchOrder(filename);
        }

        else if(strcmp(op, "runALS") == 0)
        {
            char c;
            c=getchar();
            int argc=0;
            char argv[10][32];

            memset(argv,0,sizeof(argv));

            while(argc<10)
            {
                if(c<32)break;
                argc++;
                int j=0;
                while(1)
                {
                    c=getchar();
                    if(c>32){argv[argc][j++]=c;continue;}
                    else {argv[argc][j]=0;break;}
                }
            }


            int fd[2];
            int ps=pipe(fd);

            if(ps<0)
            {
                printf("Pipe Error!\n");
                continue;
            }

            int pid=fork();

            if(pid<0)
            {
                printf("Fork error!\n");
                continue;
            }

            if(pid == 0)  //child process to ALS
            {
                close(fd[0]);
                int alg_num = runALS(argc, argv[1]);
                printf("%d\n",pid);
                printf("%s\n",argv[1]);

                char s[1024]="2335157";   //Store the result into a String
                int als,day,j=0;

                for(als=1;als<=NUM_ALS;als++)
                    for(day=1;day<=MAX_DAY;day++)
                        {
                            j+=sprintf(s+j,"%d ",ALS[als][day]);
                        }


                j+=sprintf(s+j,"%d ",alg_num);

                write(fd[1],s,sizeof(s));  // Pass it to the fathers

                close(fd[1]);

                exit(0);
            }

            else
            {
                close(fd[1]);
                wait(NULL);

                alg_used=printSchedule(fd[0],argc,argv[argc]);


                close(fd[0]);
            }
        }

        else if(strcmp(op, "viewALS") == 0)
        {
            viewALS();
        }

        else if(strcmp(op, "endProgram") == 0)
        {
            printf("Bye-bye!\n");
            break;
        }

        else if(strcmp(op, "printReport") == 0)   //Command printReport
        {
            char filename[64];
            char trash[8];
            scanf("%s %s",trash,filename);
            printReport(alg_used,filename);
        }

        else if(strcmp(op, "sort") == 0)
        {
            qsort(order+1,NUM_order,sizeof(Order),cmpFCFS);
            viewOrder();
        }

    }

    return 0;
}
