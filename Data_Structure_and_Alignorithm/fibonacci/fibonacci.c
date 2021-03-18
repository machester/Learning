#include <stdio.h>

void _func(int m, int n, int index)
{
	int sum;

	if(index > 0)
	{
		index--;
		sum = m + n;
		printf("index = %d:\t m = %d,\t n= %d,\t sum = %d,\n", index, m, n, sum);
		_func(n, sum, index);
	}
}

int main(void)
{
	int loop;
	int a;
	int b;
	
	a = 1;
	b = 1;
	loop = 40;

	printf("fibonacci sequence...\n");
	_func(a, b, loop);

	return 0;
}
