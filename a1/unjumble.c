#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define ALPHASIZE 26
#define MAXWORDLEN 50

typedef struct StringArray {
    char** words;
    int size;
} StringArray;

/* Gets the numbers of arguments at the end of the set of arguments
*/
int get_arg_has_dictionary(int argc, char** argv) {
    int hasDictionary = 0;
    int endParams = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (strcmp(argv[i - 1], "-include")) {
                endParams++;
            }
        }
    }
    if (endParams == 2) {
        hasDictionary++;
    }
    return hasDictionary;
}

/* Gets the letter immediately proceeding -include
 */
char get_arg_include_letter(int argc, char** argv) {
    char includeLetter = '\0';
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-include")) {
            includeLetter = argv[i + 1][0];
        }
    }
    if (isalpha(includeLetter) || includeLetter == '\0') {
        return includeLetter;
    } else {
        fprintf(stderr, "Usage: unjumble [-alpha|-len|-longest] " 
                "[-include letter] letters [dictionary]\n");
        exit(1);
    }
}

/* Gets & verifies the "letters" argument
 */
char* get_arg_letters(int argc, char** argv) {
    // Integer stores length of letters argument during loop
    int argLettersLength = 0;
    // Integer stores number of non-alphabetical characters in letters argument
    int argLettersNonAlpha = 0;
    // Determine whether dictionary argument is present
    int hasDictionary = get_arg_has_dictionary(argc, argv);
    // Loop through the characters of the letters argument until \0
    for (int i = 0; argv[argc - (1 + hasDictionary)][i] != '\0'; i++) {
        argLettersLength++;
        if (!isalpha(argv[argc - (1 + hasDictionary)][i]) != 0) {
            argLettersNonAlpha++;
        }
    }
    if (argLettersLength < 3) {
        // If length of letters argument is less than 3
        fprintf(stderr, "unjumble: must supply at least three letters\n");
        exit(3);
    } else if (argLettersNonAlpha > 0) {
        // If there exists a non-alphabetical character
        fprintf(stderr, "unjumble: can only unjumble alphabetic characters\n");
        exit(4);
    } else {
        // If everything is right, return
        return argv[argc - (1 + hasDictionary)];
    }
}

/* Gets the "dictionary" argument
 */
char* get_arg_dictionary(int argc, char** argv) {
    char* fallback = calloc(25, sizeof(char));
    fallback = "/usr/share/dict/words";
    if (get_arg_has_dictionary(argc, argv)) {
        return argv[argc - 1];
    } else {
        return fallback;
    }
}

/* Sets mode based on the arguments inputted by the user
 * return 0 = bad arguments
 * return 1 = -alpha
 * return 2 = -len
 * return 3 = -longest
 * return 4 = default
 */
int get_mode(int argc, char** argv) {
    // Initiates the return code variable
    int returnCode = 4;
    // Has mode/letters/dictionary  already been set (true/false)
    int argSet = 0;
    int endArgs = 0;
    // Loops through arguments inputted by the user
    for (int i = 1; i < argc; i++) {
        // If the argument begins with '-'
        if (argv[i][0] == '-') {
            // Matches argument then checks if argument has already been set
            if (!strcmp(argv[i], "-alpha") && !argSet) {
                // Tells program argument has now been set
                argSet++;
                // Sets the return code
                returnCode = 1;
            } else if (!strcmp(argv[i], "-len") && !argSet) {
                argSet++;
                returnCode = 2;
            } else if (!strcmp(argv[i], "-longest") && !argSet) {
                argSet++;
                returnCode = 3;
            } else if (!strcmp(argv[i], "-include")) {
                // If next argument is exactly 1 character long
                if (argv[i + 1][1] == '\0') {
                    // Tells the forloop to skip analysis of next argument
                    i++;
                } else {
                    // Returns and breaks loop because not exactly 1 letter
                    returnCode = 0;
                    break;
                }
            } else {
                returnCode = 0;
            }
        } else {
            // There exists some unknown option proceeding -
            if (!strcmp(argv[i - 1], "-include")) {
            } else {
                endArgs++;
            }
        }
    }
    // Dictionary optional
    if (endArgs == 1 || endArgs == 2) {
        return returnCode;
    } else {
        return 0;
    }
}

/* Creates and returns a FILE pointer iff the file exists, else exit(2)
 */
FILE* init_dictionary(char* argDictionary) {
    FILE* reader = fopen(argDictionary, "r");
    if (reader == NULL) {
        fprintf(stderr, "unjumble: file \"%s\" can not be opened\n", 
                argDictionary);
        exit(2);
    }
    return reader;
}

/* Returns an int array of the number of occurrences of letters "a" to "z"
 * in given word
 */
int* count_letters(char* letters) {
    int* count = calloc(ALPHASIZE, sizeof(int));
    for (int i = 0; i < strlen(letters); i++) {
        if (isalpha(letters[i])) {
            count[tolower(letters[i]) - 'a']++;
        }
    }
    return count;
}

/* Returns the matched words in lexicographical order (as per dictionary) 
 * this serves as a base function from which other functions may extend.
 */
StringArray* unjumble_default(int argc, char** argv) {
    StringArray* a = malloc(sizeof(StringArray)); // Init StringArray
    FILE* dictionary = init_dictionary(get_arg_dictionary(argc, argv));
    char* letters = get_arg_letters(argc, argv); // Get [letters] from argv
    int* lettersBreakdown = count_letters(letters); // Generate letters sig
    int* wordBreakdown; // Empty int* for dictionary word signature
    char includeLetter = get_arg_include_letter(argc, argv); // -include letter
    int* letterBreakdown = malloc(sizeof(char) * ALPHASIZE);
    if (includeLetter != '\0') {
        char* includeLetterItems = malloc(sizeof(char));
        includeLetterItems[0] = includeLetter;
        letterBreakdown = count_letters(includeLetterItems); 
    }
    char bufferWord[MAXWORDLEN]; // Initializes buffer for word from dictionary
    a->words = malloc(0); // Allocates memory for empty StringArray.words
    a->size = 0; // Sets empty StringArray.size to 0
    while (fgets(bufferWord, MAXWORDLEN, dictionary) != NULL) {
        int nonMatch = 0; // Match if not 0
        int wordLength = 0; 
        for (int i = 0; i < strlen(bufferWord); i++) {
            if (!isalpha(bufferWord[i]) && bufferWord[i] != '\n') {
                nonMatch++;
            }
            wordLength++;
        } 
        if (wordLength < 4) {
            nonMatch++;
        } 
        if (!nonMatch) {
            wordBreakdown = count_letters(bufferWord); // Generate sig for dict
            for (int i = 0; i < ALPHASIZE; i++) {
                if (wordBreakdown[i] > lettersBreakdown[i]) {
                    nonMatch++;
                } 
                if (includeLetter != '\0') { // Filter no letter in dict word
                    if (wordBreakdown[i] < letterBreakdown[i]) {
                        nonMatch++;
                    }
                }
            }
            if (!nonMatch) { // If word can be created from letters
                a->words = realloc(a->words, sizeof(char*) * (a->size + 1));
                a->words[a->size] = strdup(bufferWord);
                a->size++;
            }
        }
    }
    free(letterBreakdown);
    return a;
}

/* Takes char pointer and returns alphabetical difference between them
 */
int string_compare(const void* p1, const void* p2) {
    char* string1 = *(char**)p1;
    char* string2 = *(char**)p2;
    return strcasecmp(string1, string2);
}

/* Quicksort function for -alpha
 */
StringArray* qsort_alphabetical(StringArray* a) {
    qsort(a->words, a->size, sizeof(a->words[0]), string_compare);
    return a;
}

/* Takes char pointers and returns the difference in length between them
 */
int int_compare(const void* p1, const void* p2) {
    char* string1 = *(char**)p1;
    char* string2 = *(char**)p2;

    return (strlen(string2) - strlen(string1));
}

/* Quicksort function for -len
 */
StringArray* qsort_length(StringArray* a) {
    qsort(a->words, a->size, sizeof(a->words[0]), int_compare);
    return a;
}

/* An attempt was made at creating a function that allowed for any element to 
 * be removed from a given StringArray (this does not work but serves as an 
 * important lesson to anyone who attempts to implement this in the future)
 *
StringArray* removeString(StringArray* a, int indexRemove) {
    char** temp = malloc(sizeof(a) - sizeof(char*));
    memcpy(temp, a->words, indexRemove - 1);
    memcpy(temp + (indexRemove * sizeof(char*)), temp + ((indexRemove + 1) *
                sizeof(char*)), a->size-- - indexRemove);
    free(a->words);
    a->words = temp;

    return a;
}
*/

/* Filters the string array such that there exists only words with n length,
 * with n being the greatest length of a word in the initial array.
 */
StringArray* filter_longest(StringArray* a) {
    int currentLongest = 0;
    int currentLength = 0;
    char** newWords = malloc(0);
    int newSize = 0;
    for (int i = 0; i < a->size; i++) {
        currentLength = strlen(a->words[i]);      
        if (currentLength > currentLongest) {
            currentLongest = currentLength;
        }
    }
    for (int i = 0; i < a->size; i++) {
        currentLength = strlen(a->words[i]);
        if (currentLength >= currentLongest) {
            newWords = realloc(newWords, sizeof(char*) * (newSize + 1));
            newWords[newSize] = malloc(sizeof(char) * MAXWORDLEN);
            strcpy(newWords[newSize], a->words[i]);
            newSize++;
        }
    }
    a->words = newWords;
    a->size = newSize;
    return a;
}

/* Hub for managing which other functions to use to unjumble words based on
 * options and arguments specified by user.
 */
StringArray* unjumble(int argc, char** argv) {
    // Gets the mode the user has selected
    int mode = get_mode(argc, argv);
     
    StringArray* a;

    if (mode == 4) {
        a = unjumble_default(argc, argv);
    } else if (mode == 1) {
        a = qsort_alphabetical(unjumble_default(argc, argv));
    } else if (mode == 2) {
        a = qsort_length(qsort_alphabetical(unjumble_default(argc, argv)));
    } else if (mode == 3) {
        a = filter_longest(unjumble_default(argc, argv));
    }
    return a;
}

/* This is the main function, it's automagically executed each time this
 * program is run.
 */
int main(int argc, char** argv) {
    // Checks arguments inputted by the user
    if (!get_mode(argc, argv) || !get_arg_letters(argc, argv)) {
        // The argument check has failed
        fprintf(stderr, "Usage: unjumble [-alpha|-len|-longest]"
                " [-include letter] letters [dictionary]\n");
        exit(1);
    } else {
        // The argument check has passed
        StringArray* a = unjumble(argc, argv);
        // If there exists no element in the set of a->words, 
        // exit with status 10
        if (a->size < 1) {
            exit(10);
        }
        for (int i = 0; i < a->size; i++) { 
            printf("%s", a->words[i]);
        }
    }
    return 0;
}
