#get_xaxis_image_points:opt.py
#Last Update: 27-09-2020


#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from sys import argv
import logging
import cv2
import numpy as np
from get_shared_coord_opt import get_coordinates_system
import configureSystem
import math
import time

#*** CONFIGURATIONS
#-----------------------------------------------------------------
myCameraConfig=configureSystem.configureCamera()
myMarkerConfig=configureSystem.configureMarkers()

#NOTE: This form to set the path do NOT work in the Rpi --> change to the full path into " " before using
camera_path =myCameraConfig.get_camera_path()
out_path =myCameraConfig.get_out_path()
marker_size= myMarkerConfig.get_marker_size() 	#The size in meters of the marker
resize_ratio=myMarkerConfig.get_resize_ratio() 		#Leave this value as default


#-----------------------------------------------------------------
#-----------------------------------------------------------------

#-----------------------------------------------------------------		
#*** FUNCTIONS DECLARATION
#-----------------------------------------------------------------

#return the angle of orientation of the marker in rad, using as a reference the horizontal axis in the image.
#starting top-left
def get_marker_angle_rad (marker_id):
	global equations_coordinates
	myeq=equations_coordinates.get(marker_id)
	#print myeq
	x1_a= myeq[0][0]
	x2_a= myeq[1][0]
	y1_a= myeq[0][1]
	y2_a= myeq[1][1]

	if  y2_a==y1_a:
		
		if  x2_a>x1_a:
			angle_rad=0
		else:
			angle_rad=math.pi
	else:	
		cat_a= abs(x2_a - x1_a)
		cat_b= abs(y2_a - y1_a)

		angle_rad= math.atan(cat_b/cat_a)
	
		#To check the correct orientation 	
		if x2_a>x1_a and y2_a<y1_a:
		
			pass
		elif x2_a<x1_a and y2_a<y1_a:
			
			angle_rad=math.pi -angle_rad
		elif x2_a<x1_a and y2_a>y1_a:
			
			angle_rad=math.pi + angle_rad
		elif x2_a>x1_a and y2_a>y1_a:
			
			angle_rad= 2*math.pi -angle_rad
	
		else:
			pass
	
	return angle_rad

#return the angle of orientation of the marker in degrees.
def get_marker_angle_degree (marker_id):

	angle_rad= get_marker_angle_rad (marker_id)
	angle_degree= math.degrees(angle_rad)
	return angle_degree
#return the x,y position of the markers in the image (in pixels) and their angle of orientation in a dictionary.
def get_markers_info(image, resize_ratio=1):
	global equations_coordinates
	
	img=image

	#-----------------------------------------------------------------		
	#*** MARKER DETECTION
	#-----------------------------------------------------------------

	marker_ids, marker_json, marker_fs = get_id_list(
    img, camera_path, marker_size)


	
	if marker_ids is not None:
		markers = {}
		markers_int = {}
		for mid in marker_ids:
			position = get_marker_position(mid, marker_ids, marker_json)
			# Resize marker position to fit them onto the non-resized image
			x_pos = position[0] * (1 / resize_ratio)
			y_pos = position[1] * (1 / resize_ratio)
			#markers[str(mid[0])] = (int(x_pos), int(y_pos)) #add the x,y coordinates in a tuple
			markers[str(mid[0])] = (x_pos, y_pos) #add the x,y coordinates in a tuple
			markers_int[str(mid[0])] = (int(x_pos), int(y_pos))
		#-----------------------------------------------------------------		
		#*** GET DIRECTION OF THE MARKER
		#-----------------------------------------------------------------	
	
		equations_coordinates = {} #Declares a dictionary to set the values of the points in the x axis
		#of each detected marker
		markers_information = {}

		for mid in marker_ids:
			myid=mid[0]
	
			x_axis_start, x_axis_end = get_xaxis_image_points(
			myid, marker_ids, marker_json, marker_fs, marker_size)
			equations_coordinates[myid]=[x_axis_start,x_axis_end]
	
			angle_marker=get_marker_angle_degree (myid)
			#markers.get(myid)
			markers_information[myid]=[markers_int.get(str(myid)),int(angle_marker)]

		return markers_information
	
	else:
		#print ("No markers detected")
		return None 
	if marker_fs is not None:
		# Release marker FileStorage object
		print ("release fs")
		marker_fs.release()
	 


def get_id_list(input_image, camera, marker_size, out_path=None):
    """
    Perform marker recognition of the image and return the data
    for markers found.

    :param input_image: the image to run marker recognition on
    :param camera: the path to the camera calibration file
    :param marker_size: the size of the marker's sides (in meters)
    :param output_path: (optional) path to image file. If not None, 
        a copy of the input image with markers and coordinate 
        systems drawn into it will be written to this file
    
    :return: upon failure, None, None, None will be returned. Upon
        success, the method returns three objects:
        1 - A list of all marker IDs detected. Each marker is contained
            in it's own list: [[33], [18], [66], ...]
        2 - The data belonging to those IDs. Have a look at a method like
            get_xaxis_image_points to see how to retrieve it.
        3 - An OpenCV FileSystem object (or, rather, handle). This is used
            by OpenCV to access the file system and should be closed once
            we are done using it.
    """
    fs = cv2.FileStorage(camera, cv2.FILE_STORAGE_READ)
    # Get the camera model
    intrinsics = fs.getNode("camera_matrix")
    distortion = fs.getNode("distortion_coefficients")

    logging.debug('Camera Intrinsics: ')
    logging.debug(intrinsics.mat())
    logging.debug('Camera Distortion: ')
    logging.debug(distortion.mat())

    ids, _, json_content, my_image = get_coordinates_system(input_image,
                                                  intrinsics,
                                                  distortion,
                                                  marker_size=marker_size,
                                                  out_path=None, show_window=False) #change here to not show the window
    
    if ids is None:
        return None, None, None
    return ids.tolist(), json_content, fs

def get_marker_position(target_id, ids, json_content):
    if target_id not in ids:
        return None
    id = ids.index(target_id)
    return json_content["m_c"][id]

def get_xaxis_image_points(target_id, ids, json_content, fs, marker_size):
    """
    Try to extract two positions in the image from a single marker.

    If target_id is found in the id list given to this method, it
    will return two points in the image: The center point of the 
    marker and a second point, moved along its x axis.

    :param target_id: the ID of the target marker
    :param ids: the ID list as returned by get_id_list
    :param json_content: the marker data as returned by get_id_list
    :param fs: the OpenCV FileSystem handle returned by get_id_list
    :param float marker_size: The size of the marker's sides (in meters)

    :return: None, None if the marker with ID target_id was not found; 
        otherwise, returns the position of the marker's center point 
        and a second position moved along its x axis
    """
    if ids is None or json_content is None:
        return None, None
    if [target_id] not in ids:
        return None, None

    intrinsics = fs.getNode("camera_matrix")
    distortion = fs.getNode("distortion_coefficients")

    id = ids.index([target_id])

    center = json_content["m_c"][id]
    rvecs = json_content["rvecs"]
    tvecs = json_content["tvecs"]

    logging.debug('Received Centers -> {}'.format(json_content["m_c"]))

    rvec = np.array(rvecs[id])
    tvec = np.array(tvecs[id])

    xaxis_end_marker = np.array([[0, marker_size, 0]])

    logging.debug(xaxis_end_marker)
    logging.debug(rvec)
    logging.debug(tvec)

    xaxis_end, _ = cv2.projectPoints(xaxis_end_marker,
                                     rvec,
                                     tvec,
                                     intrinsics.mat(),
                                     distortion.mat())

    logging.debug('X Axis Start -> {}'.format(center))
    logging.debug('X Axis End -> {}'.format(xaxis_end[0][0]))

    return center, xaxis_end[0][0]



if __name__ == '__main__':

	out_path = out_path
	in_path = None
	params_path = None
	show_window = True
	save_image = True
	marker_size = marker_size
	
	cap = cv2.VideoCapture(myCameraConfig.get_resource())	
	cap.set(cv2.CAP_PROP_FRAME_WIDTH, myCameraConfig.get_camera_width())
	cap.set(cv2.CAP_PROP_FRAME_HEIGHT, myCameraConfig.get_camera_height())
	myScenarioConfig=configureSystem.configureScenario()
	IMAGE_MAX_WH=myScenarioConfig. get_scenario_image_size()
	while(True):

		ret, img = cap.read()
		#cv2.imshow('frame',img)
		size_image_real_scenario=tuple(img.shape[1::-1])
		print("image_real_scenario_size: ",size_image_real_scenario)
		if max(img.shape) > IMAGE_MAX_WH:
			resize_ratio = float(IMAGE_MAX_WH) / max(img.shape[0], img.shape[1])
			img = cv2.resize(img, (0, 0),fx=resize_ratio,fy=resize_ratio,interpolation=cv2.INTER_AREA)
			size_image_real_scenario=tuple(img.shape[1::-1])
			print("resized: ",size_image_real_scenario)			

		
		my_markers=(get_markers_info(img,resize_ratio))
		#cv2.imshow('iamge_or',img)

		print (my_markers)
		if my_markers:


			cv2.imshow('detected markers',img)
		time.sleep(0.2)
		if cv2.waitKey(1) & 0xFF == ord('q'):
			break

	cap.release()
	cv2.destroyAllWindows()