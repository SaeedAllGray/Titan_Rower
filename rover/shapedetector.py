#detectRectangles.py
#Last Update: 20-10-2020
#Detects the shapes present in the given image based on he recognized contours
#4 contour --> obstacle
#equal or more than 5 contour --> target


# import the necessary packages
import cv2
import imutils
import argparse
import configureSystem

myScenarioConfig_shape=configureSystem.configureScenario() 

class ShapeDetector:
	IMAGE_MAX_WH_VIRTUAL_SCENARIO=myScenarioConfig_shape.get_virtual_scenario_image_size()
	def __init__(self):
		pass
	def detect(self, c):
		# initialize the shape name and approximate the contour
		shape = "unidentified"
		peri = cv2.arcLength(c, True)
		approx = cv2.approxPolyDP(c, 0.04 * peri, True)
		# if the shape is a triangle, it will have 3 vertices
		if len(approx) == 3:
			shape = "triangle"
		# if the shape has 4 vertices, it is either a square or
		# a rectangle
		elif len(approx) == 4:
			# compute the bounding box of the contour and use the
			# bounding box to compute the aspect ratio
			(x, y, w, h) = cv2.boundingRect(approx)
			ar = w / float(h)
			# a square will have an aspect ratio that is approximately
			# equal to one, otherwise, the shape is a rectangle
			shape = "obstacle" if ar >= 0.95 and ar <= 1.05 else "obstacle"
		# if the shape is a pentagon, it will have 5 vertices
		elif len(approx) == 5:
			shape = "target"
		# otherwise, we assume the shape is a circle
		else:
			shape = "target"
		# return the name of the shape
		return shape
	def get_shapes_coordinates(self, image):
		obstacles_scenario=[]
		targets_scenario=[]
		starting_point=[]
	
		
		if max(image.shape) > self.IMAGE_MAX_WH_VIRTUAL_SCENARIO:
			resize_ratio = float(self.IMAGE_MAX_WH_VIRTUAL_SCENARIO) / max(
			image.shape[0], image.shape[1])
			image = cv2.resize(image, (0, 0),fx=resize_ratio,fy=resize_ratio,interpolation=cv2.INTER_AREA)
			print("image_resized",tuple(image.shape[1::-1]))
		resized = imutils.resize(image, width=300)
		ratio = image.shape[0] / float(resized.shape[0])
		# convert the resized image to grayscale, blur it slightly,
		# and threshold it
		gray = cv2.cvtColor(resized, cv2.COLOR_BGR2GRAY)
		blurred = cv2.GaussianBlur(gray, (5, 5), 0)
		thresh = cv2.threshold(blurred, 60, 255, cv2.THRESH_BINARY)[1]
		# find contours in the thresholded image and initialize the
		# shape detector
		cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
			cv2.CHAIN_APPROX_SIMPLE)
		cnts = imutils.grab_contours(cnts)
		#sd = ShapeDetector()

		# loop over the contours

		for c in cnts:
			# compute the center of the contour, then detect the name of the
			# shape using only the contour
			M = cv2.moments(c)
			cX = int((M["m10"] / M["m00"]) * ratio)
			cY = int((M["m01"] / M["m00"]) * ratio)
			shape = self.detect(c) #returns a string with the name of the detected shape
			
			# multiply the contour (x, y)-coordinates by the resize ratio,
			# then draw the contours and the name of the shape on the image
			c = c.astype("float")
			c *= ratio
			c = c.astype("int")
			cv2.drawContours(image, [c], -1, (0, 255, 0), 2)
			# str(cX)+","+str(cY)
			cv2.putText(image, str(cX)+","+str(cY), (cX, cY), cv2.FONT_HERSHEY_SIMPLEX,
				0.5, (255, 255, 255), 2) #puts the text of the coord in the center of each form
			# show the output image
			if shape == "obstacle":
				obstacles_scenario. append((cX-8,cY+8)) # "8" pixels to locate the cross in the center of the image
			elif shape == "target":
				targets_scenario. append((cX,cY))
			elif shape == "triangle":
				starting_point. append((cX,cY))
			else:
				print ("detected weird shape: ", shape)
				
			
		return image, obstacles_scenario, targets_scenario, starting_point
		
		

		
		
if __name__ == '__main__':

	ap = argparse.ArgumentParser()
	ap.add_argument("-i", "--image", required=True,
		help="path to the input image")
	args = vars(ap.parse_args())
	image = cv2.imread(args["image"])
	#cv2.imshow("Image_original", image)
	#image = cv2.imread(args["image"])
	print("image_original size",tuple(image.shape[1::-1]))
	sd = ShapeDetector()
	img, obstacles_scenario, targets_scenario, starting_point=sd.get_shapes_coordinates(image)
	cv2.imshow("Image", img)
	print ("detected obstacles in scenario: ", obstacles_scenario)
	print ("detected targets in scenario: ", targets_scenario)
	print ("start in: ", starting_point)		
	cv2.waitKey(0)