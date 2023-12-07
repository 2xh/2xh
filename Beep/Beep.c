#include <stdio.h>
#include <windows.h>
int main(int argv, char** argc)
{
	unsigned f, t;
	int n = argv - 1, i;
	if (n > 0)
		printf("Enter automated mode.\n2 values per argument, separated by , without space, represents frequency in Hz and time in ms.\nRepeat to play more!\n");
	else
		printf("Usage: 2 values per line for frequency in Hz and time in ms\nSleep when frequency is 0\nUse 0 0 or EOF to finish playing\n");
	do
	{
		if (n > 0)
			i = sscanf(argc[argv - n], "%u,%u", &f, &t);
		else
			i = scanf("%u %u", &f, &t);
		if (i == EOF)
			break;
		else if (i < 2)
		{
			fprintf(stderr, "Error when parsing input!\nIgnoring note %d\n", argv - n);
			if (n <= 0)
				scanf("%*[^0-9]");
			n--;
			continue;
		}
		if (t == 0)
			break;
		else if (f == 0)
			Sleep(t);
		else if (!Beep(f, t))
			fprintf(stderr, "Error when playing %uHz for %ums\n", f, t);
		else
			printf("\x0E");
		n--;
		if (n <= 0)
			printf("\n");
	} while (n);
	printf("You have played %d notes!\nBye!", argv - 1 - n);
	return 0;
}