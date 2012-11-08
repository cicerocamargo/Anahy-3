/* Alunos: Daniel Santin Debastiani
 *         Lucas Nachtigall
 *
 * Módulo MakeEntries.
 *
 * Responsável por gerar uma saída com uma quantidade de números estabelecida por
 * parâmetro passado na linha de comando.
 * 
 * Este módulo executado isoladamente dos outros scripts irá somente mostrar na
 * tela a saída.
 * 
 * A saída gerada por este módulo segue a especificação abaixo:
 *         valores inteiros não necessariamente ordenados. Cada linha tem no máximo
 *         dez (10) inteiros, separados por “;”.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char** argv) {


    int numeros = atoi(argv[1]), numRand;
 
    printf("%d\n\n",numeros); 

    for (numeros; numeros > 0; numeros--) {

        srand ( time(NULL) );
        numRand = rand() % 1000 + 1;
        
        printf("%d ", numRand);
    }

    return 0;
}
