#pragma once

#include "ofMain.h"
#include "ofxGui.h"

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	virtual bool intersectToMove(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	virtual float sdf(const glm::vec3 &p) { return 0.0; }

	// commonly used transformations
	//
	glm::mat4 getRotateMatrix() {
		return (glm::eulerAngleYXZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z)));   // yaw, pitch, roll 
	}
	glm::mat4 getTranslateMatrix() {
		return (glm::translate(glm::mat4(1.0), glm::vec3(position.x, position.y, position.z)));
	}
	glm::mat4 getScaleMatrix() {
		return (glm::scale(glm::mat4(1.0), glm::vec3(scale.x, scale.y, scale.z)));
	}

	// Generate a rotation matrix that rotates v1 to v2
	// v1, v2 must be normalized
	//
	glm::mat4 SceneObject::rotateToVector(glm::vec3 v1, glm::vec3 v2) {

		glm::vec3 axis = glm::cross(v1, v2);
		glm::quat q = glm::angleAxis(glm::angle(v1, v2), glm::normalize(axis));
		return glm::toMat4(q);
	}

	glm::mat4 getMatrix() {

		// get the local transformations + pivot
		//
		glm::mat4 scale = getScaleMatrix();
		glm::mat4 rotate = getRotateMatrix();
		glm::mat4 trans = getTranslateMatrix();

		// handle pivot point  (rotate around a point that is not the object's center)
		//
		glm::mat4 pre = glm::translate(glm::mat4(1.0), glm::vec3(-pivot.x, -pivot.y, -pivot.z));
		glm::mat4 post = glm::translate(glm::mat4(1.0), glm::vec3(pivot.x, pivot.y, pivot.z));



		return (trans * post * rotate * pre * scale);

	}

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 rotation = glm::vec3(1, 0, 0);   // rotate, 1 0 0 for this particular program
	glm::vec3 scale = glm::vec3(1, 1, 1);      // scale
	glm::vec3 pivot = glm::vec3(0, 0, 0);

	// get current Position in World Space
	//
	glm::vec3 getPosition() {
		return (getMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));
	}

	// set position (pos is in world space)
	//
	void setPosition(glm::vec3 pos) {
		position = glm::inverse(getMatrix()) * glm::vec4(pos, 1.0);
	}

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	bool isSelectable = true;
	float radius = 1.0;
	float intensity = 75;
	float coneRad = 0.75;
	//t represents the dimensions for the torus 
	ofVec2f t = ofVec2f(0, 0);				//t.x is the radius of the dount hole, t.y is the cross length of the acutal donut
	float angleRotate = 60.0;
	//
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}

	bool intersectToMove(const Ray &ray, glm::vec3 &point, glm::vec3 &normal)
	{
		// transform Ray to object space.  
	//
		glm::mat4 mInv = glm::inverse(getMatrix());
		glm::vec4 p = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
		glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
		glm::vec3 d = glm::normalize(p1 - p);

		return (glm::intersectRaySphere(glm::vec3(p), d, glm::vec3(0, 0, 0), radius, point, normal));
	}

	float sdf(const glm::vec3 &p)
	{
		//cout << "p: " << p << endl;
		//cout << "position: " << position << endl;
		//cout << "length: " <<  glm::length(p - position) << endl;
		return glm::length(p - position) - radius;			//straight from the slides
	}

	void draw() {
		//   get the current transformation matrix for this object
		//
		ofFill();
		glm::mat4 m = getMatrix();

		//   push the current stack matrix and multiply by this object's
		//   matrix. now all vertices drawn will be transformed by this matrix
		//
		ofPushMatrix();
		ofMultMatrix(m);
		ofDrawSphere(radius);
		ofPopMatrix();
	}

	//float radius = 1.0;
};

//general purpose torus
//
class Torus : public SceneObject
{
public:
	Torus(glm::vec3 p, glm::vec2 rt, ofColor diffuse = ofColor::lightGray) { position = p; t = rt; diffuseColor = diffuse; }
	Torus() {}

	//this is bogus for ray tracing, there is no intersectRayTorus, so Torus can't be rendered through ray tracing (to my current knowledge...)
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}

	bool intersectToMove(const Ray &ray, glm::vec3 &point, glm::vec3 &normal)
	{
		// transform Ray to object space.  
	//
		glm::mat4 mInv = glm::inverse(getMatrix());
		glm::vec4 p = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
		glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
		glm::vec3 d = glm::normalize(p1 - p);

		return (glm::intersectRaySphere(glm::vec3(p), d, glm::vec3(0, 0, 0), radius, point, normal));
	}

	float sdf(const glm::vec3 &p1)
	{
		//glm::mat4 m = glm::translate(glm::mat4(1.0), position);
		glm::mat4 M = glm::rotate(glm::mat4(1.0), glm::radians(angleRotate), rotation);	//m
		//glm::vec3 p = glm::inverse(M) * glm::vec4(p1, 1);
		//glm::vec2 q = glm::vec2(glm::length(glm::vec2(p.x, p.z)) - t.x, p.y);	//originally p

		
		glm::vec3 c = glm::vec3(4, 4, 4);			//make this the same as what period is
		//cout << "sdf c before: " << c << endl;
		float x = fmod(p1.x + 0.5*c.x, c.x) - 0.5*c.x;
		float y = fmod(p1.y + 0.5*c.y, c.y) - 0.5*c.y;
		float z = fmod(p1.z + 0.5*c.z, c.z) - 0.5*c.z;
		glm::vec3 p2 = glm::vec3(x, y, z);					//on the other hand, if you use this for the sdf calculations, the rendered image 
															//becomes more detailed and takes on a more shinny and nicer render
															//it's really bizzare... 
		//cout << "sdf c after: " << p2 << endl;
		
		glm::vec3 p3 = glm::inverse(M) * glm::vec4(p1, 1);

		glm::vec2 q2 = glm::vec2(glm::length(glm::vec2(p3.x, p3.z)) - t.x, p3.y);
		return glm::length(q2) - t.y;
	}

	//don't confused this "draw" with what's being "drawn" (rendered) for the output image
	//this draws in the scene
	void draw() {
		//   get the current transformation matrix for this object
		//
		ofNoFill();					//sphere's are filled, torus are wireframed 
		glm::mat4 m = getMatrix();

		//   push the current stack matrix and multiply by this object's
		//   matrix. now all vertices drawn will be transformed by this matrix
		//
		ofPushMatrix();
		ofMultMatrix(m);
		ofDrawSphere(t.x+t.y);			//there is no ofDrawTorus so it uses a sphere to represent it 
		ofPopMatrix();
	}

	
};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	void draw() { }
};


//  General purpose plane 
// the patched one posted on Canvas to create a finite plane (Plane-patch)
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen,
		float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
	}
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);

	float sdf(const glm::vec3 & p)
	{
		if (normal == glm::vec3(0, 1, 0))
		{
			return p.y - position.y;
		}
		else if (normal == glm::vec3(0, 0, 1))
		{
			return p.z - position.z;
		}
		else
		{
			return 0.0;
		}
	}

	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
	}
	ofPlanePrimitive plane;
	glm::vec3 normal;
	float width = 20;
	float height = 20;
};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);			// -3 2
		max = glm::vec2(3, 2);				//3 2
		position = glm::vec3(0, 0, 20);		//0 0 12
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}


	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(-6, -2, 25);		//-5 -1.75 20
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};

/*
	Michael Wong CS116B Project 2 Ray Marching
*/
class Light : public SceneObject
{
public:
	Light() {};
	Light(float intense, glm::vec3 pos, bool spot)
	{
		intensity = intense;
		position = pos;
		spotlight = spot;
	}
	void draw()
	{
		glm::mat4 m = getMatrix();
		ofSetColor(ofColor::yellow);
		if (btarget) {
			ofSetColor(ofColor::orangeRed);
			ball = coneRad;
			intensity = 0;		//enforces the intensity to be always 0 if its a target
		}
			
		//draw a small sphere to represent a light
		ofPushMatrix();
		ofMultMatrix(m);
		ofDrawSphere(ball);
		ofPopMatrix();

		if (spotlight)
		{
			
			pointAt = this->target->getPosition() - this->getPosition();	
			
			coneAngle = glm::atan(coneRad/coneLength);	//set the max angle of the spotlight

			glm::vec3 v1 = glm::normalize(glm::vec3(0, 1, 0));
			glm::vec3 v2 = glm::normalize(pointAt);
			glm::mat4 rotationMatrix = rotateToVector(v1, v2);								//gets the rotation matrix for the cone

			//then draw the cone
			ofPushMatrix();
			glm::mat4 transMat = glm::translate(this->getPosition());
			glm::mat4 offsetMat = glm::translate(glm::vec3(0, coneLength/2, 0));
			ofMultMatrix(transMat * rotationMatrix * offsetMat);
			ofDrawCone(coneRad, coneLength);
			
			ofPopMatrix();

		}
		
		ofSetLineWidth(1.0);

		// X Axis
		ofSetColor(ofColor::red);
		ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(1.5, 0, 0, 1)));


		// Y Axis
		ofSetColor(ofColor::green);
		ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, 1.5, 0, 1)));

		// Z Axis
		ofSetColor(ofColor::blue);
		ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, 0, 1.5, 1)));

	}

	bool intersectToMove(const Ray &ray, glm::vec3 &point, glm::vec3 &normal)
	{
		// transform Ray to object space.  
	//
		glm::mat4 mInv = glm::inverse(getMatrix());
		glm::vec4 p = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
		glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
		glm::vec3 d = glm::normalize(p1 - p);

		return (glm::intersectRaySphere(glm::vec3(p), d, glm::vec3(0, 0, 0), radius, point, normal));
	}

	bool spotlight = false;
	bool btarget = false;
	
	float ball = 0.2;
	glm::vec3 pointAt;			//vec3 that indicates what the spotlight is pointing at
	float coneAngle = 180;		//default for point light
	Light *target = NULL;		//default for point light
	float coneLength = 3;
};

/*
	Michael Wong CS 116A Final Project
*/
class ofApp : public ofBaseApp{

	public:
		
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		void drawAxis(glm::vec3 pos);
		void rayTrace();
		void rayMarch();
		bool rayMarch(Ray r, glm::vec3 &p);
		float sceneSDF(const glm::vec3 &p);
		ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse);
		ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power);
		bool isShadow(const Ray &r);
		bool isSpotlightShadow(const Ray &r, const Light &l);
		bool isSpotlightShadowRM(const Ray &r, const Light &l);
		ofColor allShader(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power, SceneObject* obj);	
		ofColor lookup(float u, float v);
		bool objSelected() { return (selected.size() ? true : false); };
		bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point);
		void printChannel();
		void deleteObj();
		bool inSpotLight(const Light &l, const glm::vec3 &p);
		glm::vec3 getNormalRM(const glm::vec3 &p);

		//the function to produce an infinte number of primitives in the scene
		float opRep(glm::vec3 p, glm::vec3 c, SceneObject* obj)
		{
			//cout << "opRep c before : " << c << endl;

			//glm::vec3 q = modf(p + 0.5*c, c) - 0.5*c;

			float x = fmod(p.x + 0.5*c.x, c.x) - 0.5*c.x;
			float y = fmod(p.y + 0.5*c.y, c.y) - 0.5*c.y;
			float z = fmod(p.z + 0.5*c.z, c.z) - 0.5*c.z;
			glm::vec3 q2 = glm::vec3(x, y, z);
			
			//cout << "opRep c after: " << c << endl;
			//cout << "opRep q2: " << q2 << endl;
			return obj->sdf(q2);			//idk why the point being passed into the sdf changes how the render works 
											//this way makes the render have less detail and appears duller; see Torus sdf for more
		}

		
		bool bMouse = true;
		bool bHide;
		bool bTrace = true;

		ofEasyCam easyCam;
		ofCamera viewCam, sideCam;			//camera to give view of the view plane and of the side
		ofCamera *theCam;					//pointer to switch cameras
		
		RenderCam renderCam;
		ofImage image, map;
		ofImage texture;

		Plane plane;
		ViewPlane vp; 
		vector<SceneObject *> scene;				//vector to hold all the objects in the scene
		vector<SceneObject*> selected;				//vector to hold an object that is selected
		int imageH = 500, imageW = 750;			//dimensions for the image to render
		float squares = 10;							//the dimensions for how many tiles you want layed on the plane
		int sceneIdx = 0;

		glm::vec3 hitpoint, normal;					//vec3s to be used later for intersect
		Light light;
		vector<Light *> lights;

		glm::vec3 lastPoint;
		glm::vec3 cursor;							//vec3 that tracks the movement of the mouse cursor
		const glm::vec3 period = glm::vec3(3.5, 3.5, 3.5);		//period of repetition for the infinite primitives 

		ofxPanel gui;
		ofxFloatSlider intensity;
		ofxFloatSlider power;
		ofxFloatSlider radiusSlider;
		ofxColorSlider colorWheel;
		ofxFloatSlider coneRadius;
		ofxFloatSlider angleRot;
		ofxVec2Slider tValue;

		ofColor ambient = ofColor(0, 0, 0);	//a constant ambient color
		float pWidth = 20, pHeight = 20;
		//double check these for what values need to be in them
		const float MAX_RAY_STEPS = 200;			//maximum amount of iterations for moving along the ray, originally 200
		const float DIST_THRESHOLD = 0.1;			//margin of seperation to deem a hit
		const float MAX_DISTANCE = 50;				//furthest distance from "bullseye" that will be considered as a hit

		bool bDrag = false;
		bool bRad = false;
		bool bColor = false;
		bool bIntense = false;
		bool bCone = false;
		bool bRotateX = false;
		bool bRotateY = false;
		bool bRotateZ = false;
		bool bAnimate = false;
		bool bAngle = false;
		bool bTValue = false;
};
