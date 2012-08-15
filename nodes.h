#ifndef __NODES_H__
#define __NODES_H__

#include <iostream>
#include <stdio.h>

#include <boost/thread.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

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

namespace bm {

// TBD need threshold filter
// resize (or build resizing into input/output
// image directory loading- just a no input buffer after finished (need to resize every loaded image)

// more advanced:
// Buffer inventory- make it easy to put any buffer into the inventory (deepish copy)
// then make them easy to swap it into place where any other buffer exists
// the idea is to navigate to a buffer on input or output, save it to inventory, then swap it into a buffer sourcing
// some patch 

// nesting, keys for moving up or down
//
//kinect support

// Basic nodes are ImageNodes, Signals (numerical), and image Buffers (TBD signal Buffers?)
// second types are vectors of image nodes and signals (probably required to be the same size,
// maybe they could be bundled into std::pairs)
//
// most nodes need to have dedicated input locations for those types so incoming connections know where to go
// each input also might want a string name?
// 
// it was nice to lump all types together in one inputs vector, but that doesn't scale right- should that
// be maintained (where disconnections are managed across both the vector of inputs and the dedicated input
// type connetions?)  Or use pointers to pointers?
//  what about having a map of maps for the inputs - inputs[SIGNAL]["min"]

class Node
{
  // this structure tracks arbitrary numbers of callers to see if there have been
  // change since the last call
  std::map<const void*, std::map<int, bool> > dirty_hash;  
  public:
  // has this node been updated this timestep, or does it need to be updated this timestep
  // because of dependencies
  bool do_update;
  
  bool enable;

  // is the output of this node different from the last  timestep
  //bool is_dirty;
  // has the node changed since the last time the pointer parameter supplied has called this function (and cleared it)
  bool isDirty(const void* caller, 
      const int ind=0, 
      const bool clear_dirty=true);
  
  bool setDirty();

  std::string name;
  cv::Point loc;
  cv::Mat graph;
  cv::Scalar vcol;

  // these are for ui display purposes, shows the current potential connection
  std::string selected_type; 
  std::string selected_port;
  bool draw_selected_port;
  //void drawSelectedPort();
  
  //std::vector<Node*> inputs;
  std::map<std::string, std::map< std::string, Node*> > inputs;
  
  Node();
  virtual ~Node() {}
    
  bool setUpdate();
  
  // the rv is so that an ineritanning function will know whether to 
  // process or not
  virtual bool update(); 

  virtual bool draw(float scale = 0.125); 

  virtual bool save(cv::FileStorage& fs);
  virtual bool load(cv::FileNodeIterator nd);

  void printInputVector();
  std::vector<Node*> getInputVector();

  bool getImage(
    //std::map<std::string, std::map< std::string, Node*> >& inputs,
    const std::string port,
    cv::Mat& image,
    bool& is_dirty);
    //const bool require_dirty= false);
 
  bool getInputPort(
      //std::map<std::string, std::map< std::string, Node*> >& inputs,
      const std::string type, 
      const std::string port,
      Node*& rv);
      
  bool getSignal(
      const std::string port, 
      float& val);

  bool getBuffer(
    const std::string port,
    const float val,
    cv::Mat& image);
};

/////////////////////////////////////////////////
//////////////////////////////////////////////////

class ImageNode : public Node
{
public:
  // TBD make all three private
  cv::Mat out;
  cv::Mat out_old;
  cv::Mat tmp; // scratch image
    
  ImageNode();// : Node()

  virtual bool update();
  // TBD could there be a templated get function to be used in different node types?
  virtual cv::Mat get();

  virtual bool draw(float scale = 0.2);
};

// TBD subclasses of Node that are input/output specific, or make that general somehow?

class Signal : public Node
{
  public:
  Signal(); // : Node()

  void setup(const float new_step=0.01, const float offset=0.0, const float min = 0.0, const float max=1.0); 
 
  virtual bool update();
  virtual bool draw(float scale);

  virtual bool load(cv::FileNodeIterator nd);
  virtual bool save(cv::FileStorage& fs);

  float min;
  float max;
  float value;
  float step;
};


////////////////////////////////
class Buffer : public ImageNode
{
  protected: 
  std::deque<cv::Mat> frames;
  
  public:

  Buffer(); 
  
  int max_size;
 
  virtual bool update();

  bool add(cv::Mat new_frame, bool restrict_size = true);
  virtual bool draw(float scale); 
   
  virtual cv::Mat get();

  cv::Mat get(const float fr);

  // TBD get(int ind), negative ind index from last
  
  virtual bool load(cv::FileNodeIterator nd);
  virtual bool save(cv::FileStorage& fs);

};

};
#endif // ifdef __NODES_H__
