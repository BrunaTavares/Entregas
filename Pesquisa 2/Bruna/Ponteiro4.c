#include <stdio.h>

void print_array(int *p,int s);

int main()
{
    int test_array[]={20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    int *p;
    int i,arraySize;
    
    p = test_array;
    arraySize = sizeof(test_array)/sizeof(test_array[0]);
    //para o correto funcionamneto deve-se passar o ponteiro par a função
    print_array(p,arraySize);
    return 0;
}

void print_array(int *p,int s){
    int i;
    printf("\nVetores Lidos\n");
    
    for(i=0;i<=s-1;i++){
        // soma 1 de inteiro que quer dizer 4 bytes de posição
        printf("test_array[%i]= %i\n", i, *(p+i));
        
    }
    
}