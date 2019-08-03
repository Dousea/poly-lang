#include <stdio.h>
#include <poly.h>

int main()
{
	PolyVM *vm = polyNewVM(NULL);
 
	polyInterpret(vm, "n = not true or not false == true");
	polyFreeVM(vm);

	return 0;
}
