#include <stdio.h>
#include <stdlib.h>

int main(void){
int v[6] ={1,3,5,7,9,115};
printf("O acesso funcionou!: %d\n",v[5]);
printf("O acesso funcionou de novo!: %d",*(v+5));
}
