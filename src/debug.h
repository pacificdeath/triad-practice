#ifdef DEBUG
    #include <stdio.h>
    __attribute__((unused))
    static void are_you_a_horrible_person(bool condition, char *condition_string, char *file_name, int line_number) {
        if (!(condition)) {
            printf("You are a horrible person\n");
            printf(" -> ");
            printf("%s", file_name);
            printf(":");
            printf("%i\n", line_number);
            printf(" -> (");
            printf("%s", condition_string);
            printf(")\n");
            exit(1);
        }
    }
    #define ASSERT(condition) do { are_you_a_horrible_person(condition, #condition, __FILE__, __LINE__); } while (0)
#else
    #define ASSERT(condition)
#endif

