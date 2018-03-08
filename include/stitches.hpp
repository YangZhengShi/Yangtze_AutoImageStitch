//
//  stiches.hpp
//  Yangtze
//
//  Created by DENG XI on 3/4/18.
//
//

#ifndef stiches_hpp
#define stiches_hpp

#include <stdio.h>
#include "floatimage.h"
#include "common.h"
#include "PanoImage.hpp"

// smallest box that bound the image
struct imagebound{
    Vec2f topleft;
    Vec2f btnright;
    
    imagebound();
    void grow(Vec3f point);
    bool inbound(Vec3f point);
    
};

// canvas that contain two images
struct canvas{
    Vec2i offset;
    int length;
    int height;
};


typedef struct canvas Canvas;
typedef struct imagebound ImageBound;

class Pano{
    float m_harris_th;
    float m_match_th;
    float m_sigma;
    int m_pwindow;
    int m_window;
    float m_portion;
public:
    
    Pano(){
        m_harris_th = 0.3;
        m_match_th = 0.6;
        m_sigma = 2;
        m_window = 9;
        m_pwindow = 21;
        m_portion = 0.2;
    }
    
    Pano(int window,float harris_th, float match_th, float sigma, int pwindow, float portion){
        m_harris_th = harris_th;;
        m_match_th = match_th;
        m_sigma = sigma;
        m_window = window;
        m_pwindow = pwindow;
        m_portion = portion;

    }
    
    // cat 2 image given corresponding poin sets
    FloatImage cat2images(const FloatImage &im1, const FloatImage &im2, Mat3f homo);
    FloatImage cat2imageBlend(const FloatImage &im1, const FloatImage &im2, Mat3f homo);
    FloatImage mancat2images(const FloatImage &im1, const FloatImage &im2, std::vector<std::vector<Vec2f>> pairs);
    FloatImage autocat2images(PanoImage &pim1, PanoImage &pim2);


    
    // solve Ax = 0 with svd
    Mat3f solveHomo(MatrixXf);
    
    // calculate new canvas given image and homo
    ImageBound boundBox(const FloatImage &im);
    ImageBound boundBoxHomo(const FloatImage &im, Mat3f homo);
    
    // calculate the shift offset when conbining two images
    Canvas calculateCanvas(ImageBound a, ImageBound b);
    
    // calculate weight map of a image of sizex * sizey
    FloatImage calweight(int sizex, int sizey);




    Mat3f RANSAC( PanoImage &pim1, PanoImage &pim2, float match_th = 0.5, float portion = 0.5, float accuracy = 0.1,
                  float threshold = 1);
    Mat3f computeHomo(std::vector<std::vector<Vec2f>> pairs);

    std::vector<std::vector<Vec2i>> matchDescriptors(PanoImage &pim1, PanoImage &pim2, float threshold=0.5);
    FloatImage vizMatches(PanoImage &pim1, PanoImage &pim2, std::vector<std::vector<Vec2i>> matches);
    FloatImage vizMatches(PanoImage &pim1, PanoImage &pim2, std::vector<std::vector<Vec2f>> matches);


};



#endif /* stiches_hpp */


