/* Multiple facility location through K-Means */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define MAX_RA 31
#define MAX_NOME 100
#define inf 1e+9
#define SANTA_MARIA 22

/* Definicao do registro (estrutura) de cada regiao administrativa */

struct regiaoAdministrativa {
    /* Nome da regiao administrativa */
    char *nome_ra;
    
    /* quantidade de setores censitarios */
    int qtdeSetores;
    
    /* quantidade de conselhos tutelares dessa RA */
    int qtdeConsTutelares;
    
    /* latitudes de cada setor censitario */
    double *lat;
    
    /* longitude de cada setor censitario */
    double *longit;
    
    /* peso de cada setor censitario */
    double *pesos; 
    
    /* matriz de distancias entre os setores censitarios */
    double **dist;  
    
    /* indica qual setor censitario terah um conselho tutelar */
    int *conselhosTutelares;
    
    /* geocode de cada setor censitario */
    double *geoCode;
    
};

/* Define o tipo de dados RAs como sendo a regiao administrativa */
typedef struct regiaoAdministrativa RAs;

char * aloca_string(int tamanho)
{
    char *str;
    str = (char *) malloc (sizeof(char) * tamanho);
    
    return str;
}

void libera_string(char *str)
{
    free(str);
}

int * aloca_int(int tamanho)
{
    int *var;
    var = (int *) malloc (sizeof(int) * tamanho);
    
    return var;
}

void libera_int(int *array)
{
    free(array);
}

double * aloca_double(int tamanho)
{
    double *var;
    var = (double *) malloc (sizeof(double) * tamanho);
    
    return var;
}

void libera_double(double *array)
{
    free(array);
}

/* Aloca um vetor de RAs */
RAs** alocaRAs(int totalRAs, int qtdeSet)
{
    int i,j,ra;
    RAs **todos;
    
    todos = (RAs **) malloc (sizeof(RAs *)*totalRAs);
    
    for(ra = 0; ra < MAX_RA; ra++)
    {
        todos[ra] = (RAs *) malloc (sizeof(RAs));
        
        todos[ra]->qtdeConsTutelares = 1;
        
        todos[ra]->dist = (double **) malloc (sizeof(double *) * qtdeSet);
        
        for(i = 0; i < qtdeSet; i++)
        {
            todos[ra]->dist[i] = (double *) malloc(sizeof(double)*qtdeSet);
            
            for(j =0; j < qtdeSet ;j++)
            {
                todos[ra]->dist[i][j] = -1;
            }
        }
        
        todos[ra]->lat = (double *) malloc(sizeof(double)*qtdeSet);
        todos[ra]->longit = (double *) malloc(sizeof(double)*qtdeSet);
        todos[ra]->pesos = (double *) malloc(sizeof(double)*qtdeSet);
        todos[ra]->conselhosTutelares = (int *) malloc(sizeof(int)*qtdeSet);
        todos[ra]->geoCode = (double *) malloc(sizeof(double)*qtdeSet);
        todos[ra]->nome_ra = (char *) malloc(sizeof(char)*MAX_NOME);
    }
    
    return todos;
}

/* funcao que libera memoria das RAs */
void liberaRAs(RAs **regioesAdm)
{
    
    int i,ra;
    
    for(ra = 0; ra < MAX_RA;ra++)
    {
        for(i = 0; i < regioesAdm[ra]->qtdeSetores; i++)
        {
            printf("ra:%d i:%d\n",ra,i);
            free(regioesAdm[ra]->dist[i]);
        }
     /*   
        free(regioesAdm[ra]->dist);

        free(regioesAdm[ra]->lat);
        
        free(regioesAdm[ra]->longit);
        
        free(regioesAdm[ra]->pesos);
        
        /*free(regioesAdm[ra]->conselhosTutelares);
        
        free(regioesAdm[ra]->nome_ra);
        
        free(regioesAdm[ra]->geoCode);
        
        free(regioesAdm[ra]);
        
        //regioesAdm[ra] = NULL;*/
    }
    //free(regioesAdm);
    
    //regioesAdm = NULL;
}

/* Algoritmo de ordenacao BubbleSort */
void bubbleSort(double *array, int *indices, int n)
{
    int i,j,aux,aux_i;
    
    for(i = 0; i < n; i++)
    {
        for(j = i; j < n; j++)
        {
            if(array[i] < array[j])
            {
                aux = array[i];
                aux_i = indices[i];
                array[i] = array[j];
                indices[i] = indices[j];
                array[j] = aux;
                indices[j] = aux_i;
                
            }
        }
    }
}

/* Algoritmo K-Means */
double KMedias(RAs *regiao, int K)
{
    int i,j,x,cent,flag,sc1,id_menor,menor_dist,dist;
    int *centroides,*centroides_past,*rotulos,*qt;
    double energia,*distCentroide,menor,dist_menor,*soma_lat,*soma_long,*media_lat,*media_long;
    
    centroides = aloca_int(K);
    
    centroides_past = aloca_int(K);
    
    soma_lat = aloca_double(K);
    
    soma_long = aloca_double(K);
    
    media_lat = aloca_double(K);
    
    media_long = aloca_double(K);
    
    qt = aloca_int(K);
    
    rotulos = aloca_int(regiao->qtdeSetores);
    
    distCentroide = aloca_double(regiao->qtdeSetores);
    
    /* Inicializacao dos centroides */
    flag = 1;
    for( j = 0; j < K ;j++)
    {
        centroides[j] = -1;
        x = rand() % regiao->qtdeSetores;
        
        while(flag)
        {
            flag  = 0;
            x = rand() % regiao->qtdeSetores;
            for(i = 0; i < K;i++)
            {
                if(x == centroides[i])
                {
                    flag = 1;
                    continue;
                }
            }
        }
        centroides[j] = x;
    }
    
    flag = 1;
    /* enquanto os centroides alterarem o posicionamento, permanece no K-Means */
    while(flag == 1)
    {
        
        /* copia os valores dos centroides */
        for(cent = 0; cent < K; cent++)
        {
            centroides_past[cent] = centroides[cent];
        }
        
        /* para cada setor censitario */
        for(sc1 = 0; sc1 < regiao->qtdeSetores ; i++)
        {
            /* calcula a distancia para cada centroide do KMedias */
            
            for(cent = 0; cent < K; cent++)
            {
                distCentroide[sc1] = regiao->dist[centroides[cent]][sc1];
            }
            
            
            /* Encontra o centroide mais proximo ao setor censitario sc1 */
            menor = 0;
            dist_menor = distCentroide[0];
            for(cent = 1; cent < K; cent++)
            {
                if(menor < distCentroide[cent])
                {
                    dist_menor = distCentroide[cent];
                    id_menor = cent;
                }
            }
            
            /* Atribui o centroide mais proximo ao setor censitario sc1*/
            rotulos[sc1] = id_menor;
            energia = energia + dist_menor; // mudar isso aqui, considerar a energia separada para cada rotulo (conselho tutelar)
        }
        
        /* Atualiza o valor dos centroides */
        
        for(cent = 0; cent < K; cent++)
        {
            qt[cent]=0;
            soma_lat[cent] = 0;
            soma_long[cent] = 0;
            for(sc1 = 0; sc1 < regiao->qtdeSetores; sc1++)
            {
                if(rotulos[sc1] == cent)
                {
                    soma_lat[cent] = soma_lat[cent] + regiao->lat[sc1];
                    qt[cent]++;
                    soma_long[cent] = soma_long[cent] + regiao->longit[sc1];
                }
            }
            
            media_lat[cent] = soma_lat[cent]/(double) qt[cent];
            media_long[cent] = soma_long[cent]/(double) qt[cent];
            
            menor_dist = inf;
            for(sc1 = 0; sc1 < regiao->qtdeSetores ; sc1++)
            {
                
                dist = (media_lat[cent] - regiao->lat[sc1])*(media_lat[cent] - regiao->lat[sc1]) + (media_long[cent] - regiao->longit[sc1])*(media_long[cent] - regiao->longit[sc1]);
                
                dist = sqrt(dist);
                
                if(dist < menor_dist)
                {
                    centroides[cent] = sc1;
                }
            }
        }
        
        /* criterio de parada */
        flag = 0;
        for(cent = 0; cent < K; cent++)
        {
            if(centroides_past[cent] != centroides[cent])
            {
                flag = 1;
                break;
            }
        }
    }
    
    for(cent = 0; cent < K; cent++)
    {
        regiao->conselhosTutelares[cent] = centroides[cent];
    }
    
    libera_int(centroides);
    
    libera_int(centroides_past);
    
    libera_int(rotulos);
    
    libera_double(distCentroide);
    
    libera_double(soma_lat);
    
    libera_double(soma_long);
    
    libera_double(media_lat);
    
    libera_double(media_long);
    
    return energia;
}

void posicionaConselhos(RAs **regioesAdministrativas)
{
    int ra,energia,maior,nroConsTutelares,sc1,sc2,cent,indi;
    int *indices,*ind_menor;
    double *menor, *soma,dist,somasc,menor_dist,menor2;
    
    menor = aloca_double(MAX_RA);
    soma = aloca_double(500);
    indices = aloca_int(MAX_RA);
    ind_menor = aloca_int(MAX_RA);
    
    /* para cada RA */
    //for(ra = 0; ra < MAX_RA; ra++)
    for(ra = SANTA_MARIA; ra <= SANTA_MARIA; ra++)
    {
        
        printf("RA: %d\nInfo: %d %s\n",ra,regioesAdministrativas[ra]->qtdeSetores,regioesAdministrativas[ra]->nome_ra);
        
        ind_menor[ra] = MAX_RA+1;
        indi = MAX_RA+1;
        menor[ra] = inf;
        menor2 = inf;
        //para cada setor censitario
        for(sc1 = 0; sc1 < regioesAdministrativas[ra]->qtdeSetores ; sc1++)
        {
            printf("Peso: %lf ",regioesAdministrativas[ra]->pesos[sc1]);
            somasc = 0;
            nroConsTutelares = 0;
            for(sc2 = 0; sc2 < regioesAdministrativas[ra]->qtdeSetores ; sc2++)
            {
                if(regioesAdministrativas[ra]->dist[sc1][sc2] > 0)
                {
                    nroConsTutelares++;
                    somasc = somasc + (regioesAdministrativas[ra]->dist[sc1][sc2])*(regioesAdministrativas[ra]->pesos[sc1]);
                    printf(" dist %d => %d = %lf\n",sc1,sc2,regioesAdministrativas[ra]->dist[sc1][sc2]);
                }
                    //
                    
            }
            printf("sc: %d somasc: %lf menor2: %lf numconselhos: %d\n",sc1,somasc,menor2,nroConsTutelares);
            if(somasc < menor2 && nroConsTutelares > 0)//menor[ra])
            {
                menor2 = somasc;
                indi = sc1;
            }
        }
        
        menor[ra] = menor2;
        ind_menor[ra] = indi;

        printf("FINAL\n menor2: %lf ind_menor: %d ra:%d\n",menor2,indi,ra);

    }
    
    bubbleSort(menor,indices,MAX_RA);
        
    // pega a RA com o maior valor da funcao objetivo 
    nroConsTutelares = 31;
    while(nroConsTutelares <= 40)
    {
        maior = indices[0];
           
        (regioesAdministrativas[ra]->qtdeConsTutelares)++;
            
        energia = KMedias(regioesAdministrativas[maior],regioesAdministrativas[ra]->qtdeConsTutelares);
          
        menor[maior] = energia;
          
        bubbleSort(menor,indices,MAX_RA);
            
        nroConsTutelares++;
    }
        
    //}
    
    //for(cent = 1; cent < regioesAdministrativas[ra]->qtdeConsTutelares
    
    libera_int(indices);
    
    libera_int(ind_menor);
    
    libera_double(menor);
    
    libera_double(soma);
}

void leArquivoPesos(FILE *fp, RAs *regiao)
{
    int i,setor;
    double geolocal,peso;
    char *nome;
    
    /* string de tamanho 100 */
    nome = aloca_string(100);

    /* le o nome dos atributos primeiramente */
    fscanf(fp,"%[^\n]s\n",nome);

    setor=0;
    while(fscanf(fp,"%lf;%lf\n",&geolocal,&peso) > 0)
    {
        for(i = 0; i < 201; i++)
        {
            if(geolocal == regiao->geoCode[i])
            {
                regiao->pesos[i] = peso;
            }
        }
        setor++;
    }
    
    libera_string(nome);
}

void leArquivoCoordenadas(FILE *fp, RAs **regioesAdm)
{
    int setor,ra;
    double geolocal,lati,longi;
    char *nome;
    
    /* string de tamanho 100 */
    nome = aloca_string(100);

    /* le o nome dos atributos primeiramente */
    fscanf(fp,"%[^\n]s",nome);
    
    ra = -1;
    while(fscanf(fp,"%d,%lf,%lf,%lf,%[^\n]s\n",&setor,&lati,&longi,&geolocal,nome) > 0)
    {
        if(setor == 1)
        {
            ra++;
        }

        if(ra == SANTA_MARIA)
        {
            strcpy(regioesAdm[ra]->nome_ra,nome);
            regioesAdm[ra]->lat[setor-1] = lati;
            regioesAdm[ra]->longit[setor-1] = longi;
            regioesAdm[ra]->geoCode[setor-1] = geolocal;
        }
    }
    
    libera_string(nome);
}

void leArquivoDistancias(FILE *fp, RAs *regiao)
{
    int sc1,sc2;
    double dist;
    char *nome_atributos;
    
    /* string de tamanho 100 */
    nome_atributos = aloca_string(100);

    /* le o nome dos atributos primeiramente */
    fscanf(fp,"%[^\n]s\n",nome_atributos);
    
    printf("%s\n",nome_atributos);

    while(fscanf(fp,"%d,%d,%lf\n",&sc1,&sc2,&dist) > 0)
    {
        regiao->dist[sc1-1][sc2-1] = dist;
    }
    
    regiao->qtdeSetores = sc2;    
    libera_string(nome_atributos);
}

/* Funcao principal */
int main(int argc, char *argv[])
{
    int ra,qtdeConsTutelares,marca,nroSetorCensitarios=202;
    
    RAs **regioesAdministrativas;
    
    FILE *file_distancias;
    FILE *file_pesos;
    FILE *file_coordenadas;
    
    if(argc != 4)
    {
        fprintf(stderr,"ERRO: Quantidade incorreta de parametros\n");
        printf("USO: executavel <nome_arquivo_distancias>.csv <nome_arquivo_pesos>.csv <nome_arquivo_coordenadas>.csv\n");
        exit(0);
    }

    regioesAdministrativas = alocaRAs(MAX_RA,nroSetorCensitarios);
    
    file_distancias = fopen(argv[1],"r");
    if(!file_distancias)
    {
        fprintf(stderr,"ERRO: arquivo nao existente\n");
        exit(0);
    }
    
    printf("Leitura de arquivos...\n");
    
    leArquivoDistancias(file_distancias,regioesAdministrativas[SANTA_MARIA]);
    
    printf("Distancias concluidas...\n");
    
    file_coordenadas = fopen(argv[3],"r");
    if(!file_coordenadas)
    {
        fprintf(stderr,"ERRO: arquivo nao existente\n");
        exit(0);
    }
    
    printf("Arquivo de coordenadas dos setores censitarios...\n");
    
    leArquivoCoordenadas(file_coordenadas,regioesAdministrativas);
    
    file_pesos = fopen(argv[2],"r");
    
    if(!file_pesos)
    {
        fprintf(stderr,"ERRO: arquivo nao existente\n");
        exit(0);
    }
    
    printf("Arquivo de pesos dos setores censitarios...\n");
    
    leArquivoPesos(file_pesos,regioesAdministrativas[SANTA_MARIA]);
        
    //posicionaConselhos(regioesAdministrativas[SANTA_MARIA]);
    
    posicionaConselhos(regioesAdministrativas);
    
    //liberaRAs(regioesAdministrativas);
    
    fclose(file_distancias);
    
    fclose(file_pesos);
    
    fclose(file_coordenadas);
    
    return 0;
}
