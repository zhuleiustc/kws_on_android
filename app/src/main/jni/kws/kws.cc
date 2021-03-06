/*
 * Created on 2018-03-06
 * Author: Zhang Binbin
 */

#include "kws.h"

bool Kws::DetectOnline(const std::vector<float> &wave, bool end_of_stream) {
    feature_pipeline_.AcceptRawWav(wave);
    if (end_of_stream) feature_pipeline_.SetDone();
    std::vector<float> feat;
    int num_frames_ready = feature_pipeline_.ReadFeature(t_, &feat);
    if (num_frames_ready == 0) return false;
    int feat_dim = feature_pipeline_.FeatureDim();
    CHECK(feat_dim == net_.InDim());
    Matrix<float> in(feat.data(), num_frames_ready, feat_dim),
                  out(num_frames_ready, net_.OutDim());
    net_.Forward(in, &out);
  
    bool spot = false;
    float confidence = 0.0;
    for (int i = 0; i < out.NumRows(); i++) {
        bool detected = keyword_spotter_.Spot(out.Row(i).Data(), out.NumCols(), &confidence); 
        //LOG("t %d confidence %f %d", t_+i, confidence, (int)detected);
        if (detected) {
            spot = true;
        }
    }
    t_ += num_frames_ready;
    if (t_ > 1000) { // 10 seconds
        t_ = 0;
        feature_pipeline_.Reset();
    }

    return spot;
}

