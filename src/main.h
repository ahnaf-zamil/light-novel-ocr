#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <ctime>
#include <chrono>
#include <cmath>
#include <map>
#include <thread>
#include <typeinfo>
#include <dirent.h>
#include <errno.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <bits/stdc++.h>
#include <string>
#include <stdio.h>

std::string exec(std::string cmd);

void analyse_image(std::string f_path, std::string img_name);

void do_ocr_for_folder(std::string out_file, std::string img_folder_path);
