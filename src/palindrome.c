#include <stdio.h>
int main() {
	int n;
	scanf("%d", &n);
  int reversedNum = 0, originalNum = n;
    
    while (n > 0) {
        reversedNum = reversedNum * 10 + n % 10;
        n /= 10;
    }

	if (reversedNum == originalNum) {
		printf("Y\n");
	} else {
		printf("N\n");
	}
	return 0;
}
