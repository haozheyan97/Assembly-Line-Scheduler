#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PRODUCTAMOUNT 5
#define EQUIPMENTAMOUNT 8
#define MAXORDER 200


struct Order{
	int num;
	int startDate;
	int dueDate;
	char product;
	int quantity;
	int remainQty;
};
typedef struct Order Order;

struct AssemblyLine{
	int ordernum;
};

struct OrderList{
	struct Order* head;
	struct Order* tail;
};
typedef struct OrderList OrderList;

/*
 * It will read a block of text from a file
 * name: the name of the file
 * it will return a string with the first element set to -1 if encounter a EOF
 */
char* readContent(FILE *name){
	char static content[50];
	char letter = fgetc(name);
	int count=0;
	if(letter == EOF){
		content[0] = -1;
		content[1] = 0;
		return content;
	}
	while(letter != ',' && letter != ' ' && letter != '\n' && letter != EOF){
		content[count] = letter;
		letter = fgetc(name);
		count++;
	}
	content[count] = 0;
	return content;
}

/*
 * This is input module for file input.
 * fileName: the name of the file contain the input.
 * writePipe: pipe No for write to the main process.
 */

void addBatchOrder(char *fileName, int writePipe){
	char orderNum[10];
	char startDate[10];
	char dueDate[10];
	char product[10];
	char quantity[10];
	char buf[80];
	FILE* name;
	name = fopen(fileName, "r");
	if(name == NULL){
		printf("file open failed\n");
		exit(1);
	}
	while(fscanf(name, "R%s D%s D%s Product_%s %s\n", orderNum, startDate, dueDate, product, quantity) != EOF){
		// Indicate the start of a new order.
		buf[0] = 'C';
		buf[1] = 'O';
		buf[2] = 0;
		write(writePipe, buf, 3);
		
		write(writePipe, orderNum, 10);
		write(writePipe, startDate,10);
		write(writePipe, dueDate, 10);
		write(writePipe, product, 10);
		write(writePipe, quantity, 10);
	}
	buf[0] = EOF;
	write(writePipe, buf, 2);
	return;
}

/*
 * This is input module for keyboard input
 * writePipe for
 */ 
void addOrder(int writePipe, int readPipe){
	
}

/*
 * This function is used by parent process to get input order
 * from input module and store it.
 * orderList: a link list store all order
 * readPipe: pipe for read
 * return: number of order
 */
int storeOrder(Order* order, int readPipe){
	int count = 0;
	char orderNum[10];
	char startDate[10];
	char dueDate[10];
	char product[20];
	char quantity[10];
	char buf[10];
	while(1){
		//check start
		read(readPipe, buf, 3);
		if(buf[0] != 'C' || buf[1] != 'O') break;

		// read from input module
		read(readPipe, orderNum, 10);
		read(readPipe, startDate, 10);
		read(readPipe, dueDate, 10);
		read(readPipe, product, 10);
		read(readPipe, quantity, 10);
		
		//store in the order array
		order[count].num = atoi(orderNum);
		order[count].startDate = atoi(startDate);
		order[count].dueDate = atoi(dueDate);
		order[count].product = product[0];
		order[count].quantity = atoi(quantity);
		order[count].remainQty = atoi(quantity);
		printf("%s %s %s %s %s\n", orderNum, startDate, dueDate, product, quantity);
		count++;
	}
	return count;
}

/* transfor a input command to corresponding int.
 * 1: addOrder
 * 2: addBatchOrder
 * 3: runALS
 * 4: printReport
 * 5: endProgram
 * -1: wrong input
 */
int commandChoose(char* input){
	if(strcmp(input, "addOrder") == 0) return 1;
	if(strcmp(input, "addBatchOrder") == 0) return 2;
	if(strcmp(input, "runALS") == 0) return 3;
	if(strcmp(input, "printReport") == 0) return 4;
	if(strcmp(input, "endProgram") == 0) return 5;
	return -1;
}

/*
 * A module to input the equipment a product need from configuration file
 * input: name of file contain the configuration info
 * It will return a 2-d array,
 * first dimension represent product
 * second dimension represent equipment
 * int[product][equipment]
 */
void inputProductInfo(char* input, int productInfo[PRODUCTAMOUNT][EQUIPMENTAMOUNT]){
	FILE* in = fopen(input, "r");
	int i,k;
	for(i=0; i<10; i++){
		for(k=0; k<10; k++){
			productInfo[i][k] = 0;
		}
	}
	char* result;
	int productNum;
	while(1){
		result = readContent(in);
		if(result[0] == -1) break;
		if(strstr(result, "Product_") != NULL)
			productNum = result[8] -'A';
		else if(strstr(result, "Equipment_") != NULL){
			char* startOfNum = strchr(result, '_');
			startOfNum++;
			int equipmentNum;
			equipmentNum = atoi(startOfNum);
			productInfo[productNum][equipmentNum] = 1;
		}
	}
	return;
}

/*
 * qsort Compare function for FCFS
 * a: Order pointer
 * b: Order pointer
 * return a positive number if a start latter than b
 */ 
int cmpFCFS(const void *a, const void *b){
	Order* aa = (Order*)a;
	Order* bb = (Order*)b;
	return (aa->startDate - bb->startDate);
}

/*
 * exam whether the line and the equipment is available for the production
 * lineState: current line state
 * equipState: current equipment state
 * productInfo: product equipment relationship
 * product: the product order needed
 * return 1 if line1 is available, 2 line2, 3 line3, 0 if not
 */
int qulifyIn(int lineState[3], int equipState[EQUIPMENTAMOUNT], int productInfo[PRODUCTAMOUNT][EQUIPMENTAMOUNT], char product, int startDate, int date){
	int productNum = product - 'A';
	int i;
	
	if(date < startDate) return 0; 
	// if the equipment product need is not available
	for(i=0; i<EQUIPMENTAMOUNT; i++){
		if(equipState[i] == 1 && productInfo[productNum][i] == 1)
			return 0;
	}

	// if productline i+1 is available
	for(i=0; i<3; i++){
		if(lineState == 0)
			return (i+1);
	}
	return 0;	
}

/* test whether the time is sufficient to finish this job
 * order: the order need to be tested
 * date: current date
 * return 1 if sufficient, 0 if not
 */
int canFinish(Order order, int date){
	int requireDate = order.remainQty / 1000;
	if((order.dueDate - date) >= requireDate) return 1;
	return 0;
}

/* Transmit orderlist and reject list to parent
 * 
 */
void transResult(int line[3][60], int rejectList[MAXORDER], int rejectNum, int writePipe){
	//communicate to the parent
	char aBuf[10];
	char bBuf[10];
	char cBuf[10];
	int i;
	for(i=0; i<60; i++){
		sprintf(aBuf, "%d\0", line[0][i]);
		sprintf(bBuf, "%d\0", line[1][i]);
		sprintf(cBuf, "%d\0", line[2][i]);
		write(writePipe, aBuf, 10);
		write(writePipe, bBuf, 10);
		write(writePipe, cBuf, 10);
	}
	
	char rejectBuf[10];
	sprintf(rejectBuf, "%d\0", rejectNum);
	write(writePipe, rejectBuf, 10);
	for(i=0; i<rejectNum; i++){
		sprintf(rejectBuf, "%d\0", rejectList[i]);
		write(writePipe, rejectBuf, 10);
	}
	return;
}

/* receive and store result of scheduler
 * line: the shedule list
 * rejectList: rejectList, fill null with 0
 */
void storeSchedule(int line[3][60], int rejectList[MAXORDER], int readPipe){
	char aBuf[10];
	char bBuf[10];
	char cBuf[10];
	int i;
	for(i=0; i<60; i++){
		read(readPipe, aBuf, 10);
		read(readPipe, bBuf, 10);
		read(readPipe, cBuf, 10);
		line[0][i] = atoi(aBuf);
		line[1][i] = atoi(bBuf);
		line[2][i] = atoi(cBuf);
	}
	char reject[10];
	int rejectNum;
	read(readPipe, reject, 10);
	rejectNum = atoi(reject);
	for(i=0; i<rejectNum; i++){
		read(readPipe, reject, 10);
		rejectList[i] = atoi(reject);
	}
	for(i=rejectNum; i<MAXORDER; i++){
		rejectList[i] = 0;
	}
	return;
}

/* Schedule core for FCFS
 * orderList: a list contain the order
 * orderNum: the total order Num.
 * 
 */ 
void FCFS(Order orderList[MAXORDER], int orderNum, int productInfo[PRODUCTAMOUNT][EQUIPMENTAMOUNT], int writePipe){
	int static line[3][60]; //store the order number each assembly line produce. 0 represent out of work
	int lineState[3]; // state of a line. 0: available 1: occupied
	int rejectList[MAXORDER];
	int i, k;

	for(i=0; i<3; i++){
		lineState[i] = 0;
	}
	for(i=0; i<3; i++){
		for(k=0; k<60; k++){
			line[i][k] = 0;
		}
	}
	int equipState[EQUIPMENTAMOUNT]; //state of each equipment. 0: available 1: occupied
	for(i=0; i<EQUIPMENTAMOUNT; i++){
		equipState[i] = 0;
	}
	
	qsort(orderList, orderNum, sizeof(Order), cmpFCFS); //sort the orderlist in ascending order of start date.
	int date = 0; //day counter.
	int rejectNum = 0; // Number of reject order
	int pointer = 0; // the next order need to be execute.
	while(date < 60){
		int productLine;
		// accept new order
		while(1){
			while(orderList[pointer].startDate < date){
				
			}
 			productLine = qulifyIn(lineState, equipState, productInfo, orderList[pointer].product, orderList[pointer].product, date); 
			if(productLine == 0) break; //the current order need to be product is not available now, we need to wait,
			if(!canFinish(orderList[pointer], date)){
				rejectList[rejectNum] = orderList[pointer].num;
				rejectNum++;
				pointer++;
			}
			else{
				lineState[productLine-1] = 1;
				line[productLine-1][date] = orderList[pointer].num;
				pointer++;
			}
		}
		
		//working
		for(i=0; i<3; i++){
			if(lineState[i] != 0){
				orderList[line[i][date]].remainQty -= 1000; // reduce remain amount
				
				//if finish, change line state
				if(orderList[line[i][date]].remainQty == 0){
					lineState[i] = 0; 
					
					//change equipmentdate
					for(k=0; k<EQUIPMENTAMOUNT; k++){
						int productNum = orderList[line[i][date]].product -'A';
						if(productInfo[productNum][k] == 1) equipState[k] = 0;
					}
				}
				
				else line[i][date+1] = line[i][date]; // if not finish, do the same job next day.
			}
		}
		date++;
	}
	
	// put all remaining job to reject list.
	for(i=pointer; i<orderNum; i++){
		rejectList[rejectNum++] = orderList[pointer].num;
		pointer++;
	}
	transResult(line, rejectList, rejectNum, writePipe);
    return;
} 
