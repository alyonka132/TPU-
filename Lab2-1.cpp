#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <chrono>
#include <fstream>
#include <omp.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace cv;
using namespace std;

void Function(Mat img1, Mat img2, int i, unsigned char** R3, float** I3, int cols)
{
	auto color1 = img1.at<cv::Vec3b>(0, 0);
	auto color2 = img2.at<cv::Vec3b>(0, 0);
	unsigned char col = 0;

	for (int j = 0; j < cols; j++)
	{
		color1 = img1.at<cv::Vec3b>(i, j);
		color2 = img2.at<cv::Vec3b>(i, j);
		R3[i][j] = color1[0] + color1[1] + color1[2]
			- (unsigned char)color2[0] - (unsigned char)color2[1] - (unsigned char)color2[2];
		/*if (j == 0)
		{
			cout << i << " " << j << endl;
			/*(int)color1[0] << " " << (int)color1[1] << " " <<
				(int)color1[2] << (int)color2[0] << " " << (int)color2[1]
				<< (int)color2[2] << " " << (int)R3[i][j] << " " << endl;
		}*/
		I3[i][j] = (float)(255 / 2.0) + (float)(R3[i][j] / 6.0);
	}
}

int main(int argc, char** argv)
{
	int thread = 4;
	int N = 0;
	cout << " Enter N: 0 or 1 or 2" << endl;
	cin >> N;
	cout << thread << endl;
	cin >> thread;
	String name[] = { "1p1024.jpg", "2p1024.jpg", "1p1280.jpg", "2p1280.jpg", "1p2048.jpg", "2p2048.jpg" };

	String inputname = "D:\\" + name[N], inputname2 = "D:\\" + name[N + 1];

	String outname = "D:\\out1" + std::to_string(N) + "-" + std::to_string(N + 1) + ".txt";
	String outname2 = "D:\\out2" + std::to_string(N) + "-" + std::to_string(N + 1) + ".txt";

	ofstream fout1;
	ofstream fout;
	fout.open(outname);
	fout1.open(outname2);

	Mat image = imread(inputname), image2 = imread(inputname2);

	/*imshow("win1", image);
	waitKey();
	imshow("win2", image2);
	waitKey();*/

	if (image.empty() || image2.empty())
	{
		cout << "Could not open or find the image" << endl;
		cin.get(); //wait for any key press
		return -1;
	}

	cout << "Size image " << image.rows << " " << image.cols << endl;

	int rows = image.rows;
	int cols = image.cols;

	unsigned char** R = new unsigned char* [rows];
	for (int i = 0; i < rows; i++)
		R[i] = new unsigned char[cols];

	unsigned char** R2 = new unsigned char* [rows];
	for (int i = 0; i < rows; i++)
		R2[i] = new unsigned char[cols];

	float** I = new float* [rows];
	for (int i = 0; i < rows; i++)
		I[i] = new float[cols];

	float** I2 = new float* [rows];
	for (int i = 0; i < rows; i++)
		I2[i] = new float[cols];


	float* timer = new float[100];

	cout << "One thread " << endl;

	int count5 = 100;
	for (int k = 0; k < count5; k++)
	{
		//One thread 
		auto start = std::chrono::system_clock::now();

		auto color1 = image.at<cv::Vec3b>(0, 0);
		auto color2 = image2.at<cv::Vec3b>(0, 0);

		for (int i = 0; i < rows; i++)
			Function(image, image2, i, R, I, cols);

		auto end = std::chrono::system_clock::now();
		auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();

		timer[k] = time;
	}

	float sum = 0;
	for (int k = 0; k < count5; k++)
	{
		sum = sum + timer[k];
		cout << timer[k] << " ";
		if (k % 10 == 9) cout << endl;
	}

	sum = sum / count5;
	cout << "Mean time is" << sum << endl;

	fout.close();

	if (!fout)
		cout << "Don't open file" << endl;
	else
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
				fout << I[i][j];
			fout << endl;
		}
	}

#pragma omp parallel num_threads(thread)
	{
#pragma omp critical
		{
			cout << "Thread " << omp_get_thread_num() << endl;
		}
	}
	// Multitread 

	omp_set_num_threads(thread);

	for (int k = 0; k < count5; k++)
	{
		//One thread 
		auto start = std::chrono::system_clock::now();

#pragma omp parallel for num_threads(thread)

		for (int i = 0; i < rows; i++)
			Function(image, image2, i, R, I2, cols);

		auto end = std::chrono::system_clock::now();
		auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();

		timer[k] = time;
	}

	sum = 0;

	for (int k = 0; k < count5; k++)
	{
		sum = sum + timer[k];
		cout << timer[k] << " ";
		if (k % 10 == 9) cout << endl;
	}
	sum = sum / count5;

	cout << "Mean time is" << sum << endl;

	fout1.width(6);

	if (!fout1.is_open())
		cout << "Don't open file" << endl;
	else
	{
		//#pragma omp parallel for num_threads(16)
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
				fout1 << I2[i][j] << " ";
			fout1 << endl;
		}
	}

	cout << "Matrix check " << endl;
	bool flag = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
			if (I[i][j] != I2[i][j])
			{
				flag = 1;
				cout << i << " " << j << " " << I[i][j] << " " << I2[i][j] << endl;
				break;
			}
	}
	if (flag == 1)
		cout << "Matrix are not equal" << endl;
	else
		cout << "Matrix are equal" << endl;

	/*int count2, x, y;
	cin >> count2;
	for (int i = 0; i < count2; i++)
	{
		cin >> x;
		cin >> y;

		auto color1 = image.at<cv::Vec3b>(x, y);
		auto color2 = image2.at<cv::Vec3b>(x, y);

		cout << x << " " << y << color1 << color2 << (int) R[x][y] << " " << (int) R2[x][y] <<
			" " << I[x][y] << " "  <<  I2[x][y] << " " << endl;

	} */

	//exit(EXIT_SUCCESS);
}