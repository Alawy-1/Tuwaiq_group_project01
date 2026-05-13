

#include <stdio.h>
#include <stdlib.h>
// #include <sys/stat.h>
#include <string.h>

int filename_number = 2;
int part_number = 3;
int flag1 = 1;
int flag2 = 3;
int number_of_rotation = 4;
int output_number = 5;

struct PMD{

    char name[50];
    int b_count;
    long b_size;
    long b_extra;
    
};

int split_metadata(char *filename_t, int partition){

    printf("Correct command4\n");
    // int partition = atoi(partitions);
    printf("partition = %d\n", partition);


    if(partition <= 0 || partition > 8){
    printf("Invalid number\n");
    printf("0 < Range < 8\nEnd of program\n");
    return 1;
    }
    else{
    printf("Success!\n");
    }

    FILE *fh = fopen(filename_t, "rb");
    if(fh == NULL){
    printf("File does not exist\n");
    // free(fh);
    return 1;
    }

    fseek(fh, 0, SEEK_END);
    int file_size = ftell(fh);
    rewind(fh);  // Go back to beginning

    printf("File: %s\n", filename_t);
    printf("File size: %d bytes\n", file_size);
    printf("Number of blocks: %d\n", partition);

    long partition_size = file_size/partition;
    long remainder = file_size % partition;

    // Save to Struct
    struct PMD metadata;
    strcpy(metadata.name, filename_t);
    metadata.b_count = partition;
    metadata.b_size = partition_size;
    metadata.b_extra = remainder;

    char metafile[50];
    sprintf(metafile, "%s.pmd", filename_t);
    FILE *mf = fopen(metafile, "wb");
    if(mf != NULL){
        fwrite(&metadata, sizeof(struct PMD), 1, mf);
        fclose(mf);
        printf("Metadata saved to %s\n", metafile);
    }

    for (int i = 0; i < partition; i++) {
    // Create filename for this block
    char filename[50];
    sprintf(filename, "%s.%d",filename_t, i + 1);
    //printf("%s.%d\n", argv[1], i+1);


    FILE *out_file = fopen(filename, "wb");
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
    fclose(fh);
    return 0;
}

int merge(char *filename, int rotation, char *output_file){
    FILE *fh = fopen(filename, "rb");
    if (fh == NULL){
        printf("file not found\n");
        //fclose(fh);
        return 1;
    }
    struct PMD metadata;
    fread(&metadata, sizeof(struct PMD), 1, fh);
    fclose(fh);



    printf("Metadata file loaded\n");
    printf("filename: %s\n", metadata.name);
    printf("Number of blocks: %d\n", metadata.b_count);
    printf("block size: %ld\n", metadata.b_size);
    printf("extra bytes: %ld\n", metadata.b_extra);

    //handle rotation

    // e_r = effective rotation
    
    if (rotation != 0){
        if(rotation > 0 && rotation < metadata.b_count){
            int e_r;
            for(int i = 0; i < metadata.b_count; i++){
                e_r = (rotation + i) % metadata.b_count;// 2 3 0 1
                printf("%d ", e_r);
            }
            printf("\n");
        }
    }

    // Get final output file name
/*
    char final_output[100];
    if (output_file == NULL) {
        // No output filename provided - use default (.restored)
        sprintf(final_output, "%s.restored", metadata.name);
    } else {
        // Use provided output filename
        strcpy(final_output, output_file);
    }

    char output_name[50];
    sprintf(output_name, "%s.restored",metadata.name);
    FILE *output = fopen(output_name, "wb");
    printf("Saving restored file as: %s\n", output_name); //check
    if(output == NULL){
        printf("Error output\n");
        return 1;
    }
*/
    // Determine output filename (ONE PLACE - CORRECT)
    char final_output[100];
    if (output_file == NULL) {
        sprintf(final_output, "%s.restored", metadata.name);
    } else {
        strcpy(final_output, output_file);
    }

    // ONLY ONE output file pointer - USE final_output
    FILE *output = fopen(final_output, "wb");
    printf("Saving to: %s\n", final_output);
    
    if(output == NULL){
        printf("Error creating output file\n");
        return 1;
    }

    // hand merge with rotationn
    for(int i = 0; i < metadata.b_count; i++){

        int block_index;
        if (rotation == 0){
            block_index = i;
        }else{
            block_index = (rotation + i) % metadata.b_count;
        }
        char block_name[50];
        sprintf(block_name, "%s.%d", metadata.name, block_index +1);

        printf("Reading block %d: %s\n", block_index, block_name);

        //printf("reading block %s\n", block_name);
        
        FILE *block = fopen(block_name, "rb");
        if(block == NULL){
            printf("File %s not found\n", block_name);
            //fclose(fm);
            return 1;
        }

        long bytes_to_write = metadata.b_size;
        if(i == metadata.b_count - 1){
            bytes_to_write = bytes_to_write + metadata.b_extra;
        }

        for(int i = 0; i < bytes_to_write; i++){
            char ch = fgetc(block);
            fputc(ch, output);
        }

        fclose(block);
    
        if (rotation == 0) {
            printf("  Merged %ld bytes from %s\n", bytes_to_write, block_name);
        } else {
            printf("  Rotated %ld bytes from block %d to position %d\n", bytes_to_write, block_index, i);
        }
    }

        if (rotation == 0) {
        printf("\nMerge Completed!\nOutput file: %s\n", final_output);
        } else {
            printf("\nRotation Completed!\nOutput file: %s\n", final_output);
        }

        // printf("Merged %ld bytes from %s\n", bytes_to_write, block_name);
        
        // FILE *output = fopen()

        // fread(&metadata, sizeof(struct PMD), 1, fm);

        fclose(output);
        printf("Merge Completed!\nOutput file: %s\n", metadata.name);
        return 0;
}

int main(int argc, char **argv){

    //printf("Please enter the number of partitions: ");
    // int partition;
        if (argc == 3 || argc == 4 || argc == 6){
        
        //int partition = (int)scanf("%d", partition);
        

        if (argc == 4){
            if(strcmp(argv[1], "-p") != 0){
                printf("Wrong command4\nExit Program");
                return 1;
            }
            else{
                int partition = atoi(argv[part_number]);
                int status = split_metadata(argv[filename_number], partition);
                if (status){
                    printf("Error split function\n");
                    return 1;
                }
            }
        }
        if(argc == 3){
            if(strcmp(argv[1], "-m") != 0){
                printf("Wrong command3\nExit Program");
                return 1;
            }
            else{
                //printf("Correct command3\n");
                int status = merge(argv[filename_number], 0, NULL);
                if(status){
                    printf("Error merge function\n");
                    return 1;
                }

            }
        }
        if(argc == 6){
            // write the rest of code.
            if(strcmp(argv[flag1], "-m") != 0 && strcmp(argv[flag2], "-r") != 0){
                printf("Wrong command7\nExit Program");
                return 1;
            }
            else{
                int rotation_num = atoi(argv[number_of_rotation]);
                int status = merge(argv[filename_number], rotation_num, argv[output_number]);
                if(status){
                    printf("Error rotation function");
                    return 1;
                }
            }
        }

        printf("End of program\n");
        return 0;
    }
    else{
        printf("usage: %s <size> did not run\n", argv[0]);
            return 1;
    }
}