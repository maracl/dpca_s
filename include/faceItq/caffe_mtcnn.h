// Created by zzb on 18-7-31.

#ifndef MTCNN_TEST_CAFFE_MTCNN_H
#define MTCNN_TEST_CAFFE_MTCNN_H

#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <algorithm>
#include <string>

//#define CPU_ONLY

using namespace caffe;

class MtcnnDetector {
public:

    enum COLOR_ORDER{
        GRAY,
        RGBA,
        RGB,
        BGRA,
        BGR
    };

    enum DEVICE_MODEL{
        GPU,
        CPU,
    };

    enum MODEL_VERSION{
        MODEL_V1,
        MODEL_V2
    };

    enum NMS_TYPE{
        MIN,
        UNION,
    };

    enum IMAGE_DIRECTION{
        ORIENT_LEFT,
        ORIENT_RIGHT,
        ORIENT_UP,
        ORIENT_DOWN,
    };

    struct BoundingBox{
        //rect two points
        float x1, y1;
        float x2, y2;
        //regression
        float dx1, dy1;
        float dx2, dy2;
        //cls
        float score;
        //inner points
        float points_x[5];
        float points_y[5];
    };

    struct CmpBoundingBox{
        bool operator() (const BoundingBox& b1, const BoundingBox& b2)
        {
            return b1.score > b2.score;
        }
    };
private:
    boost::shared_ptr< Net<float> > P_Net;
    boost::shared_ptr< Net<float> > R_Net;
    boost::shared_ptr< Net<float> > O_Net;
    //used by model 2 version
    boost::shared_ptr< Net<float> > L_Net;
    double                           img_mean;
    double                           img_var;
    cv::Size                         input_geometry_;
    int                              num_channels_;
    MODEL_VERSION                    model_version;

public:
    MtcnnDetector(const string& model_dir,
                 const MODEL_VERSION model_version, const DEVICE_MODEL device_model = CPU);

    vector< BoundingBox > Detect (const cv::Mat& img, const COLOR_ORDER color_order, const IMAGE_DIRECTION orient,
                                  int min_size = 20, float P_thres = 0.6, float R_thres = 0.7, float O_thres =0.7,
                                  bool is_fast_resize = true, float scale_factor = 0.709);

    cv::Size GetInputSize(){ return input_geometry_; }

    int GetInputChannel(){ return num_channels_; }

    std::vector<int> GetInputShape()  {
        Blob<float>* input_layer = P_Net->input_blobs()[0];
        return input_layer->shape();
    }

    void FaceAlignment(const cv::Mat &srcImg, std::vector<cv::Mat> &rotImg, vector< BoundingBox > & dets);

private:
    void generateBoundingBox(const std::vector<float>& boxRegs, const std::vector<int>& box_shape,
                             const std::vector<float>& cls, const std::vector<int>& cls_shape,
                             float scale, float threshold, std::vector<BoundingBox>& filterOutBoxes
    );

    void filteroutBoundingBox(const std::vector<BoundingBox>& boxes,
                              const std::vector<float>& boxRegs, const std::vector<int>& box_shape,
                              const std::vector<float>& cls, const std::vector<int>& cls_shape,
                              const std::vector< float >& points, const std::vector< int >& points_shape,
                              float threshold, std::vector<BoundingBox>& filterOutBoxes);
    void nms_cpu(std::vector<BoundingBox>& boxes, float threshold, NMS_TYPE type, std::vector<BoundingBox>& filterOutBoxes);


    vector<float> predict(const cv::Mat& img);
    void wrapInputLayer(boost::shared_ptr< Net<float> > net, std::vector<cv::Mat>* input_channels);
    void pyrDown(const vector<cv::Mat>& img_channels,float scale, std::vector<cv::Mat>* input_channels);
    void buildInputChannels(const std::vector<cv::Mat>& img_channels, const std::vector<BoundingBox>& boxes,
                            const cv::Size& target_size, std::vector<cv::Mat>* input_channels);

    void RotateBilinear(unsigned char *sourceData, int width, int height, int Channels, int RowBytes,
                                       unsigned char *destinationData, int newWidth, int newHeight, float angle, bool keepSize = true,
                                       int fillColorR = 0, int fillColorG = 0, int fillColorB = 0);

    void facialPoseCorrection(unsigned char *inputImage, int Width, int Height, int Channels, int left_eye_x, int left_eye_y,
                              int right_eye_x, int right_eye_y);

};

#endif //MTCNN_TEST_CAFFE_MTCNN_H
