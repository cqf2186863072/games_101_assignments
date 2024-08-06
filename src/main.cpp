#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

void draw(cv::Point2f p, cv::Mat& window, cv::Vec3b color) {
    int radius = 1;
    cv::Vec3f normalized_color = color / 255.0;

    int x = static_cast<int>(p.x);
    int y = static_cast<int>(p.y);

    int x_min = std::max(0, x - radius);
    int x_max = std::min(window.cols - 1, x + radius);
    int y_min = std::max(0, y - radius);
    int y_max = std::min(window.rows - 1, y + radius);

    for (int i = x_min; i < x_max + 1; i++) {
        for (int j = y_min; j < y_max + 1; j++) {
            // 使用欧氏距离计算距离
            float dist_squared = std::pow(i - p.x, 2) + std::pow(j - p.y, 2);
            // 使用距离计算衰减因子
            float factor = std::max(0.f, 1 - dist_squared / (radius * radius));

            for (int k = 0; k < 3; k++) {
                float new_color = (1 - factor) * window.at<cv::Vec3b>(j, i)[k] + factor * color[k];
                window.at<cv::Vec3b>(j, i)[k] = static_cast<uchar>(new_color);
            }
        }
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f>& control_points, float t)
{
    std::vector<cv::Point2f> points(control_points);

    int n = points.size();
    while (n > 1) {
        n--;
        for (int i = 0; i < n; i++) {
            points[i] = t * points[i + 1] + (1 - t) * points[i];
        }
        points.resize(n);
    }

    return points[0];
}

void bezier(const std::vector<cv::Point2f>& control_points, cv::Mat& window)
{
    float dt = 0.0005;
    cv::Vec3b color = (255, 0, 255);

    for (float t = 0; t <= 1; t += dt) {
        cv::Point2f p = recursive_bezier(control_points, t);
        //window.at<cv::Vec3b>(p.y, p.x)[2] = 255;
        
        draw(p, window, color);
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            //naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
