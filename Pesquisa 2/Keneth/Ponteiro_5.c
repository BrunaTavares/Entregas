#include <stdio.h>
#include <stdlib.h>

void print_array(int *a){
int i;
for(i=0;i<20;i++){
    printf("%d; ",a[i]);
}
}

int main(void){
int test_array[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,115,935};
print_array(test_array);
}
