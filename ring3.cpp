#include <stdio.h>
#include <string.h>
#include "mpi.h"

// 定义较大的消息结构
#define MESSAGE_SIZE 1000

int main(int argc, char *argv[])
{
    // 使用较大的数据结构作为消息
    int message[MESSAGE_SIZE];
    int received[MESSAGE_SIZE];
    double start_time, end_time;

    int myid, numprocs;
    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    
    // 初始化消息内容
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = myid * 1000 + i;  // 消息中包含发送进程的rank信息
        received[i] = -1;
    }
    
    int dest = (myid + 1) % numprocs;
    int source = (myid - 1 + numprocs) % numprocs;
    
    // 记录通信前的时间
    start_time = MPI_Wtime();
    
    // 发送
    MPI_Request send_request;
    MPI_Status send_status;
    
    MPI_Isend(message, MESSAGE_SIZE, MPI_INT, dest, 99, MPI_COMM_WORLD, &send_request);
    
    // 接收
    MPI_Request recv_request;
    MPI_Status recv_status;
    
    MPI_Irecv(received, MESSAGE_SIZE, MPI_INT, source, 99, MPI_COMM_WORLD, &recv_request);
    
    // 在这里可以进行计算操作，或者其他任务
    
    // 等待接收完成
    MPI_Wait(&recv_request, &recv_status);
    MPI_Wait(&send_request, &send_status);
    
    // 记录通信后的时间
    end_time = MPI_Wtime();
    
    // 打印结果
    printf("进程 %d 接收到来自进程 %d 的消息，首元素值: %d\n", myid, source, received[0]);
    printf("进程 %d 的通信时间: %f 秒\n", myid, end_time - start_time);
    
    MPI_Finalize();
    return 0;
} 