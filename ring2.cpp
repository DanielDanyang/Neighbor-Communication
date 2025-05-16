#include <mpi.h>
#include <iostream>
#include <string.h>

// 定义较大的消息结构
#define MESSAGE_SIZE 1000

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 使用较大的数据结构作为消息
    int message[MESSAGE_SIZE];
    int received[MESSAGE_SIZE];
    
    // 初始化消息内容
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = rank * 1000 + i;  // 消息中包含发送进程的rank信息
        received[i] = -1;
    }

    int right = (rank + 1) % size;
    int left = (rank - 1 + size) % size;

    MPI_Request send_request, recv_request;

    // 记录通信前的时间
    double start_time = MPI_Wtime();

    // 非阻塞发送与接收
    MPI_Isend(message, MESSAGE_SIZE, MPI_INT, right, 0, MPI_COMM_WORLD, &send_request);
    MPI_Irecv(received, MESSAGE_SIZE, MPI_INT, left, 0, MPI_COMM_WORLD, &recv_request);

    // 在这里进行计算操作，或者其他任务
    // 比如某些计算任务，进程可以在通信过程中继续工作
    // ...

    // 等待通信完成
    MPI_Wait(&send_request, MPI_STATUS_IGNORE);
    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

    // 记录通信后的时间
    double end_time = MPI_Wtime();

    // 输出耗时
    std::cout << "进程 " << rank << " 接收到来自进程 " << left << " 的消息，首元素值: " << received[0] << std::endl;
    std::cout << "进程 " << rank << " 的通信时间: " << end_time - start_time << " 秒" << std::endl;

    MPI_Finalize();
    return 0;
}
