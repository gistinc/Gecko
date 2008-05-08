// XXX: string apis
// - char *, utf8
// - alloc-on-return

// XXX: url apis

// XXX: WinSize

// XXX: exceptions

// XXX ?
const char *NormalizeURI(const char *);

class PlatformView {
  // XXX event synthesis
  // XXX window Tree
  // XXX focus
  // XXX a11y
};

class View {
public:
  PlatformView *getPlatformView();

  // XXX: DOM event bubbling
  Frame *getTopFrame();

  void GetRequestedSize(WinSize *width, WinSize *height);
  void SetPosition(WinPosition x, WinPosition y);
  void SetSize(WinSize width, WinSize height);
  void Show();
  void Hide();

  signal iconChanged();
  signal sizeChanged(WinSize newWidth, WinSize newHeight);
  signal statusChanged(const char *newStatus, StatusFlags flags);
  signal locationChanged(const char *newLocation);
  signal linkClicked(const char *uri);

  // XXX
  signal View *openWindow(const char *uri);
};

enum ReloadFlags {
  RELOAD_NORMAL,
  RELOAD_BYPASS_CACHE,
  RELOAD_BYPASS_PROXY,
  RELOAD_BYPASS_PROXY_AND_CACHE,
  RELOAD_CHARSET_CHANGE,
};

class Request {
  // XXX: event/element that triggered the load
  char *getURI();
  char *getContentType();
  void Cancel();

  signal progress();
  signal finished(status);
};

class Stream {
  void setURI(const char *uri);
  char *getURI();

  void setContentType(const char *uri);
  char *getContentType();

  void begin();
  void feed();
  void end();
};

class Frame {
public:
  View *getView();
  const char *getName();

  const char *getLocation();
  const char *getTitle();

  // Session history
  // Global history
  // Scripting
  // DOM events synthesis

  // XXX: watching the load status?
  LoadProgress *getRequest();
  void loadURI(const char *uri);

  void setContent(const char *baseURI,
                  const char *contentType,
                  PRUint8 *html, PRSize dataLen);

  void loadStream(Stream *stream);

  void stop();
  void reload(ReloadFlags flags = RELOAD_NORMAL);
  void goForward(int steps = 1);
  void goBack(int steps = 1);

  // Watching loads
  signal bool? navigationRequested(Frame *frame, Request *request);

  signal interceptLoad(Request *request);

  Frame *getFrame(const char *name);
  FrameList *getChildren();
};

View *v = new View();

Frame *frame = v->getTopFrame();
frame->loadURI("http://blah.com/");

v->loadURI("http://blah.com/");


onInterceptLoad(Request *request)
{
  Stream *stream = new Stream();

  request->SetStream(stream);

  stream->SetURI(request->GetURI());
  stream->SetContenType("text/html");
  stream->Begin();
  stream->Feed("<html></html");
  stream->End();
}
