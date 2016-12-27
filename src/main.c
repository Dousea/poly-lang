#include "poly_vm.h"

int main()
{
	// A test..
	PolyVM *vm = polyNewVM();

	polyInterpret(vm, "if a + b == c:\n\toutput()\n\t\toutput()\n\t\t\toutput()");
	polyFreeVM(vm);

	return 0;
}
