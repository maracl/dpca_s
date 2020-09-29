#ifndef _RMMT_SHM_IMAGE_H
#define _RMMT_SHM_IMAGE_H
#include "rmmt_wrap2.h"
#include <opencv2/core/core.hpp>

namespace rmmt{

	struct ImageHdr{
		int type;
		uint32_t width;
		uint32_t height;
		uint32_t size;
		uint64_t pts;
		uint64_t _reserved;	//16byte aligned
		//data
	};
	class ShmImage{
		std::shared_ptr<rmmt::MemNode> m_shmn;

	public:
		explicit ShmImage(const std::shared_ptr<rmmt::MemNode>& s) :m_shmn(s) {}

		ShmImage() {}

		std::shared_ptr<rmmt::MemNode>& get_shm() {
			return m_shmn;
		}

		//may throw!
		bool clone(const ShmImage& simg) {
			if (!simg.m_shmn||simg.m_shmn->get_ptr()==nullptr) return false;
			uint32_t msz = simg.m_shmn->get_size();
			m_shmn = rmmt::allocate_memnode(msz);
			memcpy(m_shmn->get_ptr(), simg.m_shmn->get_ptr(), msz);
			return true;
		}

		int checkImgValid() const {
			const ImageHdr* ph = getHdr();
			if (!ph) return -1;
			if (!(ph->width > 0 && ph->height > 0 && ph->width < 65536 && ph->height < 65536)) return 1;
			return ph->size <= m_shmn->get_size() - sizeof(ImageHdr) ? 0 : 2;
		}

		//may throw!
		void allocate(uint32_t width, uint32_t height, uint32_t type, uint32_t widthStep) {
			uint32_t sz = widthStep*height;
			m_shmn = rmmt::allocate_memnode(sz + sizeof(ImageHdr));
			ImageHdr* ph = (ImageHdr*)m_shmn->get_ptr();
			ph->width = width;
			ph->height = height;
			ph->size = sz;
			ph->type = type;
			ph->pts = 0;
		}

		char* getData() const {
			if (!m_shmn) return nullptr;
			return (char*)m_shmn->get_ptr() + sizeof(ImageHdr);
		}

		ImageHdr* getHdr() const {
			if (!m_shmn) return nullptr;
			return (ImageHdr*)m_shmn->get_ptr();
		}
#ifdef CV_VERSION //use opencv
		IplImage toCvImage() const {
			IplImage img = {};
			ImageHdr* ph = getHdr();
			if (ph) {
				img.nSize = sizeof(IplImage);
				img.align = 1;
				img.depth = 8;
				img.nChannels = ph->type & 7;
				img.width = ph->width;
				img.height = ph->height;
				img.widthStep = ph->size / ph->height;
				img.imageSize = ph->size;
				img.imageDataOrigin = img.imageData = getData();
			}
			return img;
		}
		cv::Mat toCvMat() const {
			ImageHdr* ph = getHdr();
			if (!ph) return cv::Mat();
			return cv::Mat(ph->height, ph->width, CV_MAKETYPE(CV_8U, ph->type & 7), getData(), ph->size / ph->height);
		}

		bool clone(const IplImage* img) {
			if (!img || img->imageData == nullptr) return false;
			m_shmn = rmmt::allocate_memnode(img->imageSize + sizeof(ImageHdr));
			ImageHdr* ph = (ImageHdr*)m_shmn->get_ptr();
			ph->width = img->width;
			ph->height = img->height;
			ph->size = img->imageSize;
			ph->type = img->nChannels*img->depth / 8;
			ph->pts = 0;
			memcpy(getData(), img->imageData, img->imageSize);
			return true;
		}

		bool clone(const cv::Mat& mat) {
			if (!mat.data) return false;
			int ws = mat.step[0];
			m_shmn = rmmt::allocate_memnode(ws*mat.rows + sizeof(ImageHdr));
			ImageHdr* ph = (ImageHdr*)m_shmn->get_ptr();
			ph->width = mat.cols;
			ph->height = mat.rows;
			ph->size = ws*mat.rows;
			ph->type = mat.step[1];
			ph->pts = 0;
			memcpy(getData(), mat.data, ws*mat.rows);
			return true;
		}
#endif
	};


}

#endif