#include <mpi.h>
#include <iostream>
#include <string.h>
#include <cmath>
#include <stdlib.h>

// 默认消息大小，如果没有通过命令行指定
#define DEFAULT_MESSAGE_SIZE 5000000

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

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
        message[i] = rank * 1000.0 + i % 1000;  // 简化计算
        received[i] = -1.0;
    }

    // 进程i与i+1通信，形成环形拓扑
    int right = (rank + 1) % size;
    int left = (rank - 1 + size) % size;

    // 发送到右邻居，从左邻居接收
    double start_time = MPI_Wtime();

    // 执行MPI_Send和MPI_Recv操作
    MPI_Send(message, MESSAGE_SIZE, MPI_DOUBLE, right, 0, MPI_COMM_WORLD);
    MPI_Recv(received, MESSAGE_SIZE, MPI_DOUBLE, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // 处理接收到的数据
    double sum = 0.0;
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        sum += received[i] * 0.0001;  // 简化计算
    }

    double end_time = MPI_Wtime();
    
    // 输出结果
    std::cout << "Ring0: Process " << rank << " received message from process " << left 
              << ", first element value: " << received[0] << ", checksum: " << sum << std::endl;
    std::cout << "Ring0: Process " << rank << " communication time: " << end_time - start_time 
              << " seconds, message size: " << MESSAGE_SIZE << std::endl;

    delete[] message;
    delete[] received;

    MPI_Finalize();
    return 0;
}
