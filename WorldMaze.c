/*
Autores:
    Alejandro Budiño Regueira   
    Manuel Couto Pintos
*/
#include <sys/stat.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

FILE * get_problem_head(char *fileName, int *nBlocks)
{
    FILE *fp;
    char buffer[100];
    
    printf("* %s -----------------------------------------------------\n\n",fileName);
    
    fp = fopen(fileName,"r");
    if (fp != NULL) {
        (*nBlocks) = atoi(fgets(buffer, sizeof(buffer), fp));
        printf("* BLOCKS: \n\t%d \n", (*nBlocks));
    }
    return fp;
}

void get_problem_data(FILE *fp, int **inicial, int **final, int nBlocks)
{
    int i = 0;
    int j = 0;
    char * token = NULL;
    char buffer[250];

    if (fp != NULL) {
        printf("* INITIAL CONFIGURATION :\n");
        
        while (fgets(buffer, sizeof(buffer), fp)[0] != '\n') {
            printf(" \t");
            token = strtok(buffer, " ");
            while( token != NULL && j < nBlocks) {
                inicial[i][j] = atoi(token) ;
                printf("%d ", inicial[i][j]);
                token = strtok(NULL, " ");
                j++;
            }
            printf("\n");
            i++;
            j=0;
        }

        printf("* FINAL CONFIGURATION :\n");
        
        i = 0;
        j = 0;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf(" \t");
            token = strtok(buffer, " ");
            while( token != NULL && j < nBlocks) {
                final[i][j] = atoi(token);
                printf("%d ", final[i][j]);
                token = strtok(NULL, " ");
                j++;
            }
            printf("\n");
            i++;
            j=0;
        }
        
    }
    printf("\n");

}

FILE *encode_problem(char *filePath, int **init, int **end, int nBlocks)
{
    int j = 0;
    int s = 0;
    int prioridad = 0;

    FILE* fp = fopen(filePath,"w");

    fprintf(fp, "#const n=%d. \n", nBlocks);
    fprintf(fp, "#program initial. \n");
    fprintf(fp, "over(floor, free). \n");

    for(int i=0; i<nBlocks; i++){

        if(init[i][0]!=0){
            fprintf(fp,"over(floor,box(%d)).\n", init[i][0]);
            s = init[i][0];
            fprintf(fp,"in(%d,box(%d)).\n", s, init[i][0]);

            j++;
            while(init[i][j]!=0){
                fprintf(fp,"over(box(%d), box(%d)).\n", init[i][j-1], init[i][j]);
                fprintf(fp,"in(%d,box(%d)).\n", s, init[i][j]);
                j++;
            }
        }
        j=0;
    }
    fprintf(fp, "\n");

    s=0;
    j=0;
    for(int i=0; i<nBlocks; i++){
  
        if(end[i][0]!=0){
            fprintf(fp,"fover(floor,box(%d)).\n", end[i][0]);
            s = end[i][0];
            fprintf(fp,"fin(%d,box(%d)).\n", s, end[i][0]);
            fprintf(fp,"prioridad(%d,box(%d)).\n", prioridad, end[i][0]);
            j++;
            prioridad++;
            while(j<nBlocks && end[i][j]!=0){
                fprintf(fp,"fover(box(%d), box(%d)).\n", end[i][j-1], end[i][j]);
                fprintf(fp,"fin(%d,box(%d)).\n", s, end[i][j]);
                fprintf(fp,"prioridad(%d,box(%d)).\n", prioridad, end[i][j]);
                j++;
                prioridad++;
            }
        }
        prioridad=0;
        j=0;
    }
    fprintf(fp, "\n");

    fprintf(fp, "#program final. \n");
    fprintf(fp, "goal :- over(floor,free)");
    s=0;
    j=0;
    for(int i=0; i<nBlocks; i++){
        
        if(end[i][0]!=0){
            fprintf(fp, ", ");
            fprintf(fp,"over(floor,box(%d)), ", end[i][0]);

            j++;
            while(j<nBlocks && end[i][j]!=0){
                fprintf(fp,"over(box(%d), box(%d)), ", end[i][j-1], end[i][j]);
                j++;
            }

            if((j-1)<nBlocks && end[i][j-1]!=0)
                fprintf(fp, "over(box(%d), free) ", end[i][j-1]);
        }
        j=0;
    }
    fprintf(fp, ".\n");
    fprintf(fp, ":- not goal.\n");
    s=0;
    j=0;
    /*
    for(int i=0; i<nBlocks; i++){
  
        if(end[i][0]!=0){
            s=end[i][0];
            fprintf(fp,"in(%d,box(%d)).\n", s, end[i][0]);

            j++;
            while(j<nBlocks && end[i][j]!=0){
                fprintf(fp,"in(%d,box(%d)).\n", s, end[i][j]);
                j++;
            }
        }
        j=0;
    }
    */
    return fp;
}

char *ltrim(char *str, const char *seps)
{
    size_t totrim;
    if (seps == NULL) {
        seps = "\t\n\v\f\r ";
    }
    totrim = strspn(str, seps);
    if (totrim > 0) {
        size_t len = strlen(str);
        if (totrim == len) {
            str[0] = '\0';
        }
        else {
            memmove(str, str + totrim, len + 1 - totrim);
        }
    }
    return str;
}

void decode_result(char *filePath)
{
    FILE *fp;
    FILE *fp1;

    char buffer[250];
    char * token = NULL;

    fp = fopen(filePath,"r");
    fp1 = fopen("solution.txt", "w");

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        token = ltrim(buffer," ");
        if(token[0]=='m' && token[1]=='('){
            printf("\t");
            fprintf(fp1, "\t");
            printf("%s",buffer);
            fprintf(fp1, buffer);
        }
    }

    fclose(fp);
    fclose(fp1);
}

void worldMaze(char *filePath)
{
    //char **matrix;
    FILE *fp;
    FILE *file;
    int nBlocks;
    int **inicial;
    int **final;

    struct timespec ts1, ts2;
    
    fp = get_problem_head(filePath, &nBlocks);

    inicial = (int**) malloc(nBlocks * sizeof(int *));
    final = (int**) malloc(nBlocks * sizeof(int *));

    for(int i=0; i<nBlocks; i++){
        inicial[i] = malloc(nBlocks * sizeof(int));
        final[i]= malloc(nBlocks * sizeof(int));
    }

    for(int i=0; i<nBlocks; i++)
        for(int j=0; j<nBlocks; j++){
            inicial[i][j] = 0;
            final[i][j] = 0;
        }

    get_problem_data(fp, inicial, final, nBlocks);
    fclose(fp);
    
    file =  encode_problem("config.txt", inicial, final, nBlocks);
    
    fclose(file);

    clock_gettime( CLOCK_REALTIME, &ts1 );
    system("telingo --verbose=0 config.txt WorldMaze2.txt > result.txt");
    clock_gettime( CLOCK_REALTIME, &ts2 );

    printf(" * Tiempo de ejecucion(clasp): %f\n", 
        (float) ( 1.0*(1.0*ts2.tv_nsec - ts1.tv_nsec*1.0)*1e-9 + 1.0*ts2.tv_sec - 1.0*ts1.tv_sec )/60 );


    decode_result("result.txt");

    for(int i=0; i<nBlocks; i++){
        if(inicial[i]!=NULL)
            free(inicial[i]);
        if(final[i]!=NULL)
            free(final[i]);
    }

    if(inicial != NULL)
        free(inicial);
    
    if(final != NULL)
        free(final);
}


int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int main(int argc, char *argv[]) 
{
    int i;
    DIR *dir;
    struct dirent *ent;
    char filePath[100];

    for(i=1; i<argc; i++){
        if(is_regular_file(argv[i])){
            worldMaze(argv[i]);
        } else {
            dir = opendir(argv[i]);

            if(dir == NULL)
                perror("No puedo abrir el directorio");

            while ((ent = readdir (dir)) != NULL){
                if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) ){
                    strcpy(filePath, argv[i]);
                    strcat(filePath, "/");
                    strcat(filePath, ent->d_name);
                    worldMaze(filePath);
                }
            }
        }
    }

    return 0;
}