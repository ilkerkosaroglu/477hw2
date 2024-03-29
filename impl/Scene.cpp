#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cmath>

#include "Scene.h"
#include "Camera.h"
#include "Color.h"
#include "Mesh.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Triangle.h"
#include "Vec3.h"
#include "tinyxml2.h"
#include "Helpers.h"

using namespace tinyxml2;
using namespace std;

/*
	Transformations, clipping, culling, rasterization are done here.
	You may define helper functions.
*/
void Scene::forwardRenderingPipeline(Camera *camera)
{
	//viewport transformation
	int nx = camera->horRes, ny = camera->verRes;
	double vpVal[4][4] = {{nx/2.0,0,0,(nx-1)/2.0},{0,ny/2.0,0,(ny-1)/2.0},{0,0,1/2.0,1/2.0},{0,0,0,0}};
	Matrix4 Mvp(vpVal);


	for(auto m: meshes){
		drawingMode = m->type;

		Matrix4 T(getIdentityMatrix());

		//model transformations
		for(int i=0;i<(m->numberOfTransformations);i++){
			int id=m->transformationIds[i]-1;
			char type=m->transformationTypes[i];
			if(type=='r'){
				T = multiplyMatrixWithMatrix(rotations[id]->getMatrix(),T);
			}else if(type == 't'){
				T = multiplyMatrixWithMatrix(translations[id]->getMatrix(),T);
			}else if(type == 's'){
				T = multiplyMatrixWithMatrix(scalings[id]->getMatrix(),T);
			}else{
				cerr<<"something went wrong."<<endl;
			}
		}

		for(auto t: m->triangles){
			Vec4 a = Vec4::convertFromVec3(*vertices[t.vertexIds[0]-1]);
			Vec4 b = Vec4::convertFromVec3(*vertices[t.vertexIds[1]-1]);
			Vec4 c = Vec4::convertFromVec3(*vertices[t.vertexIds[2]-1]);

			//save world coordinates
			Vec4 aW = multiplyMatrixWithVec4(T, a);
			Vec4 bW = multiplyMatrixWithVec4(T, b);
			Vec4 cW = multiplyMatrixWithVec4(T, c);

			// world to camera transformation +
			// camera to view (cvv) transformation (inverts coordinate system)
			a = multiplyMatrixWithVec4(camera->getMatrix(),aW);
			b = multiplyMatrixWithVec4(camera->getMatrix(),bW);
			c = multiplyMatrixWithVec4(camera->getMatrix(),cW);

			//backface culling
			if(cullingEnabled){
				Vec3 a3(aW.x,aW.y,aW.z,-1);
				Vec3 b3(bW.x,bW.y,bW.z,-1);
				Vec3 c3(cW.x,cW.y,cW.z,-1);

				//unit normal vector of the triangle
				Vec3 n = normalizeVec3(crossProductVec3(subtractVec3(b3,a3),subtractVec3(c3,a3)));

				//unit looking direction (to the object)
				Vec3 eyeDir;
				if(camera->projectionType==0){
					//ortho
					eyeDir = normalizeVec3(camera->w);
				}else{
					//perspective
					eyeDir = normalizeVec3(subtractVec3(camera->pos,a3));
				}

				//skip the object if it is facing away
				if(dotProductVec3(n,eyeDir)<0){
					continue;
				}
			}

			//perspective division
			if(camera->projectionType == 1){
				//! what about a.t = 0?
				a.applyPerspectiveDivision();
				b.applyPerspectiveDivision();
				c.applyPerspectiveDivision();
			}
			//clipping
			vector<Vec4> points;

			//wireframe
			if(drawingMode==0){
				clipLine(a,b,points);
				clipLine(b,c,points);
				clipLine(c,a,points);
			}

			//solid
			if(drawingMode==1){
				clipTriangle(a,b,c,points);
			}

			for(auto &k:points){
				//viewport transformation
				k = multiplyMatrixWithVec4(Mvp, k);
			}

			//rasterization

			//wireframe
			if(drawingMode==0){
				for(int i=0;i<(int)points.size()-1;i+=2){
					rasterizeLine(points[i],points[i+1]);
				}
			}

			//solid
			if(drawingMode==1){
				for(int i=0;i<(int)points.size()-2;i+=3){
					rasterizeTriangle(points[i],points[i+1],points[i+2]);
				}
			}
		}
	}
}

void Scene::addPoints(int axis, double col, bool insideIsLeft, Vec4 a, Vec4 b, vector<Vec4> &points){
	double distA = abs(a.getElementAt(axis)-col);
	double distB = abs(b.getElementAt(axis)-col);
	double t = (distA)/(distA+distB);

	bool aInside = (a.getElementAt(axis) < col) ^ !insideIsLeft;
	bool bInside = (b.getElementAt(axis) < col) ^ !insideIsLeft;

	if(aInside ^ bInside){
		colorsOfVertices.push_back(new Color(mix(indexColor(a.colorId),indexColor(b.colorId),t)));
		Vec4 p = interpVec4(a,b,t);
		p.colorId = colorsOfVertices.size();
		points.push_back(p);
	}

	if(bInside){
		points.push_back(b);
	}
}

void Scene::clipTriangle(Vec4 a, Vec4 b, Vec4 c, vector<Vec4> &points){
	vector<Vec4> p2;
	p2.push_back(a);
	p2.push_back(b);
	p2.push_back(c);

	//clip for each axis, and each column of the axis
	for(int axis=0;axis<3;axis++){
		for(int i=0;i<2;i++){
			vector<Vec4> generatedP;
			for(int p=0;p<p2.size();p++){
				addPoints(axis,2*i-1,i,p2[p],p2[(p+1)%p2.size()],generatedP);
			}
			//cull if everything is outside
			if(generatedP.size()<3){
				return;
			}
			p2.clear();
			p2.push_back(generatedP.back());
			for(int p=0;p<generatedP.size()-1;p++){
				p2.push_back(generatedP[p]);
			}
		}
	}

	//tessellate
	for(int i=1;i<p2.size()-1;i++){
		points.push_back(p2[0]);
		points.push_back(p2[i]);
		points.push_back(p2[i+1]);
	}
}

bool visible(double den, double num, double &tE, double &tL){
	// potentially entering
	if (den > 0){
		double t = num / den;
		if (t > tL){
			return false;
		}
		if (t > tE){
			tE = t;
		}
	} 
	// potentially leaving
	else if (den < 0){
		double t = num / den;
		if (t < tE){
			return false;
		}
		if (t < tL){
			tL = t;
		}

	}
	// line parallel to edge
	else if (num > 0){
		return false;
	}
	return true;
}

void Scene::clipLine(Vec4 a, Vec4 b, vector<Vec4> &points){
	double tE=0, tL=1;
	bool vis = true;
	Vec4 d(b.x-a.x,b.y-a.y,b.z-a.z,0,-1);
	for(int axis=0;axis<3;axis++){
		vis = vis && visible(d.getElementAt(axis),-1-a.getElementAt(axis),tE,tL);
		vis = vis && visible(-d.getElementAt(axis),a.getElementAt(axis)-1,tE,tL);
	}
	if(!vis) return;
	Vec4 resA = a;
	Vec4 resB = b;
	Color ca = indexColor(a.colorId);
	Color cb = indexColor(b.colorId);
	const double E = 0.0000001;
	if(tL<1){
		colorsOfVertices.push_back(new Color(ca+(cb-ca)*tL));
		Vec4 p = interpVec4(a,b,tL-E);
		p.colorId = colorsOfVertices.size();
		resB = p;
	}
	if(tE>0){
		colorsOfVertices.push_back(new Color(ca+(cb-ca)*tE));
		Vec4 p = interpVec4(a,b,tE+E);
		p.colorId = colorsOfVertices.size();
		resA = p;
	}
	points.push_back(resA);
	points.push_back(resB);
}


Color Scene::indexColor(int colorId){
	return *colorsOfVertices[colorId-1];
}

void Scene::rasterizeLine(Vec4 a, Vec4 b){
	if(a.x>b.x)
	swap(a,b);

	Color c = indexColor(a.colorId);

	int x0 = 0, y0 = 0;
	int x1 = 0, y1 = 0;

	x0 = round(a.x);
	x1 = round(b.x);
	y0 = round(a.y);
	y1 = round(b.y);

	int dx = x1-x0;
	int dy = y1-y0;

	bool flipXY = false;
	bool flipY = false;
	if(dy<0){
		flipY=true;
		dy*=-1;
		y1+=2*dy;
	}
	if(abs(dy)>abs(dx)){
		flipXY=true;
		swap(x0,y0);
		swap(x1,y1);
		swap(dx,dy);
	}

	int p = -2*dy + dx;

	Color dc = indexColor(b.colorId) - c;
	dc.r /= dx;
	dc.g /= dx;
	dc.b /= dx;

	int y=y0;
	for(int x=x0;x<=x1;x++){
		//draw px
		if(flipXY){
			swap(x,y);			
			swap(x0,y0);			
		}
		int curdy = y-y0;
		if(flipY){
			y-=2*curdy;
		}
		image[x][y] = c;
		if(flipY){
			y+=2*curdy;
		}
		if(flipXY){
			swap(x,y);
			swap(x0,y0);			
		}

		if(p<0){//NE
			y++;
			p+=2*dx-2*dy;
		}else{ //E
			p+=-2*dy;
		}

		c.addColor(dc);
	}
}
int lineEq(int x0, int y0, int x1, int y1, int x, int y){
	return x*(y0-y1) + y*(x1-x0) + x0*y1 - y0*x1;
}
int lineEqVec(Vec4 a, Vec4 b, int x, int y){
	return lineEq(a.x, a.y, b.x, b.y, x, y);
}
void Scene::rasterizeTriangle(Vec4 a, Vec4 b, Vec4 c){

	//? when should we round things?

	int minX = min(min(round(a.x),round(b.x)),round(c.x));
	int minY = min(min(round(a.y),round(b.y)),round(c.y));
	int maxX = max(max(round(a.x),round(b.x)),round(c.x));
	int maxY = max(max(round(a.y),round(b.y)),round(c.y));

	// a.x = round(a.x); a.y = round(a.y); a.z = round(a.z);
	// b.x = round(b.x); b.y = round(b.y); b.z = round(b.z);
	// c.x = round(c.x); c.y = round(c.y); c.z = round(c.z);

	for(int x=minX;x<=maxX;x++){
		for(int y=minY;y<=maxY;y++){
			double alpha = (double)lineEqVec(b, c, x, y) / lineEqVec(b, c, a.x, a.y);
			double beta = (double)lineEqVec(c, a, x, y) / lineEqVec(c, a, b.x, b.y);
			double gamma = (double)lineEqVec(a, b, x, y) / lineEqVec(a, b, c.x, c.y);
			if(alpha>=0 && beta>=0 && gamma>=0){
				Color col = indexColor(a.colorId)*alpha + indexColor(b.colorId)*beta + indexColor(c.colorId)*gamma;
				image[x][y] = Color(round(col.r), round(col.g), round(col.b));
				// image[x][y] = col; //we can also do this, but it changes the colors slightly(?)
			}
		}
	}
	
}

/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *pElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *pRoot = xmlDoc.FirstChild();

	// read background color
	pElement = pRoot->FirstChildElement("BackgroundColor");
	str = pElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	pElement = pRoot->FirstChildElement("Culling");
	if (pElement != NULL) {
		str = pElement->GetText();
		
		if (strcmp(str, "enabled") == 0) {
			cullingEnabled = true;
		}
		else {
			cullingEnabled = false;
		}
	}

	// read cameras
	pElement = pRoot->FirstChildElement("Cameras");
	XMLElement *pCamera = pElement->FirstChildElement("Camera");
	XMLElement *camElement;
	while (pCamera != NULL)
	{
		Camera *cam = new Camera();

		pCamera->QueryIntAttribute("id", &cam->cameraId);

		// read projection type
		str = pCamera->Attribute("type");

		if (strcmp(str, "orthographic") == 0) {
			cam->projectionType = 0;
		}
		else {
			cam->projectionType = 1;
		}

		camElement = pCamera->FirstChildElement("Position");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->pos.x, &cam->pos.y, &cam->pos.z);

		camElement = pCamera->FirstChildElement("Gaze");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->gaze.x, &cam->gaze.y, &cam->gaze.z);

		camElement = pCamera->FirstChildElement("Up");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->v.x, &cam->v.y, &cam->v.z);

		cam->gaze = normalizeVec3(cam->gaze);
		cam->u = crossProductVec3(cam->gaze, cam->v);
		cam->u = normalizeVec3(cam->u);

		cam->w = inverseVec3(cam->gaze);
		cam->v = crossProductVec3(cam->u, cam->gaze);
		cam->v = normalizeVec3(cam->v);

		camElement = pCamera->FirstChildElement("ImagePlane");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &cam->left, &cam->right, &cam->bottom, &cam->top,
			   &cam->near, &cam->far, &cam->horRes, &cam->verRes);

		camElement = pCamera->FirstChildElement("OutputName");
		str = camElement->GetText();
		cam->outputFileName = string(str);

		cameras.push_back(cam);

		pCamera = pCamera->NextSiblingElement("Camera");
	}

	// read vertices
	pElement = pRoot->FirstChildElement("Vertices");
	XMLElement *pVertex = pElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (pVertex != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = pVertex->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = pVertex->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		vertices.push_back(vertex);
		colorsOfVertices.push_back(color);

		pVertex = pVertex->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	pElement = pRoot->FirstChildElement("Translations");
	XMLElement *pTranslation = pElement->FirstChildElement("Translation");
	while (pTranslation != NULL)
	{
		Translation *translation = new Translation();

		pTranslation->QueryIntAttribute("id", &translation->translationId);

		str = pTranslation->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		translations.push_back(translation);

		pTranslation = pTranslation->NextSiblingElement("Translation");
	}

	// read scalings
	pElement = pRoot->FirstChildElement("Scalings");
	XMLElement *pScaling = pElement->FirstChildElement("Scaling");
	while (pScaling != NULL)
	{
		Scaling *scaling = new Scaling();

		pScaling->QueryIntAttribute("id", &scaling->scalingId);
		str = pScaling->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		scalings.push_back(scaling);

		pScaling = pScaling->NextSiblingElement("Scaling");
	}

	// read rotations
	pElement = pRoot->FirstChildElement("Rotations");
	XMLElement *pRotation = pElement->FirstChildElement("Rotation");
	while (pRotation != NULL)
	{
		Rotation *rotation = new Rotation();

		pRotation->QueryIntAttribute("id", &rotation->rotationId);
		str = pRotation->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		rotations.push_back(rotation);

		pRotation = pRotation->NextSiblingElement("Rotation");
	}

	// read meshes
	pElement = pRoot->FirstChildElement("Meshes");

	XMLElement *pMesh = pElement->FirstChildElement("Mesh");
	XMLElement *meshElement;
	while (pMesh != NULL)
	{
		Mesh *mesh = new Mesh();

		pMesh->QueryIntAttribute("id", &mesh->meshId);

		// read projection type
		str = pMesh->Attribute("type");

		if (strcmp(str, "wireframe") == 0) {
			mesh->type = 0;
		}
		else {
			mesh->type = 1;
		}

		// read mesh transformations
		XMLElement *pTransformations = pMesh->FirstChildElement("Transformations");
		XMLElement *pTransformation = pTransformations->FirstChildElement("Transformation");

		while (pTransformation != NULL)
		{
			char transformationType;
			int transformationId;

			str = pTransformation->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			mesh->transformationTypes.push_back(transformationType);
			mesh->transformationIds.push_back(transformationId);

			pTransformation = pTransformation->NextSiblingElement("Transformation");
		}

		mesh->numberOfTransformations = mesh->transformationIds.size();

		// read mesh faces
		char *row;
		char *clone_str;
		int v1, v2, v3;
		XMLElement *pFaces = pMesh->FirstChildElement("Faces");
        str = pFaces->GetText();
		clone_str = strdup(str);

		row = strtok(clone_str, "\n");
		while (row != NULL)
		{
			int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);
			
			if (result != EOF) {
				mesh->triangles.push_back(Triangle(v1, v2, v3));
			}
			row = strtok(NULL, "\n");
		}
		mesh->numberOfTriangles = mesh->triangles.size();
		meshes.push_back(mesh);

		pMesh = pMesh->NextSiblingElement("Mesh");
	}
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
			}

			this->image.push_back(rowOfColors);
		}
	}
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				this->image[i][j].r = this->backgroundColor.r;
				this->image[i][j].g = this->backgroundColor.g;
				this->image[i][j].b = this->backgroundColor.b;
			}
		}
	}
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}

/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFileName.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFileName << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
	os_type == 1 		-> Ubuntu
	os_type == 2 		-> Windows
	os_type == other	-> No conversion
*/
void Scene::convertPPMToPNG(string ppmFileName, int osType)
{
	string command;

	// call command on Ubuntu
	if (osType == 1)
	{
		command = "convert " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// call command on Windows
	else if (osType == 2)
	{
		command = "magick convert " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// default action - don't do conversion
	else
	{
	}
}