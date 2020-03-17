#include "ofApp.h"

// Intersect Ray with Plane  (wrapper on glm::intersect*
// the new one that was posted on Canvas (Plane-patch)
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 &
	normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal,
		dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width
			/ 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z +
			height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1]
			&& point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}


// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}

/*
	Michael Wong CS 116B Project 2 Ray Marching
*/

//--------------------------------------------------------------
void ofApp::setup() {
	
	//initially set the pointer cam to the easy cam
	theCam = &easyCam;

	//set ups for the cameras 
	easyCam.setDistance(10);
	easyCam.setNearClip(.1);
	easyCam.setFov(40); //65.5
	ofEnableDepthTest();

	sideCam.setPosition(glm::vec3(15, -1, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(0.1);

	viewCam.setPosition(glm::vec3(0, 0, 10));
	viewCam.lookAt(glm::vec3(0, 0, 0));
	viewCam.setNearClip(0.1);

	ofSetVerticalSync(true);

	//adds objects to the scene
	scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor::whiteSmoke));
	
	//scene.push_back(new Sphere(glm::vec3(-4, 1, 0), 1.25, ofColor::red));
	scene.push_back(new Sphere(glm::vec3(-0.2, 0.1, 1), 2, ofColor::blue));			//0 0 4
	scene.push_back(new Torus(glm::vec3(-2, 2.5, -1), glm::vec2(2, 0.75), ofColor::yellow));
	//scene.push_back(new Sphere(glm::vec3(4.5, 2.2, -1.5), 2, ofColor::yellow));
	
	//add lights to the scene
	//lights.push_back(new Light(1, glm::vec3(0, 7, -8)));
	lights.push_back(new Light(25, glm::vec3(3, 9, -10), false));
	//lights.push_back(new Light(50, glm::vec3(6, 5, 6)));
	//lights.push_back(new Light(150, glm::vec3(-6, 2, 2)));		//-3 0 5
	lights.push_back(new Light(50, glm::vec3(4, 6, 14), false));			//3 4 5   
	//lights.push_back(new Light(100, glm::vec3(-7, 2, 7)));

	image.allocate(imageW, imageH, ofImageType::OF_IMAGE_COLOR);		//allocates an image with desired dimensions
	map.allocate(imageW, imageH, ofImageType::OF_IMAGE_COLOR);
	
	texture.load("tile3.jpg");				//load the texture you want for the plane

	//set up the gui sliders and wheel
	gui.setup();
	gui.add(intensity.setup("intensity", 50, 0, 1000));
	gui.add(power.setup("power", 10, 10, 1000));
	gui.add(radiusSlider.setup("radius", 1, 0, 5));
	gui.add(coneRadius.setup("spotlightRadius", 0.5, 0.1, 3));
	gui.add(colorWheel.setup("colors", ofColor::gray, ofColor::black, ofColor::white));
	gui.add(angleRot.setup("angleRotation", 0, 0, 180));
	gui.add(tValue.setup("t", ofVec2f(2, 0.75), ofVec2f(0, 0), ofVec2f(10, 10)));
	bHide = false;

}

void ofApp::printChannel()
{
	if (objSelected())
	{
		cout << "Position: " << "x: " << selected[0]->position.x << " y: " << selected[0]->position.y << " z: " << selected[0]->position.z << endl;
		cout << "Rotation: " << "x: " << selected[0]->rotation.x << " y: " << selected[0]->rotation.y << " z: " << selected[0]->rotation.z << endl;
		cout << "Radius: " << selected[0]->radius << endl;
		cout << "Intensity: " << selected[0]->intensity << endl;
		cout << "Angle: " << selected[0]->angleRotate << endl;
		cout << "t: " << "t.x: " << selected[0]->t.x << " t.y: " << selected[0]->t.y << endl;
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	
	//update the object with values in the sliders only when a certain key is pressed
	//this way the slider's values won't always override what the current object's parameters
	if (objSelected())
	{
		if (bIntense)
		{
			selected[0]->intensity = intensity;
		}
		else if (bColor)
		{
			selected[0]->diffuseColor = colorWheel;
		}
		else if (bRad)
		{
			selected[0]->radius = radiusSlider;
		}
		else if (bCone)
		{
			selected[0]->coneRad = coneRadius;
		}
		else if (bAngle)
		{
			selected[0]->angleRotate = angleRot;
		}
		else if (bTValue)
		{
			selected[0]->t = ofVec2f(tValue);
		}
	}
	

}

// draws a three plane coordinate axis for reference
void ofApp::drawAxis(glm::vec3 pos)
{
	//X-axis
	ofSetColor(ofColor::red);
	ofDrawLine(pos, glm::vec3(1, 0, 0));
	//Y-axis
	ofSetColor(ofColor::green);
	ofDrawLine(pos, glm::vec3(0, 1, 0));
	//Z-axis
	ofSetColor(ofColor::blue);
	ofDrawLine(pos, glm::vec3(0, 0, 1));
	ofSetColor(ofColor::white);
}

//bool function to check if an object is in another object's shadow
bool ofApp::isShadow(const Ray &r)
{
	glm::vec3 hp;
	glm::vec3 nor;

	//loop through all the scene objects and see if any of them block a ray to a light source
	for (int i = 0; i < scene.size(); i++)
	{
		if (scene[i]->intersect(r, hp, nor))
		{
			return true;
		}

	}
	return false;
}

//a more fined tuned version to check for shadows
//specifically for shadows casted by a spotlight
bool ofApp::isSpotlightShadow(const Ray &r, const Light &l)
{
	glm::vec3 hp; 
	glm::vec3 norm;

	for (int i = 0; i < scene.size(); i++)
	{
		if (scene[i]->intersect(r, hp, norm))		//there's an object in the way
		{
			if (inSpotLight(l, hp))					//and that object is in the path to the spotlight
			{
				return true;
			}
			
		}
	}
	return false;
}

bool ofApp::isSpotlightShadowRM(const Ray &r, const Light &l)
{
	glm::vec3 hp;
	if (rayMarch(r, hp))
	{
		if (inSpotLight(l, hp))
		{
			return true;
		}
	}

	return false;
}

//function that checks to see if a point is within the scope of a spotlight
//by comparing angles
bool ofApp::inSpotLight(const Light &l, const glm::vec3 &p)
{
	glm::vec3 centerLine = l.pointAt - l.position;	//vector coming out of the center of the cone
	glm::vec3 offObj = p - l.position;				//vector from the tip of the cone to the point being tested

	//uses a cosine vector equation to determine the angle between two vectors
	float numerator = glm::dot(centerLine, offObj);
	float denominator = glm::distance(l.pointAt, l.position) * glm::distance(l.position, p);
	float angleBtwn = glm::acos(numerator/denominator);
	
	//compares the angles of the cone to the calcuated inbetween angles
	if (angleBtwn <= l.coneAngle)
	{
		return true;
	}

	return false;
}

//function that calculates lambert shading
ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse)
{
	ofColor color = ofColor::black;
	float bound = 0;
	for (int i = 0; i < lights.size(); i++)
	{
		float lighting = lights[i]->intensity / glm::pow((glm::length(lights[i]->position - p)), 2);

		color += diffuse * lighting * max(bound, glm::dot(glm::normalize(norm), glm::normalize(lights[i]->position - p)));
	
	}
	return color;
}

//when you call phong, pass the power slider as the power parameter
//calcuates phong shading
ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power)
{
	ofColor color = ofColor::black;
	float bound = 0;
	for (int i = 0; i < lights.size(); i++)
	{
		float lighting = lights[i]->intensity / glm::pow((glm::length(lights[i]->position - p)), 2);
		glm::vec3 h = glm::normalize(renderCam.position - p) + glm::normalize(lights[i]->position - p) / glm::normalize((renderCam.position - p) + (lights[i]->position - p));

		color += specular * lighting * glm::pow(max(bound, glm::dot(glm::normalize(norm), glm::normalize(h))), power);
	}


	return color;
}

//combining lambert and phong all in one shader
ofColor ofApp::allShader(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power, SceneObject* obj)
{
	ofColor totalColor = ofColor::black;	//set an inital color to black since its (0, 0, 0) so we can add color to it
	float bound = 0;
	
	//for each light, get a shading color value
	for (int i = 0; i < lights.size(); i++)
	{
		float lighting = lights[i]->intensity / glm::pow((glm::length(lights[i]->position - p)), 2);
		
		if (lights[i]->spotlight)
		{
			if (!inSpotLight(*lights[i], p)) {
				lighting = 0;	//that point will recieve no illumination
			}
		}

		//calculate the bisector vector of the light and the renderCam
		glm::vec3 v = glm::normalize(renderCam.position - p);
		glm::vec3 l = glm::normalize(lights[i]->position - p);
		glm::vec3 h = (v + l) / glm::length(v + l);		

		//gets lambert and phong color value
		//first line is lambert, second line is phong
		ofColor tempColor = diffuse * lighting * max(bound, glm::dot(glm::normalize(norm), glm::normalize(lights[i]->position - p))) + 
			specular * lighting * glm::pow(max(bound, glm::dot(glm::normalize(norm), glm::normalize(h))), power);

		//move the shadow ray a little away from the point of interection to test for shadows
		Ray r2 = Ray(p + (0.01 * glm::normalize(norm)), glm::normalize(lights[i]->position - p));
		if (lights[i]->spotlight)
		{
			if (bTrace)
			{
				if (isSpotlightShadow(r2, *lights[i]))
				{
					tempColor = ofColor(0, 0, 0);		//if your in a shadow, then no light is reaching resulting in no color
				}
			}
			else
			{
				if (isSpotlightShadowRM(r2, *lights[i]))
				{
					tempColor = ofColor::black;			//if your in a shadow, then no light is reaching resulting in no color
				}
			}

		}
		else
		{
			if (bTrace)
			{
				if (isShadow(r2))
				{
					tempColor = ofColor(0, 0, 0);		//if your in a shadow, then no light is reaching resulting in no color
				}
			}
			else
			{
				glm::vec3 hp;
				if (rayMarch(r2, hp))
				{
					tempColor = ofColor::black;			//if your in a shadow, then no light is reaching resulting in no color
				}
			}

		}		//but of course if a scene has multiple lights and 1 >= more reaches that point, it'll start to show color and the shadow gets dimmer

		totalColor += tempColor;	//add the calcuated color at a specific light to the total color value for that point
		
	}

	return totalColor;
}


//given (u,v) get the (i,j) image coordinates for their color value
ofColor ofApp::lookup(float u, float v)
{
	int i = round(u*texture.getWidth() - 0.5);
	int j = round(v*texture.getHeight() - 0.5);

	//return texture.getColor(i, j);
	return texture.getColor((int)fmod(i, texture.getWidth()), (int)fmod(j, texture.getHeight()));
}

// Ray Tracing algorithm to render an image
// Invoked with the key 't'
void ofApp::rayTrace()
{	
	
	//for each pixel
	for (int i = 0; i < imageW; i++)
	{
		int row = imageH - 1;
		for (int j = 0; j < imageH; j++)
		{		
			ofColor superColor = ofColor::black;
			for (int p = 0; p < 4; p++)
			{
				for (int q = 0; q < 4; q++)
				{
					//compute viewing ray, taking into cosideration that we are getting more than one point per pixel
					float u = (i + ((p + 0.5) / 4)) / imageW;
					float v = (j + ((q + 0.5) / 4))  / imageH;
					Ray r = renderCam.getRay(u, v);

					vector<float> distance;			//vector to hold all the distances in of objects from the renderCam
					bool hit = false;				//boolean to signal an intersect 
					int index = 0;					//index to scene vector to know what object was intersected
					vector<glm::vec3> points;
					vector<glm::vec3> n;
					//check if the ray intersects with any object in the scene
					for (int k = 0; k < scene.size(); k++)
					{
						if (scene[k]->intersect(r, hitpoint, normal))
						{
							//printf("scene index: %d\n", k);
							hit = true;
							float dist = glm::length(scene[k]->position - renderCam.position);
							distance.push_back(dist);
							index = k;							//set the index to the index in the scene vector
							points.push_back(hitpoint);
							n.push_back(normal);
						}
						else
						{
							distance.push_back(std::numeric_limits<float>::infinity());			//default big distance if nothing was intersected
																//ensures that distance elements line up with scene elements 
																//ex) distance[0] refers to distance from scene[0] to renderCam, etc... 
							points.push_back(glm::vec3(0, 0, 0));
							n.push_back(glm::vec3(0, 0, 0));

						}
					}

					//setColor follows (i, row, ofColor) format to flip and mirror the rendered image so that it matches 
					//what is expected to be seen as viewed from the view plane
					if (hit)
					{

						if (distance.size() == 1)
						{
							//first element is the plane so use the texture map
							if (index == 0)
							{
								//get the coordinates of the hitpoint
								float x = hitpoint.x + (pWidth / 2);
								float z = hitpoint.z + (pHeight / 2);
								//convert hitpoint coordinates to uv coordinates
								float uu = (x + .5) / pWidth;
								float vv = (z + .5) / pHeight;
								ofColor fc = allShader(hitpoint, normal, lookup(uu*squares, vv*squares), scene[index]->specularColor, power, scene[index]) + ambient;
								//image.setColor(i, row, fc);
								superColor += fc/4;			//prevents adding too much color since were getting more color samples per pixel
							}
							else
							{
								ofColor objColor = allShader(hitpoint, normal, scene[index]->diffuseColor, scene[index]->specularColor, power, scene[index]) + ambient;
								//image.setColor(i, row, objColor);
								superColor += objColor/4;	//denominator should match the pq loop variant 
							}

						}
						else
						{
							//finds the closest object in the scene to the renderCam
							float c = std::numeric_limits<float>::infinity();		//the shortest distance to the renderCam
							for (int a = 0; a < distance.size(); a++)
							{
								//sets the closest object to the renderCam
								if (distance[a] < c)
								{
									//updates the closest distance and the index of that object in the scene vector
									c = distance[a];
									index = a;
									//cout << c << " " << index << endl;

								}
							}

							//ofColor col = lambert(points[index], n[index], scene[index]->diffuseColor) + //ambient +
							//	phong(points[index], n[index], scene[index]->diffuseColor, scene[index]->specularColor, power);
							//image.setColor(i, row, putShadow(points[index], col));	//set color of pixel to value computed from hit point
							//first element is the plane so use the texture map
							if (index == 0)
							{
								//gets the coordinates of the closest object 
								float x = points[index].x + (pWidth / 2);
								float z = points[index].z + (pHeight / 2);
								//convert those coordinates to uv coordinates
								float uu = (x + .5) / pWidth;
								float vv = (z + .5) / pHeight;
								ofColor fc = allShader(points[index], n[index], lookup(uu*squares, v*squares), scene[index]->specularColor, power, scene[index]) + ambient;
								//image.setColor(i, row, fc);
								superColor += fc/4;
							}
							else
							{
								ofColor objColor = allShader(points[index], n[index], scene[index]->diffuseColor, scene[index]->specularColor, power, scene[index]) + ambient;
								//image.setColor(i, row, objColor);
								superColor += objColor/4;
							}


						}
					}
					else
					{
						//image.setColor(i, row, ofColor::black);			//set the background color to black
						//image.setColor(i, row, ambient);				//set the background color to the ambient color
						superColor += ofColor::black;
					}
					//row--;
				}
			}
			image.setColor(i, row, superColor);	//we are outside the pq loop, so we can now set the image pixel
			row--;
			
		}
		
	}
	
	image.save("traceImage.PNG");	//put result into an image
}

//returns the normal from a given point for any object using distances
glm::vec3 ofApp::getNormalRM(const glm::vec3 &p)
{
	float eps = 0.01;
	float dp = sceneSDF(p);
	glm::vec3 n(dp - sceneSDF(glm::vec3(p.x-eps, p.y, p.z)), 
				dp - sceneSDF(glm::vec3(p.x, p.y-eps, p.z)),
				dp - sceneSDF(glm::vec3(p.x, p.y, p.z-eps)));
	return glm::normalize(n); 
}

//returns the closest distance to the scene
float ofApp::sceneSDF(const glm::vec3 &p)
{
	float closestDist = std::numeric_limits<float>::infinity();
	for (int i = 0; i < scene.size(); i++)
	{
		float dist = scene[i]->sdf(p);
		//cout << "dist: " << dist << endl;
		if (dist < closestDist)
		{
			closestDist = dist;
			sceneIdx = i;
		}
	}

	return closestDist;
}

//this utilizes the ray marching algorithm to be used instead of the standard ray intersect method used prior
//ray marches across the ray to determine if an object is hit or not
bool ofApp::rayMarch(Ray r, glm::vec3 &p)
{
	bool hit = false;
	p = r.p;
	for (int i = 0; i < MAX_RAY_STEPS; i++)
	{
		//cout << "p: " << p << endl;
		float dist = sceneSDF(p);
		//cout << "distance: " << dist << endl;
		if (dist < DIST_THRESHOLD)			//hit falls under the required threshold to quantify as a hit
		{
			//cout << "hit" << endl;
			hit = true;
			break;
		}
		else if (dist > MAX_DISTANCE)		//hit is too far away from target, so it's considered a miss
		{
			//cout << "off target" << endl;
			break;
		}
		else
		{
			//cout << "marching" << endl;
			p += r.d*dist;					//march along the ray
		}
	}

	return hit;		
}

//this renders images as an output, based on the rayTrace(), but with some changes to it
void ofApp::rayMarch()
{
	//for each pixel, just like ray tracing
	for (int i = 0; i < imageW; i++)
	{
		int row = imageH - 1;
		for (int j = 0; j < imageH; j++)
		{
			ofColor superColor = ofColor::black;
			//anti-aliasing by oversampling 
			for (int p = 0; p < 4; p++)
			{
				for (int q = 0; q < 4; q++)
				{
					//compute viewing ray, taking into cosideration that we are getting more than one point per pixel
					float u = (i + ((p + 0.5) / 4)) / imageW;
					float v = (j + ((q + 0.5) / 4)) / imageH;
					Ray r = renderCam.getRay(u, v);

					bool hit = false;
					glm::vec3 pointOfIntersect;
					hit = rayMarch(r, pointOfIntersect);
					if (hit)
					{
						if (sceneIdx == 0)
						{
							//gets the coordinates of the closest object 
							float x = pointOfIntersect.x + (pWidth / 2);
							float z = pointOfIntersect.z + (pHeight / 2);
							//convert those coordinates to uv coordinates
							float uu = (x + .5) / pWidth;
							float vv = (z + .5) / pHeight;
							glm::vec3 norm = getNormalRM(pointOfIntersect);
							ofColor planeColor = allShader(pointOfIntersect, norm, lookup(uu*squares, v*squares), scene[sceneIdx]->specularColor, power, scene[sceneIdx]);
							superColor += planeColor / 4;
						}
						else
						{
							glm::vec3 norm = getNormalRM(pointOfIntersect);
							ofColor objColor = allShader(pointOfIntersect, norm, scene[sceneIdx]->diffuseColor, scene[sceneIdx]->specularColor, power, scene[sceneIdx]);
							superColor += objColor / 4;
						}
					}
					else
					{
						superColor += ofColor::black;
					}

				}
			}
			image.setColor(i, row, superColor);
			row--;
		}
	}

	image.save("marchImage.PNG");
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSetBackgroundColor(ofColor::black);
	if (!bHide)
	{
		ofSetColor(ofColor::white);
		ofDisableDepthTest();
		gui.draw();
		ofEnableDepthTest();
	}
		

	theCam->begin();

	ofNoFill();
	
	renderCam.draw();
	ofSetColor(ofColor::hotPink);
	renderCam.view.draw();
	drawAxis(glm::vec3(0, 0, 0));

	
	//draws the lights in the scene
	for (int j = 0; j < lights.size(); j++)
	{	
		lights[j]->draw();
	}

	
	//draws the objects in the scene
	for (int i = 0; i < scene.size(); i++)
	{
		ofColor ballColor = scene[i]->diffuseColor;
		ofSetColor(ballColor);
		scene[i]->draw();
	}

	theCam->end();
	//image.draw(glm::vec3(0, 0, 8), imageW, imageH);
}

//deletes the selected object in the scene
void ofApp::deleteObj()
{
	int index = -1;
	if (objSelected())
	{
		for (int i = 0; i < scene.size(); i++)
		{
			if (selected[0] == scene[i])
			{
				index = i;
				scene.erase(scene.begin() + index);		//delete the selected sphere object
				return;
			}
		}

		for (int j = 0; j < lights.size(); j++)
		{
			if (selected[0] == lights[j])
			{
				index = j;
				if (lights[index]->btarget = true)
				{
					int indexP = -1;
					for (int k = 0; k < lights.size(); k++)
					{
						if (lights[index]->target == lights[k])
						{
							indexP = k;
							lights.erase(lights.begin() + indexP);	//delete the target
						}
					}
				}
				lights.erase(lights.begin() + index);	//delete the selected light
				
				return;
			}
		}


	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key)
	{
	case 'f':
		ofToggleFullscreen();
		break;
	case 'e':
		easyCam.reset();
		break;
	case 't':
		bTrace = true;
		printf("ray tracing in progress...\n");
		rayTrace();
		printf("ray tracing complete\n");
		break;
	case 'm':
		bTrace = false;
		printf("ray marching in progress...\n");
		rayMarch();
		printf("ray march complete\n");
		break;
	case OF_KEY_F1:
		theCam = &easyCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &viewCam;
		break;
	case 'c':
		if (bMouse)
		{
			easyCam.disableMouseInput();
			bMouse = false;
			
		}
		else
		{
			easyCam.enableMouseInput();
			bMouse = true;
		}
		break;
	case 'g':
		bHide = !bHide;
		break;
	case 's':
		scene.push_back(new Sphere(cursor, 1.0));
		break;
	case 'l':
		lights.push_back(new Light(75, cursor, false));
		break;
	case 'o':
		scene.push_back(new Torus(cursor, glm::vec2(3, 2), ofColor::orange));
		break;
	case 'j':
		lights.push_back(new Light(50, cursor, true));
		lights.push_back(new Light(0, glm::normalize(lights[lights.size() - 1]->position), false));
		lights[lights.size() - 2]->target = lights[lights.size() - 1];
		lights[lights.size() - 1]->btarget = true;
		break;
	case 'd':
		deleteObj();
		break;
	case 'p':
		printChannel();
		break;
	case 'r':
		bRad = true;
		break;
	case 'a':
		bAngle = true;
		break;
	case 'u':
		bTValue = true;
		break;
	case 'i':
		bIntense = true;
		break;
	case 'w':
		bColor = true;
		break;
	case 'v':
		bCone = true;
		break;
	case 'x':
		bRotateX = true;
		break;
	case 'y':
		bRotateY = true;
		break;
	case 'z':
		bRotateZ = true;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	switch (key) {
	case 'w':
		bColor = false;
		break;
	case 'i':
		bIntense = false;
		break;
	case 'r':
		bRad = false;
		break;
	case 'v':
		bCone = false;
		break;
	case 'x':
		bRotateX = false;
		break;
	case 'y':
		bRotateY = false;
		break;
	case 'z':
		bRotateZ = false;
		break;
	case 'a':
		bAngle = false;
		break;
	case 'u':
		bTValue = false;
		break;
	default:
		break;
	}
}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	mouseToDragPlane(x, y, cursor);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if (objSelected() && bDrag) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);
		if (bRotateX) {
			selected[0]->rotation += glm::vec3((point.x - lastPoint.x) * 20.0, 0, 0);
		}
		else if (bRotateY) {
			selected[0]->rotation += glm::vec3(0, (point.x - lastPoint.x) * 20.0, 0);
		}
		else if (bRotateZ) {
			selected[0]->rotation += glm::vec3(0, 0, (point.x - lastPoint.x) * 20.0);
		}
		else {
			selected[0]->position += (point - lastPoint);
		}
		lastPoint = point;
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	// if we are moving the camera around, don't allow selection
	//
	if (easyCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	selected.clear();

	//
	// test if something selected
	//
	vector<SceneObject *> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersectToMove(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
		}
	}

	//check for selection of lights
	for (int j = 0; j < lights.size(); j++)
	{
		glm::vec3 point, norm;
		//  We hit a light
		//
		if (lights[j]->isSelectable && lights[j]->intersectToMove(Ray(p, dn), point, norm)) {
			hits.push_back(lights[j]);
		}
	}

	// if we selected more than one, pick nearest
	//
	SceneObject *selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}


	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else {
		selected.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bDrag = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
