#ifndef __DEFS_H_
#define __DEFS_H_





enum ORD_TYPE {
	TYPE_NRM,   // Normal order → corresponds to 'N' in input
	TYPE_VGAN,  // Vegan order  → corresponds to 'G' in input
	TYPE_VIP,   // VIP order     → corresponds to 'V' in input
	TYPE_CNT    // Total number of Aorder types (for loops/arrays)
};


enum ORD_STATUS	//order status
{
	WAIT,	//waiting to be served
	SRV,	//In-service but not delivered (not finished) yet
	DONE,	//Delivered (finished) to its destination
	ORD_STATUS_CNT
};


enum PROG_MODE	//mode of the program interface
{
	MODE_INTR,	//interactive mode
	MODE_STEP,	//Step-by-step mode
	MODE_SLNT,	//Silent mode
	MODE_DEMO,	//Demo mode (for introductory phase only, should be removed in phases 1&2)
	MODE_CNT	//number of possible modes
};


enum COOK_TYPE 
{
	COOK_NRM,   // Normal cook
	COOK_VGAN,  // Vegan cook
	COOK_VIP,   // VIP cook
	COOK_CNT    // Total number of cook types
};


#define MaxPossibleOrdCnt 999	//max possible order count (arbitrary value)
#define MaxPossibleMcCnt  100	//max possible cook count (arbitrary value)


#endif