#drawScenario.py
#drawable.py
#Last Update: 27-09-2020
#Author: Jaime Burbano
#Description:
#This script is used to draw the scenario in a given image 
#(captured from the camera)



import cv2
import numpy as np
import get_xaxis_image_points_opt as marker
import configureSystem
import math
import time
import drawable

#*** CONFIGURATIONS
#-----------------------------------------------------------------
myScenarioConfig=configureSystem.configureScenario()

#-----------------------------------------------------------------
#-----------------------------------------------------------------

class Borders():	
	borders={} #initializes the value
	def __init__(self):
		pass
		
#   | ID  |	----------L1---------| ID  |
#   |  1  |						 |  2  |
#	   |							|
#	   |-->L4						| -->L2
#	   |							|
#	   |							1
#   | ID  |						 | ID  |
#   |  4  |	----------L3-------- |  3  |
#draws the borders (lines) between the markers declared in configureSystem.py
#the lines will be drawn following the order defined in configureSystem.py
#Example in case of 4 border_markers  
#		  1st with 2nd
#		  2nd with 3th
#		  3th with 4th
#	      4th with 1st

#real_time --> determines if you want to implement or not the "memory" feature
# 	True: just process the borders as they are detected (real-time)
#	False: in case a border momentarily disappears, the system will remember 
#          its last detected position
	def draw_borders (self,img, my_markers, real_time):

		my_border_ids=myScenarioConfig.get_border_ids()
	
		if real_time:
			borders=self.search_borders_real_time(my_markers)
		else:
			borders=self.search_borders(my_markers,my_border_ids)
		#print("drawing borders: ",borders )
		for border in borders:
			#print (borders)
			position_border_id=my_border_ids.index(border)
			try: 
				if border != my_border_ids[len(my_border_ids)-1]: #all the markers in the list join the following, except the last. 
															  #It joins the 1st to close the figure
					#print ("draw line from: ", borders[border], "to:",  borders[my_border_ids[position_border_id+1]])
					myline= drawable.Line(borders[border],borders[my_border_ids[position_border_id+1]]) #defines an object Line to be drawn
					#receives the two points as a parameter (tuple)
				else:
					myline= drawable.Line(borders[border],borders[my_border_ids[0]])
				drawable.drawObject(myline,img )
			except:
				pass

#returns a dictionary with the coordinates of the detected markers. This method does not have any 
#kind of memory. Therefore, if a marker is not detected during a given time, the corresponding
#border won't be drawn.			
			
	def search_borders_real_time(self,my_markers):
		
		for border_marker in my_markers:
			self.borders[border_marker]=my_markers[border_marker][0]
		return self.borders
		
#stores the coordinates of the border-markers once they are detected. Therefore, the borders are most stable
#use this method when the number of markers that delimit the borders is stable. That is to say, all the markers
#will always be into the scene
	def search_borders(self,my_markers,my_border_ids):

		for border_marker in my_border_ids:
			if border_marker in my_markers:
				self.borders[border_marker]=my_markers[border_marker][0] 
		return self.borders

	def get_borders(self):
		return self.borders


		
		
		
class Obstacles():
	myBorders=Borders()
	my_border_ids=myScenarioConfig.get_border_ids()
	slopes=[0,0]
	def __init__(self):
		pass
	def calculate_slope(self):
		borders_coordinates=self.myBorders.get_borders()
		
		if self.my_border_ids[0] in borders_coordinates and self.my_border_ids[len(self.my_border_ids)-1] in borders_coordinates:
			#we have found both markers thet conform the vertical axis (1---4)
			self.slopes[1]=self.slope(borders_coordinates[self.my_border_ids[0]],borders_coordinates[self.my_border_ids[len(self.my_border_ids)-1]])
			#print ("1: ",borders_coordinates[self.my_border_ids[0]])
			#print (self.my_border_ids[len(self.my_border_ids)-1],"  : ",borders_coordinates[4] )
		if self.my_border_ids[0] in borders_coordinates and self.my_border_ids[1] in borders_coordinates:
			#we have found both markers thet conform the vertical axis (1---4)
			self.slopes[0]=self.slope(borders_coordinates[self.my_border_ids[0]],borders_coordinates[self.my_border_ids[1]])
			#print ("1: ",borders_coordinates[self.my_border_ids[0]])
			#print (self.my_border_ids[1],"  : ",borders_coordinates[2] )			

		return self.slopes

#receives two points as a tuple to calculate the slope
	def slope(self, p1,p2):
	
		m=((p2[1]-p1[1])/(p2[0]-p1[0]))*(-1)
		return m
		
	def draw_obstacles (self,img, x):
		self.img=img
		myobstacle= drawable.Cross(x)
		drawable.drawObject(myobstacle,img )
		return self.img
	
class Targets():
	target_scenario_display=[]
	default_targets=[(10,11),(20,21),(30,31), (40,41)]
	current_target=(1,1)
	def __init__(self):
		pass

		#sets a target list in the format [[(x1,y1),True], [(x2,y2),False],[(x3,y3),False]...]
	def set_target_list(self,targets_scenario, game_mode="bottom"):
		#call game mode
		targets_scenario=self.define_mode(targets_scenario,game_mode)
		print ("game mode:",targets_scenario,game_mode)
		if targets_scenario:
			count=0
			self.target_scenario_display.clear()
			for target in targets_scenario:
				count+=1
				if count ==1:
					display_state=True
				else: display_state=False
				self.target_scenario_display.append ([target,display_state])
			#print (self.target_scenario_display)		
		
	def get_target_list(self):
		return self.target_scenario_display

	def define_mode (self, targets_scenario,game_mode):
		
		if game_mode =="mixed":
			
			if len(targets_scenario)>1:
				for i in range (len(targets_scenario)-1, 1,-1):
					x_list=targets_scenario[1]
					targets_scenario[1]=targets_scenario[i]
					targets_scenario[i]=x_list
			
			return targets_scenario
		elif game_mode =="top":
			return targets_scenario[::-1]
		else:
			return targets_scenario
			
		
	def draw_current_target(self, img):
		
		self.img=img
		current_target_coordinate=None
		
		#self.set_target_list(targets_scenario)
		target_list= self.get_target_list()
		#print (target_list)
		c=0
		for i in range (0,len(target_list)):
			
			if target_list[i][1]:
				current_target_coordinate=target_list[i][0]
				self.img= self.draw_target(self.img,current_target_coordinate)
				self.current_target=i
		return 	self.img, target_list, current_target_coordinate	
				
	def check_caught(self, target_list, position_rover):
				if self.current_target<len(target_list) and self.target_caught(target_list[self.current_target][0],position_rover):
					
					target_list[self.current_target][1]=False
					try:
						target_list[self.current_target+1][1]=True
						
					except:
						print ("no more targets")
						return False
				return True
					#print ("target_list: ",target_list)

	def target_caught(self,target_position, rover_position):
		caugth=False
		target_tolerance=myScenarioConfig.get_pixels_target_tolerance()
		if rover_position:
			
			caugth=rover_position[0] in range (target_position[0]-target_tolerance,target_position[0]+target_tolerance) and rover_position[1]in range (target_position[1]-target_tolerance,target_position[1]+target_tolerance) 

		return caugth
		
		
	def draw_target (self,img, x):
		self.img=img
		mytarget= drawable.Circle(x,10) #point, radius of circle
		drawable.drawObject(mytarget,img )
		return self.img	





class StartPoint():
	length=30 #length box/arrow in pixels
	def __init__(self):
		pass

	def draw_start_arrow (self,img, x):
		self.img=img
		myarrow1= drawable.Arrow(x,(x[0],x[1]+(3*self.length)))
		myarrow2= drawable.Arrow(x,((x[0]-(2*self.length)),x[1]+(2*self.length)))	
		myarrow3= drawable.Arrow(x,((x[0]+(2*self.length)),x[1]+(2*self.length)))		
		drawable.drawObject(myarrow1,img )
		drawable.drawObject(myarrow2,img )
		drawable.drawObject(myarrow3,img )
		return self.img		
	def draw_start (self,img, x):
		self.img=img
		myrect= drawable.Rectangle((x[0]-(self.length),x[1]-(self.length)),(x[0]+(self.length),x[1]+(self.length)))
	
		drawable.drawObject(myrect,img )

		return self.img		
	
	
	
if __name__ == '__main__':
	myCameraConfig=configureSystem.configureCamera() #Change to "1" to get the USB camera
	cap = cv2.VideoCapture(myCameraConfig.get_resource())	
	myBorders=Borders()
	myObstacle=Obstacles()
	myStart=StartPoint()
	while(True):

		ret, img = cap.read()
		my_markers=(marker.get_markers_info(img))

		print (my_markers)
		if my_markers:
			
			myBorders.draw_borders(img,my_markers,False)
			print ("borders:  ", myBorders.get_borders())
			cv2.imshow('borders',img)
		time.sleep(0.1)
		if cv2.waitKey(1) & 0xFF == ord('q'):
			break

	cap.release()
	cv2.destroyAllWindows()