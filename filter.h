#ifndef __FILTER_H__
#define __FILTER_H__

#include "nodes.h"
#include "misc_nodes.h"

//#include <iostream>
//#include <stdio.h>
namespace bm {

class FilterFIR : public Buffer
{
  
  public:

  /// filter coefficients
  std::vector<float> xi;

  FilterFIR();

  void setup(const std::vector<float> new_xi);
  virtual bool update();
 
  virtual bool handleKey(int key);

};

class Sobel : public ImageNode
{
  public:
  Sobel();
  virtual bool update();
};

class GaussianBlur : public ImageNode
{
  public:
  GaussianBlur();
  virtual bool update();
};

class OpticalFlow : public Remap
{
  // a CV_32F image, so not suitable for use in a signal/connector
  cv::Mat flow;

  public:
  OpticalFlow();
  virtual bool update();
};


// TBD an IIR could be generated from a FIR chained to another FIR with an add block at the end
// but it would be nice to be able to capture that inside a single Node- how to correctly handle 
// hierarchical nodes?

} // namespace bm
#endif // __FILTER_H__
