#ifndef __NODES_H__
#define __NODES_H__

#include <iostream>
#include <sstream>
#include <stdio.h>

#include <boost/thread.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <deque>
#include <map>

#include "utility.h"

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


static bool bool_val;

class Node;

enum conType {  
  NONE,
  SIGNAL,
  IMAGE,
  BUFFER
};

// TBD original chose this type because of access convenience, but since 
// accessor function prevent others from using it directly then the convenience 
// is irrelavent.  Probably should just have a vector of structs
class Connector
{
  public:
  Connector();

  bool update();

  // the Connector that is sourcing this one, if any
  Connector* src;

  Node* parent;

  // src types
  std::string name;
  conType type;
 
  cv::Point2f loc;
  
  std::vector<cv::Point2f> connector_points;

  void draw(cv::Mat, cv::Point2f ui_offset);
  bool highlight;

  bool writeable;  

  // TBD 
  bool is_dirty;
  bool set_dirty;

  // TBD could even have a float val or Mat here to store the last value
  // only used if conType == Signal, TBD subclass?
  float value;
  // only used if conType == Image or Buffer
  cv::Mat im;
};

//typedef std::map<std::string, std::pair<Node*, std::string> > inputsItemType;
//typedef std::map<std::string, inputsItemType > inputsType;

class Node
{
  // this structure tracks arbitrary numbers of callers to see if there have been
  // change since the last call
  std::map<const void*, std::map<int, bool> > dirty_hash;  
 
  // velocity and acceleration of node screen position 
  cv::Point2f acc;
  cv::Point2f vel;

  protected:

  public:
  
  std::vector<Connector*> ports;
  
  // has this node been updated this timestep, or does it need to be updated this timestep
  // because of dependencies
  bool do_update;

  // is the output of this node different from the last  timestep
  //bool is_dirty;
  // has the node changed since the last time the pointer 
  // parameter supplied has called this function (and cleared it)
  bool isDirty(
      const void* caller, 
      const int ind=0, 
      const bool clear_dirty=true);
  
  bool setDirty();

  std::string name;
  cv::Point2f loc;
  cv::Mat graph;
  cv::Scalar vcol;

  // these are for ui display purposes, shows the current potential connection
  int selected_port_ind;
  std::string selected_port;
  conType selected_type;
  bool draw_selected_port;
  //void drawSelectedPort();
  
 
  Node();

  //Node(std::string name, cv::Point loc, cv::Mat graph ); 
  
  // TBD need to delete all the connectors
  virtual ~Node() {}
    
  bool setUpdate();
  
  // the rv is so that an ineritanning function will know whether to 
  // process or not
  virtual bool update(); 

  virtual bool draw(cv::Point2f ui_offset); 

  virtual bool save(cv::FileStorage& fs);
  virtual bool load(cv::FileNodeIterator nd);

  bool getPrevPort(const conType type=NONE);
  bool getNextPort(const conType type=NONE); 
  
  bool getInputPort(
      const conType type, 
      const std::string port,
      Connector*& con,
      std::string& src_port);

  void setInputPort(
      const conType type, 
      const std::string port,
      Node* src_node = NULL,
      const std::string src_port = "" 
    );

  // TBD calling any of these will create the input, so outside
  // nodes probably shouldn't call them?
  cv::Mat getImage(
      const std::string port,
      bool& valid = bool_val);//,
      //bool& is_dirty);
      //const bool require_dirty= false);
  
  // set image, only succeeds if not an input TBD - rw permissions?
  bool setImage(const std::string port, cv::Mat& im);

  float getSignal(
      const std::string port, 
      bool& valid = bool_val);

  bool setSignal(const std::string port, const float val);

  cv::Mat getBuffer(
    const std::string port,
    const float val);
    //cv::Mat& image);

  cv::Mat getBuffer(
    const std::string port,
    const int val);
    //cv::Mat& image);

  virtual bool handleKey(int key);
};

/////////////////////////////////////////////////
//////////////////////////////////////////////////

class ImageNode : public Node
{
public:
  // TBD make all three private
  //cv::Mat out;
  //cv::Mat out_old;
  //cv::Mat tmp; // scratch image
  //int write_count;
    
  ImageNode();// : Node()
  //ImageNode(std::string name, cv::Point loc, cv::Mat graph ); 

  virtual bool update();

  virtual bool draw(cv::Point2f ui_offset);
  
  virtual bool handleKey(int key);
  
  std::stringstream dir_name;
  virtual bool writeImage();
};

// TBD subclasses of Node that are input/output specific, or make that general somehow?

class Signal : public Node
{
  public:
  Signal(); // : Node()

  void setup(const float new_step=0.01, const float offset=0.0, const float min = 0.0, const float max=1.0); 
 
  virtual bool handleKey(int key);
  virtual bool update();
  virtual bool draw(cv::Point2f ui_offset);

  virtual bool load(cv::FileNodeIterator nd);
  virtual bool save(cv::FileStorage& fs);

/*
  float min;
  float max;
  float value;
  float step;
  */
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

  bool add(cv::Mat& new_frame, bool restrict_size = true);
  virtual bool draw(cv::Point2f ui_offset);
   
  virtual cv::Mat get();

  cv::Mat get(const float fr);
  cv::Mat get(int ind);

  // TBD get(int ind), negative ind index from last
  
  virtual bool load(cv::FileNodeIterator nd);
  virtual bool save(cv::FileStorage& fs);

  virtual bool writeImage();
};

  

};
#endif // ifdef __NODES_H__
