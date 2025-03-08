// Q. what is REPL?
// A. read-execute-print loop

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* buffer; // 사용자 입력 저장하는 문자열 포인터임
    size_t buffer_length; //버퍼 크리
    ssize_t input_length; //실제 입력받은 문자 수 저장 
} InputBuffer;

InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

//mysql 키면 mysql> 앞에 뜨는거 같은거임 입력란 앞에 
void print_prompt() {
    printf("db > ");
}

// ssize_t getline(char **lineptr, size_t *n, FILE *stream);

//동적으로 문자열 인풋 받는 함수. 동적 메모리 할당이라 길이 길어져도 상관없고, 입력 끝에 개행문자 \n 제거도 함 
void read_input(InputBuffer* input_buffer) {
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if(bytes_read <= 0) {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    // ignore trailing newline
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

// InputBuffer 메모리 해제하는 코드 
void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

int main(int argc, char* argv[]) {
    InputBuffer* input_buffer = new_input_buffer();

    while(true) {
        print_prompt(); //db >
        read_input(input_buffer); //문자열(sql) 입력받음 

        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
        }
    }

    return 0;
}
