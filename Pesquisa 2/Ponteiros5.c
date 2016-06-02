#include <stdio.h>

int primos(void);


int main()
{   
    int *ponteiro_vetor;
    
    ponteiro_vetor = primos();
    
    printf("primo 1 = %i \nprimo 2 = %i \nprimo 3= %i \nprimo 4 = %i \n",*(ponteiro_vetor+1),*(ponteiro_vetor+2),*(ponteiro_vetor+3),*(ponteiro_vetor+4));
    return 0;
}


int primos(void){
    
    int vetor_primos[4];
    int *p = malloc(sizeof(int) * 4); 
	int i = 1 ,primo = 1000;
    int x, z;
    //20 iterações
    while (i <= 4){
        //zera a variavel que tem que chegar a 2 para me dizer que 
        //a divisão inteira do numero por ele mesmo deu 0 e por 1 também deu 0
        z = 0;
        //x é igual ao numero, primeiro divido por ele mesmo, depois vou subtraindo
        //1 do valor de x até que ele divida por 1 
        for(x = primo; x > 0; x--){
            if ( primo % x == 0) {
                ++z;
                }
            }		  
            if ( z == 2){	
                vetor_primos[i]=primo;
                //printf("Primo %d=%d\n", i,primo);
                ++i;
                
                }
            ++primo;
    }	
    p = &vetor_primos;
    
    return p;
}

