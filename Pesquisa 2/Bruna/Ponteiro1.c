#include <stdio.h>

int main()
{   
    // count e sum  são variáves inteiras, temp é um ponteiro para uma variavél inteira 
    int count=10, *temp, sum=0;
    // o ponteiro recebe a posição da memória de count
    temp = &count;
    //a variável a qual o ponteiro aponta(count) recebe 20 ou seja count= 20 
    *temp =20;
    //agora o ponteiro aponta para a posição da memória onde a variável sum está
    temp = &sum;
    //a variável a qual o ponteiro aponta(sum) recebe count ou seja sum = count => sum = 20
    *temp = count;
    
    //portando o printf retorna count= 20,*temp= 20, sum= 20
    printf("count= %d,*temp= %i, sum= %d", count,*temp,sum);

    return 0;
}

