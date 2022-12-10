/***************************************
 * Dynamic allocation like as 'malloc' *
 * Author: Acool4ik                    *
 *                                     *
 *      Params:			       *
 * Thread-safety		       *
 * Min allocated mem: 4Byte            *
 * Max allocated mem: 8Gib             *
 *                                     *
 *      Complexity                     *
 * mysetup: O(1)                       *
 * myteardown: O(1)                    *
 * myalloc: O(n)                       *
 * myfree: O(1)                        *
 *                                     *
 *      Algorithm                      *
 * Border markers                      *
 *                                     *
 ***************************************/

#include <iostream>
#include <pthread.h>

#define EXIT(message) do { \
    fprintf(stderr, "Err: %s:%d, Msg: %s\n", __FUNCTION__, __LINE__, message); \
    exit(1); \
} while(0) \

struct Block {
    uint32_t blocks : 31;
    uint32_t isFree : 1;
};

struct pthread_lock_quard {
    pthread_lock_quard(pthread_mutex_t& mtx) : mtx_(mtx) {
        pthread_mutex_lock(&mtx_); }
    ~pthread_lock_quard() {
        pthread_mutex_unlock(&mtx_); }
private:
    pthread_mutex_t& mtx_;
};

/***********************
 * Global variables    *
 *                     *
 ***********************/
static Block *      HEAD        = nullptr;
static Block *      TAIL        = nullptr;
static const size_t BLOCK_SIZE  = sizeof(Block);
static pthread_mutex_t MTX      = PTHREAD_MUTEX_INITIALIZER;


/***********************
 * Pointer handlers    *
 *                     *
 ***********************/
static Block * align_ceil_ptr(void const * ptr) {
    const int shift     = BLOCK_SIZE / 8;

    uint64_t num        = reinterpret_cast<uint64_t>(ptr);
    uint64_t align_num  = (num >> shift) << shift;
    Block *  block_ptr  = reinterpret_cast<Block *>(align_num);

    return num == align_num ? block_ptr : block_ptr + 1;
}
static Block * align_round_ptr(void const * ptr) {
    Block * align_ptr = align_ceil_ptr(ptr);
    return ptr == align_ptr ? align_ptr : align_ptr - 1;
}
static bool is_align_ptr(void const * ptr) {
    Block * align_ptr = align_ceil_ptr(ptr);
    return ptr == align_ptr;
}
static void print_dump_ptr(void const * ptr, bool is64bitSys = true) {
    const uint64_t numptr = reinterpret_cast<uint64_t>(ptr);
    char buf[BUFSIZ] = {0};
    int32_t top = 0;

    top += sprintf(buf + top, "Print ptr [%p] : [%lu]\n\t", ptr, numptr);
    for (int i = is64bitSys ? 48 : 32; i >= 1; i--)
        top += sprintf(buf + top, "%d", ((numptr & (1 << (i - 1))) ? 1 : 0));

    buf[top++] = '\n';
    buf[top++] = '\0';
    printf("%s", buf);
}


/***********************
 * Fragment handlers   *
 *                     *
 ***********************/
static void fill_fragment(void * ptr, int32_t blocks, bool isFree) {
    if (ptr == nullptr)
        EXIT("Null pointer");
    if (!is_align_ptr(ptr))
        EXIT("Not-align pointer");
    if (blocks < 3)
        EXIT("Count of blocks must be more than 2");

    Block * head_block = (Block *)ptr;
    Block * tail_block = (Block *)ptr + blocks - 1;

    head_block->isFree = isFree;
    head_block->blocks = blocks;

    tail_block->isFree = isFree;
    tail_block->blocks = blocks;
}
static void print_fragment(void * ptr) {
    char buf[BUFSIZ] = {0};
    int32_t top = 0;

    Block * head_block = (Block *)ptr;
    Block * tail_block = (Block *)ptr + head_block->blocks - 1;

    auto add_meta_block = [&buf, &top](Block * block, int32_t idx) {
        top += sprintf(buf + top, "\t[%d]", idx);
        top += sprintf(buf + top, "[%lu]\t", (uint64_t)(block));
        top += sprintf(buf + top, "{blocks: %d, isFree: %d}\n", block->blocks, block->isFree);
    };
    auto add_data_block = [&buf, &top](Block * block, int32_t idx) {
        int32_t * dataptr = reinterpret_cast<int32_t *>(block);

        top += sprintf(buf + top, "\t[%d]", idx);
        top += sprintf(buf + top, "[%lu]\t", (uint64_t)(block));
        top += sprintf(buf + top, "{data: %d}\n", *dataptr);
    };

    top += sprintf(buf + top, "start fragment [%lu]\n", (uint64_t)head_block);
    add_meta_block(head_block, 1);

    for (int32_t idx = 1; idx < head_block->blocks - 1; idx++)
        add_data_block(head_block + idx, idx + 1);

    add_meta_block(tail_block, tail_block->blocks);
    top += sprintf(buf + top, "end fragment [%lu]\n", (uint64_t)head_block);

    buf[top++] = 0;
    printf("%s", buf);
}
static Block * find_free_fragment(int32_t blocks) {
    if (HEAD == nullptr || TAIL == nullptr || HEAD > TAIL)
        EXIT("Don't setup or wrong setup allocator");
    if (blocks <= 0)
        return NULL;

    Block * head = HEAD;
    while (head < TAIL && !(head->blocks >= blocks && head->isFree))
        head = head + head->blocks;

    return head > TAIL ? nullptr : head;
}
static bool is_exist_fragment(void * ptr) {
    if (ptr == nullptr || ptr < HEAD || TAIL < ptr || !is_align_ptr(ptr))
        return false;

    Block * head = HEAD;
    while (head < TAIL) {
        if (head == ptr) return true;
        head = head + head->blocks;
    }

    return false;
}
static void defragment(void * ptr) {
    if (ptr == nullptr)
        EXIT("NULL pointer");
    if (!is_align_ptr(ptr))
        EXIT("Not-align pointer");
    if (HEAD == nullptr || TAIL == nullptr || HEAD > TAIL)
        EXIT("Don't setup or wrong setup allocator");

    Block * head_block = (Block *)ptr;
    Block * tail_block = head_block + head_block->blocks - 1;

    if (!head_block->isFree)
        return;

    Block * left_tail_block = head_block - 1;
    if (left_tail_block > HEAD && left_tail_block->isFree) {
        head_block = left_tail_block - left_tail_block->blocks + 1;

        tail_block->blocks += left_tail_block->blocks;
        head_block->blocks = tail_block->blocks;
    }

    Block * right_head_block = tail_block + 1;
    if (right_head_block < TAIL && right_head_block->isFree) {
        tail_block = right_head_block + right_head_block->blocks - 1;

        head_block->blocks += right_head_block->blocks;
        tail_block->blocks = head_block->blocks;
    }
}


/***********************
 *      API            *
 *                     *
 ***********************/

/**
 * mysetup must be called before myalloc, myfree.
 * Thread-safe but don't signal-safe
 *
 * \param buf  - not marked memory for management
 * \param size - size this memory in bytes
 */
extern "C" void mysetup(void *buf, size_t size) {
    pthread_lock_quard lock(MTX);

    HEAD = align_ceil_ptr(buf);
    TAIL = align_round_ptr((uint8_t *)buf + size) - 1;

    const uint64_t bytes     = ((uint64_t)TAIL + BLOCK_SIZE) - (uint64_t)HEAD;
    const uint64_t max_bytes = ((uint64_t)(1 << 30)) * BLOCK_SIZE * 2; // 8 GiB

    if (bytes > max_bytes) {
        char buf[BUFSIZ] = {0};
        buf[sprintf(buf,
                    "Max buffer size is %lu Byte "
                    "Current is %lu Byte\n",
                    max_bytes, bytes)] = '\0';
        EXIT(buf);
    }

    fill_fragment(HEAD, bytes / BLOCK_SIZE, true);
}
/**
 * Destroy last setup options
 * Thread-safe but don't signal-safe
 */
extern "C" void myteardown(void) {
    pthread_lock_quard lock(MTX);
    HEAD = nullptr;
    TAIL = nullptr;
}
/**
 * It allocate some memory from from gotten buffer.
 * Thread-safe but don't signal-safe
 *
 * \param size - memory in bytes
 * \return if allocation is failed returns nullptr, otherwise pointer to memory >= 'size'
 */
extern "C" void * myalloc(size_t size) {
    if (size == 0)
        return nullptr;

    pthread_lock_quard lock(MTX);

    uint32_t blocks = size / BLOCK_SIZE + ((size % BLOCK_SIZE) ? 1 : 0) + 2;
    Block * head = find_free_fragment(blocks);

    if (head == nullptr)
        return nullptr;

    int32_t rem_blocks = head->blocks - blocks;

    if (blocks <= head->blocks && head->blocks <= blocks + 2) {
        fill_fragment(head, head->blocks, false);
    } else {
        fill_fragment(head, blocks, false);
        fill_fragment(head + blocks, rem_blocks, true);
    }

    return head + 1;
}
/**
 * it return memory area that is associated with pointer into buffer as 'freed'
 * Thread-safe but don't signal-safe
 *
 * \param size - memory in bytes
 * \return if allocation is failed returns nullptr, otherwise pointer to memory >= 'size'
 */
extern "C" void myfree(void * ptr) {
    pthread_lock_quard lock(MTX);

    Block * head = (Block *)ptr - 1;

    if (!is_exist_fragment(head))
        EXIT("Invalid pointer");
    if (head->isFree)
        EXIT("Double free detected");

    fill_fragment(head, head->blocks, true);
    defragment(head);
}
/**
 * print current state of memory (memory dump)
 * Thread-safe but don't signal-safe
 */
extern "C" void print_all_fragments(void) {
    pthread_lock_quard lock(MTX);

    if (HEAD == nullptr || TAIL == nullptr || HEAD > TAIL)
        EXIT("Don't setup or wrong setup allocator");

    Block * head = HEAD;
    while (head < TAIL) {
        print_fragment(head);
        head = head + head->blocks;
    }
}


/***********************
 *      Tests          *
 *                     *
 ***********************/

void * routine(void * arg) {
    (void) arg;

    const int cnt_mall = 10;
    const int max_mall = 20 * (1 << 20); // 20 MiB
    // max malloc size  = 200 MiB

    void * arr_ptrs[cnt_mall];

    for (int i = 0; i < cnt_mall; i++) {
        int bytes = rand() % max_mall;
        arr_ptrs[i] = myalloc(bytes);

        if (arr_ptrs[i] == nullptr)
            EXIT("Bad pointer");

        int * buffer = (int *)arr_ptrs[i];

        for (int j = 0; j < bytes / sizeof(int); j++)
             buffer[j] = j;
        for (int j = 0; j < bytes / sizeof(int); j++)
             if (buffer[j] != j)
                EXIT("Race condition");

        if (i % 2 == 0)
            myfree(arr_ptrs[i]);
    }

    for (int i = 0; i < cnt_mall; i++) {
        if (i % 2 != 0 && arr_ptrs[i] != NULL)
            myfree(arr_ptrs[i]);
    }

    return NULL;
}

int main() {
    srand(time(NULL));

    // test on race condition
    size_t bytes = 2 * ((size_t)1 << 30); // 2 Gib
    const int cnt_th = 10;
    pthread_t pool_th[cnt_th];

    void * memptr = malloc(bytes);
    mysetup(memptr, bytes);

    for (int i = 0; i < cnt_th; i++)
        pthread_create(&pool_th[i], NULL, &routine, NULL);

    for (int i = 0; i < cnt_th; i++)
        pthread_join(pool_th[i], NULL);

    myteardown();
    free(memptr);

    // how use 'print_all_fragments' for debug
    bytes = 50;
    memptr = malloc(bytes);
    mysetup(memptr, bytes);

    void * ptr1 = myalloc(4);
    void * ptr2 = myalloc(8);
    void * ptr3 = myalloc(12);

    // after allocation
    print_all_fragments();

    myfree(ptr1);
    myfree(ptr2);
    myfree(ptr3);

    // after free
    print_all_fragments();

    myteardown();
    free(memptr);
    return 0;
}
