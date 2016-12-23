#include "poly_vm.h"

int main()
{
	// A test..
	PolyVM *vm = polyNewVM();

	polyInterpret(vm, "if a + b == c:\n    output()");
	polyFreeVM(vm);

	return 0;
}
