#include <mpi.h>
#include <stdio.h>
#include <string.h>

// 定义较大的消息结构
#define MESSAGE_SIZE 1000

int main(int argc, char **argv) {
    int size = 0, rank = 0, next = 0, prev = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // 计算下一个和上一个进程的编号
    next = (rank + 1) % size;
    prev = (rank + size - 1) % size;

    // 使用较大的数据结构作为消息
    int message[MESSAGE_SIZE];
    int received[MESSAGE_SIZE];  // 新增独立的接收缓冲区
    
    // 初始化消息内容
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = rank * 1000 + i;  // 消息中包含发送进程的rank信息
        received[i] = -1;  // 初始化接收缓冲区
    }

    // 发送和接收消息
    int t = rank % 2;

    // 记录通信前的时间
    double start_time = MPI_Wtime();

    if (t == 0) { // 偶数号进程先发送
        MPI_Send(message, MESSAGE_SIZE, MPI_INT, next, 0, MPI_COMM_WORLD);
        MPI_Recv(received, MESSAGE_SIZE, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } 
    else { // 奇数号进程先接收
        MPI_Recv(received, MESSAGE_SIZE, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(message, MESSAGE_SIZE, MPI_INT, next, 0, MPI_COMM_WORLD);
    }

    // 记录通信后的时间
    double end_time = MPI_Wtime();

    // 输出进程信息和通信时间
    printf("进程 %d 接收到来自进程 %d 的消息，首元素值: %d\n", rank, prev, received[0]);
    printf("进程 %d 的通信时间: %f 秒\n", rank, end_time - start_time);

    MPI_Finalize();
    return 0;
}
