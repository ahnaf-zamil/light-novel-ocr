#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <bits/stdc++.h>
#include <string>
#include <cmath>
#include <stdio.h>
#include <analyser.h>
#include <typeinfo>

using Ctr = std::vector<cv::Point>;
using CtrVec = std::vector<Ctr>; // Contour vector

cv::Mat binarize(cv::Mat img)
{
    // Converting to grayscale
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    // Thresholding the grayed image
    cv::Mat binary;
    cv::threshold(gray, binary, 127, 255, 1); // 1 is for THRESH_BINARY_INV

    return binary;
}

CtrVec find_contours_text(cv::Mat img, cv::Mat kernel)
{
    // Dilation
    cv::Mat dilated;
    cv::dilate(img, dilated, kernel);

    CtrVec contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::Mat contourOutput = dilated.clone();
    cv::findContours(contourOutput, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


    std::sort(contours.begin(), contours.end(),
    [](Ctr const &a, Ctr const &b) {
        return cv::boundingRect(a).x < cv::boundingRect(b).x;
    });

    return contours;
}


cv::Mat draw_contours(cv::Mat img, CtrVec ctrs)
{
    cv::Mat img_ctrs = img.clone();
    for(unsigned int i = 0; i < ctrs.size(); i++)
    {
        // Drawing contour over given img
        cv::Rect rect = cv::boundingRect(ctrs[i]);
        cv::Point pt1(rect.x, rect.y);
        cv::Point pt2(rect.x + rect.width, rect.y + rect.height);
        cv::rectangle(img_ctrs, pt1, pt2, cv::Scalar(0, 255, 0), 2);

        cv::Point pt3(rect.x + floor(rect.width/2), rect.y + floor(rect.height/2));
        cv::putText(img_ctrs, std::to_string(i), pt3, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }

    return img_ctrs;
}

CtrVec sort_contours(CtrVec ctrs)
{
    std::sort(ctrs.begin(), ctrs.end(), [](Ctr const &a, Ctr const &b) {
        return cv::boundingRect(a).x > cv::boundingRect(b).x;
    });
    return ctrs;
}

CtrVec find_text(cv::Mat section)
{
    // Finds text from the contoured section, and draw contours around it
    cv::Mat binary = binarize(section);
    cv::Mat kernel = cv::Mat::ones(floor(50), 8, CV_8S);
    CtrVec ctrs = find_contours_text(binary, kernel);
    CtrVec sorted_ctrs = sort_contours(ctrs);
    return sorted_ctrs;
}

void analyse_image(std::string f_path, std::string img_name)
{
    // Extracting just name from filename
    img_name.erase(img_name.size() - 4);

    std::string tmp_dir = "tmp\\results_" + img_name;
    if (CreateDirectory(tmp_dir.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
    {
        cv::Mat img = cv::imread(f_path);

        int img_h = img.size[0];

        cv::Mat binary = binarize(img);
        cv::Mat kernel = cv::Mat::ones(10, 100, CV_8S);
        CtrVec ctrs = find_contours_text(binary, kernel);
        draw_contours(img, ctrs);

        // For each section, find text
        int idx = 0;
        for(unsigned int i = 0; i < ctrs.size(); i++)
        {
            cv::Rect rect = cv::boundingRect(ctrs[i]);
            double ratio_h = ((double)rect.height)/((double)img_h);

            if (ratio_h > 0.1) {
                cv::Mat subImg = img(cv::Range(rect.y, rect.y + rect.height), cv::Range(rect.x, rect.x + rect.width));

                CtrVec section_text_ctrs = find_text(subImg);
                std::string fname = tmp_dir + "\\section_" + std::to_string(idx) + "_annotated.png";
                cv::Mat ctred_img = draw_contours(subImg, section_text_ctrs);
                cv::imwrite(fname, ctred_img);

                // For each contoured text, make it a sub image and save it in its respective section folder
                std::string section_text_dir = tmp_dir + "\\section_" + std::to_string(idx);
                if (CreateDirectory(section_text_dir.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
                    for (unsigned int x = 0; x < section_text_ctrs.size(); x++) {
                        cv::Rect section_text_rect = cv::boundingRect(section_text_ctrs[x]);
                        cv::Mat ctrSelection = subImg(cv::Range(section_text_rect.y, section_text_rect.y + section_text_rect.height), cv::Range(section_text_rect.x, section_text_rect.x + section_text_rect.width));

                        cv::imwrite(section_text_dir + "\\text" + std::to_string(x) + ".png", ctrSelection);
                    }
                }
                idx++;
            }
        }
    }
}
