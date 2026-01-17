#include <stdio.h>


// Bubble sort
void bubble_Sort(int arr[],int n)
{
    int i;
    int temp;
    for (i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

//Select sort
void Select_Sort(int arr[], int n)
{
    int i, j, min_idx;
    for (i = 0; i < n; i++)
    {
        min_idx = i;
        for (j = 0; j < n; j++)
        {
            if (arr[j] < arr[min_idx])
            {
                int temp = arr[min_idx];
                arr[min_idx] = arr[j];
                arr[j] = temp;
            }
        }
    }
}

//insert sort
void Insert_Sort(int arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++)
    {
        key = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j] < key)
        {
            arr[j+1] = arr[j];
            j--;
        }
        arr[j+1] = key;
    }
}

//quick sort
void Quick_Sort(int arr[], int low, int high)
{   
    int first = low;
    int last = high;
    int key = arr[low];

    if (low >= high)
    {
        return;
    }

    while(first < last)
    {
        while (arr[last] >= key && first < last)
        {
            last--;
        }
        arr[first] = arr[last];

        while (arr[first] <= key && first < last)    
        {
            first++;
        }
        arr[last] = arr[first];
    }

    arr[first] = key;
    Quick_Sort(arr, low, first - 1);
    Quick_Sort(arr, first + 1, high);
}

// printf
void print_arr(int arr[], int n)
{
    
    for (int i = 0; i < n; i++)
    {
        printf("%d ", arr[i]);
    }
}

int main()
{
    int arr[10] = {4, 5, 2, 9, 7, 8, 6, 3, 2, 1};
    int n = sizeof(arr) / sizeof(arr[0]);
    //bubble_Sort(arr, n);
    //Select_Sort(arr, n);
    Insert_Sort(arr, n);
    //Quick_Sort(arr, 0, 9);

    print_arr(arr, n);
    return 0;
}