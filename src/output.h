#ifndef OUTPUT_H_INCLUDED
#define OUTPUT_H_INCLUDED
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

int* scheduleOutput(char* filename, int linenumber, int alg, int line[3][60], Order* order);
void storeOutput(int line[3][60], int alg, char* filename, Order* order, int writePipe);
int* storeData(int readPipe);
int* scheduleOutput(char* filename, int linenumber, int alg, int line[3][60], Order* order);
void printReport(int out[3][2], int alg, int rejectList[3][MAXORDER], char* filename);

#endif // OUTPUT_H_INCLUDED
