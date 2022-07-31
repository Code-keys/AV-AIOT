#include <gst/gst.h>
#include <glib.h>
 
/* Pipeline manipulation
 *      如何将应用程序中的数据插入管道 
 *      如何从管道中读取数据
 *      如何操纵管道的速度、长度和起点
 *      如何监听管道的数据处理。
 */

/* Types probes: 用探针获取信息 以此 采取调度措施等：   访问 pad 的监听器 pad listener.
 *      缓冲区：gst_pad_add_probe () gst_pad_remove_probe ()
 *              scheduling with GST_PAD_PROBE_TYPE_PUSH and GST_PAD_PROBE_TYPE_PULL flags.. 
 *      推送缓冲区列表：GST_PAD_PROBE_TYPE_BUFFER_LIST
 *      事件的传播：GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM和 GST_PAD_PROBE_TYPE_EVENT_UPSTREAM
 *      信息的传递：询问的传递；
 *          GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM and GST_PAD_PROBE_TYPE_QUERY_UPSTREAM   
 *          or  GST_PAD_PROBE_TYPE_QUERY_BOTH 
 *          return with GST_PAD_PROBE_DROP
 *      pad 上没有活动时收到通知: GST_PAD_PROBE_TYPE_IDLE
 */













/* Data probes: 数据流动监测
 *   GST_PAD_PROBE_TYPE_BUFFER和/或GST_PAD_PROBE_TYPE_BUFFER_LISTto gst_pad_add_probe ()用于创建这种探测
 *   用探针获取信息 以此 采取调度措施等：   访问 pad 的监听器 pad listener.
 *      缓冲区：gst_pad_add_probe () gst_pad_remove_probe ()
 *              scheduling with GST_PAD_PROBE_TYPE_PUSH and GST_PAD_PROBE_TYPE_PULL flags.. 
 *      推送缓冲区列表：GST_PAD_PROBE_TYPE_BUFFER_LIST
 *      事件的传播：GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM和 GST_PAD_PROBE_TYPE_EVENT_UPSTREAM
 *      信息的传递：询问的传递；
 *          GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM and GST_PAD_PROBE_TYPE_QUERY_UPSTREAM   
 *          or  GST_PAD_PROBE_TYPE_QUERY_BOTH 
 *          return with GST_PAD_PROBE_DROP
 *      pad 上没有活动时收到通知:GST_PAD_PROBE_TYPE_IDLE
 */ 

// 使用数据探针的示例
#include <gst/gst.h>

static GstPadProbeReturn
cb_have_data (GstPad          *pad,
              GstPadProbeInfo *info,
              gpointer         user_data)
{
  gint x, y;
  GstMapInfo map;
  guint16 *ptr, t;
  GstBuffer *buffer;

  buffer = GST_PAD_PROBE_INFO_BUFFER (info);

  buffer = gst_buffer_make_writable (buffer);

  /* Making a buffer writable can fail (for example if it
   * cannot be copied and is used more than once)
   */
  if (buffer == NULL)
    return GST_PAD_PROBE_OK;

  /* Mapping a buffer can fail (non-writable) */
  if (gst_buffer_map (buffer, &map, GST_MAP_WRITE)) {
    ptr = (guint16 *) map.data;
    /* invert data */
    for (y = 0; y < 288; y++) {
      for (x = 0; x < 384 / 2; x++) {
        t = ptr[384 - 1 - x];
        ptr[384 - 1 - x] = ptr[x];
        ptr[x] = t;
      }
      ptr += 384;
    }
    gst_buffer_unmap (buffer, &map);
  }

  GST_PAD_PROBE_INFO_DATA (info) = buffer;

  return GST_PAD_PROBE_OK;
}

gint
main_18 (gint   argc,
      gchar *argv[])
{
  GMainLoop *loop;
  GstElement *pipeline, *src, *sink, *filter, *csp;
  GstCaps *filtercaps;
  GstPad *pad;

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* build */
  pipeline = gst_pipeline_new ("my-pipeline");
  src = gst_element_factory_make ("videotestsrc", "src");
  if (src == NULL)
    g_error ("Could not create 'videotestsrc' element");

  filter = gst_element_factory_make ("capsfilter", "filter");
  g_assert (filter != NULL); /* should always exist */

  csp = gst_element_factory_make ("videoconvert", "csp");
  if (csp == NULL)
    g_error ("Could not create 'videoconvert' element");

  sink = gst_element_factory_make ("xvimagesink", "sink");
  if (sink == NULL) {
    sink = gst_element_factory_make ("ximagesink", "sink");
    if (sink == NULL)
      g_error ("Could not create neither 'xvimagesink' nor 'ximagesink' element");
  }

  gst_bin_add_many (GST_BIN (pipeline), src, filter, csp, sink, NULL);
  gst_element_link_many (src, filter, csp, sink, NULL);
  filtercaps = gst_caps_new_simple ("video/x-raw",
               "format", G_TYPE_STRING, "RGB16",
               "width", G_TYPE_INT, 384,
               "height", G_TYPE_INT, 288,
               "framerate", GST_TYPE_FRACTION, 25, 1,
               NULL);
  g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);

  pad = gst_element_get_static_pad (src, "src");
  gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
      (GstPadProbeCallback) cb_have_data, NULL, NULL);
  gst_object_unref (pad);

  /* run */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* wait until it's up and running or failed */
  if (gst_element_get_state (pipeline, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) {
    g_error ("Failed to go into PLAYING state");
  }

  g_print ("Running ...\n");
  g_main_loop_run (loop);

  /* exit */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  return 0;
}

////////////////////       在管道中手动添加或删除数据       ///////////////////////  
/* 添加或删除数据
 *      彻底了解状态更改和同步
 *      Or plugin 的插写
 */

/* Inserting yours· data with “appsrc”  
 *    -->  使用 appsrc 插入数据
 *      +---------+     +---------+     
 *      | appsrc  |     | buffer  |     
 *      |        src - sink      src  
 *      +---------+     +---------+ 
 *      appsrc operated in push or pull mode ? by random-access  or other  on  stream-type.
 *      appsrc will push out by caps property of the buffers 
 *      appsrc operates in live mode or not ?  by is-live property (with min-latency and max-latency)
 *      The format of the SEGMENT event that appsrc will push. ?  GST_FORMAT_TIME  or  GST_FORMAT_BYTES
 *      random-access mode ? with size property
 */
 
/* 
    appsrc 处理数据:
        gst_app_src_push_buffer (), 将缓冲区放入队列中，从该队列中appsrc读取其流式线程。 
        发出 push-buffer 动作信号 (不会从执行push-buffer调用的线程发生)
 
    Finally BYTES
        gst_app_src_end_of_stream () 以使其发送EOS下游  
    max-bytes属性 控制在考虑队列已满前 可以排队的数据量。
        填充的内部队列将始终发出 enough-data 信号，该信号向应用程序发出它应该停止将数据推送 . 
        该属性将导致阻塞该方法，直到空闲数据再次可用。 

    当内部队列中的数据用完时，need-data 会发出信号，通知应用程序应该开始将更多数据推送到 appsrc.

    appsrc 可以 seek-data 信号 ;
        在stream-mode属性设置为 seek ableor 时 发出random-access。
        信号参数将包含以 format属性设置的单位表示的流中新的所需位置。
        收到seek-data信号后，应用程序应该从新位置推送缓冲区。 

    这些信号允许应用程序appsrc在推拉模式下运行，如下所述。
 */


/* 推 拉模式下使用 appsrc
 * fed to appsrc from the need-data signal handler
 */
/* 生成黑/白（每秒切换一次）视频到 Xv 窗口输出*/
/* use the pull mode method of pushing new buffers into appsrc although appsrc is running in push mode. */
#include <gst/gst.h> 
static GMainLoop *loop; 
static void
cb_need_data (GstElement *appsrc,
          guint       unused_size,
          gpointer    user_data)
{
  static gboolean white = FALSE;
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  guint size;
  GstFlowReturn ret;

  size = 385 * 288 * 2;

  buffer = gst_buffer_new_allocate (NULL, size, NULL);

  /* this makes the image black/white */
  gst_buffer_memset (buffer, 0, white ? 0xff : 0x0, size);

  white = !white;

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

  timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
  gst_buffer_unref (buffer);

  if (ret != GST_FLOW_OK) {
    /* something wrong, stop pushing */
    g_main_loop_quit (loop);
  }
} 
gint
main_push (gint   argc,
      gchar *argv[])
{
  GstElement *pipeline, *appsrc, *conv, *videosink;

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* setup pipeline */
  pipeline = gst_pipeline_new ("pipeline");
  appsrc = gst_element_factory_make ("appsrc", "source");
  conv = gst_element_factory_make ("videoconvert", "conv");
  videosink = gst_element_factory_make ("xvimagesink", "videosink");

  /* setup */
  g_object_set (G_OBJECT (appsrc), "caps",
        gst_caps_new_simple ("video/x-raw",
                     "format", G_TYPE_STRING, "RGB16",
                     "width", G_TYPE_INT, 384,
                     "height", G_TYPE_INT, 288,
                     "framerate", GST_TYPE_FRACTION, 0, 1,
                     NULL), NULL);
  gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, videosink, NULL);
  gst_element_link_many (appsrc, conv, videosink, NULL);

  /* setup appsrc */
  g_object_set (G_OBJECT (appsrc),
        "stream-type", 0,
        "format", GST_FORMAT_TIME, NULL);
  g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), NULL);

  /* play */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  g_main_loop_run (loop);

  /* clean up */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));
  g_main_loop_unref (loop);

  return 0;
}


/* appsink  抓取数据
 * fed to appsrc from the need-data signal handler
 */
#include <gst/gst.h>
#ifdef HAVE_GTK
#include <gtk/gtk.h>
#endif

#include <stdlib.h>

#define CAPS "video/x-raw,format=RGB,width=160,pixel-aspect-ratio=1/1"

int
main (int argc, char *argv[])
{
  GstElement *pipeline, *sink;
  gint width, height;
  GstSample *sample;
  gchar *descr;
  GError *error = NULL;
  gint64 duration, position;
  GstStateChangeReturn ret;
  gboolean res;
  GstMapInfo map;

  gst_init (&argc, &argv);

  if (argc != 2) {
    g_print ("usage: %s <uri>\n Writes snapshot.png in the current directory\n",
        argv[0]);
    exit (-1);
  }

  /* create a new pipeline */
  descr =
      g_strdup_printf ("uridecodebin uri=%s ! videoconvert ! videoscale ! "
      " appsink name=sink caps=\"" CAPS "\"", argv[1]);
  pipeline = gst_parse_launch (descr, &error);

  if (error != NULL) {
    g_print ("could not construct pipeline: %s\n", error->message);
    g_clear_error (&error);
    exit (-1);
  }

  /* get sink */
  sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");

  /* set to PAUSED to make the first frame arrive in the sink */
  ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
  switch (ret) {
    case GST_STATE_CHANGE_FAILURE:
      g_print ("failed to play the file\n");
      exit (-1);
    case GST_STATE_CHANGE_NO_PREROLL:
      /* for live sources, we need to set the pipeline to PLAYING before we can
       * receive a buffer. We don't do that yet */
      g_print ("live sources not supported yet\n");
      exit (-1);
    default:
      break;
  }
  /* This can block for up to 5 seconds. If your machine is really overloaded,
   * it might time out before the pipeline prerolled and we generate an error. A
   * better way is to run a mainloop and catch errors there. */
  ret = gst_element_get_state (pipeline, NULL, NULL, 5 * GST_SECOND);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_print ("failed to play the file\n");
    exit (-1);
  }

  /* get the duration */
  gst_element_query_duration (pipeline, GST_FORMAT_TIME, &duration);

  if (duration != -1)
    /* we have a duration, seek to 5% */
    position = duration * 5 / 100;
  else
    /* no duration, seek to 1 second, this could EOS */
    position = 1 * GST_SECOND;

  /* seek to the a position in the file. Most files have a black first frame so
   * by seeking to somewhere else we have a bigger chance of getting something
   * more interesting. An optimisation would be to detect black images and then
   * seek a little more */
  gst_element_seek_simple (pipeline, GST_FORMAT_TIME,
      GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH, position);

  /* get the preroll buffer from appsink, this block untils appsink really
   * prerolls */
  g_signal_emit_by_name (sink, "pull-preroll", &sample, NULL);

  /* if we have a buffer now, convert it to a pixbuf. It's possible that we
   * don't have a buffer because we went EOS right away or had an error. */
  if (sample) {
    GstBuffer *buffer;
    GstCaps *caps;
    GstStructure *s;

    /* get the snapshot buffer format now. We set the caps on the appsink so
     * that it can only be an rgb buffer. The only thing we have not specified
     * on the caps is the height, which is dependant on the pixel-aspect-ratio
     * of the source material */
    caps = gst_sample_get_caps (sample);
    if (!caps) {
      g_print ("could not get snapshot format\n");
      exit (-1);
    }
    s = gst_caps_get_structure (caps, 0);

    /* we need to get the final caps on the buffer to get the size */
    res = gst_structure_get_int (s, "width", &width);
    res |= gst_structure_get_int (s, "height", &height);
    if (!res) {
      g_print ("could not get snapshot dimension\n");
      exit (-1);
    }

    /* create pixmap from buffer and save, gstreamer video buffers have a stride
     * that is rounded up to the nearest multiple of 4 */
    buffer = gst_sample_get_buffer (sample);
    /* Mapping a buffer can fail (non-readable) */
    if (gst_buffer_map (buffer, &map, GST_MAP_READ)) {
#ifdef HAVE_GTK
      pixbuf = gdk_pixbuf_new_from_data (map.data,
          GDK_COLORSPACE_RGB, FALSE, 8, width, height,
          GST_ROUND_UP_4 (width * 3), NULL, NULL);

      /* save the pixbuf */
      gdk_pixbuf_save (pixbuf, "snapshot.png", "png", &error, NULL);
#endif
      gst_buffer_unmap (buffer, &map);
    }
    gst_sample_unref (sample);
  } else {
    g_print ("could not make snapshot\n");
  }

  /* cleanup and exit */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (sink);
  gst_object_unref (pipeline);

  exit (0);
}


/* Forcing a format：capsfilter element */

#include <stdlib.h>

#include <gst/gst.h>

#define MAX_ROUND 100

int
main (int argc, char **argv)
{
  GstElement *pipe, *filter;
  GstCaps *caps;
  gint width, height;
  gint xdir, ydir;
  gint round;
  GstMessage *message;

  gst_init (&argc, &argv);

  pipe = gst_parse_launch_full ("videotestsrc ! capsfilter name=filter ! "
             "ximagesink", NULL, GST_PARSE_FLAG_NONE, NULL);
  g_assert (pipe != NULL);

  filter = gst_bin_get_by_name (GST_BIN (pipe), "filter");
  g_assert (filter);

  width = 320;
  height = 240;
  xdir = ydir = -10;

  for (round = 0; round < MAX_ROUND; round++) {
    gchar *capsstr;
    g_print ("resize to %dx%d (%d/%d)   \r", width, height, round, MAX_ROUND);

    /* we prefer our fixed width and height but allow other dimensions to pass
     * as well */
    capsstr = g_strdup_printf ("video/x-raw, width=(int)%d, height=(int)%d",
        width, height);

    caps = gst_caps_from_string (capsstr);
    g_free (capsstr);
    g_object_set (filter, "caps", caps, NULL);
    gst_caps_unref (caps);

    if (round == 0)
      gst_element_set_state (pipe, GST_STATE_PLAYING);

    width += xdir;
    if (width >= 320)
      xdir = -10;
    else if (width < 200)
      xdir = 10;

    height += ydir;
    if (height >= 240)
      ydir = -10;
    else if (height < 150)
      ydir = 10;

    message =
        gst_bus_poll (GST_ELEMENT_BUS (pipe), GST_MESSAGE_ERROR,
        50 * GST_MSECOND);
        // 请注意我们如何使用gst_bus_poll()小超时来获取消息并引入短暂的睡眠。
 
    if (message) {
      g_print ("got error           \n");

      gst_message_unref (message);
    }
  }
  g_print ("done                    \n");

  gst_object_unref (filter);
  gst_element_set_state (pipe, GST_STATE_NULL);
  gst_object_unref (pipe);

  return 0;
}




// 在 PLAYING 管道中更改 元素
// 在线 修改 网络 
//    探针 阻塞 
//    手术
//    解锁 探针

#include <gst/gst.h>

static gchar *opt_effects = NULL;

#define DEFAULT_EFFECTS "identity,exclusion,navigationtest," \
    "agingtv,videoflip,vertigotv,gaussianblur,shagadelictv,edgetv"

static GstPad *blockpad;
static GstElement *conv_before;
static GstElement *conv_after;
static GstElement *cur_effect;
static GstElement *pipeline;

static GQueue effects = G_QUEUE_INIT;

static GstPadProbeReturn
event_probe_cb (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
  GMainLoop *loop = user_data;
  GstElement *next;

  if (GST_EVENT_TYPE (GST_PAD_PROBE_INFO_DATA (info)) != GST_EVENT_EOS)
    return GST_PAD_PROBE_PASS;

  gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

  /* push current effect back into the queue */
  g_queue_push_tail (&effects, gst_object_ref (cur_effect));
  /* take next effect from the queue */
  next = g_queue_pop_head (&effects);
  if (next == NULL) {
    GST_DEBUG_OBJECT (pad, "no more effects");
    g_main_loop_quit (loop);
    return GST_PAD_PROBE_DROP;
  }

  g_print ("Switching from '%s' to '%s'..\n", GST_OBJECT_NAME (cur_effect),
      GST_OBJECT_NAME (next));

  gst_element_set_state (cur_effect, GST_STATE_NULL);

  /* remove unlinks automatically */
  GST_DEBUG_OBJECT (pipeline, "removing %" GST_PTR_FORMAT, cur_effect);
  gst_bin_remove (GST_BIN (pipeline), cur_effect);

  GST_DEBUG_OBJECT (pipeline, "adding   %" GST_PTR_FORMAT, next);
  gst_bin_add (GST_BIN (pipeline), next);

  GST_DEBUG_OBJECT (pipeline, "linking..");
  gst_element_link_many (conv_before, next, conv_after, NULL);

  gst_element_set_state (next, GST_STATE_PLAYING);

  cur_effect = next;
  GST_DEBUG_OBJECT (pipeline, "done");

  return GST_PAD_PROBE_DROP;
}

static GstPadProbeReturn
pad_probe_cb (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
  GstPad *srcpad, *sinkpad;

  GST_DEBUG_OBJECT (pad, "pad is blocked now");

  /* remove the probe first */
  gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

  /* install new probe for EOS */
  srcpad = gst_element_get_static_pad (cur_effect, "src");
  gst_pad_add_probe (srcpad, GST_PAD_PROBE_TYPE_BLOCK |
      GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, event_probe_cb, user_data, NULL);
  gst_object_unref (srcpad);

  /* push EOS into the element, the probe will be fired when the
   * EOS leaves the effect and it has thus drained all of its data */
  sinkpad = gst_element_get_static_pad (cur_effect, "sink");
  gst_pad_send_event (sinkpad, gst_event_new_eos ());
  gst_object_unref (sinkpad);

  return GST_PAD_PROBE_OK;
}

static gboolean
timeout_cb (gpointer user_data)
{
  gst_pad_add_probe (blockpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
      pad_probe_cb, user_data, NULL);

  return TRUE;
}

static gboolean
bus_cb (GstBus * bus, GstMessage * msg, gpointer user_data)
{
  GMainLoop *loop = user_data;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR:{
      GError *err = NULL;
      gchar *dbg;

      gst_message_parse_error (msg, &err, &dbg);
      gst_object_default_error (msg->src, err, dbg);
      g_clear_error (&err);
      g_free (dbg);
      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }
  return TRUE;
}

int
main (int argc, char **argv)
{
  GOptionEntry options[] = {
    {"effects", 'e', 0, G_OPTION_ARG_STRING, &opt_effects,
        "Effects to use (comma-separated list of element names)", NULL},
    {NULL}
  };
  GOptionContext *ctx;
  GError *err = NULL;
  GMainLoop *loop;
  GstElement *src, *q1, *q2, *effect, *filter1, *filter2, *sink;
  gchar **effect_names, **e;

  ctx = g_option_context_new ("");
  g_option_context_add_main_entries (ctx, options, NULL);
  g_option_context_add_group (ctx, gst_init_get_option_group ());
  if (!g_option_context_parse (ctx, &argc, &argv, &err)) {
    g_print ("Error initializing: %s\n", err->message);
    g_clear_error (&err);
    g_option_context_free (ctx);
    return 1;
  }
  g_option_context_free (ctx);

  if (opt_effects != NULL)
    effect_names = g_strsplit (opt_effects, ",", -1);
  else
    effect_names = g_strsplit (DEFAULT_EFFECTS, ",", -1);

  for (e = effect_names; e != NULL && *e != NULL; ++e) {
    GstElement *el;

    el = gst_element_factory_make (*e, NULL);
    if (el) {
      g_print ("Adding effect '%s'\n", *e);
      g_queue_push_tail (&effects, el);
    }
  }

  pipeline = gst_pipeline_new ("pipeline");

  src = gst_element_factory_make ("videotestsrc", NULL);
  g_object_set (src, "is-live", TRUE, NULL);

  filter1 = gst_element_factory_make ("capsfilter", NULL);
  gst_util_set_object_arg (G_OBJECT (filter1), "caps",
      "video/x-raw, width=320, height=240, "
      "format={ I420, YV12, YUY2, UYVY, AYUV, Y41B, Y42B, "
      "YVYU, Y444, v210, v216, NV12, NV21, UYVP, A420, YUV9, YVU9, IYU1 }");

  q1 = gst_element_factory_make ("queue", NULL);

  blockpad = gst_element_get_static_pad (q1, "src");

  conv_before = gst_element_factory_make ("videoconvert", NULL);

  effect = g_queue_pop_head (&effects);
  cur_effect = effect;

  conv_after = gst_element_factory_make ("videoconvert", NULL);

  q2 = gst_element_factory_make ("queue", NULL);

  filter2 = gst_element_factory_make ("capsfilter", NULL);
  gst_util_set_object_arg (G_OBJECT (filter2), "caps",
      "video/x-raw, width=320, height=240, "
      "format={ RGBx, BGRx, xRGB, xBGR, RGBA, BGRA, ARGB, ABGR, RGB, BGR }");

  sink = gst_element_factory_make ("ximagesink", NULL);

  gst_bin_add_many (GST_BIN (pipeline), src, filter1, q1, conv_before, effect,
      conv_after, q2, sink, NULL);

  gst_element_link_many (src, filter1, q1, conv_before, effect, conv_after,
      q2, sink, NULL);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  loop = g_main_loop_new (NULL, FALSE);

  gst_bus_add_watch (GST_ELEMENT_BUS (pipeline), bus_cb, loop);

  g_timeout_add_seconds (1, timeout_cb, loop);

  g_main_loop_run (loop);

  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  return 0;
}