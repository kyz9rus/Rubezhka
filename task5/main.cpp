#include <iostream>
#include "ThreadPool.h"
#include <mutex>

using namespace std;

#define THREADS_COUNT 1
#define MAX 2000000
int temp[MAX];
int temp_curr_index = 0;

int current_point;
int thread_counter = 1;
ThreadPool my_pool(THREADS_COUNT);

mutex bounds_mutex;
mutex temp_mutex;

typedef struct Interval {
    int low;
    int high;
} Interval;

void merge(int a[], int low, int mid, int high) {
    int left[mid - low + 1];
    int right[high - mid];

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

void merge_sort(int a[], int low, int high) {
    int mid = low + (high - low) / 2;
    if (low < high) {
        merge_sort(a, low, mid);
        merge_sort(a, mid + 1, high);

        merge(a, low, mid, high);
    }
}

Interval init_bounds() {
    int low, high;

    low = current_point;
    if(thread_counter < THREADS_COUNT)
        high = (thread_counter * MAX / THREADS_COUNT) - 1;
    else
        high = MAX - 1;
    current_point = high + 1;

    thread_counter++;
    // cout << low << " " << high << endl;

    Interval *interval;
    interval = (Interval *) calloc(1, sizeof(Interval));
    interval->low = low;
    interval->high = high;

    return *interval;
}

void is_sorted(int arr[]) {
    int errors = 0;
    int ids[10];

    for(int & id : ids)
        id = 0;

    for (int i = 0; i < MAX - 1; i++)
        if (arr[i] > arr[i + 1]){
            ids[errors] = i;
            errors++;
        }

    if (errors != 0) {
        cout << "\nслучилось что-то не очень хорошее...(" << endl;
        cout << "errors: " << errors << endl << endl;

        for(int i = 0; i < sizeof(ids) / sizeof(ids[0]); i++){
            if(ids[i] != 0){
                for(int j = -10; j < 10; j++){
                    if(j == i){
                        cout << ">>> [" << ids[i] + j << "]: " << arr[ids[i] + j] << endl;
                    }
                    else{
                        cout << "[" << ids[i] + j << "]: " << arr[ids[i] + j] << endl;
                    }
                }
                cout << endl;
            }
        }
    }
}

void merge_sort_threads(int a[]) {
    bounds_mutex.lock();
    Interval interval = init_bounds();
    bounds_mutex.unlock();

    int low = interval.low,
        high = interval.high;

    int mid = low + (high - low) / 2;
    if (low < high) {
        merge_sort(a, low, mid);
        merge_sort(a, mid + 1, high);
        merge(a, low, mid, high);
    }

    // merge parts
    temp_mutex.lock();
    int added = 0;
    for(int i = low; i <= high; i++){
        temp[temp_curr_index] = a[i];
        temp_curr_index++;
        added++;
    }
    int local_temp_curr_index = temp_curr_index;
    int new_mid = local_temp_curr_index - added;

    merge(temp, 0, new_mid - 1, local_temp_curr_index - 1);
    temp_mutex.unlock();

}

int* init_arr(int arr[]) {
    for(int i = 0; i < MAX; i++){
        int num = rand() % MAX;
        arr[i] = num;
    }

    return arr;
}

void very_smart_merge_sort(int arr[]){
    for (int i = 0; i < THREADS_COUNT; ++i) {
        my_pool.add_task(merge_sort_threads, arr);
    }

    my_pool.shutdown();
}

void show_computation_time(int arr[]){
    auto start = chrono::system_clock::now();

    very_smart_merge_sort(arr);

    auto end = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << "computation time: " << elapsed_seconds.count() << endl;
}

void print_arr(){
    for(int i : temp)
        cout << i << " ";
}

int main() {
    my_pool.init();

    int arr[MAX];
    init_arr(arr);

    show_computation_time(arr);
    is_sorted(temp);

    return 0;
}


