#include <stdio.h>
#include <poly.h>

int main()
{
	PolyVM *vm = polyNewVM(NULL);

	polyInterpret(vm,
"# Let's do a simple arithmetic\n\
one = 1\n\
two = 2\n\
three = one + two");
	polyFreeVM(vm);

	return 0;
}
