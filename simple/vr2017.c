#include <stdio.h>
#include <stdlib.h>
#include "headers/convfunc.h"
#include "headers/mathfunc.h"
#include "headers/vr2017.h"

int main(int argc, char **argv) { 

    unsigned char b0;
    unsigned char b1;
    unsigned char b2; 
    unsigned char swizzle_byte; 

    char* swizzle; 
    char* parity_byte; 

    // Trailer counts end at file length. 
    int trailers_count = 1; 
    int parity_checksum;
    int x = 0; 
    int y = 0; 
    int z = 0; 
    int valid_packets = 0;
    int chunk_size;
    int offset = -4; 
    int prev_offset = 0; 

    int x_chunk_average[640];
    int y_chunk_average[640];  
    int z_chunk_average[640]; 
 
    FILE* binary_file; 

    // Check that the correct number of command line arguments were given 
    if (argc > 5) { 
        printf("Error: Too many command line arguments.\n");
        return -1; 
    } else if (argc < 5) { 
        printf("Error: Not enough command line arguments.\n"); 
        return -1; 
    }

    // Check that the path to the file it has been provided refers to a file that actually exists.
    binary_file = fopen(argv[1], "rb"); 
    if (!binary_file) { 
        printf("Error: File %s does not exist!\n", argv[1]);
        return -1; 
    }

    // Check that the format of the delimiter is correct (2 Hex Character) 
    for (int i = argc - 3; i < argc; i++) { 
        if (is_valid_delimiter(argv[i]) == -1) { 
            printf("Error: Argument for delimiter byte %d is not of the correct length\n", i - 2); 
            return -1; 
        } else if (!is_valid_delimiter(argv[i])) { 
            printf("Error: Argument for delimiter byte %d is not a valid hex value\n", i - 2); 
            return -1;
        } else if (is_valid_delimiter(argv[i]) == -2) { 
            printf("Error: Argument for delimiter byte %d does not begin with 0x\n", i - 2); 
            return -1;
        }
    }

    // Print the decimal values of the delimitiers: 
    for (int i = argc - 3; i < argc; i++) {
        printf("Delimiter byte %d is: %d\n", i - 2, hex_to_decimal(argv[i])); 
    }
    
    // Checksum: 
    parity_byte = compute_parity_byte(argc, argv);
    printf("Checksum is: %d\n", binary_to_decimal(parity_byte));

    // Read File: 
    binary_file = fopen(argv[1], "rb");

    // Get File Length 
    fseek(binary_file, 0, SEEK_END); 
    int file_len = ftell(binary_file); 
    fseek(binary_file, 0, SEEK_SET);

    // Find how many trailers there are: 
    for (int i = 0; i < file_len; i++) { 
        if (fgetc(binary_file) == hex_to_decimal(argv[2]) && 
            fgetc(binary_file) == hex_to_decimal(argv[3]) && 
            fgetc(binary_file) == hex_to_decimal(argv[4]) &&
            fgetc(binary_file) == binary_to_decimal(parity_byte))
                trailers_count++; 
        else 
            fseek(binary_file, i, SEEK_SET);  
    } fseek(binary_file, 0, SEEK_SET);

    // Chunks Loop: 
    for (int i = 0; i < trailers_count; i++) {

        // Get offset: 
        // If it's the last trailer, then set the offset to the end of the file: 
        if (i == trailers_count - 1) { 
            offset = file_len; 
        } else { 
            for (int k = offset + 4; k < file_len; k++) { 
                if (fgetc(binary_file) == hex_to_decimal(argv[2]) && 
                    fgetc(binary_file) == hex_to_decimal(argv[3]) && 
                    fgetc(binary_file) == hex_to_decimal(argv[4]) &&
                    fgetc(binary_file) == binary_to_decimal(parity_byte)) {
                        if (k == 0) 
                            offset = k;
                        else  
                            offset = k - 1;
                        break; 
                } else { 
                    fseek(binary_file, k, SEEK_SET);
                }  
            }                
        }
        
        // Calculate chunk size:  
        if ( i == 0) { 
            chunk_size = offset; 
        } else { 
            // Subtract 4 to eliminate the trailer size in calculations 
            chunk_size = abs(offset - prev_offset) - 4;
        }

        // Skip last chunk if empty:   
        if (chunk_size == 0 && i == trailers_count - 1)
            continue; 

        // Print current chunk 
        printf("\nChunk: %d at offset: %d\n", i, offset - chunk_size);
        
        // Check that the chunk is not larger than 640 bytes.  
        if (chunk_size > 640) { 
            printf("Error: Chunk size exceeds the maximum allowable chunk size of 640 bytes.\n\n"); 
            return -1; 
        }

        // Check that the chunk is divisible into packets (5): 
        if (chunk_size % 5 != 0) { 
            printf("Error: Chunk must be divisible by 5 bytes.\n\n"); 
            return -1; 
        }

        // Set file pointer to the beginning of the chunk. 
        fseek(binary_file, offset - chunk_size, SEEK_SET);

        // Save current offset to calculate future chunk sizes. 
        prev_offset = offset; 

        // Packets Loop: 
        for (int j = 0; j < chunk_size / 5; j++) { 
            printf("    Packet: %d\n", j);

            // Read current packet data: 
            b0 = fgetc(binary_file);  
            b1 = fgetc(binary_file); 
            b2 = fgetc(binary_file); 
            swizzle_byte = fgetc(binary_file);
            parity_checksum = fgetc(binary_file);

            // Check that the checksum matches, otherwise skip packet:
            if (parity_checksum != binary_to_decimal(compute_parity_checksum(b0, b1, b2, swizzle_byte))) { 
                printf("        Ignoring packet. Checksum was: %d instead of %d.\n", 
                    binary_to_decimal(compute_parity_checksum(b0, b1, b2, swizzle_byte)), parity_checksum); 
                continue; 
            } 

            // Check valid swizzle value: 
            if (swizzle_byte > 6 || swizzle_byte < 1) { 
                printf("        Ignoring packet. Swizzle byte was: %d but can only be between 1 and 6.\n", swizzle_byte); 
                continue; 
            }

            // Print Results: 
            printf("        Data before swizzle -> B0: %d, B1: %d, B2: %d\n", b0, b1, b2);

            // Switch values according to swizzle byte: 
            swizzle = compute_swizzle(swizzle_byte, &b0, &b1, &b2);
            printf("        Swizzle: %s\n", swizzle);
            printf("        Data after swizzle -> X: %d, Y: %d, Z: %d\n", b0, b1, b2); 

             // Check that XYZ values are reasonable
            if (valid_packets) { 
                if ((abs(x - b0) > 25)) { 
                    printf("        Ignoring packet. X: %d. Previous valid packet's X: %d. %d > 25.\n", b0, x, abs(x - b0)); 
                    continue;
                }

                if ((abs(y - b1) > 25)) { 
                    printf("        Ignoring packet. Y: %d. Previous valid packet's Y: %d. %d > 25.\n", b1, y, abs(y - b1)); 
                    continue;
                }

                if ((abs(z - b2) > 25)) { 
                    printf("        Ignoring packet. Z: %d. Previous valid packet's Z: %d. %d > 25.\n", b2, z, abs(z - b2)); 
                    continue;
                }
            }

            // Save XYZ Values for comparison
            x = b0; 
            y = b1; 
            z = b2; 
            
            // Save Valid Packets for Average Calculation 
            x_chunk_average[valid_packets] = b0; 
            y_chunk_average[valid_packets] = b1; 
            z_chunk_average[valid_packets] = b2; 
            valid_packets++;  
        }
        if (valid_packets)
            printf("    Chunk Average X: %.2f, Average Y: %.2f, Average Z: %.2f\n", average(x_chunk_average, valid_packets), 
                average(y_chunk_average, valid_packets), average(z_chunk_average, valid_packets));
        else 
            printf("    No valid packets were found for this chunk.\n");
        // Next Chunk: 
        fseek(binary_file, offset + 4, SEEK_SET); 
        // Reset Packets Found: 
        valid_packets = 0; 
    }
    printf("\n"); 
    return 0; 
}



