struct SCharacterRung
{
	int type;
	boolean blocked;
	int detailedState;
	SCharacterRung() : blocked(0) {}
};

struct SRequirement
{
	int cl, type, state, detailedState, requiredCount, existingCount;
	DynList<goref> foundations, ordersAssigned;
	DynList<SCharacterRung> charrungs;
};

struct SCommission
{
	int type; goref obj;
	boolean complete, forceBalance;
	DynList<SRequirement> reqs;
	SCommission() : complete(0), forceBalance(0) {}
};

struct SWorkOrder
{
	goref obj;
	int ofind, type;
};

struct SAIController
{
	int masterPlan; //SPlan *masterPlan;
	DynList<SCommission> commissions;
	DynList<SWorkOrder> workOrders;
	SAIController() : masterPlan(-1) {}
};

char *ReadAIController(char *fp, GameObject *o);
void WriteAIController(FILE *f, GameObject *o);
