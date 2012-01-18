#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>

#include "../Packet.h"
#include "Camera.h"

/*! \file Camera.cpp
 */

/*!
  Set the image channel used for gstreamer.
*/
#define NCHANNELS 3

using namespace cv;

/*! The only Camera object that this program should have. */
extern Camera * camera;

/**
 * 	\class Camera Camera.h "Camera.h"
 *	\brief The class that does everything related to the vision sensor. Using gstreamer in this class.
 */

/*! \fn Camera::Camera(int remoteSock, struct sockaddr_in & videoPort, struct sockaddr_in & artagPort)
 * 	\brief A constructor for Camera class. Init _sock, _videoPort, and _artagPort. Also call the constructor of ARtagLocalizer.
 *  \param remoteSock the socket that was initalized and ready to send to remote.
 * 	\param videoPort the udp port for sending over image stream.
 *  \param artagPort the udp port for sending detected ARtag id and pose information.
 */
Camera::Camera(int remoteSock, struct sockaddr_in & videoPort, struct sockaddr_in & artagPort)
{
	_sock = remoteSock;
	_videoPort = videoPort;
	_artagPort = artagPort;
	_isBroadcast = false;
	ar = new ARtagLocalizer();
}

/*! \fn Camera::~Camera()
 *  \brief A destructor for Camera class. It calls the destructor of ARtagLocalizer and deletes it.
 */
Camera::~Camera()
{
	delete ar;
}

/*! \fn void Camera::SendImage(IplImage * image)
 *  \brief Send the image object over through udp.
 *  \param image the image to be sent over.
 */
void Camera::SendImage(IplImage * image)
{
	Packet packet;
	packet.type = IMAGE;
	packet.u.image.width = image->width;
	packet.u.image.height = image->height;
	memcpy(&packet.u.image.data, image->imageData, sizeof(packet.u.image.data));

	if (sendto(_sock, (unsigned char*)&packet, sizeof(packet), 0, (const struct sockaddr *)&_videoPort, sizeof(struct sockaddr_in)) < 0) printf("sendto\n");
}

/*! \fn void Camera::SendARtag()
 * 	\brief Send the ARtag id and pose info over to remote through udp.
 * 	\see ARtag
 */
void Camera::SendARtag()
{
	Packet packet;
	memset(&packet, 0, sizeof(packet));
	packet.type = DATA;
	int numARtags = ar->getARtagSize();
	numARtags = numARtags < MAXARTAGSEEN ? numARtags:MAXARTAGSEEN;
	for (int i = 0; i < numARtags; ++i)
	{
		ARtag* tag = ar->getARtag(i);
		packet.u.data.tagId[i] = tag->getId();
		cv::Mat pose = tag->getPose();
		packet.u.data.x[i] = pose.at<float>(0,3)/1000.f;
		packet.u.data.y[i] = pose.at<float>(1,3)/1000.f;
		packet.u.data.z[i] = pose.at<float>(2,3)/1000.f;
		packet.u.data.yaw[i] = -atan2(pose.at<float>(1,0), pose.at<float>(0,0));
		if (packet.u.data.yaw[i] < 0)
		{
			packet.u.data.yaw[i] += 6.28;
		}
	}
	if (sendto(_sock, (unsigned char*)&packet, sizeof(packet), 0, (const struct sockaddr *)&_artagPort, sizeof(struct sockaddr_in)) < 0) printf("sendto\n");
}

/*! \fn void Camera:;SetVideoBroadcast(bool isBroadcast)
 *  \param isBroadcast Set the program whether to broadcast video stream to MATLAB
 */
void Camera::SetVideoBroadcast(bool isBroadcast)
{
	_isBroadcast = isBroadcast;
}

/*! \fn bool Camera::isBroadcast();
 *	\brief A getter for the private variable _isBroadcast to broadcast video to MATLAB
 *	\return _isBroadcast
 */
bool Camera::isBroadcast()
{
	return _isBroadcast;
}

/*! \fn GstFlowReturn new_buffer (GstAppSink *app_sink, gpointer user_data)
 * 	\brief The callback function when a new image is ready in the buffer.
 *  \param app_sink the appsink object used for gstreamer.
 *  \param user_data data that gets passed along the callback.
 *  \return the status of executing this function. GST_FLOW_OK if okay.
 */
GstFlowReturn new_buffer (GstAppSink *app_sink, gpointer user_data)
{
	GstBuffer *buffer = gst_app_sink_pull_buffer( (GstAppSink*) gst_bin_get_by_name( GST_BIN(camera->pipeline1), APPSINKNAME));

	//processing...
	//handle imageData for processing
	camera->IMG_data = (uchar*) camera->img->imageData;
	// copies AppSink buffer data to the uchar vector of IplImage */
	memcpy(camera->IMG_data, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));
	// Image buffer is RGB, but OpenCV handles it as BGR, so channels R and B must be swapped */
	cvConvertImage(camera->img,camera->img,CV_CVTIMG_SWAP_RB);
	cvCvtColor(camera->img,camera->gray,CV_BGR2GRAY);

	//detect a image ...
	if (!camera->ar->getARtagPose(camera->gray, camera->img, 0))
	{
//		printf("No artag in the view.\n");
	}
	camera->SendARtag();
	if (camera->isBroadcast())
	{
		IplImage* grayrz = cvCreateImage(cvSize(160,120), IPL_DEPTH_8U, 1);
		cvResize(camera->gray, grayrz);
		camera->SendImage(grayrz);
		cvReleaseImage(&grayrz);
	}
	gst_object_unref(buffer);
	
	return GST_FLOW_OK;
}

/*! \fn int Camera::Setup()
 *  \brief Setup function to get ready for the gstreamer.
 *  \return 0 on success, -1 on fail.
 */
int Camera::Setup()
{
	// GStreamer stuff...
	GError *error = NULL;
	GstAppSinkCallbacks callbacks;
	gchar pipeline1_str[256];

	// OpenCV stuff...
	img = cvCreateImage( cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, NCHANNELS);
	gray = cvCreateImage( cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);

	// Initializing GStreamer
	//g_print("Initializing GStreamer.\n");
	gst_init(NULL, NULL);

	//g_print("Creating Main Loop.\n");
	loop = g_main_loop_new(NULL,FALSE);

	// Initializing ARtagLocalizer
	if (ar->initARtagPose(320, 240, 180.f) != 0)
	{
		printf("Failed to init ARtagLocalizer!\n");
		return -1;
	}
	else
	{
		printf("ARtagLocalizer init successfully.\n");
	}

	//configuring pipeline parameters string
	// obs.: try g_strdup_printf
	int res = 0;
	res =  sprintf(pipeline1_str, "v4l2src ! ffmpegcolorspace ! videorate ! video/x-raw-rgb, width=%d, height=%d, framerate=15/1 ! appsink name=\"%s\"", IMG_WIDTH, IMG_HEIGHT, APPSINKNAME);
	if (res < 0)
	{
		g_printerr("Error configuring pipeline1's string\n");
		return -1;
	}

	//debugging
	//g_print("%s\n",pipeline1_str);
	//creating pipeline1
	pipeline1 = gst_parse_launch(pipeline1_str, &error);

	if (error)
	{
		g_printerr("Error [%s]\n",error->message);
		return -1;
	}

	if (!gst_bin_get_by_name( GST_BIN(pipeline1), APPSINKNAME))
	{
		g_printerr("Error creating app-sink\n");
		return -1;
	}

	//configuring AppSink's callback  (Pipeline1)
	callbacks.eos = NULL;
	callbacks.new_preroll = NULL;
	callbacks.new_buffer = new_buffer;
	gst_app_sink_set_callbacks( (GstAppSink*) gst_bin_get_by_name(GST_BIN(pipeline1), APPSINKNAME), &callbacks, NULL, NULL);
	
	return 0;
}

/*! \fn void Camera::CleanUp()
 *  \brief Clean up function to clean up gstreamer related stuff.
 */
void Camera::CleanUp()
{
	//g_print("Stopping playback - pipeline1\n");
	gst_element_set_state(pipeline1, GST_STATE_NULL);

	//g_print("Deleting pipeline1.\n");
	gst_object_unref( GST_OBJECT(pipeline1) );

	//deleting image
	cvReleaseImage(&img);
	cvReleaseImage(&gray);

	//unref mainloop
	g_main_loop_unref(loop);
}

/*! \fn int Camera::StreamARtagVideo()
 * 	\brief The main loop for Camera class. Also calls CleanUp when out of the loop.
 * 	\return 0 on success, -1 on fail.
 */
int Camera::StreamARtagVideo()
{
	if (Setup() != 0)
		return -1;
		
	//Set the pipeline to "playing" state
	//g_print("Setting pipeline1's state to \"playing\".\n");
	gst_element_set_state(pipeline1, GST_STATE_PLAYING);

	// Iterate
	printf("Running ARtag detection.\n");
	g_main_loop_run(loop);

	// Out of the main loop, clean up nicely
	CleanUp();

	return 0;
}

/*! \fn void Camera::QuitMainLoop()
 * 	\brief A public function to quit the Camera class main loop.
 */
void Camera::QuitMainLoop()
{
	g_main_loop_quit(loop);
}