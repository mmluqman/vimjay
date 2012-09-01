#ifndef __MISC_NODES_H__
#define __MISC_NODES_H__

#include <iostream>
#include <stdio.h>

#include <boost/thread.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//#include <random>
#include <deque>
#include <map>

// bash color codes
#define CLNRM "\e[0m"
#define CLWRN "\e[0;43m"
#define CLERR "\e[1;41m"
#define CLVAL "\e[1;36m"
#define CLTXT "\e[1;35m"

#define MAT_FORMAT_C3 CV_8UC3
#define MAT_FORMAT CV_8U
//#define MAT_FORMAT_C3 CV_16SC3
//#define MAT_FORMAT CV_16S
//#define MAT_FORMAT_C3 CV_32FC3
//#define MAT_FORMAT CV_32F

#include "nodes.h"

namespace bm {

// TBD make signals.h?
class Saw : public Signal
{
  public:
  Saw(); // : Signal()
  void setup(const float new_step=0.01, const float offset=0.0, const float min =0.0, const float max=1.0); 
  virtual bool update();
  virtual bool handleKey(int key);
};

class Random : public Signal
{
  std::random_device rd;
  std::uniform_real_distribution<> dis;
  std::mt19937 gen;

  public:
  Random(); // : Signal()
  virtual bool update();
  //virtual bool handleKey(int key);
};

// TBD allow multiple?
class Output : public ImageNode
{
  public:
  Output() { cv::Mat out; setImage("in",out);}
  // doesn't have anything special, just a class to be detected with a dynamic_cast upon loading
};

class Rot2D : public ImageNode
{
  public:
  Rot2D();
  virtual bool update();
};

class Webcam : public ImageNode
{

  cv::VideoCapture capture; //CV_CAP_OPENNI );
  void runThread();
  bool is_thread_dirty;
  bool do_capture;
  bool run_thread;
  boost::thread cam_thread;

  int error_count;

  public:
  Webcam();
  virtual ~Webcam();

  virtual bool update();

};


/////////////////////////////////
class ImageDir : public Buffer
{
  public:

  ImageDir() {}

  std::string dir;
  
  bool loadImages();

  virtual bool load(cv::FileNodeIterator nd);
  virtual bool save(cv::FileStorage& fs);
};

///////////////////////////////////////////////////////////
class Tap : public ImageNode
{
  public:

  bool changed;

  //float value;

  Tap();// : ImageNode()

  void setup(Signal* new_signal =NULL, Buffer* new_buffer=NULL); 
  
  virtual bool update();
  
  virtual bool draw();
};

class TapInd : public Tap
{
  public:

  TapInd() {}// : ImageNode()
  
  // TBD make an sval?
  //int ind;
  virtual bool update();
  virtual bool draw();
};

class Add : public ImageNode
{
  public:
  Add(); // : ImageNode()
  // TBD could require pair be passed in to enforce size
  // TBD get rid of this?
  void setup(std::vector<ImageNode*> np, std::vector<float> nf); 
  virtual bool update();
  virtual bool handleKey(int key);
};

class Multiply : public ImageNode
{
  public:
  Multiply(); 
  virtual bool update();
};

class AbsDiff : public ImageNode
{
  public:
  AbsDiff(); 
  virtual bool update();
};

class Resize : public ImageNode
{
  public:
  Resize();

  //float fx;
  //float fy;

  virtual bool update();
  virtual bool draw();
  //virtual bool load(cv::FileNodeIterator nd);
  //virtual bool save(cv::FileStorage& fs);
};


} // bm
#endif // MISC_NODES
