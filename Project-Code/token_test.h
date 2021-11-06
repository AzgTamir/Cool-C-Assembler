

#ifndef _TOKEN_TEST_H_
#define _TOKEN_TEST_H_

#define MAX_TEST_TOKENS 1000
#define MAX_TEST_LINES 500
#define LINE_LEN 200

struct test_data_t
{
    int _num_tokens;
    char *_asm[MAX_TEST_TOKENS];
    int _num_lines;
    int _lines[MAX_TEST_LINES];

    void (*ctor)(struct test_data_t *);
    void (*dtor)(struct test_data_t *);

    void (*prep)(struct test_data_t *, char *filename);

    int _cur_token_line;
    int _cur_line;
    char **(*get_line)(struct test_data_t *, int *num_tokens);
};

struct test_data_t *new_test_data();
void del_test_data(struct test_data_t *);

#endif /*_TOKEN_TEST_H_*/
