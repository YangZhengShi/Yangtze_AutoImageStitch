//
//  stiches.cpp
//  Yangtze
//
//  Created by DENG XI on 3/4/18.
//
//
#include <iostream>
#include "stiches.hpp"
#include <Eigen/SVD>

FloatImage Pano::cat2images(const FloatImage &im1, const FloatImage &im2, std::vector<Vec2f> ref1, std::vector<Vec2f> ref2){
    
    // construct Ax = b homogenous equation systems
    MatrixXf A;
    A.resize(8,9);
    for(int i = 0; i < 4 ; i++){
        float x = ref1[i].x();
        float y = ref1[i].y();
        float x1 = ref2[i].x();
        float y1 = ref2[i].y();
        A.row(i * 2) << y, x, 1, 0, 0, 0, -y1 * y1, -y1 * x, -y1;
        A.row(i * 2 + 1) << 0, 0, 0, y, x, 1, -x1 * y, -x1 * x, -x1;
    }
    
    // solve by svd
    Mat3f homo;
    homo = solveHomo(A);
    Mat3f homo_inverse = homo.inverse();
    
    // calculate canvas of output image
    ImageBound im1bound = boundBox(im1);
    ImageBound im2bound = boundBoxHomo(im2, homo);
    Canvas canv = calculateCanvas(im1bound, im2bound);
    
    
//    //paste image1 onto canvas
    FloatImage output(canv.length, canv.height, im1.channels());
    for(int i = 0 ; i < im1.sizeX() ; i++)
        for(int j = 0 ; j < im1.sizeY() ; j++){
            int nx  = i - canv.offset.x();
            int ny = j - canv.offset.y();
            for(int c = 0 ; c < im1.channels() ; c++)
                output(nx, ny, c) = im1(i, j, c);
        }
    
    //query image2 and map onto canvas
    Vec2i offsetImage2 = Vec2i(floor(im2bound.topleft.x()), floor(im2bound.btnright.y())) - canv.offset;
    Vec2f sizeTransedImage2 = im2bound.btnright - im2bound.topleft;
    
    for(int i = 0 ; i < sizeTransedImage2.x(); i++){
        for(int j = 0 ; j < sizeTransedImage2.y() ; j++){
            Vec2f transed_pos = im2bound.topleft + Vec2f(i,j);
            Vec3f pos_f = homo_inverse * Vec3f(transed_pos.x(), transed_pos.y(), 1);
            Vec2i pos(floor(pos_f.x()), floor(pos_f.y()));
            if(pos.x() >= 0 && pos.y() >= 0 && pos.x() < im2.sizeX() && pos.y() < im2.sizeY()){
                Vec2i canvas_pos = offsetImage2 + Vec2i(i,j);
                if(canvas_pos.x() > 0 && canvas_pos.y() > 0 && canvas_pos.y() < canv.height && canvas_pos.x() < canv.length)
                 for(int c = 0 ; c < im2.channels() ; c++)
                    output(canvas_pos.x(), canvas_pos.y(), c) = im2(pos.x(), pos.y(),c);
            }
        }
    }
    return output;
}



Mat3f Pano::solveHomo(MatrixXf m){
    SvdXf svd(m, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Vecxf b, x;
    b.resize(8);
    x.resize(9);
    b << 0, 0, 0, 0, 0, 0, 0, 0;
    MatrixXf t,s;
    t = svd.matrixV();
//    for(int i = 0 ; i < 9 ; i++)
//        for(int j = 0 ; j < 8 ; j++)
//            std::cout<<t(i,j)<<std::endl;
    x = (svd.matrixV()).col(8);
    
    MatrixXf a;
    a.resize(2,3);
    a.row(0) << 1,0,0;
    a.row(1) << 0,1,0;
    
    SvdXf ss(a, Eigen::ComputeFullU | Eigen::ComputeFullV);
    s = ss.matrixV();
    Vec3f res = ss.matrixV().col(2);
    

//    t = svd.matrixV();
    Mat3f H;
    float k = 1.0 / x[8];
    H.row(0) <<  k * x[0], k * x[1],k * x[2];
    H.row(1) <<  k * x[3], k * x[4],k * x[5];
    H.row(2) << k * x[6], k * x[7], k * x[8];
//
    return H;
}


ImageBound Pano::boundBox(const FloatImage &im){
    ImageBound hb;
    
    Vec3f tl,tr,bl,br;
    tl << 0,  0, 1;
    tr << 0, im.sizeX(), 1;
    bl << im.sizeY(), 0, 1;
    br << im.sizeY(), im.sizeX(), 1;
    
    hb.grow(tl);
    hb.grow(tr);
    hb.grow(bl);
    hb.grow(br);
    
    return hb;
    
}

ImageBound Pano::boundBoxHomo(const FloatImage &im, Mat3f homo){
    ImageBound hb;
    
    Vec3f tl,tr,bl,br;
    tl << 0,  0, 1;
    tr << 0, im.sizeX(), 1;
    bl << im.sizeY(), 0, 1;
    br << im.sizeY(), im.sizeX(), 1;
    
    tl = homo * tl;
    tr = homo * tr;
    bl = homo * bl;
    br = homo * br;
    
    
    hb.grow(tl);
    hb.grow(tr);
    hb.grow(bl);
    hb.grow(br);
    
    return hb;
    
}

Canvas Pano::calculateCanvas(ImageBound a, ImageBound b){
    ImageBound ab;
    
    
    ab.grow(Vec3f(a.topleft.x(),  a.topleft.y(), 1));
    ab.grow(Vec3f(a.btnright.x(), a.btnright.y(),1));
    ab.grow(Vec3f(b.topleft.x(),  b.topleft.y(), 1));
    ab.grow(Vec3f(b.btnright.x(), b.btnright.y(),1));
    
    Canvas canv;
    
    // calculate offset of image canvas and the size of canvas
    Vec2i can;
    can << floor(ab.topleft.x()), floor(ab.topleft.y());
    canv.offset = can;
    canv.length = ceil(ab.btnright.x() - ab.topleft.x());
    canv.height = ceil(ab.btnright.y() - ab.topleft.y());
    return canv;
    
}

ImageBound::imagebound(){
    Vec2f v1, v2;
    v1 << INFINITY,INFINITY;
    v2 << -INFINITY,-INFINITY;
    topleft = v1;
    btnright = v2;
}

void ImageBound::grow(Vec3f point){
    Vec2f v1, v2;
    v1 << std::min(topleft.x(), point.x()),std::min(topleft.y(), point.y());
    v2 << std::max(btnright.x(),point.x()),std::max(btnright.y(), point.y());
    topleft = v1;
    btnright = v2;
}







