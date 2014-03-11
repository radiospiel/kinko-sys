#include <stdio.h>

int main(int argc, char** argv) {
	int lines = 0;
	int chars = 0;
	
	FILE* in = argc == 1 ? stdin : fopen(argv[1], "r");
	if(!in) {
		perror(argv[1]);
		return 1;
	}
	
	int ch;
	while((ch = getc(in)) >= 0) {
		if(ch == '\n') {
			lines ++;
			chars = 0;
		}
		else {
			chars += 1;
		}
	}

	if(chars) lines++;
	
	printf("%d\n", lines);
	
	if(in != stdin)
		fclose(in);
	
	return 0;
}