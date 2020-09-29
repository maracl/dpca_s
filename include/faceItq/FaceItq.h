//
// Created by abu on 18-8-29.
//

#ifndef FACE_ITQ_FACEITQ_H
#define FACE_ITQ_FACEITQ_H
#include <string>
#include <Eigen/Dense>
#include <vector>

#define FACEFEADIM 512
#define RFACEFEADIM_128 128
#define RFACEFEADIM_256 256

class FaceItq{
    public:
        enum DRTYPE{
            D256 = 0,
            D128 = 1,
            DALL = 2
        };

        struct ItqFeature{
            float floatFea[RFACEFEADIM_256];
            uint64_t quant256[RFACEFEADIM_256 / 64];
            uint64_t quant128[RFACEFEADIM_128 / 64];
        };

        FaceItq(std::string itqPath, DRTYPE dimType);
        void ReadPcaPara(std::string itqPath);
        void PcaDimRedu(std::vector<float> &srcFeature, ItqFeature &itqFeature);
        void PcaDimRedu(float *srcFeature, int feaNum, ItqFeature &itqFeature);
        void QuantiFea(std::string itqPath);

    private:
        Eigen::VectorXd Mean, sigma;
        Eigen::Matrix <double, Eigen::Dynamic, Eigen::Dynamic> pca_128, pca_256;
        Eigen::Matrix <double, Eigen::Dynamic, Eigen::Dynamic> R_128, R_256;
        DRTYPE dimReduce;
};

#endif //FACE_ITQ_FACEITQ_H
