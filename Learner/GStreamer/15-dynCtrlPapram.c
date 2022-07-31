#include <gst/gst.h>
#include <gst/controller/gstinterpolationcontrolsource.h>
#include <gst/controller/gstdirectcontrolbinding.h>
// pkg-config --libs --cflags gstreamer-controller-1.0


/*  g_object_set();  
        设置 GStreamer 属性
            Times in streams -->> GstControlSource 

    控制器子系统具有内置的实时模式。 
    适用于平滑参数，但它不适用于 不断更新属性 
*/


void demo{
    // 创建
    csource = gst_interpolation_control_source_new ();
    g_object_set (csource, "mode", GST_INTERPOLATION_MODE_LINEAR, NULL);

    // bind GstControlSource 到 gobject 属性
    gst_object_add_control_binding (object, gst_direct_control_binding_new (object, "prop1", csource));

    // 设置一些控制点
    GstTimedValueControlSource *tv_csource = (GstTimedValueControlSource *)csource;
    gst_timed_value_control_source_set (tv_csource, 0 * GST_SECOND, 0.0);
    gst_timed_value_control_source_set (tv_csource, 1 * GST_SECOND, 1.0);

}