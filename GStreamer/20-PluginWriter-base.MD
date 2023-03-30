# <a href="https://gstreamer.freedesktop.org/documentation/plugin-development/index.html?gi-language=c"> 插件编写者指南 </a>

<details open>
<summary>介绍</summary>
<p> 
- 在本指南中，您将学习如何应用基本的 GStreamer 编程概念来编写一个简单的插件。 可用于 视音频的 AI 处理。 
</p> 
</details>

<details open>
<summary> 编写插件的基础流程 </summary><p> 

1. 构建样板

    - 通过下载<a href="https://gitlab.freedesktop.org/gstreamer/gst-template/-/archive/1.18/gst-template-1.18.zip">模板参考</a>，重写、添加相应的函数
        - $ cd ~/gst-template/gst-plugin/src
        - $ ../tools/make_element MyFilter
        - 大写对于插件的名称很重要。请记住，在某些操作系统下，大写通常在指定目录和文件名时也很重要。 
    - 得到 gstmyfilter.c和gstmyfilter.h.
```c 
    //  数据结构 && 双构造函数（gst_XXX_class_init + XXX_init ）

    typedef struct _GstMyFilter {
        GstElement element;

        GstPad *sinkpad, *srcpad;

        gboolean silent; 
    } GstMyFilter;

    gst_XXX_class_init(){
    // 1. 设置属性  
        gst_my_filter_set_property();
        gst_my_filter_set_property();
        g_object_class_install_property() ; 
    // 2. 元素元数据  元素详细信息 数据提供额外的元素信息。
        gst_element_class_set_details_simple() || 
        gst_element_class_set_metadata() || gst_element_class_set_static_metadata();   
    // 3. pad模版： GstStaticPadTemplate 是对元素将（或可能）创建和使用的pad的描述。
        static GstStaticPadTemplate sink_factory =
        GST_STATIC_PAD_TEMPLATE (
            "sink",
            GST_PAD_SINK,
            GST_PAD_ALWAYS,
            GST_STATIC_CAPS ("ANY")  // or others
        ); 
        gst_element_class_add_pad_template ( gst_static_pad_template_get() ). 
    // 4. 管理过滤器状态
        element_class->change_state = gst_my_filter_change_state;
    // 5. 
    }; 

    xxxxxx_init(){
        GST_DEBUG_CATEGORY_INIT();
        gst_element_register();
    /* 一旦我们编写了定义插件所有部分的代码，我们就需要编写 plugin_init() / xxxxxx_init() 函数。
     * 这是一个特殊的函数，它在插件加载后立即被调用，并且应该返回 TRUE 或FALSE，
     * 具体取决于它是否正确加载了任何依赖项。
     * 此外，在此函数中，应注册插件中任何受支持的元素类型。
     * 返回的信息将缓存在中央注册表中。
     */
    }GST_PLUGIN_DEFINE(); 
``` 
 
2. 自定义/指定 pad: 数据进出元素的端口

    -  静态 pad 模板如何将 pad 模板注册到元素类。
    -  事件功能：_event () -function 配置特定格式以及如何注册函数以让数据流过元素。
    -  创建 pad 后，您必须设置一个 _chain ()函数指针，该指针将接收和处理 sinkpad 上的输入数据。
    -  dequery 询问功能：

```c
static void
gst_my_filter_init (GstMyFilter *filter)
{ 
  filter->sinkpad = gst_pad_new_from_static_template (
    &sink_template, "sink");// "src"  is same
 
//   gst_pad_set_event_function (filter->sinkpad,
//       GST_DEBUG_FUNCPTR (gst_my_filter_sink_event));
//   gst_pad_set_chain_function (filter->sinkpad,
//       GST_DEBUG_FUNCPTR (gst_my_filter_chain));
//   gst_pad_set_query_function (filter->sinkpad,
//       GST_DEBUG_FUNCPTR (gst_my_filter_src_query)); 


  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
    
  filter->silent = FALSE;
}
```

3.  PAD ->_chain 功能:  是进行所有<STRONG>数据处理</STRONG>的函数。
    - SISO 函数，线性函数
    - 因此对于每个传入缓冲区，也会有一个缓冲区流出。
    - 缓冲区并不总是可写的。 
    
```c
    static GstFlowReturn
    gst_my_filter_chain(GstPad    *pad,   GstObject *parent, GstBuffer *buf){
        // ...
        return gst_pad_push (filter->srcpad, buf);
    }
```

4. PAD ->_事件功能: 事件函数
    - 会通知您数据流中发生的特殊事件（例如 caps、end-of-stream、newsegment、tags 等）。
    - 事件可以在上游和下游传播，因此您可以在接收垫和源垫上接收它们。

```c
static gboolean gst_my_filter_sink_event (GstPad    *pad,
                                          GstObject *parent,
                                          GstEvent  *event){
    switch (GST_EVENT_TYPE (event)) {
        
    default: 
        ret = gst_pad_event_default (pad, parent, event);
        break;
    }
  return ret;
};
```
 
5. PAD -> dequery() 查询功能 必须回复 
    - 诸如位置、持续时间之类的查询，
    - 查询可以在上游和下游传播， 

    ```c  
    static gboolean
    gst_my_filter_src_query (GstPad    *pad,
                    GstObject *parent,
                    GstQuery  *query){
        switch (GST_QUERY_TYPE (query)) {
            
        default: 
            ret = gst_pad_query_default (pad, parent, query);
            break;
        }
        return ret;
    };
    ```
 

6. 4 种 states 状态：状态描述了元素实例是否已初始化、是否准备好传输数据以及当前是否正在处理数据
    - GST_STATE_NULL 
        GST_STATE_NULL是元素的默认状态。
        在这种状态下，它没有分配任何运行时资源，它没有加载任何运行时库，它显然不能处理数据。 
    - GST_STATE_READY 
        GST_STATE_READY是元素可以处于的下一个状态。
        在 READY 状态下，元素分配了所有默认资源（运行时库、运行时内存）。但是，它还没有分配或定义任何特定于流的东西。
        当从 NULL 变为 READY 状态 ( GST_STATE_CHANGE_NULL_TO_READY) 时，元素应分配任何非流特定资源并应加载运行时可加载库（如果有）。
        当反过来（从 READY 到 NULL, GST_STATE_CHANGE_READY_TO_NULL）时，元素应该卸载这些库并释放所有分配的资源。
        这种资源的例子是硬件设备。
        请注意，文件通常是流，因此应将它们视为特定于流的资源；因此，不应在此状态下分配它们。
    - GST_STATE_PAUSED  
        GST_STATE_PAUSED是元素准备好接受和处理数据的状态。
        对于大多数元素，此状态与 PLAYING 相同。此规则的唯一例外是接收器元素。接收器元素只接受一个数据缓冲区然后阻塞。此时管道已“预卷”并准备好立即呈现数据。
    - GST_STATE_PLAYING 
        GST_STATE_PLAYING是元素可以处于的最高状态。对于大多数元素，此状态与 PAUSED 完全相同，
        它们接受并处理带有数据的事件和缓冲区。只有 sink 元素需要区分 PAUSED 和 PLAYING 状态。在 PLAYING 状态下，sink 元素实际渲染传入的数据，例如将音频输出到声卡或将视频图片渲染到图像接收器。

    1. 派生自新的基类之一, 重写基类的 start() 和 stop() 虚函数

    2. 从 GstElement 或其他未构建在基类之上的类派生: 实现自己的状态更改函数才能收到状态更改的通知. as demuxer 或 muxer 
        - 可以通过虚函数指针通知元素状态变化。在此函数中，元素可以初始化元素所需的任何类型的特定数据，并且可以选择无法从一种状态转到另一种状态。
        - 不要为未处理的状态更改 g_assert；这由 GstElement 基类处理。


```c
    static GstStateChangeReturn
    gst_my_filter_change_state (GstElement *element, GstStateChange transition){

        switch (transition) {
            case GST_STATE_CHANGE_NULL_TO_READY:
            case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        };
        /*  向上 (NULL=>READY, READY=>PAUSED, PAUSED=>PLAYING)
         *  向下 (PLAYING=>PAUSED, PAUSED=>READY, READY=>NULL) 状态更改在两个单独的switch块中处理
         * 向下状态更改只有在我们链接到父类的状态更改函数之后才能处理。为了安全地处理多个线程的并发访问 
         */
        switch (transition) {
            case GST_STATE_CHANGE_READY_TO_NULL:
        }; 

        GST_ELEMENT_CLASS (parent_class)->change_state (element, transition) == GST_STATE_CHANGE_FAILURE;  
    };  
```

7. 属性 成员变量 控制
    - 控制元素行为方式的主要和最重要的方法 

```c
static void
gst_my_filter_set_property (GObject      *object,
                guint         prop_id,
                const GValue *value,
                GParamSpec   *pspec);
static void
gst_my_filter_get_property (GObject    *object,
                guint       prop_id,
                GValue     *value,
                GParamSpec *pspec);
```

8. signal
    - GObject signal 可用于通知应用程序特定于该对象的事件。

9. 测试 plugin 的程序代码

    初始化 GStreamer 核心库 gst_init ()/gst_init_get_option_group ()
    
    使用 创建元素gst_element_factory_make ()

    链接期间-运行期间
    
    永远不要忘记清理插件或测试应用程序中的内存。
    当进入 NULL 状态时，你的元素应该清理分配的内存和缓存 应该关闭所有对可能的支持库的引用 应用程序应该unref ()通过管道并确保它不会崩溃。
```c
#include <gst/gst.h>

static gboolean
bus_call (GstBus     *bus,
      GstMessage *msg,
      gpointer    data)
{
  GMainLoop *loop = data;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End-of-stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR: {
      gchar *debug = NULL;
      GError *err = NULL;

      gst_message_parse_error (msg, &err, &debug);

      g_print ("Error: %s\n", err->message);
      g_error_free (err);

      if (debug) {
        g_print ("Debug details: %s\n", debug);
        g_free (debug);
      }

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

gint
main (gint   argc,
      gchar *argv[])
{
  GstStateChangeReturn ret;
  GstElement *pipeline, *filesrc, *decoder, *filter, *sink;
  GstElement *convert1, *convert2, *resample;
  GMainLoop *loop;
  GstBus *bus;
  guint watch_id;

  /* initialization */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);
  if (argc != 2) {
    g_print ("Usage: %s <mp3 filename>\n", argv[0]);
    return 01;
  }

  /* create elements */
  pipeline = gst_pipeline_new ("my_pipeline");

  /* watch for messages on the pipeline's bus (note that this will only
   * work like this when a GLib main loop is running) */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  filesrc  = gst_element_factory_make ("filesrc", "my_filesource");
  decoder  = gst_element_factory_make ("mad", "my_decoder");

  /* putting an audioconvert element here to convert the output of the
   * decoder into a format that my_filter can handle (we are assuming it
   * will handle any sample rate here though) */
  convert1 = gst_element_factory_make ("audioconvert", "audioconvert1");

  /* use "identity" here for a filter that does nothing */
  filter   = gst_element_factory_make ("my_filter", "my_filter");

  /* there should always be audioconvert and audioresample elements before
   * the audio sink, since the capabilities of the audio sink usually vary
   * depending on the environment (output used, sound card, driver etc.) */
  convert2 = gst_element_factory_make ("audioconvert", "audioconvert2");
  resample = gst_element_factory_make ("audioresample", "audioresample");
  sink     = gst_element_factory_make ("pulsesink", "audiosink");

  if (!sink || !decoder) {
    g_print ("Decoder or output could not be found - check your install\n");
    return -1;
  } else if (!convert1 || !convert2 || !resample) {
    g_print ("Could not create audioconvert or audioresample element, "
             "check your installation\n");
    return -1;
  } else if (!filter) {
    g_print ("Your self-written filter could not be found. Make sure it "
             "is installed correctly in $(libdir)/gstreamer-1.0/ or "
             "~/.gstreamer-1.0/plugins/ and that gst-inspect-1.0 lists it. "
             "If it doesn't, check with 'GST_DEBUG=*:2 gst-inspect-1.0' for "
             "the reason why it is not being loaded.");
    return -1;
  }

  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);

  gst_bin_add_many (GST_BIN (pipeline), filesrc, decoder, convert1, filter,
                    convert2, resample, sink, NULL);

  /* link everything together */
  if (!gst_element_link_many (filesrc, decoder, convert1, filter, convert2,
                              resample, sink, NULL)) {
    g_print ("Failed to link one or more elements!\n");
    return -1;
  }

  /* run */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    GstMessage *msg;

    g_print ("Failed to start up pipeline!\n");

    /* check if there is an error message with details on the bus */
    msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
    if (msg) {
      GError *err = NULL;

      gst_message_parse_error (msg, &err, NULL);
      g_print ("ERROR: %s\n", err->message);
      g_error_free (err);
      gst_message_unref (msg);
    }
    return -1;
  }

  g_main_loop_run (loop);

  /* clean up */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  g_source_remove (watch_id);
  g_main_loop_unref (loop);

  return 0;
}

```
    
</details> 