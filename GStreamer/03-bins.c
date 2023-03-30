/*
 
  */ 
#include <gst/gst.h>

int
CreatingBin (int   argc,
      char *argv[])
{
  /*
      (context) graph
    */
  GstElement *bin, *pipeline, *source, *sink;

  /* init */
  gst_init (&argc, &argv);

  /* create */
  pipeline = gst_pipeline_new ("my_pipeline");
  bin = gst_bin_new ("my_bin");
  source = gst_element_factory_make ("fakesrc", "source");
  sink = gst_element_factory_make ("fakesink", "sink");

  /* First add the elements to the bin */
  gst_bin_add_many (GST_BIN (bin), source, sink, NULL);
  /* add the bin to the pipeline */
  gst_bin_add (GST_BIN (pipeline), bin);

  /* link the elements */
  gst_element_link (source, sink);

// [..]

}


int // your owner pipeline
customBin (int   argc,
      char *argv[])
{
  GstElement *player;

  /* init */
  gst_init (&argc, &argv);

  /* create player */
  player = gst_element_factory_make ("oggvorbisplayer", "player");

  /* set the source audio file */
  g_object_set (player, "location", "helloworld.ogg", NULL);

  /* start playback */
  gst_element_set_state (GST_ELEMENT (player), GST_STATE_PLAYING);

// [..]

}

/*

    gst_element_set_state () or gst_element_sync_state_with_parent ()
  
  */