#include <unistd.h> /* pentru open(), exit() */
#include <fcntl.h> /* O_RDWR */
#include <errno.h> /* perror() */
#include <stdlib.h>
#include <stdio.h>

void fatal(char * mesaj_eroare)
{
    perror(mesaj_eroare);
    exit(0);
}
 
int main(void)
{
    int miner_sursa, miner_destinatie;
    int copiat;
    char buf[1024];
 
    miner_sursa = open("sursa", O_RDONLY);
    miner_destinatie = open("destinatie", O_WRONLY | O_CREAT, 0644);
 
    if (miner_sursa < 0 || miner_destinatie < 0)
        fatal("Nu pot deschide un fisier");
    
    lseek(miner_sursa, 0, SEEK_END);
    lseek(miner_destinatie, 0, SEEK_SET);

    int k = 0;
    char aux[1];
    while (lseek(miner_sursa, -1, SEEK_CUR) > 0) {
        read(miner_sursa, aux, 1);
        if(aux[0] == '\n'){
            // write current line
            read(miner_sursa, buf, k);
            write(miner_destinatie, buf, k);   
            write(miner_destinatie, "\n", 1);
            lseek(miner_sursa, -k - 2, SEEK_CUR);
            k = 0;
        }
        else{
            // count current line number of characters
            lseek(miner_sursa, -1, SEEK_CUR);
            k++;
        }
    }
    // write first line
    lseek(miner_sursa, 0, SEEK_SET);
    read(miner_sursa, buf, k);
    write(miner_destinatie, buf, k);   
    
    close(miner_sursa);
    close(miner_destinatie);
    return 0;
}