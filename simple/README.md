Name: Leonard Alqaseer
unikey: malq6239 

1. File pointer is at 0 at fopen
2. File pointer is set to the end of the file to get the length of the file using ftell(), and then set back to the start of the file. 
3. Every character is then iterated through multiple times, so the file pointer moves from 1 - 4, then back to 2, then from 2 - 5, then back to 3, etc. 
4. File pointer is set back to 0 after all the characters have been visited. 
5. The same logic is repeated to find the beginning of the first/next trailer. 
6. File pointer is set to the beginning of the chunk which is the absolute difference in the current offset and the previous offset found, subtracted by 4 to not include the trailer bytes in the calculation. 
7. File pointer then moves 1 by 1 to read the each individual byte of the current packet and stores it in a variable. This is repeated for each packet in every chunk. 
8. Pointer should then be at the end of the chunk/ at the beginning of the next trailer, so it's moved by 4 bytes to skip the trailer. 

