#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#define N_INST 30
#define N_FOR 5
#define N_MEM 200
#define N_PROB 50
#define N_OPCODE 6
#define N_WORD 4


// Frase de motivação : Sem lutar não tem vitória 

int cont = 0;
int cacheHits ;
int cacheNiss;
int contHd = 0 , contMemoriaRam =0 , contCache1 = 0 ,contCache2 = 0 ,contCache3 = 0; 
int rep[5][7];
int inst[200][7];



/*typedef struct PalavrasMemoria{
    int endPalavra;
    int conteudo;
}Palavras;
*/
typedef struct BlocoDeMemoria {
    int *palavras;
    time_t tempo;// esse tempo aqui tem que ser passado através de uma funcão que meche com o tempo 
    int quantidadeDeAcessor;
    int endBloco;
    int atualizado; // responsavel por saber se o bloco foi atualizado
}Bloco;

//Estrutura responsavel pela cache 
typedef struct Cache {
    
    Bloco **bloco;
    int cacheHits;
    int cacheMiss;
    int tamanhoCache;
}Cache;

// Fim das  Estuturas 

// inicio do cabeçalho das funcoes 

    int MMU ( int  endBloco  , int endPalavra ,int tipoOperacao  , int resultado);
    Bloco *CriarBlocoDeMemoria();
    int TrocaEntreMemoriaCache3EMemoriaRam( int endBloco  , int end);
    int TrocaEntreAsMemoriasCache(Cache *cacheBaixo , Cache *cacheAlto , int endBloco);
    Cache *CriarBlocoDeMemoriaCache(int tamanho);
    void PreencheMemoriaRam();
    void BuscaNaMemoriaRam ();
    int **CriarIntrucoes(int tamanho);
    void PrencheIntrucoes(int tamanho);
    void  executar ();
    int procuraNasCaches(Cache *cache , int endBloco , int endPalavra);
    int  Maquina(int pc , int ** Instrucoes , int NumeroInterrupcao);
    void criarHd();
    int *TrazDoHd(int endBloco);
    void AtualizarHd(int endBloco , Bloco *blocoParaATualizar);
    int trocarEntreMemoriaRamEHd(int *blocoDoHd);
    void trocaEntrePosicoesDaRamEHd ( Bloco *blocoAtual  , int *bloco);
    void liberaMemoria( int  **matriz , int tamanho);
// fim do cabeçalho das funcoes 

// Criação das melhoras 

Bloco *CriarBlocoDeMemoria(){
   Bloco *new  =  malloc(sizeof(Bloco));
   if ( cont >= 5){
       cont = 0;
   }
   new->atualizado = 0;
   new->quantidadeDeAcessor = 0;
   new->palavras = malloc( 4 * sizeof(int));
   Sleep(cont);
   cont++;
   new->tempo = time(NULL) ;
   new->endBloco = -1;
   return new;
}

Cache *CriarBlocoDeMemoriaCache(int tamanho){
    //aqui vem os reaproveitamento do Bloco da memoria Ram 
    //Porém tem a cacheHits e cacheMiss
    //Para saber o controler das caches 
     
    Cache *cache  = malloc (sizeof(Cache));
    cache->bloco  =  malloc (tamanho * sizeof(Bloco*));
    cache->tamanhoCache = tamanho;
    for ( int i =  0  ; i < tamanho ; i++){
       cache->bloco[i] = CriarBlocoDeMemoria();
    }
    return cache;
}


//Variaveis globais 

   Bloco **ram;
   int tamanhoRam , tamanhoHd;
   Cache *L1 , *L2 , *L3;
   int pc = 0;
   int **VetorInstrucoes;
   Bloco **Hd;


//fim variaveis Globais 



void liberaMemoria( int  **matriz , int tamanho){
   
      for ( int  i  = 0 ; i < tamanho ; i++){
          free(matriz[i]);
      }
}

void geradorDeIntrucoes(){

    FILE *arquivo = fopen("instrucoes.txt", "w");
    int random;
    int instruc[8] = {6, 6, 6, 6, 6, 6, 6, 6};
    srand(time(NULL));

    for (int i = 0; i < N_FOR; i++) {
        random = rand() % N_OPCODE;
        rep[i][0] = random;
        for (int j = 1; j <= (instruc[random]); j += 2) {
            rep[i][j] = rand() % N_MEM;
            rep[i][j + 1] = rand() % N_WORD;
        }
    }

    for (int i = 0; i < N_INST;) {
        random = (rand() % 100) + 1;
        if (random <= N_PROB && i + N_FOR < N_INST) {
            for (int j = 0; j < N_FOR; j++) {
                for (int k = 0; k < 7; k++) {
                    inst[i + j][k] = rep[j][k];
                }
            }
            i += N_FOR;
        } else {
            random = rand() % N_OPCODE;
            rep[i][0] = random;
            for (int j = 1; j <= instruc[random]; j += 2) {
                inst[i][j] = rand() % N_MEM;
                rep[i][j + 1] = rand() % N_WORD;
            }
            i++;
        }
    }
    for (int i = 0; i < N_INST; i++) {
        for (int k = 0; k < 6; k++) {
            fprintf(arquivo, "%d ", inst[i][k]);
        }
        fprintf(arquivo, "%d\n", inst[i][6]);
    }
    for ( int i =  0  ; i <  6 ; i++){
         fprintf(arquivo, "%d ", -1);
    }
    fprintf(arquivo, "\n");

    fclose(arquivo);
}

time_t getTime (Bloco *bloco){
    return bloco->tempo;
}

void  criarHd(){
    //criacao de um vetor para fazer a passagem  para o arquivo
    // ou seja  , vai ser varios vetores dentro do arquivo 
    int *blocoHd = malloc (  5 * sizeof(int));
    
    FILE *arquivo  = fopen ("arquivo.txt", "wb");
     
    for ( int i  = 0  ; i < 200 ; i++){
        //Primeiro do Bloco vai ser o endereço
        // depois do endereço vem o valor de cada palavra
        blocoHd[0] = i;
        blocoHd[1] = rand () % 15 + 1;
        blocoHd[2] = rand () % 15 + 1 ;
        blocoHd[3] = rand () % 15 + 1 ;
        blocoHd[4] = rand () % 15 + 1;
        
        fseek(arquivo, 0 , SEEK_END);
        fwrite(blocoHd ,sizeof(int), 5  , arquivo);
        // escrever no arquivo cada Bloco com 4 palavras e um endereço cada
    }
     
     fclose(arquivo);
}   

//função que efetuar a trocar dos bloco que estão 
//desatualizados 
void trocaEntrePosicoesDaRamEHd ( Bloco *blocoAtual  , int *bloco){
    printf("\ntestfetsfteftfste\n");
    printf("\n%d ",blocoAtual->endBloco);
    printf("%d",bloco[0]);
    blocoAtual->endBloco  = bloco[0];
    blocoAtual->atualizado  =  time(NULL) ;
    int contPalavra = 1;
    printf("\naqui fora\n");
    for ( int i  =  0  ; i < 4  ; i++){
        printf("\naqui dentro\n");
        blocoAtual->palavras[i] =  bloco[contPalavra];
        printf("\naqui antes do contador dentro\n");
        contPalavra++;
    }
    
}

int  *TrazDoHd(int endBloco){
    
     int *bloco  = malloc ( 5 * sizeof(int));
     FILE *leitura = fopen ("arquivo.txt", "rb+");
     fseek(leitura, 5 * endBloco *sizeof(int), SEEK_SET);
     fread(bloco, sizeof(int),5 ,leitura);
    
     fclose(leitura);
     printf("\nVindo do HD\n");
     /*for ( int  i  =  0 ; i < 5 ; i++){
         printf("%d ",bloco[i]);
     }

     */
    printf("\nFim do Hd\n");
    return bloco;

}

// funcão responsavel por realizar a atualizar no Hd
void AtualizarHd(int endBloco , Bloco *blocoParaAtualizar){
    //vai receber o Bloco que foi atualizado para ser atualizado dentro do hd

    // bloco Responsavel por passa para o Hd o seja 
    //ele que vai atualizar a posição do hd que e necessaria
    int *blocoAtualizar  = malloc (  5 * sizeof(int));

    blocoAtualizar[0] = blocoParaAtualizar->endBloco;
    int contPalavra = 1;

    for ( int i = 0  ; i <  4 ; i++){
        blocoAtualizar[contPalavra] = blocoParaAtualizar->palavras[i];
        contPalavra++;
    }
    // abrir o arquivo no formato de atualização
    FILE *arquivo = fopen ("arquivo.txt", "rb+");
    // para começa na linha do endereço 
    // ou seja multiplica o endereco do bloco desejado 
    // vs 5 que e a quantidade de palavra que tem em cada Bloco
    fseek(arquivo, 5 * endBloco *sizeof(int), SEEK_SET);
    fwrite(blocoAtualizar, sizeof(int), 5 ,arquivo);
    
    fclose(arquivo);
    

    int *bloco  = malloc ( 5 * sizeof(int));
     FILE *leitura = fopen ("arquivo.txt", "rb+");
     fseek(leitura, 5 * endBloco *sizeof(int), SEEK_SET);
     fread(bloco, sizeof(int),5 ,leitura);
    
     fclose(leitura);
    
     printf("\n %d atualizei no hd \n" , bloco[1]);
     system("pause");
}

//Responsavel por  preencher a memoria Ram 
void PreencheMemoriaRam ( ){
       
     for ( int i  =  0  ; i < tamanhoRam  ; i++){

         ram[i]->endBloco  = -1;
         
        for ( int j  =  0  ; j <  4  ; j++){

             ram[i]->palavras[j] = 0;
        }

     }

}

//Responsavel Por criar a interrupcão 
void geradorDeInterrupcoes( int **Inst , int tamanho){
     int i ;
     for ( i  = 0   ; i < tamanho ; i++){

         Inst[i][0] = rand() % 2 ;
         Inst[i][1] = rand() % 200;
         Inst[i][2] = rand() % 4;
         Inst[i][3] = rand() % 200;
         Inst[i][4] = rand() % 4;
         Inst[i][5] = rand() % 200;
         Inst[i][6] = rand() % 4;
     }
    
     Inst[i][0] =  -1 ;
     Inst[i][1] = -1;
     Inst[i][2] = -1;
     Inst[i][3] = -1;
     Inst[i][4] = -1;
     Inst[i][5] = -1;
     Inst[i][6] = -1;
}


// Criar Vetor de instrucoes 

int **CriarIntrucoes(int tamanho){
       
       // tem que ser criado um vetor que tem 7  colunas
       
       int **Inst = malloc( tamanho * sizeof (int*));

       for ( int i  =  0 ;  i <  tamanho ; i++ ){

           Inst[i] =  malloc ( 7 * sizeof(int));
       }

      return  Inst;
}  

//Preenche o vetor de instruções 
void PrencheIntrucoes(int tamanho)
{
   FILE *arquivo = fopen ("instrucoes.txt", "r");
    if (arquivo == NULL)
        exit(0);
    
    //printf("testestetsfetftsfe");
    int opcode  = 1 , end1 , enPal1 , end2 , enPal2 ,end3 , enPal3;
    int i  = 0;
    while(opcode != -1){
        
        fscanf (arquivo, "%d ",&opcode);
        fscanf (arquivo, "%d ",&end1);
        fscanf (arquivo, "%d ",&enPal1);
        fscanf (arquivo, "%d ",&end2);
        fscanf (arquivo, "%d ",&enPal2);
        fscanf (arquivo, "%d ",&end3 );
        fscanf (arquivo, "%d ",&enPal3);
       

       VetorInstrucoes[i][0] = opcode;
       VetorInstrucoes[i][1] = end1 ;
       VetorInstrucoes[i][2] = enPal1;
       VetorInstrucoes[i][3] =  end2;
       VetorInstrucoes[i][4] = enPal2;
       VetorInstrucoes[i][5] = end3;
       VetorInstrucoes[i][6] = enPal3;
       i++;
    }
    fclose (arquivo);


    // essa parte vai ser provisoria até criar o arquivo
    // que vem do gerador de arquivos
}


// final vetor de instrucoes 

// Fazer Buscar na cache e na memoria principal 

int  procuraNasCaches(Cache *cache , int endBloco , int endPalavra){
       
      //Procura na cache 1 para ver se o bloco está nela 
    for ( int i =  0  ; i < cache->tamanhoCache ; i++){ 
        
          if (cache->bloco[i]->endBloco  == endBloco){
              return i;
          }
    }
     
    return -1;
}

// Funcao para procura Na memoria Ram 
int ProcuraNaMemoriaRam( int endBloco  , int endPalavra){

     for ( int i  =  0 ; i < tamanhoRam ; i++){
            if ( ram[i]->endBloco  == endBloco ){

                return i;
            }
     }
    
    return -1;
}


// Trocar entre as memorias caches e a memoria ram

int  TrocaEntreAsMemoriasCache(Cache *cacheBaixo , Cache *cacheAlto , int indBloco){
    // primeiro procura se existe um bloco vazio dentro da memoria de 
    // mais alto nivel 
    //printf("\ncheguei aqui\n");
    for ( int i  =  0 ; i < cacheAlto->tamanhoCache  ; i++){
    
         if ( cacheAlto->bloco[i]->endBloco == -1){

              // temos que verifica a funcão do time 
              //time_t a = time(NULL);
              cacheAlto->bloco[i]->endBloco = cacheBaixo->bloco[indBloco]->endBloco;
              cacheAlto->bloco[i]->atualizado = 0;
              cacheAlto->bloco[i]->quantidadeDeAcessor =+ 1;
              cacheAlto->bloco[i]->tempo = cacheBaixo->bloco[indBloco]->tempo;

              // Responsavel por zera o bloco Anterior 
              cacheBaixo->bloco[indBloco]->endBloco = -1;

              for ( int j  = 0  ; j < 4; j++){
                   
                   
                   cacheAlto->bloco[i]->palavras[j] = cacheBaixo->bloco[indBloco]->palavras[j];
                  
                   cacheBaixo->bloco[i]->palavras[j] = 0;
              }
              //printf("\nEndereco ta passando aqui\n");
              //printf("\n %d \n",cacheAlto->bloco[i]->endBloco);
              return  i ;
              
              //Aqui em baixo eu tenho que apagas os dados da cache de baixo nivel 
              // tenho que passa que passa qual que e o endereço do bloco e da palavra
              // esse endereço do bloco vao receber o bloco que está sendo
              //procurado
         }
    }
    
    //printf("\n Essa parte do tempo ta dando muito erro \n");
    // termina A procura por Blocos Vazios na Cache 
    
    //caso as memorias de alto niveis tenham sido completadas 
    
    // caso em que as caches estão cheias 
    
    long int tempPrimeiro = time(NULL);
    int guardaIndice = 0;

    // criar  um cache  auxiliar  para ajuda na passagem de uma cache 
    // para a outra sem perder nenhum dado
    Cache *aux  =  malloc (sizeof(Cache));
    aux->bloco = malloc (sizeof( Bloco*));
    
    aux->bloco[0] = CriarBlocoDeMemoria();;
    //printf("!!!!!!Tempo memoria cache ");

    //printf("\nRoda tempo\n");
  
    for (int i = 0; i < cacheAlto->tamanhoCache; i++)
    {   //printf("\nteste %ld  vs cache 3 %ld \n",tempPrimeiro , getTime(cacheAlto->bloco[i]));
        
        if (tempPrimeiro > cacheAlto->bloco[i]->tempo)
        {
            tempPrimeiro = cacheAlto->bloco[i]->tempo;
            guardaIndice = i;
         
            //printf("\nTempos %d ", tempPrimeiro);
            //printf("\nIndice %d" ,i);
            // verificacao para ver qual que e o Bloco de memoria
            // que está a mais  tempo sem ser atualizado
            // , ou seja , que tem o menor tempo medido em segundos
        }
    }
    //printf("\nFim Roda tempo\n");
    // troca entre as caches 
    /* printf("\nteste cache inicio\n");
    printf("\n endereco Bloco -> %d \n",cacheBaixo->bloco[indBloco]->endBloco);
    printf("\n Palavra 1  -> %d \n",cacheBaixo->bloco[indBloco]->palavras[0]);
    printf("\n Palavra 1  -> %d \n",cacheBaixo->bloco[indBloco]->palavras[1]);
    printf("\n Palavra 1  -> %d \n",cacheBaixo->bloco[indBloco]->palavras[2]);
    printf("\n Palavra 1  -> %d \n",cacheBaixo->bloco[indBloco]->palavras[3]); */
    aux->bloco[0] = cacheAlto->bloco[guardaIndice];

    //printf("\nporque parou eu não entendir\n");
    //printf("\n alto antes  %d \n",cacheAlto->bloco[guardaIndice]->endBloco);
    cacheAlto->bloco[guardaIndice] = cacheBaixo->bloco[indBloco];
    //printf("\nparou aqui \n");
    
    cacheBaixo->bloco[indBloco] = aux->bloco[0];
  
    /*printf("\n endereco Bloco -> %d \n",cacheBaixo->bloco[indBloco]->endBloco);
    printf("\n Palavra 1  -> %d \n",cacheBaixo->bloco[indBloco]->palavras[0]);
    printf("\n Palavra 1  -> %d \n",cacheBaixo->bloco[indBloco]->palavras[1]);
    printf("\n Palavra 1  -> %d \n",cacheBaixo->bloco[indBloco]->palavras[2]);
    printf("\n Palavra 1  -> %d \n",cacheBaixo->bloco[indBloco]->palavras[3]); 
    */
    //printf("Aqui então jovem");
    //printf("\nEndereco ta passando aqui\n");
    //printf("\n alto %d \n",cacheAlto->bloco[guardaIndice]->endBloco);
    //printf("\n baixo%d \n",cacheBaixo->bloco[indBloco]->endBloco);
    //printf("\n %d \n",guardaIndice);

    
    return guardaIndice ;
   // fim 
}

// troca entre as memoria cache e a memoria Ram 

int TrocaEntreMemoriaCache3EMemoriaRam(int endBloco  , int end){
    
     
     // Procura na cache  3 um Bloco vazio para  fazer a trasferencias 
     // assim que achado efetua a trocar entre  a memoria principal 
     //e a cache 3 
     for (int i =  0 ; i < L3->tamanhoCache; i++){
         
          if ( L3->bloco[i]->endBloco == -1){
             
             L3->bloco[i]->endBloco = ram[endBloco]->endBloco;
             L3->bloco[i]->tempo  = ram[endBloco]->tempo;
             for ( int j =  0  ; j <  4  ; j++){

                 L3->bloco[i]->palavras[j] = ram[endBloco]->palavras[j];
             }
            
             return i;
          }
     } // procura bloco vazio dentro da cache  3 
     
     // Procura um bloco que esteja mais tempo sem ser usado na memoria cache 3
     //printf("\nDeu pau aqui que comeca a merda\n");

     long int temPrimeiro = time(NULL);
     //printf("\nteste do tempo aqui %ld \n",teste);
     int guardaIndice = 0;
     //Cache *aux  =  malloc (sizeof(Cache));
    // aux->bloco = malloc (sizeof( Bloco*));
     //aux->bloco[0] = CriarBlocoDeMemoria();
     
     // procura um bloco com mais tempo sem ser acessado  na 
     // cache  3 
     //printf("\n!!!!!!Tempo memoria Ram\n");
     for ( int i  =  0  ; i < L3->tamanhoCache ; i++){
         //printf("\n\nteste %ld  vs cache 3 %ld",temPrimeiro , L3->bloco[i]->tempo);
         if ( temPrimeiro > L3->bloco[i]->tempo ){
                temPrimeiro = L3->bloco[i]->tempo ;
                guardaIndice = i;
         }
     }
    //aux->bloco[0]->endBloco = L3->bloco[guardaIndice]->endBloco;
    //aux->bloco[0]->atualizado  = L3->bloco[guardaIndice]->atualizado;

    // verifica se o bloco que foi achado com mais tempo sem ser acessador
    // foi atualizado em alguma cache 

    if ( L3->bloco[guardaIndice]->atualizado == 1){

        for ( int i  = 0  ; i < tamanhoRam ; i++){

            if ( L3->bloco[guardaIndice]->endBloco == ram[i]->endBloco){
                //ram[i]->endBloco = L3->bloco[guardaIndice]->endBloco;
                //ram[i]->atualizado  =  0;
                
                for ( int j  =  0  ; j < 4 ; j++){
                    ram[i]->palavras[j] =  L3->bloco[guardaIndice]->palavras[j];

                }
                
            }
        }
    }

    // efetua a troca do conteudo entre memoria principal e 
    // a memoria cache 
    L3->bloco[guardaIndice]->endBloco = ram[endBloco]->endBloco;
    L3->bloco[guardaIndice]->tempo = ram[endBloco]->tempo;
    L3->bloco[guardaIndice]->atualizado = ram[endBloco]->atualizado;
    for ( int i  =  0  ; i < 4 ; i++){
        L3->bloco[guardaIndice]->palavras[i]  = ram[endBloco]->palavras[i];
    }

    return guardaIndice;

}

int trocarEntreMemoriaRamEHd(int *blocoDoHd){

    // se existe Bloco vazio na memoria Ram 
    printf("\nto aqui ze ruela\n");
    for ( int i  =  0  ; i < tamanhoRam ; i ++){

        if ( ram[i]->endBloco  == -1){
            
            printf("\nentrei aqui\n");
            ram[i]->endBloco = blocoDoHd[0];
            ram[i]->tempo  = time(NULL);
            ram[i]->atualizado = 0 ;
            int contPalavra  = 1;
            for ( int j  = 0  ; j < 4  ; j++){

                ram[i]->palavras[j] = blocoDoHd[contPalavra];
                contPalavra++;
            }
            
           return i;
        }
    }

    
    //vai repertir um monte de código aquiii 
    // tenho que criar um funcão para mexer com isso 

    
     long int temPrimeiro = time(NULL);
     printf("\ncheguei aqui ze da silva\n");
     //printf("\nteste do tempo aqui %ld \n",teste);
     int guardaIndice = 0;
     //Cache *aux  =  malloc (sizeof(Cache));
    // aux->bloco = malloc (sizeof( Bloco*));
     //aux->bloco[0] = CriarBlocoDeMemoria();
     
     // procura um bloco com mais tempo sem ser acessado  na 
     // cache  3 
     //printf("\n!!!!!!Tempo memoria Ram\n");
     for ( int i  =  0  ; i <tamanhoRam; i++){
         printf("\n\nteste %ld  vs cache 3 %ld",temPrimeiro ,  ram[i]->tempo );
         
         if ( temPrimeiro > ram[i]->tempo ){
                temPrimeiro = ram[i]->tempo ;
                guardaIndice = i;
                printf("entrei aqui em");
               
         }
     }
    //aux->bloco[0]->endBloco = L3->bloco[guardaIndice]->endBloco;
    //aux->bloco[0]->atualizado  = L3->bloco[guardaIndice]->atualizado;

    // verifica se o bloco que foi achado com mais tempo sem ser acessador
    // foi atualizado em alguma cache 

    if ( ram[guardaIndice]->atualizado == 1){
        printf("entrei aqui e deu merda de mais ");
        system("pause");
        //funcao responsavel por atualizar o hd 
        //de acordo com os dados que vão sair da memoria 
        //Ram 
        AtualizarHd( guardaIndice , ram[guardaIndice]);
        /*
        for ( int i  = 0  ; i < tamanhoRam ; i++){

            if ( ram[guardaIndice]->endBloco == Hd[i]->endBloco){
                Hd[i]->endBloco = ram[guardaIndice]->endBloco;
                Hd[i]->atualizado  =  0;

                for ( int j  =  0  ; j < 4 ; j++){
                    Hd[i]->palavras[j] =  ram[guardaIndice]->palavras[j];

                }
            }
        }
        */
    }

    // efetua a troca do conteudo entre memoria principal e 
    // o hd 

    // funcão que efetuar a trocar entre  o bloco que  tem menos 
    // tempo sem ser acessador na Ram e o Bloco que estão esperando 
    // na memoria cache  1 
     printf("teste");
     trocaEntrePosicoesDaRamEHd ( ram[guardaIndice], blocoDoHd );
    /*ram[guardaIndice]->endBloco = Hd[endBloco]->endBloco;
    
    for ( int i  =  0  ; i < 4 ; i++){
        ram[guardaIndice]->palavras[i]  = Hd[endBloco]->palavras[i];
    }
    
    */
    printf("\ncheguei no final\n");
    return guardaIndice;

    
}


//Funcao que executar as verificações nas memorias Ram e  cache 

int  MMU ( int  endBloco  , int endPalavra ,int tipoOperacao  , int resultado){
    
    int indBloco;
    
    if (procuraNasCaches(L1, endBloco, endPalavra) >= 0)
    {  
        indBloco = procuraNasCaches(L1, endBloco, endPalavra);
        printf("\nAchei na cache  1 !!!!!!\n");
        cacheHits += 100;
        contCache1++; 
        Sleep(100);
        L1->bloco[indBloco]->tempo = time(NULL);
        /* printf("\n endereco Bloco -> %d \n",L1->bloco[indBloco]->endBloco);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[0]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[1]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[2]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[3]);
        */
        //return L1->bloco[indBloco]->palavras[endPalavra];

    }
    else if (procuraNasCaches(L2, endBloco, endPalavra) >= 0)
    {   printf("\nAchei Na cache  2 !!!!!!!\n");
        indBloco  = procuraNasCaches(L2, endBloco, endPalavra);
        indBloco = TrocaEntreAsMemoriasCache(L2, L1, indBloco);
        cacheHits += 100;
        contCache2++;

        /* printf("\n endereco Bloco -> %d \n",L1->bloco[indBloco]->endBloco);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[0]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[1]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[2]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[3]);
        */
    }
    else if (procuraNasCaches(L3, endBloco, endPalavra) >= 0)
    {   printf("\nAchei Na cache  3 !!!!!!! \n");
        indBloco  = procuraNasCaches(L3, endBloco, endPalavra);
        indBloco = TrocaEntreAsMemoriasCache(L3, L2, indBloco);
        //printf("\n I = 2 \n");
        indBloco = TrocaEntreAsMemoriasCache(L2, L1, indBloco);
        printf("\nDepois de travar\n");
        
        cacheHits += 100;
        contCache3++;

         /* printf("\n endereco Bloco -> %d \n",L1->bloco[indBloco]->endBloco);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[0]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[1]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[2]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[3]);
        */
    }
    // Buscar na memoria Cache em Buscar do conteudo 
    //caso seja encontrado vai apenas fazer a transferencias  entre as caches
    
    else if ( ProcuraNaMemoriaRam( endBloco , endPalavra) >= 0)
    {   
        printf("\nachei na Ram\n");

        indBloco = ProcuraNaMemoriaRam( endBloco , endPalavra);
        indBloco  = TrocaEntreMemoriaCache3EMemoriaRam(indBloco, endPalavra);
        //printf("\n I = 1 \n");
        indBloco = TrocaEntreAsMemoriasCache(L3, L2, indBloco);
        //printf("\n I = 2 \n");
        indBloco = TrocaEntreAsMemoriasCache(L2, L1, indBloco);
        //printf("\n I = 3 \n");
        //precisa atualiza os dados aqui na cache  1 
        //quantAcessor  , atualização e tempo do ultimo acessor 
         cacheNiss += 100;
         contMemoriaRam++;
         /* printf("\n endereco Bloco -> %d \n",L1->bloco[indBloco]->endBloco);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[0]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[1]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[2]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[3]);
        */
    }
    else {

        printf("\nAchei no Hd !!!!!\n");
        // Retorna um array com os dados daquele endereço 
        printf("Deu lenha demais ");

        indBloco = trocarEntreMemoriaRamEHd(TrazDoHd(endBloco));
        indBloco  = TrocaEntreMemoriaCache3EMemoriaRam(indBloco, endPalavra);
        printf("\n I = 1 \n");
        indBloco = TrocaEntreAsMemoriasCache(L3, L2, indBloco);
        printf("\n I = 2 \n");
        indBloco = TrocaEntreAsMemoriasCache(L2, L1, indBloco);
        printf("\n I = 3 \n");
        //precisa atualiza os dados aqui na cache  1 
        //quantAcessor  , atualização e tempo do ultimo acessor 
         cacheNiss += 100;
         contHd++;
         
        /* printf("\n endereco Bloco -> %d \n",L1->bloco[indBloco]->endBloco);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[0]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[1]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[2]);
         printf("\n Palavra 1  -> %d \n",L1->bloco[indBloco]->palavras[3]);
        */
    }
    


    if (tipoOperacao == 1){
        //printf("\nAntes %d \n",L1->bloco[indBloco]->palavras[endPalavra]);
        L1->bloco[indBloco]->palavras[endPalavra] = resultado;
        L1->bloco[indBloco]->atualizado = 1;
        //printf("Depois %d\n",L1->bloco[indBloco]->palavras[endPalavra]);
        return 1;
    }
    Sleep(100);
    L1->bloco[indBloco]->tempo = time(NULL);
    return L1->bloco[indBloco]->palavras[endPalavra];
}  


// executar as intrucoes
int Maquina(int pc, int **instrucoes, int NumeroInterrupcao)
{

    int opcode = 1, armazenarRetorno;
    int operador1, operador2, resultado;
    printf("\n-> %d <-\n", pc);
    int quantDivisao = 1;
    while (opcode != -1)
    {
        opcode = instrucoes[pc][0];
        switch (opcode)
        {
        //soma
        case 0:
            printf("\n----->>>Somandooooooooooo aqui <------------\n");
            // tem que ver esse retorno aqui

            operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
            printf("\n\t\t ->Operador 1 : %d <-\n", operador1);
            //printf(" \nteste \n");
            operador2 = MMU(instrucoes[pc][3], instrucoes[pc][4], 0, 0);
            printf("\n\t\t ->operador 2 : %d <-\n", operador2);

            resultado = operador1 + operador2;

            armazenarRetorno = MMU(instrucoes[pc][5], instrucoes[pc][6], 1, resultado);

            printf("\n\t\t->Fim da soma: %d\n<-", resultado);

            printf("\n<----------------------------------------->\n");
            break;
            //subtracao
        case 1:

            printf("\n----->>>Subtracao  aqui <------------\n");
            operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
            printf("\n\t\t ->Operador 1 : %d <-\n", operador1);
            operador2 = MMU(instrucoes[pc][3], instrucoes[pc][4], 0, 0);
            printf("\n\t\t ->operador 2 : %d <-\n", operador2);
            resultado = operador1 - operador2;
            armazenarRetorno = MMU(instrucoes[pc][5], instrucoes[pc][6], 1, resultado);
            printf("\n\t\t ->Subtracaooo : %d <-\n", resultado);
            printf("\n<----------------------------------------->\n");

            break;

        //Multiplicacao
        case 2:

            printf("\n-------->>>>Multiplicacao Aqui mano <<<---------\n");
            int quantOperacao;
            operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
            printf("\nOperador 1 ->> %d\n", operador1);
            quantOperacao = MMU(instrucoes[pc][3], instrucoes[pc][4], 0, 0);
            printf("\nOperador 2 -->>%d\n", quantOperacao);
            MMU(instrucoes[pc][5], instrucoes[pc][6], 1, operador1);
            for (int i = 1; i < quantOperacao; i++)
            {

                operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
                operador2 = MMU(instrucoes[pc][5], instrucoes[pc][6], 0, 0);
                resultado = operador1 + operador2;
                MMU(instrucoes[pc][5], instrucoes[pc][6], 1, resultado);
            }

            printf("\n->>Multiplicacao %d -->\n", resultado);
            break;
        //divisão
        case 3:
            //opcode da Divisão
            
            printf("\nDivisao !!!!!\n");
            
            // valor fixo so para testa a divisao 
            
            
            operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
            printf("Divisor: %d " , operador1);
            
            operador2 = MMU(instrucoes[pc][3], instrucoes[pc][4], 0, 0);
            printf("Dividendo : %d ",operador2);
            if (operador1 > operador2)
            {

                while (operador1 > operador2)
                {

                    resultado = operador1 - operador2;
                    MMU(instrucoes[pc][1], instrucoes[pc][2], 1, resultado);
                    operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
                    MMU(instrucoes[pc][5], instrucoes[pc][6], 1, quantDivisao);
                    quantDivisao++;
                }
            }
            
            printf("\n Valor Divisao : %d\n",quantDivisao);
            break;

        // Calculo da Area
        case 4:

            printf("\n-------->>>>Calculo da Area<<<---------\n");

            operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
            printf("\nLargura ->> %d\n", operador1);
            quantOperacao = MMU(instrucoes[pc][3], instrucoes[pc][4], 0, 0);
            printf("\nAltura -->>%d\n", quantOperacao);
            MMU(instrucoes[pc][5], instrucoes[pc][6], 1, operador1);
            for (int i = 1; i < quantOperacao; i++)
            {

                operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
                operador2 = MMU(instrucoes[pc][5], instrucoes[pc][6], 0, 0);
                resultado = operador1 + operador2;
                MMU(instrucoes[pc][5], instrucoes[pc][6], 1, resultado);
            }

            printf("\n->>Multiplicacao %d -->\n", resultado);
            break;

        case 5:

            printf("\n --> O Fatorial <----\n");
            resultado = 0;
            int operador3;
            //coloca o Valor do segundo endereco com sendo  1
            //Para efetuar a operacao
            
            //exemplo = 7!
            // 7 * 6 * 5 * 4 * 3 * 2 * 1
            operador1 = MMU(instrucoes[pc][1], instrucoes[pc][2], 0, 0);
            printf("--->Numero Fatorial %d \n", operador1);
            MMU(instrucoes[pc][3], instrucoes[pc][4], 1, 2);
            MMU(instrucoes[pc][5], instrucoes[pc][6], 1, 0);

            for (int i = 2; i <= operador1; i++)
            {

                printf("\ni -> %d \n", i);

                for (int j = 1; j < i; j++)
                {
                    printf("\n j -> %d \n", j);
                    operador2 = MMU(instrucoes[pc][3], instrucoes[pc][4], 0, 0);
                    operador3 = MMU(instrucoes[pc][5], instrucoes[pc][6], 0, 0);
                    resultado = operador2 + operador3;
                    MMU(instrucoes[pc][5], instrucoes[pc][6], 1, resultado);
                }
                MMU(instrucoes[pc][3], instrucoes[pc][4], 1, resultado);
                printf("\n resultado -> %d \n", resultado);
            }

            printf("\nFatorial -> %d \n", resultado);

            // fim fatorial
            break;
        }

        pc++;
        printf("\nquantidade de instrucoes :  %d\n", pc);

        int inter;

        inter = rand() % 10;

        if (inter == 1 && opcode != -1 && NumeroInterrupcao == 0)
        {
            printf("\n_>>>!!!!!Deu uma interrupcao o mouse foi alterado !!!!->>>\n");
            system("pause");
            system("cls");

            FILE *arquivo = fopen("gravaPc.txt", "w");
            fprintf(arquivo, "%d", pc);
            fclose(arquivo);
            pc = 0;
            int tamanho =  30;
            int **interrupcao = CriarIntrucoes(tamanho + 4);
            
            geradorDeInterrupcoes(interrupcao, tamanho);
            Maquina(pc, interrupcao, 1);
            //tenho que ver alguma coisa para fazer quando
            // a interrupcao for acionada
            //Tem que existir alguma alteração cache
            printf(" ->%d <-", pc);

            printf("\n<---- Fim Interrupcao !!!!!!!!!!!!!! -->>>\n");

            arquivo = fopen("gravaPc.txt", "r");
            fscanf(arquivo, "%d", &pc);
            fclose(arquivo);
            liberaMemoria( interrupcao , tamanho);
            printf("Fim arquivo");
        }
    }
    return 0;
    printf("\nso para sair dos opcodes\n");
}

int main (){
    tamanhoRam = 200;
    tamanhoHd = 300;// 
    int tamCache1  , tamCache2 , tamCache3 , opcao;

    printf("\n\n-->Eis Aqui  o Funcionamento de Um computador Real --<< \n\n");
    printf("\n\t\tQual os Tamanhos da Cache ???\t\n");
    printf("\n 1 - \tCache 1 -> 8  \tCache 2 -> 16  \tCache -> 32\n");
    printf("\n 2 -  \tCache 1 -> 16 \tCache 2 -> 32  \tCache -> 64\n");
    printf("\n 3 -  \tCache 1 -> 32 \tCache 2 -> 64  \tCache -> 128\n");
    printf("\nopcao ?? ");
    scanf("%d",&opcao);
    printf("\n\n");

    if ( opcao == 1  ){
       tamCache1 = 4;
       tamCache2 = 8;
       tamCache3 = 16;
    }
    else if ( opcao == 2) {
         tamCache1 = 12;
         tamCache2 = 32;
         tamCache3 = 64;
    }
    else if ( opcao == 3 ){
         tamCache1 = 32;
         tamCache2 = 64;
         tamCache3 = 128;
    }
    
    int pc = 0;
    srand(time(NULL));
    L1 =  CriarBlocoDeMemoriaCache(tamCache1);
    L2 = CriarBlocoDeMemoriaCache(tamCache2);
    L3 = CriarBlocoDeMemoriaCache(tamCache3);
    
    criarHd();
    //Criacao de um endereço de memoria para as três caches

    ram  = malloc( tamanhoRam * sizeof(Bloco*));
    for( int i  =  0  ; i < tamanhoRam ; i++){
        ram[i] = CriarBlocoDeMemoria();
    }
    PreencheMemoriaRam();
    geradorDeIntrucoes();

    
    system("pause");
    system("cls");
    //Funcao responsavel pela Criacao do espaco de memoria para o hd 
    criarHd();
    // funcao responsavel por inicializar o Hd;
    

    VetorInstrucoes = CriarIntrucoes(N_INST + 4);
    PrencheIntrucoes( N_INST);
    
    int tipoIntrucao  = 1;
    Maquina(pc ,  VetorInstrucoes , 0);
    printf("\nCache Hits : %d \n" ,cacheHits);
    printf("\nCache Niss : %d \n" , cacheNiss);
    
    printf("\nHd ->%d | Ram -> %d | Cache 1 -> %d | Cache 2 -> %d | Cache 3 -> %d\n",
    contHd,contMemoriaRam ,contCache1 ,contCache2 ,contCache3);

    printf("\nMemoria Ram\n");
    for ( int i  = 0  ; i <  tamanhoRam ; i++ ){

        printf("%d endereco -> %d ",i ,ram[i]->endBloco);

        for ( int j  =  0  ;  j < 4  ; j++){
            printf(" %d ",ram[i]->palavras[j]);
        }
        printf("\n");
    }
    
    printf("\n\nCache 3\n");
    for ( int i  = 0  ; i <  L3->tamanhoCache ; i++ ){

        printf(" %d endereco %d ",i , L3->bloco[i]->endBloco);

        for ( int j  =  0  ;  j < 4  ; j++){
            printf(" %d ",L3->bloco[i]->palavras[j]);
        }
        printf("\n");
    }
    
    
    
}