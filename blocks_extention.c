#include <stdio.h>
#include <stdlib.h>
// #include <sys/stat.h>
#include <string.h>
#include <time.h>

int filename_number = 2;
int part_number = 3;
int flag1 = 1;
int flag2 = 3;
int flagR = 4;
int number_of_rotation = 4;
int output_number = 4;
int flagR_merge = 3;

struct PMD{
    char name[50];
    int b_count;
    int fake_block_count;
    long b_size;
    long b_extra;
    unsigned short RN;
    
};

void create_random_block(char *filename, long block_size) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error creating random block: %s\n", filename);
        return;
    }
    
    srand(time(NULL)); 
    
    for (long i = 0; i < block_size; i++) {
        unsigned char random_byte = rand() % 256; 
        fputc(random_byte, file);
    }
    
    fclose(file);
    printf("Created random block: %s (%ld random bytes)\n", filename, block_size);
}

//change if have time
void assign_random_positions(int total_blocks, int fake_count, int *is_random) {
    // Initialize all blocks as normal (0)
    for (int i = 0; i < total_blocks; i++) {
        is_random[i] = 0;
    }
    
    // Randomly place 'random_count' random blocks
    int placed = 0;
    while (placed < fake_count) {
        int position = rand() % total_blocks;
        if (is_random[position] == 0) {
            is_random[position] = 1;
            placed++;
        }
    }
}

int split_metadata(char *filename_t, int partition, int fake_count){

    int total_count = partition + fake_count;
    
    printf("Correct command -p\n");
    // int partition = atoi(partitions);
    printf("partition = %d\n", partition);
    printf("fake blocks = %d\n", fake_count);
    printf("Total number of blocks = %d\n", total_count);

    if (total_count > 16) {
        printf("Error: Total blocks exceed 16 (unsigned short limit)\n");
        return 1;
    }


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
    long file_size = ftell(fh);
    rewind(fh);  // Go back to beginning

    printf("File: %s\n", filename_t);
    printf("File size: %ld bytes\n", file_size);
    printf("Number of blocks: %d\n", partition);

    long partition_size = file_size/partition;
    long remainder = file_size % partition;

    // Randomly assign which blocks are random
    int *is_random = (int*)malloc(total_count * sizeof(int)); //is_random is an array of integers
    assign_random_positions(total_count, fake_count, is_random);

    // Build random_index bitmask (make function)
    unsigned short random_index = 0;
    for(int i = 0; i < total_count; i++){
        if(is_random[i] == 1){
            random_index |= (1 << i);
            printf("Index %d is RANDOM\n", i);
        }
        else{
            printf("Index %d is NORMALL\n", i);
        }
    }

    printf("Random index bitmask: %u (0x%X)\n", random_index, random_index);

    // Save to Struct
    struct PMD metadata;
    strcpy(metadata.name, filename_t);
    metadata.b_count = partition;
    metadata.fake_block_count = fake_count;
    metadata.b_size = partition_size;
    metadata.b_extra = remainder;
    metadata.RN = random_index;


    char metafile[50];
    sprintf(metafile, "%s.pmd", filename_t);
    FILE *mf = fopen(metafile, "wb");
    if(mf != NULL){
        fwrite(&metadata, sizeof(struct PMD), 1, mf);
        fclose(mf);
        printf("Metadata saved to %s\n", metafile);
    }

    //tracking normal blocks
    int normal_counter = 0;

    for (int i = 0; i < total_count; i++) {
    // Create filename for this block
    char filename[50];
    sprintf(filename, "%s.%d",filename_t, i + 1);
    //printf("%s.%d\n", argv[1], i+1);


    if (is_random[i] == 1) {
            // Create random block
            create_random_block(filename, partition_size);
        }
        else {
        
            FILE *out_file = fopen(filename, "wb");
            if (out_file == NULL) {
            printf("Error: Could not create %s\n", filename);
            fclose(fh);
            return 1;
            }

            // Calculate how many bytes for this block
            long bytes_to_write = partition_size;
            if (normal_counter == partition - 1) {
            bytes_to_write += remainder;  // Last Normal block gets remainder
            }

            // Read and write block content
            for (long j = 0; j < bytes_to_write; j++) {
            char ch = fgetc(fh);      // Read one byte
            fprintf(out_file, "%c", ch); // Write to output file
            }

            fclose(out_file);
            printf("Created %s (%ld bytes)\n", filename, bytes_to_write);
            normal_counter++;   
        }
        
    }
    fclose(fh);
        free(is_random);
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

int merge_all_blocks(char *pmd_filename, char *output_filename) {
    // Load metadata
    FILE *fh = fopen(pmd_filename, "rb");
    if (fh == NULL) {
        printf("Error: Cannot open metadata file %s\n", pmd_filename);
        return 1;
    }
    
    struct PMD metadata;
    fread(&metadata, sizeof(struct PMD), 1, fh);
    fclose(fh);
    int total_b_count = metadata.b_count + metadata.fake_block_count;
    printf("=== Merge ALL Blocks Mode (Including Random) ===\n");
    printf("Metadata loaded\n");
    printf("Original file: %s\n", metadata.name);
    printf("normal block count: %d\n", metadata.b_count);
    printf("fake blocks: %d\n", metadata.fake_block_count);
    printf("Total block count: %d\n", total_b_count);
    printf("Normal block size: %ld bytes\n", metadata.b_size);
    printf("Extra bytes (last normal block): %ld\n", metadata.b_extra);
    printf("Random block mask: %u (0x%X)\n", metadata.RN, metadata.RN);
    
    // Find the last normal block index
    int last_normal_index = -1;
    int total_normal_blocks = 0;
    
    for (int i = 0; i < total_b_count; i++) {
        if (((metadata.RN >> i) & 1) == 0) {
            total_normal_blocks++;
            last_normal_index = i;  // Keep updating to get the last one
        }
    }
    
    printf("Total normal blocks: %d\n", total_normal_blocks);
    printf("Last normal block index: %d\n\n", last_normal_index);
    
    // Open output file
    FILE *output = fopen(output_filename, "wb");
    if (output == NULL) {
        printf("Error: Cannot create output file %s\n", output_filename);
        return 1;
    }
    
    int normal_counter = 0;
    long total_bytes_written = 0;
    
    printf("Merging all blocks:\n");
    
    for (int i = 0; i < total_b_count; i++) {
        char block_name[50];
        sprintf(block_name, "%s.%d", metadata.name, i + 1);
        
        int is_random = (metadata.RN >> i) & 1;
        long bytes_to_read;
        
        if (is_random) {
            bytes_to_read = metadata.b_size;
            printf("Block %d: %s [RANDOM]\n", i, block_name);
        } else {
            normal_counter++;
            // Check if this is the last normal block
            if (i == last_normal_index) {
                bytes_to_read = metadata.b_size + metadata.b_extra;
                printf("Block %d: %s [NORMAL - LAST - gets remainder]\n", i, block_name);
            } else {
                bytes_to_read = metadata.b_size;
                printf("Block %d: %s [NORMAL]\n", i, block_name);
            }
        }
        
        FILE *block = fopen(block_name, "rb");
        if (block == NULL) {
            printf("Error: Block file %s not found\n", block_name);
            fclose(output);
            return 1;
        }
        
        // Read and write the block
        for (long j = 0; j < bytes_to_read; j++) {
            char ch = fgetc(block);
            fputc(ch, output);
        }
        
        fclose(block);
        printf("Merged %ld bytes\n", bytes_to_read);
        total_bytes_written += bytes_to_read;
    }
    
    fclose(output);
    
    printf("\n=== Merge All Blocks Complete ===\n");
    printf("Output file: %s\n", output_filename);
    printf("Total bytes written: %ld\n", total_bytes_written);
    
    return 0;
}

int main(int argc, char **argv){

    //printf("Please enter the number of partitions: ");
    // int partition;
        if (argc == 3 || argc == 4 || argc == 5 || argc == 6){
        
        //int partition = (int)scanf("%d", partition);

        if (argc == 4){
            if(strcmp(argv[1], "-p") != 0){
                printf("Wrong command4\nExit Program");
                return 1;
            }
            else{
                int partition = atoi(argv[part_number]);
                int status = split_metadata(argv[filename_number], partition, 0);
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
        if (argc == 5){
            if(strcmp(argv[flag1], "-m") == 0 && strcmp(argv[flagR_merge], "-R") == 0){
                int status = merge_all_blocks(argv[filename_number], argv[output_number]);
                if(status == 1){
                    printf("Error merge all blocks function\n");
                    return 1;
                }
            }
        }
        if(argc == 6){
            // write the rest of code.
            if(strcmp(argv[flag1], "-m") == 0 && strcmp(argv[flag2], "-r") == 0){
                int rotation_num = atoi(argv[number_of_rotation]);
                int status = merge(argv[filename_number], rotation_num, argv[output_number]);
                if(status){
                    printf("Error rotation function");
                    return 1;
                }

            }
            else if (strcmp(argv[flag1], "-p") == 0 && strcmp(argv[flagR], "-R") == 0){
                int partition = atoi(argv[part_number]);
                static int number_of_fake = 5;
                int fake = atoi(argv[number_of_fake]);
                int status = split_metadata(argv[filename_number], partition, fake);
                if (status) {
                    printf("Error split with random function\n");
                    return 1;
                }
            }
            else{
                printf("Wrong command6\nExit Program");
                return 1;
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