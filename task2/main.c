#include <assert.h>
#include "task2.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

ThinList *list;

typedef struct ThinTest {
    int low;
    int high;
} ThinTest;

void *remove_test(void *data) {
    ThinTest *test = (ThinTest *) data;

    for (int i = test->low; i < test->high; i += 3)
        myRemove(list, i);
}

void *create_arr() {
    for (int i = 0; i < 100; i += 3)
        insert(list, i, i);
}

void *insert_test(void *data) {
    ThinTest *test = (ThinTest *) data;
    for (int i = test->low; i < test->high; i += 3)
        insert(list, i, i);
}

void *insert_test_reverse(void *data) {
    ThinTest *test = (ThinTest *) data;
    for (int i = test->high; i > test->low; i -= 3)
        insert(list, i, i);
}

void test() {

    pthread_t create_thread;
    pthread_create(&create_thread, NULL, create_arr, NULL);
    pthread_join(create_thread, NULL);

    pthread_t thread1, thread2, thread3, thread4, thread5;

    ThinTest *thinTest1;
    thinTest1 = (ThinTest *) calloc(1, sizeof(ThinTest));
    thinTest1->low = 0;
    thinTest1->high = 32;
    pthread_create(&thread1, NULL, remove_test, thinTest1);

    ThinTest *thinTest2;
    thinTest2 = (ThinTest *) calloc(1, sizeof(ThinTest));
    thinTest2->low = 32;
    thinTest2->high = 65;
    pthread_create(&thread2, NULL, remove_test, thinTest2);

    ThinTest *thinTest3;
    thinTest3 = (ThinTest *) calloc(1, sizeof(ThinTest));
    thinTest3->low = 65;
    thinTest3->high = 100;
    pthread_create(&thread3, NULL, remove_test, thinTest3);


    ThinTest *thinTest4;
    thinTest4 = (ThinTest *) calloc(1, sizeof(ThinTest));
    thinTest4->low = 1;
    thinTest4->high = 50;
    pthread_create(&thread4, NULL, insert_test, thinTest4);

    ThinTest *thinTest5;
    thinTest5 = (ThinTest *) calloc(1, sizeof(ThinTest));
    thinTest5->low = 50;
    thinTest5->high = 99;
    pthread_create(&thread5, NULL, insert_test_reverse, thinTest5);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    pthread_join(thread4, NULL);
    pthread_join(thread5, NULL);

    {
        for (int i = 1; i < 50; i += 3) {
            FindResult result = find(list, i);
            printf("%d ", i);
            assert(result.exists == 1);
        }

        for (int i = 50; i < 100; i += 3) {
            FindResult result = find(list, i);
            printf("%d ", i);
            assert(result.exists == 1);
        }

        printf("\n");
    }
}

int main() {
    list = init_list();
    insert(list, 1, 2);
    {
        FindResult result = find(list, 1);
        assert(result.exists == 1);
        assert(result.value == 2);
    }

    FindResult result = find(list, 0);
    assert(result.exists == 0);

    insert(list, 0, 1);
    {
        FindResult result = find(list, 0);
        assert(result.exists == 1);
        assert(result.value == 1);
        insert(list, 2, 3);
        {
            FindResult result = find(list, 2);
            assert(result.exists == 1);
            assert(result.value == 3);
        }
        assert(myRemove(list, 1));
    }
    {
        FindResult result = find(list, 1);
        assert(result.exists == 0);
    }
    assert(myRemove(list, 0));
    assert(myRemove(list, 2));
    {
        FindResult result = find(list, 0);
        assert(result.exists == 0);
    }
    {
        FindResult result = find(list, 1);
        assert(result.exists == 0);
    }
    {
        FindResult result = find(list, 2);
        assert(result.exists == 0);
    }

    insert(list, 0, 1);
    insert(list, 1, 2);
    insert(list, 2, 3);
    insert(list, 3, 4);

    ThinList *copyThinList = get_snapshot(list);
    assert(copyThinList->head != NULL);

    for (int i = 0; i < 1000; i++)
        test();
}

