#include <stdio.h>
#include <stdlib.h>

int maximodc(int a, int b)
{
    int i,mdc;
    for(i=1; i<=a|| i<=b; i++)
    {
        if(a%i==0 && b%i==0)
            mdc=i;
    }
    return mdc;
}

int main(void)
{
    int num1,num2,mdc;

    printf("Digite o numero1: \n");
    scanf("%d",&num1);

    printf("Digite o numero2: \n");
    scanf("%d",&num2);

    maximodc(num1,num2);

    printf("O MDC de %d e %d: %d",num1,num2,maximodc(num1,num2));
}