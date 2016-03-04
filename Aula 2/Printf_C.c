#include <stdio.h>

int main(void){		
    int i = 0, 
    primo = 2;
    int x, z;
    //20 iterações
    while (i != 20){
        //zera a variavel que tem que chegar a 2 para me dizer que 
        //a divisão inteira do numero por ele mesmo deu 0 e por 1 também deu 0
        z = 0;
        //x é igual ao numero, primeiro divido por ele mesmo, depois vou subtraindo
        //1 do valor de x até que ele divida por 1 
        for(x = primo; x > 0; x--){
            if ( primo % x == 0) {
                ++z;
                printf("z= %i\n",z);}
            }		  
            if ( z == 2){	
                ++i;
                printf("Primo %d=%d\n", i,primo);	
                }
            ++primo;
            printf("primo= %i\n",primo);
        }	        
    
}

  