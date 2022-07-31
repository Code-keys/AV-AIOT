# <a href="https://gstreamer.freedesktop.org/documentation/plugin-development/index.html?gi-language=c"> 插件编写者指南 </a>

<details open>
<summary>介绍</summary>
<p> 
- 在本指南中，您将学习如何应用高级的 GStreamer 编程概念来编写一个简单的插件。 
</p> 
</details>

<details open>
<summary> 接上文：编写插件的基础流程 </summary> 
</details> 

<details open>
<summary>创建 高级 元素类型</summary> <p> 

1.  PAD -> sometimes Request 功能 
    - sometimes 在某些条件下创建的垫，但并非在所有情况下都创建
    - “请求” Request垫类似于有时垫，除了请求是根据元素外部的某些东西而不是元素内部的东西创建的。
    
```c
static GstStaticPadTemplate src_factory =
GST_STATIC_PAD_TEMPLATE (
  "src_%u",
  GST_PAD_SRC,
  GST_PAD_SOMETIMES,
  GST_STATIC_CAPS ("ANY")
);

static void
gst_my_filter_class_init (GstMyFilterClass *klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
[..]
  gst_element_class_add_pad_template (element_class,
    gst_static_pad_template_get (&src_factory));
[..]
}

static void
gst_my_filter_init (GstMyFilter *filter)
{
[..]
  filter->firstrun = TRUE;
  filter->srcpadlist = NULL;
}

/*
 * Get one line of data - without newline.
 */

static GstBuffer *
gst_my_filter_getline (GstMyFilter *filter)
{
  guint8 *data;
  gint n, num;

  /* max. line length is 512 characters - for safety */
  for (n = 0; n < 512; n++) {
    num = gst_bytestream_peek_bytes (filter->bs, &data, n + 1);
    if (num != n + 1)
      return NULL;

    /* newline? */
    if (data[n] == '\n') {
      GstBuffer *buf = gst_buffer_new_allocate (NULL, n + 1, NULL);

      gst_bytestream_peek_bytes (filter->bs, &data, n);
      gst_buffer_fill (buf, 0, data, n);
      gst_buffer_memset (buf, n, '\0', 1);
      gst_bytestream_flush_fast (filter->bs, n + 1);

      return buf;
    }
  }
}

static void
gst_my_filter_loopfunc (GstElement *element)
{
  GstMyFilter *filter = GST_MY_FILTER (element);
  GstBuffer *buf;
  GstPad *pad;
  GstMapInfo map;
  gint num, n;

  /* parse header */
  if (filter->firstrun) {
    gchar *padname;
    guint8 id;

    if (!(buf = gst_my_filter_getline (filter))) {
      gst_element_error (element, STREAM, READ, (NULL),
             ("Stream contains no header"));
      return;
    }
    gst_buffer_extract (buf, 0, &id, 1);
    num = atoi (id);
    gst_buffer_unref (buf);

    /* for each of the streams, create a pad */
    for (n = 0; n < num; n++) {
      padname = g_strdup_printf ("src_%u", n);
      pad = gst_pad_new_from_static_template (src_factory, padname);
      g_free (padname);

      /* here, you would set _event () and _query () functions */

      /* need to activate the pad before adding */
      gst_pad_set_active (pad, TRUE);

      gst_element_add_pad (element, pad);
      filter->srcpadlist = g_list_append (filter->srcpadlist, pad);
    }
  }

  /* and now, simply parse each line and push over */
  if (!(buf = gst_my_filter_getline (filter))) {
    GstEvent *event = gst_event_new (GST_EVENT_EOS);
    GList *padlist;

    for (padlist = srcpadlist;
         padlist != NULL; padlist = g_list_next (padlist)) {
      pad = GST_PAD (padlist->data);
      gst_pad_push_event (pad, gst_event_ref (event));
    }
    gst_event_unref (event);
    /* pause the task here */
    return;
  }

  /* parse stream number and go beyond the ':' in the data */
  gst_buffer_map (buf, &map, GST_MAP_READ);
  num = atoi (map.data[0]);
  if (num >= 0 && num < g_list_length (filter->srcpadlist)) {
    pad = GST_PAD (g_list_nth_data (filter->srcpadlist, num);

    /* magic buffer parsing foo */
    for (n = 0; map.data[n] != ':' &&
                map.data[n] != '\0'; n++) ;
    if (map.data[n] != '\0') {
      GstBuffer *sub;

      /* create region copy that starts right past the space. The reason
       * that we don't just forward the data pointer is because the
       * pointer is no longer the start of an allocated block of memory,
       * but just a pointer to a position somewhere in the middle of it.
       * That cannot be freed upon disposal, so we'd either crash or have
       * a memleak. Creating a region copy is a simple way to solve that. */
      sub = gst_buffer_copy_region (buf, GST_BUFFER_COPY_ALL,
          n + 1, map.size - n - 1);
      gst_pad_push (pad, sub);
    }
  }
  gst_buffer_unmap (buf, &map);
  gst_buffer_unref (buf);
}
```
  
2. 不同的调度模式
    - updating

3. 上限谈判
    - updating

4. 内存分配
    - updating

5. 媒体类型和属性
    - updating

6. 事件：搜索、导航等
    - updating

7. 时钟
    - updating

8. 服务质量 (QoS)
    - updating

9. 支持动态参数
    - updating

10. 接口
    - updating

11. 标记（元数据和流信息）
    - updating

</p>  
</details> 