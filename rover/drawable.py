#drawable.py
#Last Update: 27-09-2020
#Author: Jaime Burbano
#Description:
#This script is used to draw objects in a given image 
#(captured from the camera)

import cv2
import numpy as np
import configureSystem
import math
import time


class Rectangle(): 
	color = (0, 0, 230) 
	thickness = 1
	def __init__(self,x ,y):
		self._x = x
		self._y = y  
	def get_x_obstacle(self): 
		return self._x 		
	def get_y_obstacle(self): 
		return self._y 		
	def draw(self, img):
		#print ("draw obstacle:", self.get_x_obstacle(), self.get_y_obstacle())
		img = cv2.rectangle(img, self.get_x_obstacle(), self.get_y_obstacle(), self.color, self.thickness)



class Line(): 
	color = (0, 230, 0) 
	thickness = 5
	def __init__(self,x ,y):
		self._x = x
		self._y = y  
	def get_x_obstacle(self): 
		return self._x 		
	def get_y_obstacle(self): 
		return self._y 	
	def draw(self, img):
		#print ("draw line:", self.get_x_obstacle(), self.get_y_obstacle())
		img = cv2.line(img, self.get_x_obstacle(), self.get_y_obstacle(), self.color, self.thickness)



class Circle(): 
	color = (200, 50, 0) 
	thickness = -1
	def __init__(self,x ,y):
		self._x = x
		self._y = y  
	def get_x_obstacle(self): 
		return self._x 		
	def get_y_obstacle(self): 
		return self._y 	
	def draw(self, img):
		#print ("draw circle:", self.get_x_obstacle(), self.get_y_obstacle())
		img = cv2.circle(img, self.get_x_obstacle(), self.get_y_obstacle(), self.color, self.thickness)

class Cross(): 
	color = (0, 0, 200) 
	thickness = 3
	def __init__(self,x):
		self._x = x
 
	def get_x_obstacle(self): 
		return self._x 		

	def draw(self, img):
		#print ("draw circle:", self.get_x_obstacle(), self.get_y_obstacle())
		img = cv2.putText(img,"X", self.get_x_obstacle(), (cv2.FONT_HERSHEY_SIMPLEX),1, self.color, self.thickness)

		
class Arrow(): 
	color = (0, 200, 50) 
	thickness = 2
	def __init__(self,x ,y):
		self._x = x
		self._y = y  
	def get_x_obstacle(self): 
		return self._x 		
	def get_y_obstacle(self): 
		return self._y 	
	def draw(self, img):
		#print ("draw line:", self.get_x_obstacle(), self.get_y_obstacle())
		img = cv2.arrowedLine(img, self.get_x_obstacle(), self.get_y_obstacle(), self.color, self.thickness)
		
		


def drawObject(obj,img):
	obj.draw(img)


if __name__ == '__main__':	
	#*** CONFIGURATIONS
	#-----------------------------------------------------------------
	myCameraConfig=configureSystem.configureCamera()

	cap = cv2.VideoCapture(myCameraConfig.get_resource())	
	myRectangle= Rectangle((5,3),(120,45))
	myline= Line((80,30),(170,405)) #Just dummy points to draw the object
	mycircle= Circle((50,130),10)
	mycross= Cross((150,150))
	myarrow= Arrow((200,200),(200,230))
	myarrow2= Arrow((200,200),(180,220))	
	myarrow3= Arrow((200,200),(220,220))	
	while(True):

		ret, img = cap.read()
		#cv2.imshow('frame',img)

		time.sleep(0.1)
		drawObject(myRectangle,img )
		drawObject(myline,img )	
		drawObject(mycircle,img )	
		drawObject(mycross,img )
		drawObject(myarrow,img )	
		drawObject(myarrow2,img )
		drawObject(myarrow3,img )		
		cv2.imshow('frame',img)
		if cv2.waitKey(1) & 0xFF == ord('q'):
			break

	cap.release()
	cv2.destroyAllWindows()


	
	
	
	
	
	