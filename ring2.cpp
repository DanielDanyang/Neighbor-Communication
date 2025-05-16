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

    int right = (rank + 1) % size;
    int left = (rank - 1 + size) % size;

    MPI_Request send_request, recv_request;

    // 记录通信前的时间
    double start_time = MPI_Wtime();

    // 奇偶进程处理
    int t = rank % 2;  // 确定进程是奇数还是偶数

    // 立即启动所有通信操作，顺序基于进程奇偶性
    if (t == 0) { // 偶数进程
        MPI_Isend(message, MESSAGE_SIZE, MPI_DOUBLE, right, 0, MPI_COMM_WORLD, &send_request);
        MPI_Irecv(received, MESSAGE_SIZE, MPI_DOUBLE, left, 0, MPI_COMM_WORLD, &recv_request);
    } 
    else { // 奇数进程
        MPI_Irecv(received, MESSAGE_SIZE, MPI_DOUBLE, left, 0, MPI_COMM_WORLD, &recv_request);
        MPI_Isend(message, MESSAGE_SIZE, MPI_DOUBLE, right, 0, MPI_COMM_WORLD, &send_request);
    }

    // 轻量级计算同时检查通信状态
    double local_work = 0.0;
    int test_counter = 0;
    int recv_done = 0;
    
    // 不过度计算，避免与通信竞争
    while (!recv_done && test_counter < 1000) {
        // 做少量计算
        for (int i = 0; i < 5000; i++) {
            local_work += sin(i * 0.01);
        }
        
        // 定期测试通信完成情况
        test_counter++;
        if (test_counter % 5 == 0) {
            MPI_Test(&recv_request, &recv_done, MPI_STATUS_IGNORE);
        }
    }

    // 如果接收未完成，等待完成
    if (!recv_done) {
        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
    }
    
    // 处理接收到的数据
    double sum = 0.0;
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        sum += received[i] * 0.0001;  // 简化计算
    }
    
    // 确保发送也完成
    MPI_Wait(&send_request, MPI_STATUS_IGNORE);

    // 记录通信后的时间
    double end_time = MPI_Wtime();

    // 输出结果
    std::cout << "Ring2: Process " << rank << " received message from process " << left 
              << ", first element value: " << received[0] << ", checksum: " << sum << std::endl;
    std::cout << "Ring2: Process " << rank << " communication time: " << end_time - start_time 
              << " seconds, message size: " << MESSAGE_SIZE << std::endl;

    delete[] message;
    delete[] received;

    MPI_Finalize();
    return 0;
}
