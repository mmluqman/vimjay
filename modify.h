#ifndef __MODIFY_H__
#define __MODIFY_H__

#include <iostream>
#include <stdio.h>

#include <boost/thread.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//#include <random>
#include <deque>
#include <map>

#include "nodes.h"

namespace bm {

class Rot2D : public ImageNode
{
  public:
  Rot2D(const std::string name);
  virtual bool update();
};

class Kaleid : public ImageNode
{
  protected:
  cv::Mat base_x;
  cv::Mat base_y;

  public:
  Kaleid(const std::string name);
  virtual bool update();
};


class Undistort : public ImageNode
{
  public:
  Undistort(const std::string name);
  virtual bool update();
};

class Remap : public ImageNode
{
protected:
// don't expose these to ui with set/getSignals
  cv::Mat base_x;
  cv::Mat base_y;
  cv::Mat base_xy;

  public:
  Remap(const std::string name);
  virtual bool update();
};
///////////////////////////////////////////////////////////
class Tap : public ImageNode
{
  public:

  bool changed;
  //float value;

  Tap(const std::string name);// : ImageNode()

  void setup(Signal* new_signal =NULL, Buffer* new_buffer=NULL); 
  
  virtual bool update();
  virtual bool draw(cv::Point2f ui_offset);
};

class TapInd : public Tap
{
  public:

  TapInd(const std::string name) : Tap(name) {}
  
  // TBD make an sval?
  //int ind;
  virtual bool update();
  virtual bool draw(cv::Point2f ui_offset);
};

// Arbitrary inputs 
class Add : public ImageNode
{
  public:
  Add(const std::string name); 
  // TBD could require pair be passed in to enforce size
  // TBD get rid of this?
  void setup(std::vector<ImageNode*> np, std::vector<float> nf); 
  virtual bool update();
  virtual bool handleKey(int key);
};

// Only 2 inputs 
class AddMasked : public ImageNode
{
  public:
  AddMasked(const std::string name); 
  virtual bool update();
  //virtual bool handleKey(int key);
};

class Multiply : public ImageNode
{
  public:
  Multiply(const std::string name); 
  virtual bool update();
};

// Double inputs
class AbsDiff : public ImageNode
{
  public:
  AbsDiff(const std::string name); 
  virtual bool update();
};

class Max : public ImageNode
{
  public:
  Max(const std::string name); 
  virtual bool update();
};

class Greater : public ImageNode
{
  public:
  Greater(const std::string name); 
  virtual bool update();
};

// Single inputs
class Resize : public ImageNode
{
  public:
  Resize(const std::string name);
  virtual bool update();
};

class Flip : public ImageNode
{
  public:
  Flip(const std::string name);
  virtual bool update();
};

class EqualizeHist : public ImageNode
{
  public:
  EqualizeHist(const std::string name);
  virtual bool update();
};

class Normalize : public ImageNode
{
  public:
  Normalize(const std::string name);
  virtual bool update();
};



} // bm
#endif // MODIFY
