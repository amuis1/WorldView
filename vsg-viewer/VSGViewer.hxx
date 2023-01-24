/* ------------------------------------------------------------------   */
/*      item            : VSGViewer.hxx
        made by         : Rene van Paassen
        date            : 090616
        category        : header file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#ifndef VSGViewer_hxx
#define VSGViewer_hxx

#include <string>
#include <map>
#include <vector>
#include "VSGObject.hxx"

#ifdef OWN_WIN_AND_CONTEXT
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#endif

#include <vsg/all.h>
#include <vsgXchange/all.h>
#include <WorldViewerBase.hxx>
#include "VSGObjectFactory.hxx"

// Used the vsgwindows example as inspiration

/** This is a wrapper that can load and display VSG scenes,
    optionally in multiple windows and viewports. It is inteded for
    encapsulation in a DUECA module, but can also be used stand-alone
    (see the test souce main.cpp in this directory). */
class VSGViewer: public WorldViewerBase
{
  // Advance definition, collection of data for a window.
  struct WindowSet;

  // Advance definition, collection of data for a viewport
  struct Private;

  /** Specification for the render windows. */
  std::list<WinSpec> winspec;

  /** scene manager */
  vsg::ref_ptr<vsg::Group>  root;

  /** A single viewer, matching a single scene */
  vsg::ref_ptr<vsgViewer::Viewer> oviewer;

  /** A composite viewer, flexibility multiple scenes */
  vsg::ref_ptr<vsgViewer::CompositeViewer> cviewer;

  /** counter dynamical creation */
  unsigned config_dynamic_created;

protected:
  /** Composite viewer used */
  bool use_compositeviewer;

  /** Accept unknown/unconfigured objects */
  bool allow_unknown;
private:



  /** This class can generate multiple views on the same world. A
      ViewSet encapsulates the stuff needed for a single view. */
  struct ViewSet {

    /** Name, for debugging purposes. */
    std::string name;

    /** The render window */
    vsg::ref_ptr<vsg::Camera> camera;

    /** Rotation offset */
    vsg::Matrix view_offset;

    /** View use, if compositeviewer */
    vsg::ref_ptr<vsgViewer::Viewer> sview;

    /** Constructor */
    ViewSet();

    /** Initialise */
    void init(const ViewSpec& vs, WindowSet& window,
              vsg::ref_ptr<vsgViewer::CompositeViewer> cviewer,
              vsg::ref_ptr<vsgViewer::Viewer> viewer,
              vsg::ref_ptr<vsg::Group> root,
              int zorder, const std::vector<double>& bg_color,
              vsg::Camera::Camera::DrawCallback *cb = NULL);

    /** create the camera and window. */
    void complete();
  };

  /** Each render window needs a specification and possibly a set of
      views */
  struct WindowSet {

    std::string name;

    /** Traits for this thing, to remember */
    vsg::ref_ptr<vsg::GraphicsContext::Traits> traits;

    /** Graphics context for this window/this window itself */
    vsg::ref_ptr<vsg::GraphicsContext> gc;

    /** Compositeviewer, one per window */
    vsg::ref_ptr<vsgViewer::CompositeViewer> cviewer;

    /** A list of view sets; these represent the different render
        areas within the window */
    std::map<std::string,ViewSet> viewset;

    /** Constructor */
    WindowSet() { }
  };

  /** Map of windows */
  typedef std::map<std::string,WindowSet> WindowsMap;

  /** map with windows. */
  WindowsMap windows;

  /** Object list type */
  typedef std::list< boost::intrusive_ptr<VSGObject> > ObjectListType;

  /** Map with created (due to presence in the world channel)
      objects. They are indexed with channel entry index, and removed
      from the map when the entry is removed from the channel. */
  typedef std::map<creation_key_t,
                   boost::intrusive_ptr<VSGObject> > created_objects_t;

  /** Objects creates automatically */
  created_objects_t active_objects;

  /** Objects that are static, just get calls about new positioning */
  ObjectListType static_objects;

  /** Objects that need post-draw access */
  ObjectListType post_draw;

  /** Helper */
  WindowSet myCreateWindow(const WinSpec &ws, vsg::ref_ptr<vsg::Group> root);

  /** List of specifications for the wiews, will be applied later */
  std::list<ViewSpec> viewspec;

  /** Global draw callback, for all views. */
  vsg::Camera::Camera::DrawCallback *global_draw_callback;

  /** View-specific draw callbacks, per viewspec name. */
  std::map<string, vsg::Camera::Camera::DrawCallback *> draw_callbacks;

public:
  /** Setting of global draw callback. */
  void setDrawCallback(vsg::Camera::Camera::DrawCallback *cb) {global_draw_callback=cb;}

  /** Setting of viewspec-specific draw callback. */
  void setDrawCallback(const string& view_spec_name, vsg::Camera::Camera::DrawCallback *cb);

  /** Set a view-specific draw callback. If the view is not specified,
      set the callback for all views. */
  void installPostDrawCallback(vsg::Camera::Camera::DrawCallback *cb,
                               const std::string& viewname="");
private:
  /** Helper function, loads resources from resources.cfg */
  void setupResources();

public:
  /** Constructor */
  VSGViewer();

  /** Destructor */
  ~VSGViewer();

  /** Open the window(s) */
  void init(bool waitswap);

  /** Add a window */
  void addViewport(const ViewSpec& vp);

  /** Add a window */
  void addWindow(const WinSpec& window) {winspec.push_back(window);}

  /** set the base camera position */
  void setBase(TimeTickType tick, const BaseObjectMotion& base, double late);

  /** Create a controllable object. Object creation depends on class of
      data supplied, further init may rely on fist data entering. */
  bool createControllable
  (const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
   uint32_t creation_id, const std::string& data_class,
   const std::string& entry_label, Channel::EntryTimeAspect time_aspect);

  /** Remove a controllable */
  void removeControllable(const dueca::NameSet& cname,
                          uint32_t creation_id);

  /** Create a static (not controlled) controllable object. */
  bool createStatic(const std::vector<std::string>& classname);

  /** Do a re-draw
      \param wait   If true, do now swap the buffers. The application
                    must later wait and swap with the waitSwap function. */
  void redraw(bool wait = false, bool reset_context = false);

  /** Wait for the swap. */
  void waitSwap();

  /** Change the configuration of the scene graph, returns true if
      successful */
  bool adaptSceneGraph(const WorldViewConfig& adapt);

  /** Get back the main camera of a view, for initialisation */
  vsg::ref_ptr<vsg::Camera> getMainCamera(const std::string& wname,
                                          const std::string& vname);

protected:
  /** Path to the resources */
  std::string resourcepath;

  /** Keep the cursor or not? */
  bool keep_pointer;

  /** background/clear color */
  std::vector<double> bg_color;

  /** fog */
  enum FogMode {
    Off,
    Linear,
    Exponential,
    Exponential2
  };

  /** Fog mode */
  FogMode fog_mode;

  /** Fog density */
  double fog_density;

  /** Fog colour */
  vsg::Vec4 fog_colour;

  /** Fog start and end */
  double fog_start, fog_end;
};


#endif
