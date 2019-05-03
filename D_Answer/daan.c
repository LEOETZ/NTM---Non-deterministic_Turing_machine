#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#define MAX(a,b) (((a)>(b))?(a):(b))

#define BLANK '_'
#define ALPHABET_SIZE 94

#define LINE_LENGTH 256
#define TAPE_BLOCK_SIZE 48
#define ALLOCATION_BLOCK_SIZE 1024
#define USE_CUSTOM_ALLOCATOR

enum Move{S,L,R};

typedef unsigned int state;



//函数
int symbol_to_int(char input){
    if(input < 33 || input > 126){
        return BLANK;
    }
    return input - 33;
}

struct linked_list_t{
    void* value;
    struct linked_list_t* next;
};




struct linked_list_t* free_linked_list_t = NULL;
struct linked_list_t* block_linked_list_t = NULL;
unsigned int position_linked_list_t = 0;


//函数
struct linked_list_t* allocate_linked_list_t(){
#ifdef USE_CUSTOM_ALLOCATOR
    if(free_linked_list_t != NULL){
        struct linked_list_t* result = free_linked_list_t;
        free_linked_list_t = free_linked_list_t->next;
        return result;
    }
    if(block_linked_list_t == NULL){
        block_linked_list_t = (struct linked_list_t*)malloc(sizeof(struct linked_list_t) * ALLOCATION_BLOCK_SIZE);
    }
    if(position_linked_list_t == ALLOCATION_BLOCK_SIZE){
        position_linked_list_t = 0;
        block_linked_list_t = (struct linked_list_t*)malloc(sizeof(struct linked_list_t) * ALLOCATION_BLOCK_SIZE);
    }
    return &(block_linked_list_t[position_linked_list_t++]);
#else
    return (struct linked_list_t*)malloc(sizeof(struct linked_list_t));
#endif
}



//函数
void deallocate_linked_list_t(struct linked_list_t* ll){
#ifdef USE_CUSTOM_ALLOCATOR
    ll->next = free_linked_list_t;
    free_linked_list_t = ll;
#else
    free(ll);
#endif
}




struct transition_t{
    state from_state;
    state to_state;
    enum Move move;
    char read_symbol;
    char write_symbol;
};



//函数
struct transition_t* create_transition(const char* line){
    char head_movement;
    struct transition_t* transition = (struct transition_t*) malloc(sizeof(struct transition_t));
    int result = sscanf(line, "%u %c %c %c %u",  &transition->from_state, &transition->read_symbol, &transition->write_symbol, &head_movement, &transition->to_state);
    if(result != 5){
        free(transition);
        return NULL;
    }
    switch(head_movement){
    case 'S':
        transition->move = S;
        break;
    case 'L':
        transition->move = L;
        break;
    case 'R':
        transition->move = R;
        break;
    }
    return transition;
}




struct turing_machine_t{
    struct linked_list_t*** transitions;
    bool* acceptors;
    unsigned int max_steps;
    unsigned int num_states;
};



//函数
void delete_turing_machine(struct turing_machine_t* tm){
    assert(tm != NULL);
    state i;
    int j;
    for(i = 0; i < tm->num_states; i++){
        if(tm->transitions[i] != NULL){
            for(j = 0; j < ALPHABET_SIZE; j++){
                struct linked_list_t* tl = tm->transitions[i][j];
                while(tl != NULL){
                    struct linked_list_t* last = tl;
                    tl = tl->next;
                    free(last->value);
                    free(last);
                }
            }
            free(tm->transitions[i]);
        }
    }
    free(tm->transitions);
    free(tm->acceptors);
    free(tm);
}




struct single_tape_t {
    struct single_tape_t* next;
    unsigned int references;
    char symbols[TAPE_BLOCK_SIZE];
};




struct tape_t {
    struct single_tape_t* forward;
    struct single_tape_t* backward;
    unsigned int position;
    char symbols[TAPE_BLOCK_SIZE];
};



struct linked_list_t* free_single_tape_t = NULL;
struct single_tape_t* block_single_tape_t = NULL;
unsigned int position_single_tape_t = 0;


//函数
struct single_tape_t* allocate_single_tape_t(){
#ifdef USE_CUSTOM_ALLOCATOR
    if(free_single_tape_t != NULL){
        struct single_tape_t* result = (struct single_tape_t*)(free_single_tape_t->value);
        struct linked_list_t* tmp = free_single_tape_t;
        free_single_tape_t = free_single_tape_t->next;
        deallocate_linked_list_t(tmp);
        return result;
    }
    if(block_single_tape_t == NULL){
        block_single_tape_t = (struct single_tape_t*)malloc(sizeof(struct single_tape_t) * ALLOCATION_BLOCK_SIZE);
    }
    if(position_single_tape_t == ALLOCATION_BLOCK_SIZE){
        position_single_tape_t = 0;
        block_single_tape_t = (struct single_tape_t*)malloc(sizeof(struct single_tape_t) * ALLOCATION_BLOCK_SIZE);
    }
    return &(block_single_tape_t[position_single_tape_t++]);
#else
    return (struct single_tape_t*)malloc(sizeof(struct single_tape_t));
#endif
}



//函数
void deallocate_single_tape_t(struct single_tape_t* tape){
#ifdef USE_CUSTOM_ALLOCATOR
    struct linked_list_t* ll = allocate_linked_list_t();
    ll->next = free_single_tape_t;
    ll->value = tape;
    free_single_tape_t = ll;
#else
    free(tape);
#endif
}

struct linked_list_t* free_tape_t = NULL;
struct tape_t* block_tape_t = NULL;
unsigned int position_tape_t = 0;


//函数
struct tape_t* allocate_tape_t(){
#ifdef USE_CUSTOM_ALLOCATOR
    if(free_tape_t != NULL){
        struct tape_t* result = (struct tape_t*)(free_tape_t->value);
        struct linked_list_t* tmp = free_tape_t;
        free_tape_t = free_tape_t->next;
        deallocate_linked_list_t(tmp);
        return result;
    }
    if(block_tape_t == NULL){
        block_tape_t = (struct tape_t*)malloc(sizeof(struct tape_t) * ALLOCATION_BLOCK_SIZE);
    }
    if(position_tape_t == ALLOCATION_BLOCK_SIZE){
        position_tape_t = 0;
        block_tape_t = (struct tape_t*)malloc(sizeof(struct tape_t) * ALLOCATION_BLOCK_SIZE);
    }
    return &(block_tape_t[position_tape_t++]);
#else
    return (struct tape_t*)malloc(sizeof(struct tape_t));
#endif
}


//函数
void deallocate_tape_t(struct tape_t* tape){
#ifdef USE_CUSTOM_ALLOCATOR
    struct linked_list_t* ll = allocate_linked_list_t();
    ll->next = free_tape_t;
    ll->value = tape;
    free_tape_t = ll;
#else
    free(tape);
#endif
}



//函数
void delete_single_tape(struct single_tape_t* tape){
    assert(tape != NULL);
    if(tape->references == 0){
        if(tape->next != NULL){
            tape->next->references--;
            delete_single_tape(tape->next);
        }
#ifdef ENABLE_DEBUG
        memset(tape, 0, sizeof(struct single_tape_t));
#endif
        deallocate_single_tape_t(tape);
    }
}



//函数
void delete_tape(struct tape_t* tape){
    assert(tape != NULL);
    if(tape->forward != NULL){
        tape->forward->references--;
        delete_single_tape(tape->forward);
    }
    if(tape->backward != NULL){
        tape->backward->references--;
        delete_single_tape(tape->backward);
    }
#ifdef ENABLE_DEBUG
    memset(tape, 0, sizeof(struct tape_t));
#endif
    deallocate_tape_t(tape);
}



//函数
bool is_valid_character(int input){
    return input != EOF && input != '\n';
}


//函数
struct tape_t* init_tape(FILE* file){	
    int input = fgetc(file);
    if(input == EOF){
        return NULL;
    }

    struct tape_t* result = allocate_tape_t();
    result->forward = NULL;
    result->backward = NULL;
    result->position = 0;
    memset(result->symbols, BLANK, TAPE_BLOCK_SIZE);

    unsigned int pos = 0;
    while(is_valid_character(input) && pos < TAPE_BLOCK_SIZE){
        result->symbols[pos++] = input;
        input = fgetc(file);
    }
    struct single_tape_t* next = NULL;
    if(is_valid_character(input)){
        next = allocate_single_tape_t();
        next->next = NULL;
        next->references = 1;
        memset(next->symbols, BLANK, TAPE_BLOCK_SIZE);
        pos = 0;
        result->forward = next;
    }
    while(is_valid_character(input)){
        if(pos == TAPE_BLOCK_SIZE){
            next->next = allocate_single_tape_t();
            next = next->next;
            next->next = NULL;
            next->references = 1;
            memset(next->symbols, BLANK, TAPE_BLOCK_SIZE);
            pos = 0;
        }
        next->symbols[pos++] = input;
        input = fgetc(file);
    }
    return result;
}

struct tape_t* _write_tape(struct tape_t* tape, char symbol, bool in_place){	
    if(in_place){
        tape->symbols[tape->position] = symbol;
        return tape;
    }
    struct tape_t* result = allocate_tape_t();
    memcpy(result, tape, sizeof(struct tape_t));
    if(tape->backward != NULL){
        tape->backward->references++;
    }
    if(tape->forward != NULL){
        tape->forward->references++;
    }
    result->symbols[tape->position] = symbol;
    return result;
}

struct single_tape_t* to_single(struct tape_t* tape, bool invert, unsigned int position, char new_symbol){
    struct single_tape_t* result = allocate_single_tape_t();
    result->next = invert ? tape->backward : tape->forward;
    if(result->next != NULL){
        result->next->references++;
    }
    memcpy(result->symbols, tape->symbols, TAPE_BLOCK_SIZE);
    result->symbols[position] = new_symbol;
    result->references = 1;
    return result;
}

struct tape_t* perform_move(struct tape_t* tape, enum Move move, char new_symbol, bool in_place){
    if(move == L){
        if(tape->position > 0){
            if(in_place){
                tape->symbols[tape->position] = new_symbol;
                tape->position--;
                return tape;
            }
            struct tape_t* result = _write_tape(tape, new_symbol, in_place);
            result->position--;
            return result;
        }
        struct single_tape_t* backward = tape->backward;
        struct tape_t* result = allocate_tape_t();
        result->position = TAPE_BLOCK_SIZE - 1;

        if(backward == NULL){
            result->backward = NULL;
            memset(result->symbols, BLANK, TAPE_BLOCK_SIZE);
        }
        else{
            result->backward = backward->next;
            if(result->backward != NULL){
                result->backward->references++;
            }
            memcpy(result->symbols, backward->symbols, TAPE_BLOCK_SIZE);
        }

        result->forward = to_single(tape, false, 0, new_symbol);
        if(in_place){
            delete_tape(tape);
        }
        return result;
    }
    if(move == R){
        if(tape->position < TAPE_BLOCK_SIZE-1){
            if(in_place){
                tape->symbols[tape->position] = new_symbol;
                tape->position++;
                return tape;
            }
            struct tape_t* result = _write_tape(tape, new_symbol, in_place);
            result->position++;
            return result;
        }
        struct single_tape_t* forward = tape->forward;
        struct tape_t* result = allocate_tape_t();
        result->position = 0;

        if(forward == NULL){
            result->forward = NULL;
            memset(result->symbols, BLANK, TAPE_BLOCK_SIZE);
        }
        else{
            result->forward = forward->next;
            if(result->forward != NULL){
                result->forward->references++;
            }
            memcpy(result->symbols, forward->symbols, TAPE_BLOCK_SIZE);
        }
        result->backward = to_single(tape, true, TAPE_BLOCK_SIZE - 1, new_symbol);
        if(in_place){
            delete_tape(tape);
        }
        return result;
    }
    assert(move == S);
    return _write_tape(tape, new_symbol, in_place);
}

struct configuration_t{
    state current_state;
    struct tape_t* tape;
    unsigned int steps;
};

struct linked_list_t* free_configuration_t = NULL;
struct configuration_t* block_configuration_t = NULL;
unsigned int position_configuration_t = 0;

struct configuration_t* allocate_configuration_t(){
#ifdef USE_CUSTOM_ALLOCATOR
    if(free_configuration_t != NULL){
        struct configuration_t* result = (struct configuration_t*)(free_configuration_t->value);
        struct linked_list_t* tmp = free_configuration_t;
        free_configuration_t = free_configuration_t->next;
        deallocate_linked_list_t(tmp);
        return result;
    }
    if(block_configuration_t == NULL){
        block_configuration_t = (struct configuration_t*)malloc(sizeof(struct configuration_t) * ALLOCATION_BLOCK_SIZE);
    }
    if(position_configuration_t == ALLOCATION_BLOCK_SIZE){
        position_configuration_t = 0;
        block_configuration_t = (struct configuration_t*)malloc(sizeof(struct configuration_t) * ALLOCATION_BLOCK_SIZE);
    }
    return &(block_configuration_t[position_configuration_t++]);
#else
    return (struct configuration_t*)malloc(sizeof(struct configuration_t));
#endif
}

void delete_configuration(struct configuration_t* configuration){
    assert(configuration != NULL);
    delete_tape(configuration->tape);
#ifdef USE_CUSTOM_ALLOCATOR
    struct linked_list_t* ll = allocate_linked_list_t();
    ll->next = free_configuration_t;
    ll->value = configuration;
    free_configuration_t = ll;
#else
    free(configuration);
#endif
}

struct configuration_t* apply_transition(struct configuration_t* current, const struct transition_t* transition, bool in_place){	
    struct configuration_t* result = in_place ? current : allocate_configuration_t();
    result->current_state = transition->to_state;
    result->tape = perform_move(current->tape, transition->move, transition->write_symbol, in_place);
    result->steps = current->steps + 1;
    return result;
}

struct configuration_t* create_configuration(FILE* input_file){
    struct tape_t* tape = init_tape(input_file);
    if(tape == NULL){
        return NULL;
    }
    struct configuration_t* result = allocate_configuration_t();
    result->current_state = 0;
    result->tape = tape;
    result->steps = 0;
    return result;
}

struct configuration_queue_t{
    struct linked_list_t* head;
    struct linked_list_t* tail;
};

struct configuration_queue_t* init_queue(){
    struct configuration_queue_t* result = (struct configuration_queue_t*)malloc(sizeof(struct configuration_queue_t));
    result->head = NULL;
    result->tail = NULL;
    return result;
}

void enqueue(struct configuration_queue_t* queue, struct configuration_t* value){
    assert(value != NULL);
    struct linked_list_t* next_item = allocate_linked_list_t();
    next_item->value = value;
    next_item->next = NULL;
    if(queue->head == NULL){
        queue->head = next_item;
        queue->tail = next_item;
        return;
    }
    queue->tail->next = next_item;
    queue->tail = next_item;
}

bool is_empty(const struct configuration_queue_t* queue){
    return queue->head == NULL;
}

struct configuration_t* dequeue(struct configuration_queue_t* queue){
    assert(queue->head != NULL);
    struct linked_list_t* first_item = queue->head;
    queue->head = first_item->next;
    struct configuration_t* result = (struct configuration_t*) first_item->value;
    deallocate_linked_list_t(first_item);
    return result;
}

void delete_queue(struct configuration_queue_t* queue){
    while(!is_empty(queue)){
        delete_configuration(dequeue(queue));
    }
    free(queue);
}

struct turing_machine_t* init_turing_machine(unsigned int num_states){
    struct turing_machine_t* result = (struct turing_machine_t*)malloc(sizeof(struct turing_machine_t));
    result->transitions = (struct linked_list_t***)malloc(sizeof(struct linked_list_t**) * num_states);
    memset(result->transitions, 0, sizeof(struct linked_list_t**) * num_states);

    result->acceptors = (bool*)malloc(sizeof(bool) * num_states);
    memset(result->acceptors, 0, sizeof(bool) * num_states);
    result->num_states = num_states;

    return result;
}

struct linked_list_t* add_transition(struct linked_list_t* list, struct transition_t* transition){
    struct linked_list_t* result = (struct linked_list_t*)malloc(sizeof(struct linked_list_t));
    result->value = transition;
    result->next = list;
    return result;
}

char tree_search(const struct turing_machine_t* tm, struct configuration_t* initial_configuration){
    struct configuration_queue_t* queue = init_queue();
    enqueue(queue, initial_configuration);
    bool is_unknown = false;
    while(!is_empty(queue)){
        struct configuration_t* next = dequeue(queue);
        if(tm->acceptors[next->current_state]){
            delete_configuration(next);
            delete_queue(queue);
            return '1';
        }
        if(next->steps == tm->max_steps){
            delete_configuration(next);
            is_unknown = true;
            continue;
        }
        if(tm->transitions[next->current_state] == NULL){
            delete_configuration(next);
            continue;
        }
        struct linked_list_t* transitions = tm->transitions[next->current_state][symbol_to_int(next->tape->symbols[next->tape->position])];
        if(transitions == NULL){
            delete_configuration(next);
        }
        while(transitions != NULL){
            struct configuration_t* adjacent_configuration = apply_transition(next, (struct transition_t*) transitions->value, transitions->next == NULL);
            enqueue(queue, adjacent_configuration);
            transitions = transitions->next;
        }
    }
    delete_queue(queue);
    return is_unknown ? 'U' : '0';
}

struct turing_machine_t* read_file(FILE* file, struct configuration_queue_t* initial_configurations){
    if(file == NULL){
        return NULL;
    }
    char temp[LINE_LENGTH];

    do{
        if(fgets(temp, LINE_LENGTH, file) == NULL){
            return NULL;
        }
    } while(strncmp(temp, "tr", 2));

    struct linked_list_t* transitions = NULL;
    state num_states = 0;
    if(fgets(temp, LINE_LENGTH, file) == NULL){
        return NULL;
    }
    while(strncmp(temp, "acc", 3)){
        struct transition_t* transition = create_transition(temp);
        if(transition != NULL){
            transitions = add_transition(transitions, transition);
            state max_state = MAX(transition->from_state, transition->to_state);
            num_states = MAX(num_states, max_state + 1);
        }
        if(fgets(temp, LINE_LENGTH, file) == NULL){
            while(transitions != NULL){
                struct linked_list_t* tmp = transitions;
                transitions = transitions->next;
                free(tmp);
            }
            return NULL;
        }
    }

    struct turing_machine_t* result = init_turing_machine(num_states);
    struct linked_list_t* i = transitions;
    while(i != NULL){
        struct transition_t* transition = (struct transition_t*) i->value;
        if(result->transitions[transition->from_state] == NULL){
            result->transitions[transition->from_state] = (struct linked_list_t**)malloc(sizeof(struct linked_list_t*) * ALPHABET_SIZE);
            memset(result->transitions[transition->from_state], 0, sizeof(struct linked_list_t*) * ALPHABET_SIZE);
        }
        int symbol = symbol_to_int(transition->read_symbol);
        result->transitions[transition->from_state][symbol] = add_transition(result->transitions[transition->from_state][symbol], transition);
        struct linked_list_t* prev = i;
        i = i->next;
        free(prev);
    }

    if(fgets(temp, LINE_LENGTH, file) == NULL){
        delete_turing_machine(result);
        return NULL;
    }
    while(strncmp(temp, "max", 3)){
        state accepting_state;
        if(sscanf(temp, "%u", &accepting_state) == 1 && accepting_state < num_states){
            result->acceptors[accepting_state] = true;
        }
        if(fgets(temp, LINE_LENGTH, file) == NULL){
            delete_turing_machine(result);
            return NULL;
        }
    }
    if(fgets(temp, LINE_LENGTH, file) == NULL){
        delete_turing_machine(result);
        return NULL;
    }

    bool parsed_max_steps = false;
    while(!parsed_max_steps){
        parsed_max_steps = (sscanf(temp, "%u", &result->max_steps) == 1);
        if(fgets(temp, LINE_LENGTH, file) == NULL){
            delete_turing_machine(result);
            return NULL;
        }
    }

    while(strncmp(temp, "run", 3)){
        if(fgets(temp, LINE_LENGTH, file) == NULL){
            delete_turing_machine(result);
            return NULL;
        }
    }

    struct configuration_t* next_configuration = create_configuration(file);
    while(next_configuration != NULL){
        enqueue(initial_configurations, next_configuration);
        next_configuration = create_configuration(file);
    }
    return result;
}

int main(){
    struct configuration_queue_t* initial_configurations = init_queue();
    struct turing_machine_t* tm = read_file(stdin, initial_configurations);
    if(tm == NULL){
        return EX_DATAERR;
    }
    while(!is_empty(initial_configurations)){
        struct configuration_t* initial_configuration = dequeue(initial_configurations);
        char result = tree_search(tm, initial_configuration);
        printf("%c\n", result);
    }
    delete_queue(initial_configurations);
    delete_turing_machine(tm);
    return EX_OK;
}
