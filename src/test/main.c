#include <stdio.h>
#include <poly.h>

int main()
{
	PolyVM *vm = polyNewVM(NULL);
 
	polyInterpret(vm, "n = -3 + 4 * 2 / -( 1 - 5 ) ^ 2 ^ 3");
	polyFreeVM(vm);

	return 0;
}
