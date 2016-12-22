#include "poly_vm.h"

int main()
{
	// A test..
	PolyVM *vm = polyNewVM();

	polyCompile(&vm->compiler, "if a + b == c:\n    output()");
	polyFreeVM(vm);

	return 0;
}
