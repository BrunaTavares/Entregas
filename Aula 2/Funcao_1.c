// C Program to Find Highest Common Factor. 
#include <stdio.h>

int main(void){		
   int num1,num2,i,hcf;

printf("Numero 1:\n") ;

scanf("%d" , &num1 );

printf("Numero 2: \n" );
scanf("%d" , &num2 );

hcf = MDC(num1,num2);
printf("H.C.F of %d and %d is %d \n", num1, num2, hcf );
    return 0;
    
}

int MDC(int num1,int num2){
    int hcf,i;
    for(i =1;i<=num1 || i<=num2; ++i){
    if(num1%i==0 && num2%i==0) 
    //Checking whether i i s a factor of both number 
    hcf=i ;
    return(hcf);
}

}
  