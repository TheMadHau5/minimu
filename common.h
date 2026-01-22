struct minimu_cpu {
	int psize;
	int exit;
	char* (*stdin)();
	void (*stdout)(char*);
	void (*stderr)(char*);
};

void cpu_execute(struct minimu_cpu *);
