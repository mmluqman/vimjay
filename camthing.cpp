
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <typeinfo>
#include <cxxabi.h> // non portable

#include <deque>
//#include <pair>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "nodes.h" 
#include "filter.h"

using namespace cv;
using namespace std;

DEFINE_string(graph_file, "graph.yml", "yaml file to load with graph in it");

namespace bm {

string getId(Node* ptr) 
  {
    int status; 
    return (abi::__cxa_demangle(typeid(*ptr).name(), 0, 0, &status));
  }

class CamThing
{
  // TBD informal timer for the system
  
  // make sure all Nodes are stored here
  deque<Node*> all_nodes;

  // the final output 
  // TBD make this a special node type
  ImageNode* output;

  public:

  // conveniently create and store node
  template <class nodeType>
    nodeType* getNode(string name = "", cv::Point loc=cv::Point(0.0, 0.0))
    {
      nodeType* node = new nodeType();

      node->name = name;
      node->loc = loc;
      node->graph = graph;

      LOG(INFO) << "new node " << name << " " << loc.x << ", " << loc.y;

      all_nodes.push_back(node);
      return node;
    }

  void clearAllNodeUpdates() 
  {
    for (int i = 0; i < all_nodes.size(); i++) {
      all_nodes[i]->do_update = false;
      // TBD for asynchronous this fails, but need buffering to make that work anyhow
      //all_nodes[i]->is_dirty = false;
    }
  }

  int getIndFromPointer(Node* node)
  {
    for (int i = 0; i < all_nodes.size(); i++) {
      if (all_nodes[i] == node) return i; 
    }

    return -1;
  }
  
  

  /// save the graph to an output yaml file
  bool saveGraph() 
  {
    cv::FileStorage fs("graph.yml", cv::FileStorage::WRITE);

    // TBD save date and time
    time_t rawtime; time(&rawtime);
    fs << "date" << asctime(localtime(&rawtime)); 
    
    fs << "nodes" << "[";
    for (int i = 0; i < all_nodes.size(); i++) {

      fs << "{";
      
      all_nodes[i]->save(fs);
      fs << "ind" << i; //(int64_t)all_nodes[i];   // 
      fs << "inputs" << "[:";
      for (int j = 0; j < all_nodes[i]->inputs.size(); j++) {
        fs << getIndFromPointer(all_nodes[i]->inputs[j]);   
      }
      fs << "]";


      fs << "}";
    }
    fs << "]";
    fs.release();
    return true;
  }

  Webcam* cam_in;
  int count;

  cv::Mat test_im;

  cv::Mat cam_image;
  cv::Mat graph;
 
  int selected_ind;
  Node* selected_node;
  
  CamThing() : selected_ind(0), selected_node(NULL) 
  {
    count = 0;

    // TBD make internal type a gflag
    graph = cv::Mat(cv::Size(1280, 720), MAT_FORMAT_C3);
    graph = cv::Scalar(0);
 
    loadGraph(FLAGS_graph_file);
   
    LOG(INFO) << all_nodes.size() << " nodes total";
    output->loc = cv::Point(graph.cols - (test_im.cols/2+100), 20);
    
    exit(0);
     

    cv::namedWindow("graph", CV_GUI_NORMAL);
    cv::moveWindow("graph", 0, 500);

/*
    // Bring this back when there is a fullscreen/decoration free output window
    cv::namedWindow("out", CV_GUI_NORMAL);
    cv::moveWindow("out", 420, 0);
*/

  }

  bool loadGraph(const std::string graph_file)
  {
    LOG(INFO) << "loading " << graph_file;
    FileStorage fs; 
    fs.open(graph_file, FileStorage::READ);
    
    if (!fs.isOpened()) {
      LOG(ERROR) << "couldn't open " << graph_file;
      return false;
    }
  
    FileNode nd = fs["nodes"]; 
    if (nd.type() != FileNode::SEQ) {
      LOG(ERROR) << "no nodes";

      return false;
    }

    FileNodeIterator it = nd.begin(), it_end = nd.end(); // Go through the node
    for (; it != it_end; ++it) {
      string type_id = (*it)["typeid"];
      string name;
      (*it)["name"] >> name;
      cv::Point loc;
      loc.x = (*it)["loc"][0];
      loc.y = (*it)["loc"][1];
      bool enable;
      (*it)["enable"] >> enable;
      LOG(INFO) << type_id << " " << name << " " << loc << " " << enable;
      

      if (type_id.compare("bm::Webcam") == 0) {
        // TBD make a version of getNode that takes a type_id string
        Webcam* nd = getNode<Webcam>(name, loc);
      }
      else if (type_id.compare("bm::ImageNode") == 0) {
        ImageNode* nd = getNode<ImageNode>(name, loc);
      }
      else if (type_id.compare("bm::Add") == 0) {
        Add* nd = getNode<Add>(name, loc);
      }
      else if (type_id.compare("bm::Rot2D") == 0) {
        Rot2D* nd = getNode<Rot2D>(name, loc);
      }
      else if (type_id.compare("bm::Signal") == 0) {
        Signal* nd = getNode<Signal>(name, loc);
      }
      else if (type_id.compare("bm::Saw") == 0) {
        Saw* nd = getNode<Saw>(name, loc);
      }
    }
  }

  void defaultGraph() 
  {
    ///////////////
    const float advance = 0.1;

    cam_in = getNode<Webcam>("webcam", cv::Point(50, 20) );
    test_im = cv::Mat(cam_in->get().size(), cam_in->get().type());
    test_im = cv::Scalar(200,200,200);

    ImageNode* passthrough = getNode<ImageNode>("image_node_passthrough", cv::Point(400, 50) );
    passthrough->inputs.push_back(cam_in);
    passthrough->out = test_im;
    passthrough->out_old = test_im;
    //output = passthrough;

    Add* add_loop = getNode<Add>("add_loop", cv::Point(600,100) );
    add_loop->out = test_im;

    Rot2D* rotate = getNode<Rot2D>("rotation", cv::Point(400,400));
    rotate->inputs.push_back(add_loop);
    rotate->out = test_im;
    rotate->out_old = test_im;
    rotate->angle = 50.0;
    rotate->center = cv::Point2f(test_im.cols/2, test_im.rows/2);
    //output = rotate;

    Signal* sr = getNode<Saw>("saw_rotate", cv::Point(350,400) ); 
    sr->setup(0.02, -5.0, -5.0, 5.0);
    rotate->inputs.push_back(sr);

    Signal* scx = getNode<Saw>("saw_center_x", cv::Point(350,450) ); 
    scx->setup(5, test_im.cols/2, 0, test_im.cols);
    rotate->inputs.push_back(scx);
    
    Signal* scy = getNode<Saw>("saw_center_y", cv::Point(350,500) ); 
    //scy->setup(6, test_im.rows/2, test_im.rows/2 - 55.0, test_im.rows/2 + 55.0);
    scy->setup(6, test_im.rows/2, 0, test_im.rows);
    rotate->inputs.push_back(scy);

    add_loop->setup(passthrough, rotate, 0.1, 0.89);
    #if 0
    if (false) {
    // test dead branch (shouldn't be updated)
    ImageNode* passthrough2 = getNode<ImageNode>("image_node_passthrough2", cv::Point(400, 450) );
    passthrough2->inputs.push_back(cam_in);
    passthrough2->out = test_im;
    passthrough2->out_old = test_im;
    
    output = passthrough2;
    }

    if (false) {

      // buffer test
      Buffer* cam_buf = getNode<Buffer>("buffer", cv::Point(500,50) );  
      cam_buf->max_size = ( 60 );
      cam_buf->out = test_im;

      output = cam_buf;

      if (false) {
        Add* add = getNode<Add>("addloop", cv::Point(50,500) );
        add->out = test_im;
        add->setup(passthrough, cam_buf, 0.8, 0.2);

        cam_buf->inputs.push_back(add);
      } else {
        cam_buf->inputs.push_back(passthrough);
      }

    
    Signal* s1 = getNode<Saw>("saw", cv::Point(500,400) ); 
    s1->setup(advance, 0);
    //Signal* s1 = getNode<Signal>("fixed signal", cv::Point(300,50) ); 
    //s1->setup(advance, 0);

    Tap* p1 = getNode<Tap>("tap", cv::Point(500,450) );
    //static_cast<Tap*>
    p1->setup(s1, cam_buf);
    p1->out = test_im;

    output = p1;
    } else
    #endif

    #if MAKE_FIR
    {
      FilterFIR* fir = getNode<FilterFIR>("fir_filter", cv::Point(500,150));

      // http://www-users.cs.york.ac.uk/~fisher/cgi-bin/mkfscript
      static float arr[] =  { -1.0, 3.0, -3.0, 1.0, }; //{ 0.85, -0.35, 0.55, -0.05, };
       /* {
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          0.2, -0.1,
          };*/
      /*  { +0.0000000000, -0.0025983155, +0.0000000000, +0.0057162941,
          +0.0000000000, +0.0171488822, +0.0000000000, -0.1200421755,
          -0.0000000000, +0.6002108774, +1.0000000000, +0.6002108774,
          -0.0000000000, -0.1200421755, +0.0000000000, +0.0171488822,
          +0.0000000000, +0.0057162941, +0.0000000000, -0.0025983155,
          +0.0000000000,
        };*/

      vector<float> vec (arr, arr + sizeof(arr) / sizeof(arr[0]) );

      fir->setup(vec);
      fir->inputs.push_back(passthrough);

      // IIR denominator
      FilterFIR* denom = getNode<FilterFIR>("iir_denom", cv::Point(500,350));
      static float arr2[] =  { 
                 1.7600418803, // y[n-1]
                -1.1828932620, // * y[n- 2])
                0.2780599176, // * y[n- 3])
                }; 

      vector<float> vec2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );
      denom->setup(vec2);
      denom->inputs.push_back(fir);
       
      Add* add_iir = getNode<Add>("add_iir", cv::Point(400,100) );
      add_iir->out = test_im;
      add_iir->setup(fir, denom, 1.0, 1.0);
      
      //output = add_iir;
    }
    #endif

    #if 0
    // make a chain, sort of a filter
    bool toggle = false;
    for (float ifr = advance; ifr <= 1.0; ifr += advance ) {

      Signal* s2 = getNode<Saw>("sawl", cv::Point(400.0 + ifr*400.0, 200.0 + ifr*40.0) );
      s2->setup(advance, ifr);

      Tap* p2 = getNode<Tap>("tapl", cv::Point(400.0 + ifr*410.0, 300.0 + ifr*30.0) );
      p2->setup(s2, cam_buf);
      p2->out = test_im;

      Add* add = getNode<Add>("addl", cv::Point(400.0 + ifr*430.0, 400.0 + ifr*10.0) );
      add->out = test_im;
      add->setup(nd, p2, toggle ? 1.5 : 0.5, toggle? -0.5 :0.5);
      toggle = !toggle;
      /*
         Signal* s3 = new Saw(advance, ifr -advance*2.5);
         Tap* p3 = new Tap(s2, cam_buf);

         add = new Add(add, p3, 2.0, -1.0);
         */
      nd = add;
    }
  #endif



    //output = nd;
    //output = p1;
/*
    cv::namedWindow("cam", CV_GUI_NORMAL);
    cv::moveWindow("cam", 0, 0);
*/

    saveGraph();
  }
   


  bool update() {
    count++;
    
    // TBD put this in different thread 
      { 
        VLOG(3) << "";
        output->setUpdate();
        output->update();
      }
    
    const char key = waitKey(20);
    if( key == 'q' )
      return false;

    // TBD look for /, then make next letters type search for nodes with names container the following string
    
    if (key == 's') {
      if (selected_node) selected_node->enable = !selected_node->enable;
    }
    
    if (key == 'j') {
      selected_ind--;
      if (selected_ind < 0) selected_ind = all_nodes.size()-1;
      selected_node = all_nodes[selected_ind];
    }
    if (key == 'k') {
      selected_ind++;
      if (selected_ind >= all_nodes.size()) selected_ind = 0;
      selected_node = all_nodes[selected_ind];
    }

    return true;
  }
  
  void draw() 
  {
    graph = cv::Scalar(0,0,0);
  
    // TBD could all_nodes size have
    if (selected_node) cv::circle(graph, selected_node->loc, 18, cv::Scalar(0,220,1), -1);
    // draw input and outputs
    /*
    imshow("cam", cam_buf->get());

    cv::Mat out = output->get();
    if (out.data) {
      //imshow("out", out);
    } else {
      LOG(ERROR) << "out no data";
    }*/

    // loop through
    for (int i = 0; i < all_nodes.size(); i++) {
      float scale = 0.2;
      if (all_nodes[i] == output) scale = 0.5;
      if (all_nodes[i] == cam_in) scale = 0.5;
      all_nodes[i]->draw(scale);
    } 

    imshow("graph", graph);
  }

  };

}

/*
 * To work with Kinect the user must install OpenNI library and PrimeSensorModule for OpenNI and
 * configure OpenCV with WITH_OPENNI flag is ON (using CMake).

TBD have a mode that takes a webcam, uses brightness as depth, and thresholds it for the valid map

 */
int main( int argc, char* argv[] )
{
 // google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::LogToStderr();
  google::ParseCommandLineFlags(&argc, &argv, false);
  

  bm::CamThing* cam_thing = new bm::CamThing();
  
  bool rv = true;
  while(rv) {
    rv = cam_thing->update();
    cam_thing->draw();
    cam_thing->clearAllNodeUpdates();
  }

  return 0;
}

