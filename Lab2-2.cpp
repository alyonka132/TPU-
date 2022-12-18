#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <chrono>
#include <omp.h>

using namespace std;

const int thread = 32;

int main(int argc, char** argv)
{
	int N = 4096;

	cout << "Enter the size " << endl;
	cin >> N;

	int** mas1 = new int* [N];
	int** mas2 = new int* [N];
	long int** result = new long int* [N];
	long int** result2 = new long int* [N];

	for (int i = 0; i < N; i++)
	{
		mas1[i] = new int[N];
		mas2[i] = new int[N];
		result[i] = new long int[N];
		result2[i] = new long int[N];
		for (int j = 0; j < N; j++)
		{
			mas1[i][j] = 1 + rand() % 65535; // INT_MAX;
			mas2[i][j] = 1 + rand() % 65535; //INT_MAX;
			result[i][j] = 0;
			result2[i][j] = 0;
		}
	}

	auto start = std::chrono::system_clock::now();

	cout << "One thread" << endl;

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			for (int k = 0; k < N; k++)
			{
				result[i][j] += mas1[i][k] * mas2[k][j];
				//cout << mas1[i][k] << " " << mas2[k][j] << " " << result[i][j] << " " << i << " " << j << endl;
			}

	auto end = std::chrono::system_clock::now();

	auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();

	cout << "1 Thread time is " << time << endl;
	cout << endl;
	cout << " Multi" << endl;

#pragma omp parallel num_threads(thread)
	{
#pragma omp critical
		{
			cout << "Thread " << omp_get_thread_num() << endl;
		}
	}

	// Multitread

	start = std::chrono::system_clock::now();

#pragma omp parallel for num_threads(thread)
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			for (int k = 0; k < N; k++)
			{
				result2[i][j] += mas1[i][k] * mas2[k][j];
				//cout << mas1[i][k] << " " << mas2[k][j] << " " << result2[i][j] << " " << i << " " << j << endl;
			}

	end = std::chrono::system_clock::now();

	time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();

	cout << "MultiThread time is " << time << endl;

	cout << "Matrix check " << endl;

	bool flag = 0;
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			if (result2[i][j] != result[i][j])
			{
				flag = 1;
				cout << i << " " << j << " " << result[i][j] << " " << result2[i][j] << endl;
				break;
			}

	if (flag == 1)
		cout << "Matrix are not equal" << endl;
	else
		cout << "Matrix are equal" << endl;

	//cin >> N;
	//cin >> N;
	//exit(EXIT_SUCCESS);
}