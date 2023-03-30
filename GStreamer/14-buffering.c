// 缓冲的目的是在管道中积累足够的数据，以便播放可以顺利进行，不会中断。

// GST_STATE_PLAYING  、  GST_STATE_PAUSED

/*  Buffering strategies
        No-rebuffer strategy
            我们希望在管道中缓冲足够的数据，以便播放继续而不会中断。
            实现这一点我们需要知道的是知道文件中的总剩余播放时间和总剩余下载时间。如果缓冲时间小于播放时间，我们可以不中断地开始播放。

            我们可以通过 DURATION、POSITION 和 BUFFERING 查询获得所有这些信息。
            我们需要定期执行缓冲查询来获取当前的缓冲状态。在最坏的情况下，我们还需要有足够大的缓冲区来保存完整的文件。
            最好将此缓冲策略与下载缓冲一起使用 
*/

/*  1.  Stream Buffering：

        +---------+     +---------+     +-------+
        | httpsrc |     | buffer  |     | demux |
        |        src - sink      src - sink     ....
        +---------+     +---------+     +-------+
    buffer: 波动 0~100%的 BUFFERING，指示应用程序继续播放
            以补偿网络不规则性
            可以插入管道中的任何位置
*/

/*  2.  Downloading Buffering：

      +---------+     +---------+     +-------+
      | httpsrc |     | buffer  |     | demux |
      |        src - sink      src - sink     ....
      +---------+     +----|----+     +-------+
                           V
                          file-total 
    buffer: 缓冲区元素将为解复用器提供基于推送或拉取的 srcpad 以在下载的文件中导航。
*/

/*  3.  Timeshift Buffering：

      +---------+     +---------+     +-------+
      | httpsrc |     | buffer  |     | demux |
      |        src - sink      src - sink     ....
      +---------+     +----|----+     +-------+
                           V
                          file-ringbuffer
    buffer: 缓冲区元素将为 循环的 前进几秒 和后退几秒。
*/ 

/*  4.  Live buffering
        捕获和播放元素之间引入一些固定延迟
*/ 

#include <gst/gst.h>

GstState target_state;
static gboolean is_live;
static gboolean is_buffering;

static gboolean
buffer_timeout (gpointer data)
{
  GstElement *pipeline = data;
  GstQuery *query;
  gboolean busy;
  gint percent;
  gint64 estimated_total;
  gint64 position, duration;
  guint64 play_left;

  query = gst_query_new_buffering (GST_FORMAT_TIME);

  if (!gst_element_query (pipeline, query))
    return TRUE;

  gst_query_parse_buffering_percent (query, &busy, &percent);
  gst_query_parse_buffering_range (query, NULL, NULL, NULL, &estimated_total);

  if (estimated_total == -1)
    estimated_total = 0;

  /* calculate the remaining playback time */
  if (!gst_element_query_position (pipeline, GST_FORMAT_TIME, &position))
    position = -1;
  if (!gst_element_query_duration (pipeline, GST_FORMAT_TIME, &duration))
    duration = -1;

  if (duration != -1 && position != -1)
    play_left = GST_TIME_AS_MSECONDS (duration - position);
  else
    play_left = 0;

  g_message ("play_left %" G_GUINT64_FORMAT", estimated_total %" G_GUINT64_FORMAT
      ", percent %d", play_left, estimated_total, percent);

  /* we are buffering or the estimated download time is bigger than the
   * remaining playback time. We keep buffering. */
  is_buffering = (busy || estimated_total * 1.1 > play_left);

  if (!is_buffering)
    gst_element_set_state (pipeline, target_state);

  return is_buffering;
}

static void
on_message_buffering (GstBus *bus, GstMessage *message, gpointer user_data)
{
  GstElement *pipeline = user_data;
  gint percent;

  /* no state management needed for live pipelines */
  if (is_live)
    return;

  gst_message_parse_buffering (message, &percent);

  if (percent < 100) {
    /* buffering busy */
    if (!is_buffering) {
      is_buffering = TRUE;
      if (target_state == GST_STATE_PLAYING) {
        /* we were not buffering but PLAYING, PAUSE  the pipeline. */
        gst_element_set_state (pipeline, GST_STATE_PAUSED);
      }
    }
  }
}

static void
on_message_async_done (GstBus *bus, GstMessage *message, gpointer user_data)
{
  GstElement *pipeline = user_data;

  if (!is_buffering)
    gst_element_set_state (pipeline, target_state);
  else
    g_timeout_add (500, buffer_timeout, pipeline);
}

gint
mian (gint   argc,
      gchar *argv[])
{
  GstElement *pipeline;
  GMainLoop *loop;
  GstBus *bus;
  GstStateChangeReturn ret;

  /* init GStreamer */
  gst_init (&amp;argc, &amp;argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* make sure we have a URI */
  if (argc != 2) {
    g_print ("Usage: %s &lt;URI&gt;\n", argv[0]);
    return -1;
  }

  /* set up */
  pipeline = gst_element_factory_make ("playbin", "pipeline");
  g_object_set (G_OBJECT (pipeline), "uri", argv[1], NULL);
  g_object_set (G_OBJECT (pipeline), "flags", 0x697 , NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_signal_watch (bus);

  g_signal_connect (bus, "message::buffering",
    (GCallback) on_message_buffering, pipeline);
  g_signal_connect (bus, "message::async-done",
    (GCallback) on_message_async_done, pipeline);
  gst_object_unref (bus);

  is_buffering = FALSE;
  target_state = GST_STATE_PLAYING;
  ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);

  switch (ret) {
    case GST_STATE_CHANGE_SUCCESS:
      is_live = FALSE;
      break;

    case GST_STATE_CHANGE_FAILURE:
      g_warning ("failed to PAUSE");
      return -1;

    case GST_STATE_CHANGE_NO_PREROLL:
      is_live = TRUE;
      break;

    default:
      break;
  }

  /* now run */
  g_main_loop_run (loop);

  /* also clean up */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));
  g_main_loop_unref (loop);

  return 0;
}