/*

Implemente uma aplicacao paralela/concorrente multi-threaded, usando linguagem C
com pthreads. Nao use recursos de orientacao a objetos, apenas linguagem C padrao
com as bibliotecas constante no modelo basico. Nao use outras bibliotecas. Nao use
comandos de threads que nao foram trabalhados em sala. Implemente uma aplicacao
completa, com todas as funcoes que forem necessarias, para que ela funcione de
forma segura, sem erros, ainda que raros. Desenvolva por si soh toda a codificacao
que for necessaria. Se estas instru��es nao forem seguidas, sua resposta sera
anulada/zerada.

A aplicacao deve processar uma matriz A de tamanho mxn (m linhas x n Colunas. Obtenha
n e n do usuario. Gere aleatoriamente os elementos inteiros da matriz. Mostre na
tela a matriz que foi gerada. A leitura de m e n, a gera��o da matriz e a mostragem
na tela deve ser feita pela thread 0 (main). O processamento que deve ser feito eh
explicado a seguir.

A matriz deve ser percorrida linha por linha, elemento apos elemento, e cada elemento
deve ser modificado de acordo com os valores dos elementos vizinhos. O elemento
da posicao (i,j) deve ser substituido pela somatoria dos 8 elementos vizinhos, ou
seja, o anterior, o posterior, o acima, o abaixo e os elementos das 4 diagonais.

O proximo elemento somente poderah ser modificado depois que o anterior foi
modificado, seguindo a ordem do percurso. Algo similar ao que foi feito no primeiro
trabalho.

Esse processamento deverah ser feito por k threads, sendo k informado pelo usuario.

Cada processo ira se responsabilizar por um conjunto de linhas equidistantes. Pode
supor que o numero de linhas da matriz seja multiplo de k. Portanto, cada thread
ira processar m/k linhas.

Na medida que uma thread vai processando uma linha, os elementos ja processados
ficam disponiveis para a proxima thread, mas uma sincronizacao deve ser feita
para impedir que elementos que nao estejam prontos sejam acessados.

Alem disso, ao final do processamento de sua linha, a thread que a processou deve
somar os seus elementos e apagar a linha processada (zerar os elementos). Mas ela
somente poderah fazer isso quando a thread seguinte jah terminou seu processamento,
ou seja, jah usou toda a linha anterior.

No final da aplicacao, a thread 0 deverah informar a somatoria total de todos os
elementos processados e imprimir a matriz final (que deverah estar zerada).

Restricoes Obrigatorias:
1) A primeira e ultima linha, bem como a primeira e ultima coluna, nao
devem ser processadas, mantendo os elementos originais.
2) A solu��o devera utlizar no maximo um semaforo e/ou um mutex por linha.

Penalidade: o nao cumprimento das restricoes obrigatorias implica em resposta
errada.


ALUNO: Vinícius Schmitz Freitas de Oliveira
RA: 109887
FILEIRA 3 - MAQUINA 6

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

int matriz[1000][1000];
int vetorSomatorias[1000];

int m, n, k, auxLinha;

int parcela,
    resto;

typedef struct{
    int id;

} tPack;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void leMatriz()
{   int i, j;
    printf("\nEntre com a quantidade de linhas e colunas respectivamente:\n");
    scanf("%d", &m);
    scanf("%d", &n);

    for(i=0; i<m; i++)
    {
        for(j=0; j<n; j++)
        {
            matriz[i][j] = rand() % 10;
        }
    }
}

void mostraMatriz()
{
    int i, j;

    printf("\nMatriz principal:\n\n");

    printf("%5d", 0);
    for(j=1; j<n; j++)
        printf("%4d", j);
    printf("\n");

    for(i=0; i<m; i++)
    {
        printf("%d", i);
        for(j=0; j<n; j++)
        {
            printf("%4d", matriz[i][j]);
        }
        printf("\n");
    }

}

void *funcao_da_thread(void *ptr)
{
    tPack *pack = (tPack *) ptr;

    int aux, i, j;

    for(i=(pack->id); i<m-1; i=i+k)
    {
        // threads esperam enquanto nao chega sua vez
        while(auxLinha != i);

        pthread_mutex_lock(&mutex);

        printf("\n**thread %d comecou a fazer a linha %d**", pack->id, i);

        for(j=1; j<n-1; j++)
        {
            matriz[i][j]=matriz[i-1][j]+matriz[i+1][j]+matriz[i][j-1]+matriz[i][j+1] + matriz[i-1][j-1]+matriz[i+1][j+1]+matriz[i-1][j+1]+matriz[i+1][j-1]; 
        }

        // faz as somatorias
        for(j=0; j<n-1; j++)
        {
            vetorSomatorias[i-1]+=matriz[i][j];
        }

        printf("\n**thread %d terminou de fazer a linha %d**", pack->id, i);
        
        pthread_mutex_unlock(&mutex);

        printf("\n**thread %d comecou a zerar a linha %d**", pack->id, i);

        // libera para a proxima thread sair do while infinito, e já começar a fazer o processamento da sua no mesmo momento que a thread atual desbloquear o mutex
        auxLinha++;

        while(auxLinha <= i + 1 && auxLinha != 9);

        // então, zera a linha
        for(j=1; j<n-1; j++)
        {
            printf("\n##thread %d zerou matriz[%d][%d] ##", pack->id, i, j);
            matriz[i][j]=0;
        }
    }
}


void main(void)
{   int i;

    srand(time(NULL));

    leMatriz();

    mostraMatriz();

    printf("\nDigite a quantidade de threads:\n");
    scanf("%d", &k);

    pthread_t thread[k];
    int iret;

    tPack pack[k];

    //parcela=m/k;
    //resto=m%k;

    // inicializa o vetor auxiliar que conterah as somatorias de cada linha nas suas posicoes [0...m-2], 
    // pois nao eh incluso as linhas 0 e m-1
    for(i=0; i<m; i++)
    {
        vetorSomatorias[i]=0;
    }

    auxLinha = 1;

    for(i=0; i<k; i++)
    {
        pack[i].id=i+1;

        iret = pthread_create(&thread[i], NULL, funcao_da_thread, (void*) &pack[i]);
    }


    for(i=0; i<k; i++)
    {
        pthread_join(thread[i], NULL);
    }

    printf("\nProcessamento feito! Agora a matriz novamente e como ela ficou. ");

    int somatoriaTotal=0;

    mostraMatriz();

    printf("\nVetor de somatorias: ");
    for(i=0; i<m-3; i++)
    {
        printf("%d, ", vetorSomatorias[i]);
        somatoriaTotal+=vetorSomatorias[i];
    }
    printf("%d", vetorSomatorias[i]);

    printf("\n\nSomatoria total: %d\n", somatoriaTotal);
}