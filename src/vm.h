#ifndef VM_H_
#define VM_H_

#define MAX_STACK 128

typedef enum
{
	INST_LITERAL_NUMBER,
	INST_LITERAL_FALSE,
	INST_LITERAL_TRUE,
	INST_EOF
} Instruction;

typedef struct
{
	int stacksize;
	Value *stack[MAX_STACK];
} VM;

void VM_push(VM *vm, Value *value);
Value *VM_pop(VM *vm);
void VM_interpret(VM *vm, Instruction *bytecode);

#endif /* VM_H_ */
