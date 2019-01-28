#include <stdio.h>
#include <poly.h>

int main()
{
	PolyVM *vm = polyNewVM(NULL);

	polyInterpret(vm,
"one = 1\
two = 2\
three = one + two");
	polyFreeVM(vm);

	return 0;
}
