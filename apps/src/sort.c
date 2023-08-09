#include <syscall.h>
#include <string.h>
#include <arg.h>
void quickSort(int *number, int first, int last) {
    int i, j, pivot;
    int temp;
    if (first<last) {
        pivot = first;
        i = first;
        j = last;
        while (i<j) {
            while (number[i] <= number[pivot] && i<last)
                i++;
            while (number[j]>number[pivot])
                j--;
            if (i<j) {
                temp = number[i];
                number[i] = number[j];
                number[j] = temp;
            }
        }
        temp = number[pivot];
        number[pivot] = number[j];
        number[j] = temp;
        quickSort(number, first, j - 1);
        quickSort(number, j + 1, last);
    }
}
int main(int argc,char **argv) // entry
{
    if(argc < 2)
    {
        printf("You no input the array.\n");
    }
    int *array = malloc(argc * sizeof(int)); // array of argc
    
    for (int i = 1; i < argc; i++)
    {
        array[i-1] = atoi(argv[i]);
    }
    //进行排序
    quickSort(array, 0, argc-1);
    //打印排序结果
	char *buf = malloc(100);
    for (int i = 0; i < argc; i++)
    {
        printf("%d\n", array[i]);
    }
	api_free(buf,100);
	return 0;
}