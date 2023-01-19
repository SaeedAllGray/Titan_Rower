#transform.py

#Last Update: 08-10-2020

#Description:
#Gets 4 poins from an image and rectifies the are cointained
#within those 4 points.

import numpy as np
import cv2
import time 
import argparse
import configureSystem
	
	
#   | ID  |	----------L1---------| ID  |
#   |  1  |						 |  2  |
#	   |							|
#	   |-->L4						| -->L2
#	   |							|
#	   |							1
#   | ID  |						 | ID  |
#   |  4  |	----------L3-------- |  3  |

def order_points(pts): #pts is a list of points (x,y) 
	# initialzie a list of coordinates that will be ordered
	# such that the first entry in the list is the top-left,
	# the second entry is the top-right, the third is the
	# bottom-right, and the fourth is the bottom-left
	rect = np.zeros((4, 2), dtype = "float32")
	# the top-left point will have the smallest sum, whereas
	# the bottom-right point will have the largest sum
	s = pts.sum(axis = 1)
	rect[0] = pts[np.argmin(s)]
	rect[2] = pts[np.argmax(s)]
	# now, compute the difference between the points, the
	# top-right point will have the smallest difference,
	# whereas the bottom-left will have the largest difference
	diff = np.diff(pts, axis = 1)
	rect[1] = pts[np.argmin(diff)]
	rect[3] = pts[np.argmax(diff)]
	
	return rect # return the ordered coordinates

#param:
#coordinates: a list cointaining the 4 points (x,y)
#the correct format will be as in the image above. 
def validate_coordinates(coordinates):
	if coordinates[0][0]<coordinates[1][0] and coordinates[3][0]<coordinates[2][0]:
		print ("x is correct")
	
		if coordinates[0][1]<coordinates[3][1] and coordinates[1][1]<coordinates[2][1]:
			print ("y is correct")
			return True
	return False
	
	
	
def four_point_transform(image, pts):
	# obtain a consistent order of the points and unpack them
	# individually
	rect = order_points(pts)
	(tl, tr, br, bl) = rect
	# compute the width of the new image, which will be the
	# maximum distance between bottom-right and bottom-left
	# x-coordiates or the top-right and top-left x-coordinates
	widthA = np.sqrt(((br[0] - bl[0]) ** 2) + ((br[1] - bl[1]) ** 2))
	widthB = np.sqrt(((tr[0] - tl[0]) ** 2) + ((tr[1] - tl[1]) ** 2))
	maxWidth = max(int(widthA), int(widthB))
	# compute the height of the new image, which will be the
	# maximum distance between the top-right and bottom-right
	# y-coordinates or the top-left and bottom-left y-coordinates
	heightA = np.sqrt(((tr[0] - br[0]) ** 2) + ((tr[1] - br[1]) ** 2))
	heightB = np.sqrt(((tl[0] - bl[0]) ** 2) + ((tl[1] - bl[1]) ** 2))
	maxHeight = max(int(heightA), int(heightB))
	# now that we have the dimensions of the new image, construct
	# the set of destination points to obtain a "birds eye view",
	# (i.e. top-down view) of the image, again specifying points
	# in the top-left, top-right, bottom-right, and bottom-left
	# order
	dst = np.array([
		[0, 0],
		[maxWidth - 1, 0],
		[maxWidth - 1, maxHeight - 1],
		[0, maxHeight - 1]], dtype = "float32")
	# compute the perspective transform matrix and then apply it
	M = cv2.getPerspectiveTransform(rect, dst)
	warped = cv2.warpPerspective(image, M, (maxWidth, maxHeight))
	# return the warped image
	return warped
	
	
if __name__ == "__main__":
	myScenarioConfig=configureSystem.configureScenario()
	#-----------------------------------------------------------------
	myCameraConfig=configureSystem.configureCamera()
	cap = cv2.VideoCapture(myCameraConfig.get_resource())	
	coordinates=[(20,10),(120,10),(120,120),(20,120)]#just some example coordinates

	while(True):
		ret, image = cap.read()
	
		if validate_coordinates(coordinates):
			pts = np.array(coordinates, dtype = "float32")
			print (pts)
# apply the four point tranform to obtain a "birds eye view" of
# the image
			warped = four_point_transform(image, pts)
			cv2.imshow("Warped", warped)
		else:
			print ("markers are not well positioned")
		if cv2.waitKey(1) & 0xFF == ord('q'):
			break