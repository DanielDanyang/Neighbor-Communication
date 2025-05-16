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

    // 进程 i 与 i+1 通信，形成环形拓扑
    int right = (rank + 1) % size;
    int left = (rank - 1 + size) % size;

    // 发送到右邻进程，接收来自左邻进程
    double start_time, end_time;
    start_time = MPI_Wtime();

    // 执行MPI_Send和MPI_Recv操作
    MPI_Send(message, MESSAGE_SIZE, MPI_INT, right, 0, MPI_COMM_WORLD);
    MPI_Recv(received, MESSAGE_SIZE, MPI_INT, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    end_time = MPI_Wtime();
    std::cout << "进程 " << rank << " 通信时间: " << end_time - start_time << " 秒" << std::endl;
    std::cout << "进程 " << rank << " 接收到来自进程 " << left << " 的消息，首元素值: " << received[0] << std::endl;

    MPI_Finalize();
    return 0;
}
