#include <iostream>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
#include <time.h>

using namespace std;

int main(int argc, char** argv) {
   
    srand(time(NULL));
    int n = 4096;

    int rank, numprocs, line;

    int* matrix1 = (int*)malloc(sizeof(int) * n * n);
    int* matrix2 = (int*)malloc(sizeof(int) * n * n);

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    line = ceil(n / numprocs) + 1;

    int* buffer = (int*)malloc(sizeof(int) * n * line);
    int* buffer1 = (int*)malloc(sizeof(int) * n * n);
    int* buffer2 = (int*)malloc(sizeof(int) * n * n);
    int* buffer3 = (int*)malloc(sizeof(int) * n * n);
    int* result2 = (int*)malloc(sizeof(int) * n * n);
    int* ans = (int*)malloc(sizeof(int) * line * n);
    int* result = (int*)malloc(sizeof(int) * n * n);

    auto start = std::chrono::system_clock::now();
   
    if (rank == 0)
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                matrix1[i*n +j] = rand() % 65; // 536;
                matrix2[i*n+j] = rand() % 65; // 536;
            }
        }

        cout << n << endl;
        auto start1 = std::chrono::system_clock::now();

        MPI_Send(matrix2, n * n, MPI_INT, 1, 4, MPI_COMM_WORLD);
        //std::cout << "send m4" << rank << n << endl;

        MPI_Send(matrix1, n * n, MPI_INT, 1, 5, MPI_COMM_WORLD);
        //std::cout << "send m5" << rank << n << endl;

        MPI_Recv(result, n * n, MPI_INT, 1, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //std::cout << "recv r2" << rank << n << endl;

        // Статистика по времени
        auto end1 = std::chrono::system_clock::now();

        auto time1 = std::chrono::duration_cast<std::chrono::milliseconds> (end1 - start1).count();

        cout << "One thread time is " << time1 << endl;

        std::cout << " MPI send data" << endl;
        
        auto start = std::chrono::system_clock::now();

        for (int i = 1; i < numprocs; i++)
        {
            MPI_Send(matrix2, n * n, MPI_INT, i, 0, MPI_COMM_WORLD);
          //  std::cout << "send m2" << i << n << endl;

            MPI_Send(matrix1 + (i - 1) * line * n, n * line, MPI_INT, i, 1, MPI_COMM_WORLD); 
          // std::cout << "send m1" << i << line << endl;
        }

        cout << numprocs << endl;
        
        for (int k = 1; k < numprocs; k++)
        {
            MPI_Recv(buffer, n * line, MPI_INT, k, 3, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
           // std::cout << "recv ans2 " << rank << endl;

            for (int i = 0; i < line; i++)
                for (int j = 0; j < n; j++)
                {
                    int c = (k - 1) * line * n + i * n;
         //           cout << k << " " << i << " " << j << " " << c << " " << endl;
                    result2[c + j] = buffer[i * n + j];
                }
        }

        for (int i = (numprocs - 1) * line; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                int c = 0;
                for (int k = 0; k < n; k++)
                    c += matrix1[i * n + k] * matrix2[k * n + j];
                result2[i * n + j] = c;
            }
        }

        auto end = std::chrono::system_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();
        cout << " Multi thread time is " << time << endl;

        /*cout << "result2" << endl;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
                cout << result2[i * n + j] << " ";
            cout << endl;
        } */

        int flag = 0;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
                if (result[i*n + j] != result2[i * n + j])
                {
                   // cout << i << " " << j << " " << result[i*n+j] << " " << result2[i * n + j] << endl;
                    flag = 1;
                    break;
                }

            if (flag == 1) break;
        }

        if (flag == 1)
            cout << " Matrix is not equals" << endl;
        else
            cout << "Matrix is equals" << endl;

        free(matrix1);
        free(matrix2);
    }
    else
        if (rank == 1)
        {
            MPI_Recv(buffer2, n* n, MPI_INT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // cout << "recv m4" << rank << n << endl;
            MPI_Recv(buffer3, n* n, MPI_INT, 0, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // cout << "recv m5" << rank << n << endl;
            
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    result[i * n + j] = 0;
                    for (int k = 0; k < n; k++)
                        result[i * n + j] += (buffer3[i * n + k] * buffer2[k * n + j]);
                }
            }

            MPI_Send(result, n* n, MPI_INT, 0, 6, MPI_COMM_WORLD);
            //cout << "send r1" << rank << endl;

            // std::cout << rank << endl;
            MPI_Recv(buffer1, n * n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
           // std::cout << "recv m2" << rank << n << endl;
            MPI_Recv(buffer, n * line, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
           // std::cout << "recv m1" << rank << line << endl;
            
            for (int i = 0; i < line; i++)
                for (int j = 0; j < n; j++)
                {
                    int c = 0;
                    for (int k = 0; k < n; k++)
                    {
                        c += buffer[i * n + k] * buffer1[k * n + j];
                        // cout << buffer[i * n + k] << " " << buffer1[k * n + j] << endl;
                    }
                    ans[i * n + j] = c;
                }
           
            
            /*cout << "result 1" << endl;
              for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    cout << result[i*n+j] << " ";
                }
                cout << endl;
            }*/
            // Отправить результат расчета в основной процесс
            
            MPI_Send(ans, line* n, MPI_INT, 0, 3, MPI_COMM_WORLD);
            //std::cout << "send ans1" << rank << endl;

        }
        else if (!((rank == 0) || (rank == 1)))
        {
            //std::cout << rank << endl;
            MPI_Recv(buffer1, n * n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            //std::cout << "recv m2" << rank << endl;
            MPI_Recv(buffer, n * line, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            //std::cout << "recv m1" << rank << endl;

            for (int i = 0; i < line; i++)
                for (int j = 0; j < n; j++)
                {
                    int c = 0;
                    for (int k = 0; k < n; k++)
                        c += buffer[i*n + k] * buffer1[k * n + j];
                    ans[i * n + j] = c;
                }

            MPI_Send(ans, line * n, MPI_INT, 0, 3, MPI_COMM_WORLD);
            //std::cout << "send ans1" << rank << endl;
        }

    MPI_Finalize();
    return 0;
}