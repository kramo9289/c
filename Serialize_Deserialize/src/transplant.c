#include "const.h"
#include "transplant.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif


/*
 *@brief Checks the next three chars from stdin and checks to see if they are the magic bytes
 *@details This function calls getchar() three times and uses if statements to check
 *and see if the next 3 characters follow the sequence 0x0c 0x0d 0xed
 *
 *@return 0 in case that the next 3 characters are indeed the wanted sequence,
 *-1 in the case of an error or the next characters were the incorrect sequence
*/
int check_magic_bytes() {
    //place holder for the char that we will get
    unsigned char c;
    c = getchar();
    if(c == EOF) return -1;
    if(c!= MAGIC0) return -1;

    c = getchar();
    if(c == EOF) return -1;
    if(c!= MAGIC1) return -1;

    c = getchar();
    if(c == EOF) return -1;
    if(c!= MAGIC2) return -1;
    return 0;
}

/*
 *@brief Sends the magic bytes to standard output
 *@details This function calls putchar() 3 times on the following
 *sequence: 0x0c 0x0d 0xed
 *
*/
void print_magic_bytes() {
    putchar(MAGIC0);
    putchar(MAGIC1);
    putchar(MAGIC2);
}

/*
 *@brief Sends the depth to stdout
 *@details This function breaks the interger recieved into 4 bytes
 *and puts it into stdout
 *
 *@param the depth to be put into stdout
*/

void print_depth(unsigned int depth){
    //shift the bits and send it out to putchar()
    int temp = (depth >> 24) & 0xff;
    putchar(temp);
    temp = (depth >> 16) & 0xff;
    putchar(temp);
    temp = (depth >> 8) & 0xff;
    putchar(temp);
    temp = (depth) & 0xff;
    putchar(temp);
}

/*
 *@brief Sends the size to stdout
 *@details This function breaks the interger recieved into 8 bytes
 *and puts it into stdout
 *
 *@param the string
 *@return length of the string
*/
int str_length(char* str){
    int counter = 0;
    while(*str != 0){
        counter ++;
        str ++;
    }
    return counter;
}

/*
 *@brief attempts to putchar() on every char recieved
 *@details This function takes in a string and pushchar()
 *each individual char until it reaches a null terminator
 *
 *@param the string to be printed out
*/
void print_str(char* str){
    while(*str != 0){
        putchar(*str);
        str++;
    }
}

void print_size(unsigned long long size){
    //shift the bits and send it out to putchar()
    int temp = (size >> 56) & 0xff;
    putchar(temp);
    temp = (size >> 48) & 0xff;
    putchar(temp);
    temp = (size >> 40) & 0xff;
    putchar(temp);
    temp = (size >> 32) & 0xff;
    putchar(temp);
    temp = (size >> 24) & 0xff;
    putchar(temp);
    temp = (size >> 16) & 0xff;
    putchar(temp);
    temp = (size >> 8) & 0xff;
    putchar(temp);
    temp = (size) & 0xff;
    putchar(temp);
}

/*
 *@brief Return the lenght of char
 *@details This function takes in a char* with null terminator
 *and returns the length not including 0
 *
 *@param the string to get the legnth
*/

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/*
 * A function that returns printable names for the record types, for use in
 * generating debugging printout.
 */
static char *record_type_name(int i) {
    switch(i) {
    case START_OF_TRANSMISSION:
	return "START_OF_TRANSMISSION";
    case END_OF_TRANSMISSION:
	return "END_OF_TRANSMISSION";
    case START_OF_DIRECTORY:
	return "START_OF_DIRECTORY";
    case END_OF_DIRECTORY:
	return "END_OF_DIRECTORY";
    case DIRECTORY_ENTRY:
	return "DIRECTORY_ENTRY";
    case FILE_DATA:
	return "FILE_DATA";
    default:
	return "UNKNOWN";
    }
}

/*
 * @brief  Initialize path_buf to a specified base path.
 * @details  This function copies its null-terminated argument string into
 * path_buf, including its terminating null byte.
 * The function fails if the argument string, including the terminating
 * null byte, is longer than the size of path_buf.  The path_length variable
 * is set to the length of the string in path_buf, not including the terminating
 * null byte.
 *
 * @param  Pathname to be copied into path_buf.
 * @return 0 on success, -1 in case of error
 */
int path_init(char *name) {
    // Create a counter to count how long the path is
    int lengthCounter = 0;

    // Initialize a ptr to the char arr
    char * pathPtr = path_buf;

    for(int i = 0; i<PATH_MAX; i++){
        //gets the next char from the *name parameter
        char currentChar = *(name + i);
        //sets path_buf to the current char
        *pathPtr = currentChar;
        //Goes to the next iteration
        pathPtr ++;
        //Adds to the counter
        lengthCounter ++;
        //If the currentChar is the null terminator then the length is added to path_lenght not
        //including the terminating null byte
        if(currentChar == 0){
            path_length = lengthCounter - 1;
            //returns with success
            return 0;
        }

    }

    //If the program passes PATH_MAX then then it is an error
    return -1;
}

/*
 * @brief  Append an additional component to the end of the pathname in path_buf.
 * @details  This function assumes that path_buf has been initialized to a valid
 * string.  It appends to the existing string the path separator character '/',
 * followed by the string given as argument, including its terminating null byte.
 * The length of the new string, including the terminating null byte, must be
 * no more than the size of path_buf.  The variable path_length is updated to
 * remain consistent with the length of the string in path_buf.
 *
 * @param  The string to be appended to the path in path_buf.  The string must
 * not contain any occurrences of the path separator character '/'.
 * @return 0 in case of success, -1 otherwise.
 */
int path_push(char *name) {
    //go the last char in the path_buf arr to begin adding the new char
    char* pathPtr = path_buf + path_length;
    //adds '/' to the existing path_buf before adding the name
    *pathPtr = '/';
    //goes to the next char
    pathPtr ++;
    //adds one to the length counter
    path_length ++;

    //Create a counter to keep track of the new length of the path
    int lengthCounter = 0;

    //iterate throught until PATH_MAX - path_length which is how much remaining space there is path_buf
    for(int i = 0; i<=PATH_MAX - path_length; i++){
        //gets the next char from the *name parameter
        char currentChar = *(name + i);
        //sets path_buf to the current char
        *pathPtr = currentChar;
        //Goes to the next iteration
        pathPtr ++;
        //Adds to the counter
        lengthCounter ++;
        //If the currentChar is the null terminator then the length is added to path_lenght not
        //including the terminating null byte
        if(currentChar == 0){
            path_length += lengthCounter - 1;
            //returns with success
            return 0;
        }
    }

    //If the program PATH_MAX - path_length then it is greater than the size of path_buf which is an error
    return -1;
}

/*
 * @brief  Remove the last component from the end of the pathname.
 * @details  This function assumes that path_buf contains a non-empty string.
 * It removes the suffix of this string that starts at the last occurrence
 * of the path separator character '/'.  If there is no such occurrence,
 * then the entire string is removed, leaving an empty string in path_buf.
 * The variable path_length is updated to remain consistent with the length
 * of the string in path_buf.  The function fails if path_buf is originally
 * empty, so that there is no path component to be removed.
 *
 * @return 0 in case of success, -1 otherwise.
 */
int path_pop() {
    //The first thing to check is if path_buf is already empty
    if(path_length == 0){
        return -1;
    }
    //Create a counter to keep track of how many chars we go through
    //before we reach a '/' or iterate through the whole path_buf
    int lengthCounter = 0;

    //go the last char in the path_buf arr
    char* pathPtr = path_buf + path_length;

    for(int i= 0; i<path_length; i++){
        //goes through the array backwards until it reaches '/'
        char currentChar = *(pathPtr - i);
        //If we reach a '/' then we break out the loop
        //Adds a null terminator to the end of the path to remove then end from the string
        if(currentChar == '/'){
            *(pathPtr - i) = 0;
            break;
        }
        //Adds to the length counter
        lengthCounter ++;

    }
    //subtracts the length of path_legnth with the number of times we iterated
    //until we reached '/' or the whole path_buf

    path_length -= lengthCounter;

    return 0;
}

/*
 * @brief Deserialize directory contents into an existing directory.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory.  It reads (from the standard input) a sequence of DIRECTORY_ENTRY
 * records bracketed by a START_OF_DIRECTORY and END_OF_DIRECTORY record at the
 * same depth and it recreates the entries, leaving the deserialized files and
 * directories within the directory named by path_buf.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * each of the records processed.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including depth fields in the records read that do not match the
 * expected value, the records to be processed to not being with START_OF_DIRECTORY
 * or end with END_OF_DIRECTORY, or an I/O error occurs either while reading
 * the records from the standard input or in creating deserialized files and
 * directories.
 */
int deserialize_directory(int depth) {
    //Checks to see if there are magic bytes
    if(check_magic_bytes() != 0){
        return -1;
    }

    //gets the next byte which is the transmission type. This will decide what to do with this record
    int transmissionType;
    if((transmissionType = getchar()) == EOF) return -1;

    //get the depth of the current record for error checking
    //create a counter to check and see if all 4 bytes were read
    int counter = 0;
    unsigned int recordDepth = 0;
    while(counter < 4){
        if((recordDepth += getchar()) == EOF) return -1;
        counter ++;
        if(counter != 4)recordDepth = recordDepth << 8;
    }
    //If the counter didnt reach 4 then it is an error
    if(counter != 4){
        return -1;
    }
    //If the depth doesn't match the depth from the parameter then it is an error
    if(recordDepth != depth){
        return -1;
    }

    //The next thing to retreive is the size which are the next 8 bytes
    //reset the counter
    counter = 0;
    //create an int for the size of the record
    unsigned int recordSize = 0;
    while(counter < 8){
        if((recordSize += getchar()) == EOF) return -1;
        counter ++;
        if(counter != 8)recordSize = recordSize << 8;
    }
    //If the counter didnt reach 8 then it is an error
    if(counter != 8){
        return -1;
    }
    //Up until this point the generic information that all records contain has been gathered
    //based on the transmission type the program will determine what to do with the information

    //If transmissionType is 3 then it is the end of a directory so we exit out of this instance
    if(transmissionType == 3){
        if(recordDepth != depth) return -1;
        if(recordSize != 16) return -1;
        //Since it is the end of this directory we pop the name from the path_buffer
        if(path_pop()!= 0) return -1;
        if(depth == 1) return 0;
        if(depth > 1){
            if(deserialize_directory(depth -1) != 0) return -1;
        }
    }

    if(transmissionType == 2){
        if(recordDepth != depth) return -1;
        if(recordSize != 16) return -1;
        if(deserialize_directory(depth) != 0) return -1;
    }

    //If transmissionType is 4 then is is a directory entry and we need to gather more information about the record
    if(transmissionType == 4){
        //Use the record size to determine how many bytes are remaining
        int remainingBytes = recordSize - HEADER_SIZE;
        //Set the counter to 0 to retrieve the mode_t first
        counter = 0;
        //place holder for both mode_t
        mode_t modeT = 0;
        while(counter < 4 ){
            if((modeT += getchar()) == EOF) return -1;
            counter ++;
            if(counter != 4) modeT = modeT << 8;
        }
        //If the program exited the loop without the counter reaching 4 then it is an error
        if(counter != 4){
            return -1;
        }
        //subtract counter from the remainingBytes since we retrieved 4
        remainingBytes -= 4;

        //Set the counter to 0 to retrieve the offT
        counter = 0;
        //place holder for both offT
        off_t offT = 0;
        while(counter < 8){
            if((offT += getchar()) == EOF) return -1;
            counter ++;
            if(counter != 8)offT = offT << 8;
        }
        //If the program exited the loop without the counter reaching 8 then it is an error
        if(counter != 8){
            return -1;
        }
        //subtract counter from the remainingBytes since we retrieved 8
        remainingBytes -= 8;

        //Now we will retrieve the name of the dir/file
        //create a pointer to name_buf
        char* namePtr = name_buf;
        //create a char to retrieve information from the record
        unsigned char c;
        //go through the remaining bytes and add them to name_buf
        while(remainingBytes > 0 ){
            if((c = getchar()) == EOF) return -1;
            //adds the char to the name_buf arr
            *namePtr = c;
            //goes to the next char
            namePtr ++;
            //decrease the counter
            remainingBytes --;
        }
        //Set the end of name_buf to a null terminator
        *(namePtr++) = 0;
        //If the program exited the loop without remaining bytes reaching 0 then it is an error
        if(remainingBytes != 0){
            return -1;
        }

        //The program has retrieved all information about the record and we will determine what to do with it
        //first push the name of the file/dir to the path_buf
        if(path_push(name_buf) != 0) return -1;


        //we will now determine what type of file is this entry is
        //if it returns true then it is a regular file so we will call deserialize_file() with the new path and call
        //deserialize directory with the same depth
        if(S_ISREG(modeT)){
            if(deserialize_file(depth) != 0) return -1;
            //after the file was created we pop the name and go on to the next entry
            path_pop();
            if((deserialize_directory(depth)!= 0)) return -1;
        }
        //It is a directory so we will call on deserialize directory() to resolve the new directory once we create it
        if(S_ISDIR(modeT)){
            //create the new directory with the current path name
            if(mkdir(path_buf, 0700) != 0 ){
                //the directory already exists and if clobber isnt set then it is an error
                if(global_options < 7) return -1;
                //Clober is set and we need to change permission of the dir to be able to access it
                chmod(path_buf, 0700);
            }
            //call deserialzie to resolve the new directory
            if(deserialize_directory(depth + 1)!=0) return -1;
            //once the directory is resolved then change the permission of the directory
            chmod(path_buf, modeT & 0777);
        }
    }

    //the function is done and passed with no errors
    return 0;
}

/*
 * @brief Deserialize the contents of a single file.
 * @details  This function assumes that path_buf contains the name of a file
 * to be deserialized.  The file must not already exist, unless the ``clobber''
 * bit is set in the global_options variable.  It reads (from the standard input)
 * a single FILE_DATA record containing the file content and it recreates the file
 * from the content.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * the FILE_DATA record.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including a depth field in the FILE_DATA record that does not match
 * the expected value, the record read is not a FILE_DATA record, the file to
 * be created already exists, or an I/O error occurs either while reading
 * the FILE_DATA record from the standard input or while re-creating the
 * deserialized file.
 */
int deserialize_file(int depth){
    //Checks to see if there are magic bytes
    if(check_magic_bytes() != 0){
        return -1;
    }
    //gets the next byte which is the transmission type.
    int transmissionType;
    if((transmissionType = getchar()) == EOF) return -1;
    //This has to be 5 or it is an error
    if(transmissionType != 5) return -1;

    //get the depth of the current record for error checking
    //create a counter to check and see if all 4 bytes were read
    int counter = 0;
    unsigned int recordDepth = 0;
    while(counter < 4){
        if((recordDepth += getchar()) == EOF) return -1;
        counter ++;
        if(counter != 4)recordDepth = recordDepth << 8;
    }
    //If the counter didnt reach 4 then it is an error
    if(counter != 4){
        return -1;
    }
    //If the depth doesn't match the depth from the parameter then it is an error
    if(recordDepth != depth){
        return -1;
    }

    //The next thing to retreive is the size which are the next 8 bytes
    //reset the counter
    counter = 0;
    //create an int for the size of the record
    int recordSize = 0;
    while(counter < 8){
        if((recordSize += getchar())== EOF) return -1;
        counter ++;
        if(counter != 8)recordSize = recordSize << 8;
    }
    //If the counter didnt reach 8 then it is an error
    if(counter != 8){
        return -1;
    }

    //Use the record size to determine how many bytes are remaining
    int remainingBytes = recordSize - HEADER_SIZE;

    //Check to see if the file already exists
    struct stat stat_buf;
    if(stat(path_buf, &stat_buf) == 0){
        if(S_ISREG(stat_buf.st_mode)){
            // If clobber isnt set then it is an error
            if(global_options < 7){
                return -1;
            }
        }
    }

    //We will now put the data from the record onto the file. Since clobber was set we can overwrite the file
    FILE *f = fopen(path_buf, "w");
    if(f == NULL) return -1;
    //An empty char which will hold the data
    unsigned char c;
    //go through the remaining bytes and add them to name_buf
    while(remainingBytes > 0){
        if((c = getchar()) == EOF) return -1;
        //Adds the char to the file
        fputc(c, f);
        //decrease the counter
        remainingBytes --;
    }
    //If the program exited the loop without remaining bytes reaching 0 then it is an error
    if(remainingBytes != 0){
        return -1;
    }

    if(fclose(f) != 0) return -1;
    //Close the file as we are done writing to it
    return 0;
}
/*
 * @brief  Serialize the contents of a directory as a sequence of records written
 * to the standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory to be serialized.  It serializes the contents of that directory as a
 * sequence of records that begins with a START_OF_DIRECTORY record, ends with an
 * END_OF_DIRECTORY record, and with the intervening records all of type DIRECTORY_ENTRY.
 *
 * @param depth  The value of the depth field that is expected to occur in the
 * START_OF_DIRECTORY, DIRECTORY_ENTRY, and END_OF_DIRECTORY records processed.
 * Note that this depth pertains only to the "top-level" records in the sequence:
 * DIRECTORY_ENTRY records may be recursively followed by similar sequence of
 * records describing sub-directories at a greater depth.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open files, failure to traverse directories, and I/O errors
 * that occur while reading file content and writing to standard output.
 */
int serialize_directory(int depth) {
    //first start of with the START OF DIRECTORY entry
    print_magic_bytes();
    putchar(START_OF_DIRECTORY);
    print_depth(depth);
    print_size(HEADER_SIZE);
    //open the current directory if its null then return -1
    DIR* dir;

    if((dir = opendir(path_buf)) == NULL ) return -1;
    //now open the entries in directory
    struct dirent *de;
    //while there are still entries in the directory
    while( (de = readdir(dir)) !=  NULL){
        //if de is NULL then we reached the end of the dir
        if(de == NULL) break;
        //get the name of the entry and get the stats of that entry
        char * entryName = de->d_name;
        //check for '.' and '..' directory entry files
        if(*entryName == '.') continue;

        if(path_push(entryName) != 0) return -1;
        struct stat stat_buf;
        stat(path_buf, &stat_buf);

        //print the magic bytes
        print_magic_bytes();
        //print directory entry
        putchar(DIRECTORY_ENTRY);
        //print the depth
        print_depth(depth);
        //print the size of the record
        int strLength = str_length(entryName);
        print_size(strLength + HEADER_SIZE + 12);
        //print the modeT of the entry
        print_depth(stat_buf.st_mode);
        //print the size of the entry
        print_size(stat_buf.st_size);
        //print the entry name of the string
        print_str(entryName);

        //the directory entry has been printed now we will check to see if its a dir
        //or if its a file. If it is neither then it is an error
        if(S_ISDIR(stat_buf.st_mode)){
            if(serialize_directory(depth+1)!= 0) return-1;
        }
        if(S_ISREG(stat_buf.st_mode)){
            if(serialize_file(depth, stat_buf.st_size)!= 0) return -1;
        }
        if(path_pop()!= 0) return -1;
    }
    if(closedir(dir) != 0) return -1;
    //print the end of directory entry
    print_magic_bytes();
    putchar(END_OF_DIRECTORY);
    print_depth(depth);
    print_size(HEADER_SIZE);
    return 0;
}

/*
 * @brief  Serialize the contents of a file as a single record written to the
 * standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * file to be serialized.  It serializes the contents of that file as a single
 * FILE_DATA record emitted to the standard output.
 *
 * @param depth  The value to be used in the depth field of the FILE_DATA record.
 * @param size  The number of bytes of data in the file to be serialized.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open the file, too many or not enough data bytes read
 * from the file, and I/O errors reading the file data or writing to standard output.
 */
int serialize_file(int depth, off_t size) {
    //print out the header of the file data entry
    int offT = size;
    print_magic_bytes();
    putchar(FILE_DATA);
    print_depth(depth);
    print_size(HEADER_SIZE + offT);

    //we will now attempt to open the file
    FILE * f = fopen(path_buf, "r");
    //if f is NULL then there was an error
    if(f == NULL) return -1;

    //set a counter with the file size
    int counter = offT;
    while(counter > 0){
        char c;
        c = fgetc(f);
        //if there is an eof without matching the size then there is an error
        /*if(c == EOF) {
        debug("this is a an incorrect size file %s", path_buf);
        debug("this size is incorrect %d and this is where the coutner stopped %d", offT, counter);
        return -1;}
        */
        putchar(c);
        counter --;
    }

    if(fclose(f)!= 0) return -1;

    return 0;
}

/**
 * @brief Serializes a tree of files and directories, writes
 * serialized data to standard output.
 * @details This function assumes path_buf has been initialized with the pathname
 * of a directory whose contents are to be serialized.  It traverses the tree of
 * files and directories contained in this directory (not including the directory
 * itself) and it emits on the standard output a sequence of bytes from which the
 * tree can be reconstructed.  Options that modify the behavior are obtained from
 * the global_options variable.
 *
 * @return 0 if serialization completes without error, -1 if an error occurs.
 */
int serialize() {
    //if the current path file isnt a directory then it is an error
    struct stat stat_buf;
    stat(path_buf, &stat_buf);
    if(!(S_ISDIR(stat_buf.st_mode))) return -1;

    //The first thing to do is print out the start of transmission
    print_magic_bytes();
    //The transmission type is the start of transmission
    putchar(START_OF_TRANSMISSION);
    //Print the depth of 0
    print_depth(0);
    //Print the size which is a regular record size
    print_size(HEADER_SIZE);

    //Call on serialize directory to resolve the next directories
    if(serialize_directory(1)!= 0) return -1;

    //print the end of transmission
    print_magic_bytes();
    putchar(END_OF_TRANSMISSION);
    print_depth(0);
    print_size(HEADER_SIZE);

    //flush out standard data
    fflush(stdout);

    return 0;
}

/**
 * @brief Reads serialized data from the standard input and reconstructs from it
 * a tree of files and directories.
 * @details  This function assumes path_buf has been initialized with the pathname
 * of a directory into which a tree of files and directories is to be placed.
 * If the directory does not already exist, it is created.  The function then reads
 * from from the standard input a sequence of bytes that represent a serialized tree
 * of files and directories in the format written by serialize() and it reconstructs
 * the tree within the specified directory.  Options that modify the behavior are
 * obtained from the global_options variable.
 *
 * @return 0 if deserialization completes without error, -1 if an error occurs.
 */
int deserialize() {
    //Checks to see if there are magic bytes
    if(check_magic_bytes() == -1){
        return -1;
    }

    //gets the next byte which is the transmission type. This will decide what to do with this record
    int transmissionType;
    if((transmissionType = getchar()) == EOF) return -1;

    //get the depth of the current record for error checking
    //create a counter to check and see if all 4 bytes were read
    int counter = 0;
    unsigned int recordDepth = 0;
    while(counter < 4){
        if((recordDepth += getchar())== EOF) return -1;
        counter ++;
        if(counter != 4) recordDepth = recordDepth << 8;
    }
    //If the counter didnt reach 4 then it is an error
    if(counter != 4){
        return -1;
    }

    //The next thing to retreive is the size which are the next 8 bytes
    //reset the counter
    counter = 0;
    //create an int for the size of the record
    unsigned int recordSize = 0;
    while(counter < 8 ){
        if((recordSize += getchar()) == EOF) return -1;
        counter ++;
        if(counter != 8)recordSize = recordSize << 8;
    }
    //If the counter didnt reach 8 then it is an error
    if(counter != 8){
        return -1;
    }

    //Check to see if its a start record, if everything checks out then call deserialize again
    if(transmissionType == 0){
        if(recordDepth != 0){
            return -1;
        }
        if(recordSize != 16){
            return -1;
        }
        if(deserialize_directory(1)!= 0) return -1;
    }

    //Checks to see if there are magic bytes
    if(check_magic_bytes() == -1){
        return -1;
    }

    //gets the next byte which is the transmission type. This will decide what to do with this record
    transmissionType = 0;
    if((transmissionType = getchar()) == EOF) return -1;

    //get the depth of the current record for error checking
    //create a counter to check and see if all 4 bytes were read
    counter = 0;
    recordDepth = 0;
    while(counter < 4){
        if((recordDepth += getchar())== EOF) return -1;
        counter ++;
        if(counter != 4) recordDepth = recordDepth << 8;
    }
    //If the counter didnt reach 4 then it is an error
    if(counter != 4){
        return -1;
    }

    //The next thing to retreive is the size which are the next 8 bytes
    //reset the counter
    counter = 0;
    //create an int for the size of the record
    recordSize = 0;
    while(counter < 8 ){
        if((recordSize += getchar()) == EOF) return -1;
        counter ++;
        if(counter != 8)recordSize = recordSize << 8;
    }
    //If the counter didnt reach 8 then it is an error
    if(counter != 8){
        return -1;
    }

    //Check to see if its a end record
    if(transmissionType == END_OF_TRANSMISSION){
        if(recordDepth != 0){
            return -1;
        }
        if(recordSize != 16){
            return -1;
        }
        return 0;
    }
    //It is not any of these records so it is an error
    return -1;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    //set global options to 0
    global_options = 0;
    //if argc <= 1 then the user did not specify any commands after bin/transplant
    if(argc <= 1){
        return -1;
    }

    //Retrieves the next 4 arguements after "bin/transplant"
    char* argOne = *(argv + 1);
    char* argTwo = *(argv + 2);
    char* argThree = *(argv + 3);
    char* argFour = *(argv + 4);

    //Checks to see if the first argument is "-h", then sets the appropriate bit to 1
    if(*argOne == '-' && *(argOne + 1) == 'h' && *(argOne + 2) == 0){
        global_options = global_options | 1;
        //Since any of the other inputs do not matter, it returns 0
        return 0;
    //Checks to see if the first arguement is -s then sets the serialize bit to 1
    }else if(*argOne == '-' && *(argOne + 1) == 's' && *(argOne + 2) == 0){
        global_options = global_options | 2;
    //Checks to see if the first arguement is -d then sets the deserialize bit to 1
    }else if(*argOne == '-' && *(argOne + 1) == 'd' && *(argOne + 2) == 0){
        global_options = global_options | 4;
        // The first arguement is not any of the positional arguements so it is an invalid input
    }else{
        return -1;
    }

    //Checks to see if this is the only command
    if(argc == 2){
        //If it doesn't specify a path then it is set to the current directory
        path_init(".");
        return 0;
    }
    //Checks to see if there are too many arguements
    if(argc > 5) {
        return -1;
    }

    //Checks to see if the second arguement is -c
    if(*argTwo == '-' && *(argTwo + 1) == 'c' && *(argTwo + 2) == 0){
        //You cant have -s and -c
        if(*argOne == '-' && *(argOne + 1) == 's' && *(argOne + 2) == 0){
            return -1;
        }

        //set the clobber bit on
        global_options = global_options | 8;

        //Checks to see if this is the only optional input
        if(argc == 3){
            //If it doesn't specify a path then it is set to the current directory
            path_init("./");
            return 0;
        }
        //Checks to see if the third argument is -p with DIR
        if(*argThree == '-' && *(argThree + 1) == 'p' && *(argThree + 2) == 0){
            //If there is no DIR after -p then it is an error.
            if(argc == 4){
            //There is no DIR after '-p' which means there is an error;
                return -1;
            }
            // else it has a DIR so we set the path to DIR
            path_init(argFour);
            return 0;
        }
    }

    //Checks to see if the second arguement is -p
    if(*argTwo == '-' && *(argTwo + 1) == 'p' && *(argTwo + 2) == 0){
        //There is no DIR after '-p' which means there is an error;
        if(argc == 3){
            return -1;
        }
        //Since there is a DIR we set the path to it
        path_init(argThree);
        //Checks to see if this is the only optional input
        if(argc == 4){
            return 0;
        }
        //Checks to see if the fourth arguement is -c
        if(*argFour == '-' && *(argFour + 1) == 'c' && *(argFour + 2) == 0){
            if(*argOne == '-' && *(argOne + 1) == 's' && *(argOne + 2) == 0){
                return -1;
            }
            global_options = global_options | 8;
            return 0;
        }
    }

    //The input is invalid so it returns -1
    return -1;
}
