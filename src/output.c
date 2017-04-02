#include<stdlib.h>
#include<stdio.h>

/* print the schedule to the specific txt file
 * alg: indicates which algorithm used
 * filename: output filename
 */

int* scheduleOutput(char* filename, int linenumber, int alg, int line[3][60], Order* order);

void storeOutput(int line[3][60], int alg, char* filename, Order* order, int writePipe){
	char buf[10];
	int *in;
	int i;
	for(i=0; i<3; i++){
		in = scheduleOutput(filename, i, alg, line, order);
		sprintf(buf, "%d", in[0]);
		write(writePipe, buf, 10);
		sprintf(buf, "%d", in[1]);
		write(writePipe, buf, 10);
	}
}

/* store the accepted orders and workingdays
 */
int* storeData(int readPipe) {
	char buf[10];
	int static out[6];
 	int i;
	for(i=0; i<6; i++){
		read(readPipe, buf, 10);
		out[i] = atoi(buf);
	}
	return out;
}

/* print the schedule to the specific txt file
 * alg: indicates which algorithm used
 * filename: output filename
 */
int* scheduleOutput(char* filename, int linenumber, int alg, int line[3][60], Order* order){
	int static *out;
	FILE *fp;
	fp = fopen(filename, "a+");
	fprintf(fp, "Assenbly Line %d\n\n", linenumber);
	fprintf(fp, "Algorithm:\t%d\n", alg);
	fprintf(fp, "Start Date:	D001\n");
	fprintf(fp, "End Date:	D060\n,");

	fprintf(fp, "Order Number	Start Date	End Date	Due Date	Quantity Requested	Quantity Produced\n," );
	int workingDay = 0;
	int startDate = 0;
	int endDate = 0;
    Order curOrder;
    int i = 0;
    int acceptedOrder = 0;

	while (i < 60){
		if (line[linenumber][i] != 0 ){
			acceptedOrder ++;
			startDate = i + 1;
			for (int j = 0; j < 200; j++){
				if (line[linenumber][i] == order[j].num){
					curOrder = order[j];
				}
			}
			while(line[linenumber][i] == line[linenumber][i + 1]){
				i++;
			}
			endDate = i+1;
			workingDay += (endDate - startDate);
			fprintf(fp, "R%10d D%10d D%10d D%10d %10d %10d \n", curOrder.num, startDate, endDate, curOrder.dueDate, curOrder.quantity, curOrder.quantity - curOrder.remainQty);
		}
		i++;
	}

	fprintf(fp, "Total number of working days: 60 days\n");
	fprintf(fp, "Order accepted: %d orders\n", acceptedOrder);
	fprintf(fp, "Days not in use: %d days\n", 60 - workingDay);
	fprintf(fp, "Days in use: %d days\n", workingDay);
	fprintf(fp, "Utilization: %.1f days\n", workingDay / 60);
	out[0] = acceptedOrder;
	out[1] = workingDay;
	fclose(fp);
	return *out;
}

/* print the report
 * out: the array stores three lines' accepted orders and working days
 * alg: indicates which algorithm used
 * filename: report filename
 */
void printReport(int out[3][2], int alg, int rejectList[3][MAXORDER], char* filename){
	int totalOrder = out[0][0] + out[1][0] + out[2][0];
	int totalWorkingday = out[0][1] + out[1][1] + out[2][1];
	float aveWorkingday = totalWorkingday / 3;
	float aveUtilization = totalWorkingday / 180;
	int rejectedOrder = 0;
	Order curOrder;

	FILE *fp;
	fp = fopen(filename, "a+");
	fprintf(fp, "***Summary of Schedules***\n\n");
	fprintf(fp, "Algorithm used:\t%d\n\n", alg);
	fprintf(fp, "There are %d orders scheduled in total. Details are as followed:\n\n", totalOrder);
	fprintf(fp, "Assenbly Line | Order Accepted | Working Day | Utilization\n");
	fprintf(fp, "==========================================================\n");
	for (int i = 0; i < 3; i++){
		fprintf(fp, "Line_%d      %10d %10d %.1f\n", i + 1, out[i][0], out[i][1], out[i][1] /60);
	}
	fprintf(fp, "==========================================================\n");
	fprintf(fp, "***PERFORMANCE***\n\n");
	fprintf(fp, "AVERAGE OF WORKING DAYS FOR THE 3 ASSEMBLY LINES: %.1f\n", aveWorkingday);
	fprintf(fp, "AVERAGE OF UTILIZATION: %.1f\n\n", aveUtilization);
	fprintf(fp, "TOTAL WORKING DAYS OF THE 3 ASSEMBLY LINES: %.1f\n", totalWorkingday);
	fprintf(fp, "UTILIZATION OF THE 3 ASSEMBLY LINES%.1f\n", aveUtilization);
	fprintf(fp, "***Order Rejected List***\n\n");

	while (rejectList[alg][rejectedOrder] != 0)	rejectedOrder++;
	fprintf(fp, "TOTAL NUMBER OF ORDER RECEIVED\n\n", rejectedOrder + totalOrder);
	fprintf(fp, "ORDER ACCEPTED: %d", totalOrder);
	fprintf(fp, "ORDER REJECTED: %d", rejectedOrder);
	fprintf(fp, "==========================================================\n");

	for (int i = 0; i < rejectedOrder; i++){
		for (int j = 0; j < MAXORDER; j++){
				if (rejectList[alg][i] == order[j].num){
					curOrder = order[j];
				}
			}
		fprintf(fp, "R%d D%d D%d Product_%c %d\n", rejectList[alg][i], curOrder.startDate, curOrder.dueDate, curOrder.product, curOrder.quantity);
	}

	fprintf(fp, "There are %d Orders rejected.\n", rejectedOrder);


}



