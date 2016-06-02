#include <stdio.h>

void leArray(int *p);

int main()
{
    int x[5];
    int *p;
    int i;
    
    // p aponta para x[0]. Equivale a fazer p = &x[0]
    p = x;
    
    for(i=1; i<= sizeof(*x);i++){
        x[i]=i;
        printf(" x[%i]= %i",i,x[i]);
    }
    leArray(p);
    return 0;
}

void leArray(int *p){
    int i;
    printf("\nVetores Lidos\n");
    for(i=1;i<= sizeof(*p);i++){
        // soma 1 de inteiro que quer dizer 4 bytes de posição
        printf(" posição &i => x[%i]= %i",p+i, i, *(p+i));
        
    }}