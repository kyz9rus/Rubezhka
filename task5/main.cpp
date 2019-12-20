#include <iostream>
#include "ThreadPool.h"
#include <mutex>
#include <list>

using namespace std;

int THREADS_COUNT = 4;
const int MAX = 1000000;
int temp[MAX];
int temp_curr_index = 0;

int current_point;
int thread_counter = 1;
ThreadPool my_pool(THREADS_COUNT);

mutex bounds_mutex;

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

void is_sorted(int arr[]) {
    int errors = 0;
    int ids[10];

    for (int &id : ids)
        id = 0;

    for (int i = 0; i < MAX - 1; i++)
        if (arr[i] > arr[i + 1]) {
            ids[errors] = i;
            errors++;
        }

    if (errors != 0) {
        cout << "\nслучилось что-то не очень хорошее...(" << endl;
        cout << "errors: " << errors << endl << endl;

        for (int i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
            if (ids[i] != 0) {
                for (int j = -10; j < 10; j++) {
                    if (j == i) {
                        cout << ">>> [" << ids[i] + j << "]: " << arr[ids[i] + j] << endl;
                    } else {
                        cout << "[" << ids[i] + j << "]: " << arr[ids[i] + j] << endl;
                    }
                }
                cout << endl;
            }
        }
    }
}

Interval init_bounds() {
    int low, high;

    low = current_point;
    if (thread_counter < THREADS_COUNT)
        high = (thread_counter * (MAX / THREADS_COUNT)) - 1;
    else
        high = MAX - 1;
    current_point = high + 1;

    thread_counter++;
//    cout << low << " " << high << endl;

    Interval *interval;
    interval = (Interval *) calloc(1, sizeof(Interval));
    interval->low = low;
    interval->high = high;

    return *interval;
}

void merge_sort_threads(int a[]) {
    bounds_mutex.lock();
    Interval interval = init_bounds();
    bounds_mutex.unlock();

    int low = interval.low, high = interval.high;

    int mid = low + (high - low) / 2;
    if (low < high) {
        merge_sort(a, low, mid);
        merge_sort(a, mid + 1, high);
        merge(a, low, mid, high);
    }
}

int *init_arr(int arr[]) {
    for (int i = 0; i < MAX; i++) {
        int num = rand() % MAX;
        arr[i] = num;
    }

    return arr;
}

void copy(int arr[], int low, int high) {
    for (int i = low; i <= high; i++) {
        temp[temp_curr_index] = arr[i];
        temp_curr_index++;
    }
}

void print_arr(int arr[]) {
    for (int i = 0; i < MAX; i++)
        cout << arr[i] << " ";
    cout << "\n";
}

// 1. sort all the parts separately
// 2. merge pairs
// 3. merge pairs of pairs

void very_smart_merge_sort(int arr[]) {
    // sort all parts separately
    list<future<void>> sorting;
    for (int i = 0; i < THREADS_COUNT; i++) {
        sorting.push_back(move(my_pool.add_task(merge_sort_threads, arr)));
    }

    // wait for all parts to be sorted
    _List_iterator<future<void>> it;
    for (it = sorting.begin(); it != sorting.end(); ++it)
        it->get();

    if(THREADS_COUNT == 1){
        merge(arr, 0, MAX / 2, MAX - 1);
        return;
    }

    int part = (MAX / THREADS_COUNT) * 2;
    int low = 0,
        high = part - 1,
        mid = (high - low) / 2;

//        print_arr(arr);
    list<future<void>> merging;
    bool wait = false;
    while(true){
        merging.push_back(move(my_pool.add_task(merge, arr, low, mid, high)));
//        merge(arr, low, mid, high);
//        print_arr(arr);


        // если мы дошли до конца круга
        // и шаг меньше половины круга, что значит, что у нас осталось больше 2 частей для merge
        if(high == MAX - 1 && part <= MAX / 2) {
            for (it = merging.begin(); it != merging.end(); ++it)
                if(it->valid())
                    it->get();

            part *= 2;
            low = 0;
            high = part - 1;
            mid = (high - low) / 2;
        }
        // merged
        else if(high == MAX - 1 && part > MAX / 2 - 1) {
            for (it = merging.begin(); it != merging.end(); ++it)
                if(it->valid())
                    it->get();

            break;
        }
        // для нечетного числа частей
        else if(high > MAX - part) {
            for (it = merging.begin(); it != merging.end(); ++it)
                if(it->valid())
                    it->get();

            mid = high;
            high = MAX - 1;
        }
        // сместить интервал merge
        else {
            low = high + 1;
            high += part;
            mid = low + (high - low) / 2;
        }
    }
}

void show_computation_time(int arr[]) {
    auto start = chrono::system_clock::now();

    very_smart_merge_sort(arr);

    auto end = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << "computation time: " << elapsed_seconds.count() << endl;
}

int main() {
    my_pool.init();

    int arr[MAX];
    init_arr(arr);

    show_computation_time(arr);
    is_sorted(arr);

    return 0;
}


