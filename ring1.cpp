#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

// 默认消息大小，如果没有通过命令行指定
#define DEFAULT_MESSAGE_SIZE 5000000

int main(int argc, char **argv) {
    int size = 0, rank = 0, next = 0, prev = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // 从命令行参数获取消息大小
    int MESSAGE_SIZE = DEFAULT_MESSAGE_SIZE;
    if (argc > 1) {
        MESSAGE_SIZE = atoi(argv[1]);
    }
    
    // 计算下一个和上一个进程的编号
    next = (rank + 1) % size;
    prev = (rank + size - 1) % size;

    // 使用较大的数据结构作为消息
    double* message = new double[MESSAGE_SIZE];
    double* received = new double[MESSAGE_SIZE];  // 单独的接收缓冲区
    
    // 初始化消息内容
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = rank * 1000.0 + i % 1000;  // 简化计算
        received[i] = -1.0;  // 初始化接收缓冲区
    }

    // 发送和接收消息
    int t = rank % 2;

    // 记录通信前的时间
    double start_time = MPI_Wtime();

    if (t == 0) { // 偶数号进程先发送
        MPI_Send(message, MESSAGE_SIZE, MPI_DOUBLE, next, 0, MPI_COMM_WORLD);
        MPI_Recv(received, MESSAGE_SIZE, MPI_DOUBLE, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } 
    else { // 奇数号进程先接收
        MPI_Recv(received, MESSAGE_SIZE, MPI_DOUBLE, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(message, MESSAGE_SIZE, MPI_DOUBLE, next, 0, MPI_COMM_WORLD);
    }

    // 计算接收到的数据
    double sum = 0.0;
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        sum += received[i] * 0.0001;  // 简化计算
    }

    // 记录通信后的时间
    double end_time = MPI_Wtime();

    // 输出进程信息和通信时间
    printf("Ring1: Process %d received message from process %d, first element value: %f, checksum: %f\n", 
           rank, prev, received[0], sum);
    printf("Ring1: Process %d communication time: %f seconds, message size: %d\n", 
           rank, end_time - start_time, MESSAGE_SIZE);

    delete[] message;
    delete[] received;

    MPI_Finalize();
    return 0;
}
