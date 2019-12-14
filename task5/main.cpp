#include <iostream>
#include "ThreadPool.h"
#include "assert.h"

using namespace std;

#define THREADS_COUNT 8
#define MAX 99
int arr1[MAX];
int arr2[MAX];

typedef struct Interval {
    int low;
    int high;
} Interval;

pthread_mutex_t m_mutex;
atomic_int current_point;

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

void merge_sort_threads(int a[])
{
    pthread_mutex_lock(&m_mutex);

    int local_current_point = current_point.load();
    int low, high;

    low = local_current_point;
    if(local_current_point < MAX - MAX / THREADS_COUNT - MAX % THREADS_COUNT)
        high = local_current_point + MAX / THREADS_COUNT - 1;
    else
        high = MAX - 1;

    current_point = high + 1;
    cout << low << " " << high << "\n";

    int mid = low + (high - low) / 2;
    if (low < high) {
        merge_sort(a, low, mid);
        merge_sort(a, mid + 1, high);
        merge(a, low, mid, high);
    }

    pthread_mutex_unlock(&m_mutex);
}


int main() {
    pthread_mutex_init(&m_mutex, nullptr);

    ThreadPool pool(THREADS_COUNT);
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
        pool.shutdown();

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


    for(int i = 0; i < MAX; i++){
        if(i % (MAX / THREADS_COUNT) == 0)
            cout << "\n";
        cout << "[" << i << "]" << ":" << arr1[i] << " ";
    }

//    for (int i = 1; i < MAX; i++) {
//        assert(arr1[i - 1] <= arr1[i]);
//    }



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


