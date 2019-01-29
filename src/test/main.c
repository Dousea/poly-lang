#include <stdio.h>
#include <poly.h>

int main()
{
	PolyVM *vm = polyNewVM(NULL);

	polyInterpret(vm, "a = b + c");
	polyFreeVM(vm);

	return 0;
}
