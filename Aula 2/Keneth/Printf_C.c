#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int vetor[20]= {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71};
    int i,j;
    i = 0;
        for(j=0; j<20; j++)
            {
                i++;
                printf("Primo %d: %d\n",i,vetor[j]);
            }
}
