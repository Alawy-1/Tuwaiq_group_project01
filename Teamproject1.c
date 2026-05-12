//  ALI ALNEWAISSR , SUHAIL ALRAHILI , IBRAHIM BOHASSAN


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>



int main(int argc, char **argv){

    //printf("Please enter the number of partitions: ");
    // int partition;
    if (argc != 3){
        printf("usage: %s <size>\n", argv[0]);
        return 1;
    }
    //int partition = (int)scanf("%d", partition);

    int partition = atoi(argv[2]);
    printf("partition = %d\n", partition);


    if(partition <= 0 || partition > 8){
        printf("Invalid number\n");
        printf("0 < Range < 8\nEnd of program\n");
        return 1;
    }
    else{
    printf("Success!\n");
    }

    FILE *fh = fopen(argv[1], "rb");
    if(fh == NULL){
        printf("File does not exist\n");
        free(fh);
        return 1;
    }

    fseek(fh, 0, SEEK_END);
    int file_size = ftell(fh);
    rewind(fh);  // Go back to beginning

    printf("File: %s\n", argv[2]);
    printf("File size: %d bytes\n", file_size);
    printf("Number of blocks: %d\n", partition);

    long partition_size = file_size/partition;
    long remainder = file_size % partition;

    for (int i = 0; i < partition; i++) {
        // Create filename for this block
        char filename[50];
        sprintf(filename, "%s.%d\n",argv[1], i + 1);
        //printf("%s.%d\n", argv[1], i+1);
        
        
        FILE *out_file = fopen(filename, "w+");
        if (out_file == NULL) {
            printf("Error: Could not create %s\n", filename);
            fclose(fh);
            return 1;
        }
        
        // Calculate how many bytes for this block
        long bytes_to_write = partition_size;
        if (i == partition - 1) {
            bytes_to_write += remainder;  // Last block gets remainder
        }
        
        // Read and write block content
        for (long j = 0; j < bytes_to_write; j++) {
            char ch = fgetc(fh);      // Read one byte
            fprintf(out_file, "%c", ch); // Write to output file
        }
        
        fclose(out_file);
        printf("Created %s (%ld bytes)\n", filename, bytes_to_write);
    }

    return 0;
}