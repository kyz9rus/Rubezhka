#include <iostream>
#include <unistd.h>
#include "ThreadPool.h"
#include "assert.h"

using namespace std;

#define THREADS_COUNT 7
#define MAX 40
int arr1[MAX];
int arr2[MAX];
int temp[MAX];
int temp_curr_index = 0;

typedef struct Interval {
    int low;
    int high;
} Interval;

pthread_mutex_t bounds_mutex;
pthread_mutex_t temp_mutex;

int current_point;
int thread_counter = 1;
ThreadPool pool(THREADS_COUNT);

void merge(int a[], int low, int mid, int high)
{
    int* left = new int[mid - low + 1];
    int* right = new int[high - mid];

    int n1 = mid - low + 1,
        n2 = high - mid, i, j;

    for (i = 0; i < n1; i++)
        left[i] = a[i + low];

    for (i = 0; i < n2; i++)
        right[i] = a[i + mid + 1];

    int k = low;
    i = j = 0;

    while (i < n1 && j < n2) {
        if (left[i] <= right[j])
            a[k++] = left[i++];
        else
            a[k++] = right[j++];
    }

    while (i < n1) {
        a[k++] = left[i++];
    }

    while (j < n2) {
        a[k++] = right[j++];
    }
}

void merge_sort(int a[], int low, int high)
{
    int mid = low + (high - low) / 2;
    if (low < high) {
        merge_sort(a, low, mid);
        merge_sort(a, mid + 1, high);

        merge(a, low, mid, high);
    }
}

Interval init_bounds()
{

    int local_current_point = current_point;//.load();
    int low, high;

    low = current_point;

//    if(local_current_point < MAX - MAX / THREADS_COUNT - MAX % THREADS_COUNT)
//        high = local_current_point + MAX / THREADS_COUNT - 1;
//    else
//        high = MAX - 1;

    if(thread_counter < THREADS_COUNT)
        high = (thread_counter * MAX / THREADS_COUNT) - 1;
    else
        high = MAX - 1;
    cout << low << " " << high << "\n";


    current_point = high + 1;

    Interval *interval;
    interval = (Interval *) calloc(1, sizeof(Interval));
    interval->low = low;
    interval->high = high;



    return *interval;
}

void merge_sort_threads(int a[])
{

    // int local_thread_counter = thread_counter++;


    pthread_mutex_lock(&bounds_mutex);
    Interval interval = init_bounds();
    pthread_mutex_unlock(&bounds_mutex);


    int low = interval.low,
        high = interval.high;


    int mid = low + (high - low) / 2;
    if (low < high) {
        merge_sort(a, low, mid);
        merge_sort(a, mid + 1, high);
        merge(a, low, mid, high);
    }

    // fill in temp array after a thread has sorted smth
    pthread_mutex_lock(&temp_mutex);
    thread_counter++;

    int local_temp_curr_index = temp_curr_index;
    for(int i = low; i <= high; i++){
        temp[temp_curr_index] = a[i];
        temp_curr_index++;
    }


    if(thread_counter >= 2){
        // pool.add_task(merge_sort_threads);
        // первый поток отработал
        // второй поток отработал -> merge(1, 2)
        // третий поток отработал -> merge((1, 2), 3)
        // четвертый поток отработал -> merge(((1, 2), 3), 4)
        // пятый поток отработал -> merge((((1, 2), 3), 4), 5)
        merge(temp, 0, temp_curr_index - MAX / THREADS_COUNT - 1, temp_curr_index - 1);

    }
//    for(int i = 0; i < temp_curr_index; i++)
//        cout << temp[i] << " ";
//    cout << "\n";


    pthread_mutex_unlock(&temp_mutex);


}


int main() {
    pthread_mutex_init(&bounds_mutex, nullptr);
    pthread_mutex_init(&temp_mutex, nullptr);

    pool.init();


    for(int i = 0; i < MAX; i++){
        int num = rand() % MAX;
        arr1[i] = num;
        //arr2[i] = num;
    }

//    for(int i : arr1)
//        cout << i << " ";
//
//    cout << "\n";

    {
        //auto start = std::chrono::system_clock::now();

        for (int i = 0; i < THREADS_COUNT; ++i) {
            pool.add_task(merge_sort_threads, arr1);
        }


//        merge(arr1, 0, (MAX / 2 - 1) / 2, MAX / 2 - 1);
//        merge(arr1, MAX / 2, MAX/2 + (MAX-1-MAX/2)/2, MAX - 1);
//        merge(arr1, 0, (MAX - 1)/2, MAX - 1);
//
//        auto end = std::chrono::system_clock::now();
//        std::chrono::duration<double> elapsed_seconds = end - start;
//        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
//        std::cout << "finished computation at " << std::ctime(&end_time)
//                  << "elapsed time: " << elapsed_seconds.count() << "s\n";

    }

    sleep(0);
    pool.shutdown();

    sleep(0);

//    for(int i = 0; i < MAX; i++){
//        if(i % (MAX / THREADS_COUNT) == 0)
//            cout << "\n";
//        cout << "[" << i << "]" << ":" << temp[i] << " ";
//    }
    cout << "\n";

    for (int i = 0; i < sizeof(temp) / sizeof(temp[0]); i++) {
        // cout << i << "\n";
        if(temp[i] > temp[i + 1])
            cout << "чорт";
        // assert(temp[i] <= temp[i + 1]);
    }



    {
     //   auto start = std::chrono::system_clock::now();

        //merge_sort(arr2, 0, MAX);
//
//        auto end = std::chrono::system_clock::now();
//        std::chrono::duration<double> elapsed_seconds = end - start;
//        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
//        std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

    }

//    for (size_t i = 1; i < sizeof(arr2) / sizeof(arr2[0]); i++) {
//        assert(arr2[i - 1] <= arr2[i]);
//    }

    return 0;
}


