#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "mpi.h"

// 默认消息大小，如果没有通过命令行指定
#define DEFAULT_MESSAGE_SIZE 5000000
// 定义测试块数量
#define NUM_CHUNKS 20

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    
    int myid, numprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    
    // 从命令行参数获取消息大小
    int MESSAGE_SIZE = DEFAULT_MESSAGE_SIZE;
    if (argc > 1) {
        MESSAGE_SIZE = atoi(argv[1]);
    }
    
    // 使用数据结构作为消息
    double* message = new double[MESSAGE_SIZE];
    double* received = new double[MESSAGE_SIZE];
    
    // 初始化消息内容
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = myid * 1000.0 + i % 1000;  // 简化计算
        received[i] = -1.0;
    }
    
    int dest = (myid + 1) % numprocs;
    int source = (myid - 1 + numprocs) % numprocs;
    
    // 记录通信前的时间
    double start_time = MPI_Wtime();
    
    // 奇偶进程处理
    int t = myid % 2;  // 确定进程是奇数还是偶数
    
    // 发送和接收请求
    MPI_Request send_request;
    MPI_Status send_status;
    MPI_Request recv_request;
    MPI_Status recv_status;
    
    // 立即启动非阻塞操作
    if (t == 0) { // 偶数进程 
        MPI_Isend(message, MESSAGE_SIZE, MPI_DOUBLE, dest, 99, MPI_COMM_WORLD, &send_request);
        MPI_Irecv(received, MESSAGE_SIZE, MPI_DOUBLE, source, 99, MPI_COMM_WORLD, &recv_request);
    } 
    else { // 奇数进程
        MPI_Irecv(received, MESSAGE_SIZE, MPI_DOUBLE, source, 99, MPI_COMM_WORLD, &recv_request);
        MPI_Isend(message, MESSAGE_SIZE, MPI_DOUBLE, dest, 99, MPI_COMM_WORLD, &send_request);
    }
    
    // 首先等待接收完成，同时做少量计算
    int recv_done = 0;
    double partial_sum = 0.0;
    for (int chunk = 0; chunk < NUM_CHUNKS && !recv_done; chunk++) {
        // 少量计算，避免与通信竞争资源
        for (int i = 0; i < 1000; i++) {
            partial_sum += sin(i * 0.1);
        }
        
        // 测试接收是否完成
        MPI_Test(&recv_request, &recv_done, &recv_status);
    }
    
    // 如果接收未完成，等待完成
    if (!recv_done) {
        MPI_Wait(&recv_request, &recv_status);
    }
    
    // 接收完成后立即处理数据
    double sum = 0.0;
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        sum += received[i] * 0.0001;  // 简化计算
    }
    
    // 处理发送完成
    int send_done = 0;
    MPI_Test(&send_request, &send_done, &send_status);
    
    if (!send_done) {
        // 如果发送未完成，等待发送完成
        MPI_Wait(&send_request, &send_status);
    }
    
    // 记录通信后的时间
    double end_time = MPI_Wtime();
    
    // 输出结果
    printf("Ring3: Process %d received message from process %d, first element value: %f, checksum: %f\n", 
           myid, source, received[0], sum);
    printf("Ring3: Process %d communication time: %f seconds, message size: %d\n", 
           myid, end_time - start_time, MESSAGE_SIZE);
    
    delete[] message;
    delete[] received;
    
    MPI_Finalize();
    return 0;
} 