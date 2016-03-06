#include <stdio.h>

void swap(int *p1, int *p2);
int main()
{   
    int x = 10;
    int y = 20;
    int *p1,*p2;
    
    p1 = &x;
    p2 = &y;
    swap(p1,p2);
    //Deve imprimir na tela x : 20 , y : 10
    printf( "x : %d , y : %d\n" , x , y );
    return 0;
}
void swap(int *p1, int *p2){
        
        int aux;
        aux= *p1;
        *p1 = *p2;
        *p2 = aux;
    }
