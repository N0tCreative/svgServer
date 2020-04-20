/* svg parser
 * creates, converts to text, and deletes svg images
 * jerad Arnold
 * 13-01-20
 */

/* TODO write the tostring function similar to the way the rect to string was done for the other elements and add xmlns to attributes at top beside svg attributes*/

#include "ctype.h"
#include "SVGParser.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

//for some reason math constants dont work so this is just in case it suddenly starts working
#if !defined(M_PI)
#define M_PI 3.14159265359

#endif // M_PI

//essential functions
SVGimage* createSVGimage(char* fileName);
char* SVGimageToString(SVGimage* img); //used mostly for debugging
void deleteSVGimage(SVGimage* img);
SVGimage* createValidSVGimage(char* fileName, char* schemaFile);
bool writeSVGimage(SVGimage* doc, char* fileName);
bool validateSVGimage(SVGimage* doc, char* schemaFile);
void setAttribute(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute);
void addComponent(SVGimage* image, elementType elemType, void* newComponent);

//helper functions for lists
//write functions
bool writeSVGAttr(List *attr, xmlNodePtr node);
bool writeSVGRect(List *rect, xmlNodePtr parent);
xmlDocPtr SVGToXml(SVGimage* doc);
bool writeSVGRect(List *rect, xmlNodePtr parent);
bool writeSVGCirc(List *circ, xmlNodePtr parent);
bool writeSVGPath(List *path, xmlNodePtr parent);
bool writeSVGAttr(List *attr, xmlNodePtr node);
bool writeSVGGroup(List* group, xmlNodePtr parent);
bool writeValidSVGimage(SVGimage* doc, char* fileName, char* schema);
//set attribute helper
void setOtherAttr(List* list, Attribute* newAttribute);
//other
void addChildrenToSVG(SVGimage* img, xmlNode* root);
Group* addChildrenToGroup(xmlNode* group); //get xmlgroup into Group format
Rectangle* parseRect(xmlNode* node); //get xmlrect into Rectangle format
Circle* parseCirc(xmlNode* node); //get xmlcircle into Circle format
Path* parsePath(xmlNode* node); //get xmlpath into Path format
List* parseAllAttributes(xmlNode* node); //get all xmlattr of one node into list of Attribute type
Attribute* parseAttribute(xmlAttr* attr); //get one xmlattr into an Attribute format
char* addNodeDescToString(char* mainStr, List* toAdd); //adds output from list into main string
char* addStr(char* str, char* toAdd);
//attributes
void deleteAttribute(void* data);
char* attributeToString( void* data);
int compareAttributes(const void *first, const void *second);
//groups
void deleteGroup(void* data);
char* groupToString( void* data);
int compareGroups(const void *first, const void *second);
//rectangles
void deleteRectangle(void* data);
char* rectangleToString(void* data);
int compareRectangles(const void *first, const void *second);
//circles
void deleteCircle(void* data);
char* circleToString(void* data);
int compareCircles(const void *first, const void *second);
//paths
void deletePath(void* data);
char* pathToString(void* data);
int comparePaths(const void *first, const void *second);
//getters
List* getRects(SVGimage* img);
List* getCircles(SVGimage* img);
List* getPaths(SVGimage* img);
List* getGroups(SVGimage* img);
//finders
int numRectsWithArea(SVGimage* img, float area);
int numCirclesWithArea(SVGimage* img, float area);
int numPathsWithdata(SVGimage* img, char* data);
int numGroupsWithLen(SVGimage* img, int len);
int numAttr(SVGimage* img);
//to json functions
char* attrToJSON(const Attribute *a);
char* circleToJSON(const Circle *c);
char* rectToJSON(const Rectangle *r);
char* pathToJSON(const Path *p);
char* groupToJSON(const Group *g);
char* attrListToJSON(const List *list);
char* circListToJSON(const List *list);
char* rectListToJSON(const List *list);
char* pathListToJSON(const List *list);
char* groupListToJSON(const List *list);
char* SVGtoJSON(const SVGimage* imge);
//not required
SVGimage* JSONtoSVG(const char* svgString);
Rectangle* JSONtoRect(const char* svgString);
Circle* JSONtoCircle(const char* svgString);

//wrapper functions (any function that doesnt return a json returns a string indicating success/ failure)
char* convSVGToJSON(char* fileName); //turns filename into format readable by javascript /svglist
char* titleToJSON(char* fileName); //finds the title of filenames svg and returns it as a string /titledesc
char* descToJSON(char* fileName); //finds the description of filenames svg and returns it as a string /titledesc
char* rectsToJSON(char* fileName); //gets the list of rects from filename and returns it as a json string /components
char* circsToJSON(char* fileName); //gets the list of circs from filename and returns it as a json string /components
char* pathsToJSON(char* fileName); //gets the list of paths from filename and returns it as a json string /components
char* groupsToJSON(char* fileName); //gets the list of groups from filename and returns it as a json string /components
char* attributeToJSON(char* fileName, char* type, int index); //gets the list of attributes from shape at index in fileNames image /attributes
char* updateTitle(char* fileName, char* title); //changes title of filename /title
char* updateDesc(char* fileName, char* desc); //changes description of filename /desc
char* addAttr(char* fileName, char* shape, int index, char* attrType, char* attrVal); //add/change attribute types value to attrVal of shape at index of filename /attribute
char* addShape(char* fileName, char* shape, char* attributes); //add a shape to filename with attributes /addshape
char* scaleShapes(char* fileName, char* shape, double factor); //scale all shapes of fileName by factor /scaleshape
char* createXML(char* fileName); //creates an empty svg file on the server


//sets up important aspects of the svg image then calls addchildrentosvg to add all the elements
SVGimage* createSVGimage(char* fileName) {
	SVGimage* img = malloc(sizeof(SVGimage));
	xmlDoc *doc = NULL;
	xmlNode *rootElement = NULL;

	//make sure that there is a file name
	if(fileName == NULL){
		free(img);
		return NULL;
	}

	doc = xmlReadFile(fileName, NULL, 0);

	//make sure that the file is in valid xml format
	if(doc == NULL) {
		free(img);
		return NULL;
	}

	//get the namespace of the first element
	rootElement = xmlDocGetRootElement(doc);

	strcpy(img->title, "");
	strcpy(img->description, "");

	//if for some reason there is no namespace
	if(!rootElement || !(rootElement->ns) || !(rootElement->ns->href)) {
		xmlFreeDoc(doc);
		xmlCleanupParser();
		return NULL;
	}
	strncpy(img->namespace, (char*) rootElement->ns->href, 255);
	img->namespace[255] = '\0';

	//initialize all the lists for the svg image
	img->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
	img->circles = initializeList(circleToString, deleteCircle, compareCircles);
	img->paths = initializeList(pathToString, deletePath, comparePaths);
	img->groups = initializeList(groupToString, deleteGroup, compareGroups);
	//intentionally left out other attributes because its done in different function
	//img->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);

	//find if there is any title, description or shapes/groups and add them to their place/list
	addChildrenToSVG(img, rootElement);


	xmlFreeDoc(doc);
	xmlCleanupParser();
	return img;
}

//vaildates a xml file vs a schema file
SVGimage* createValidSVGimage(char* fileName, char* schemaFile) {
	SVGimage* img =NULL;
	//creates the image
	img = createSVGimage(fileName);
	if(!img) {
		return NULL;
	}
	if(!validateSVGimage(img, schemaFile)) {
		deleteSVGimage(img);
		return NULL;
	}
	return img;
}

//outputs the svgimage into a file with both valid svg and valid xml formatting
bool writeValidSVGimage(SVGimage* doc, char* fileName, char* schema) {

	if(!doc || !fileName || !schema) {
		return false;
	}

	if(!validateSVGimage(doc, schema)){
		return false;
	} else {
		return writeSVGimage(doc, fileName);
	}
	
}

//outputs the svgimage into a file with both valid svg and valid xml formatting
bool writeSVGimage(SVGimage* doc, char* fileName) {
	xmlDocPtr xml= NULL;

	if(!doc || !fileName) {
		return false;
	}

	xml= SVGToXml(doc);

	//once everything is made to an xml tree try to write to a file
	if(xmlSaveFormatFileEnc(fileName, xml, "UTF-8", 1) <=0) {
		xmlFreeDoc(xml);
		xmlCleanupParser();
		return false;
	}
	xmlFreeDoc(xml);
	xmlCleanupParser();
	return true;
}

//converts an image into an xmldoc
xmlDocPtr SVGToXml(SVGimage* doc) {
	xmlDocPtr xml= NULL;
	xmlNodePtr root= NULL;

	//add all important stuff to the doc including namespace, title(if possible), description(if possible) (nothing here should be able to fail)
	xml = xmlNewDoc(BAD_CAST "1.0"); //xml type
	root =xmlNewNode(NULL, BAD_CAST "svg"); //root node should say svg
	xmlDocSetRootElement(xml, root); //dont see why this could fail
	if(doc->namespace ==NULL || strlen(doc->namespace)<1) {
		xmlFreeDoc(xml);
		xmlCleanupParser();
		return NULL;
	}
	xmlSetNs(root, xmlNewNs(root, BAD_CAST doc->namespace, NULL)); //neither for this
	if(!writeSVGAttr(doc->otherAttributes, root)) { //add any attributes of the root(svg) to it
		xmlFreeDoc(xml);
		xmlCleanupParser();
		return NULL;
	}
	//add these important nodes first
	if(strlen(doc->title)>0) {
		xmlNewChild(root, NULL, BAD_CAST "title", BAD_CAST doc->title);
	}
	if(strlen(doc->description)>0) {
		xmlNewChild(root, NULL, BAD_CAST "desc", BAD_CAST doc->description);
	}

	//add all rects, circles, paths, then groups in order of occurance (if any fail free everything and return false)
	if(!writeSVGRect(doc->rectangles, root)) {
		xmlFreeDoc(xml);
		xmlCleanupParser();
		return NULL;
	}
	if(!writeSVGCirc(doc->circles, root)) {
		xmlFreeDoc(xml);
		xmlCleanupParser();
		return NULL;
	}
	if(!writeSVGPath(doc->paths, root)) {
		xmlFreeDoc(xml);
		xmlCleanupParser();
		return NULL;
	}
	if(!writeSVGGroup(doc->groups, root)) {
		xmlFreeDoc(xml);
		xmlCleanupParser();
		return NULL;
	}
	return xml;
}

//used to write groups into xml formatting
bool writeSVGGroup(List* group, xmlNodePtr parent) {
	ListIterator itr;
	Group* grp= NULL;
	xmlNodePtr tmpNode= NULL;
	if(!group || !parent) {
		return false;
	}
	itr =createIterator(group);
	for(grp=nextElement(&itr); grp; grp= nextElement(&itr)) {
		//make a group child for each group in the list
		tmpNode =xmlNewChild(parent, NULL, BAD_CAST "g", NULL);
		//add all constituants of the group
		if(!writeSVGRect(grp->rectangles, tmpNode)) {
			return false;
		}
		if(!writeSVGCirc(grp->circles, tmpNode)) {
			return false;
		}
		if(!writeSVGPath(grp->paths, tmpNode)) {
			return false;
		}
		if(!writeSVGGroup(grp->groups, tmpNode)) {
			return false;
		}
		//add any other attributes in order after
		if(!writeSVGAttr(grp->otherAttributes, tmpNode)) {
			return false;
		}
	}
	return true;
}

//used to write attributes into xml properties of their parent node
bool writeSVGAttr(List *attr, xmlNodePtr node) {
	ListIterator itr;
	Attribute* att= NULL;
	char* name =malloc(1);
	char* value=malloc(1);
	if(!attr || !node) {
		free(name);
		free(value);
		return false;
	}
	itr =createIterator(attr);
	//adds all otherAttributes to the nodes properties
	for(att=nextElement(&itr); att; att= nextElement(&itr)) {
		//if the node has no name or value then free everything and return
		if(!(att->name) || !(att->value)) {
			free(name);
			free(value);
			return false;
		}
		name=realloc(name, strlen(att->name) +1);
		strcpy(name, att->name);
		value=realloc(value, strlen(att->value) +1);
		strcpy(value, att->value);
		xmlNewProp(node, BAD_CAST att->name, BAD_CAST att->value);
	}

	free(name);
	free(value);
	return true;
}

//creates xml rect, children node of parent(root/group)
bool writeSVGRect(List *rect, xmlNodePtr parent) {
	ListIterator itr;
	Rectangle* rct= NULL;
	xmlNodePtr tmpNode= NULL;
	char temp[1100]; //in theory at max doubles go up to 1030 or something digits long + 50 for units so this should never break
	char* value =malloc(1);
	char* name =malloc(1);
	if(!rect || !parent) {
		free(name);
		free(value);
		return false;
	}
	itr =createIterator(rect);
	for(rct=nextElement(&itr); rct; rct= nextElement(&itr)) {
		//assumed that units are a valid string (has a null terminator)(no way to check otherwise without memory errors)
		//makes sure the other numbers are valid
		if(rct->x<0 || rct->y<0 || rct->width<0 || rct->height<0) {
			free(name);
			free(value);
			return false;
		}
		//make a rect child of root/group for each rect in the list
		tmpNode =xmlNewChild(parent, NULL, BAD_CAST "rect", NULL);

		//add all the standard attributes to the new node
		name =realloc(name, sizeof("x"));
		strcpy(name, "x");
		if(strlen(rct->units) >0) {
			sprintf(temp, "%f%s", rct->x, rct->units);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("y"));
			strcpy(name, "y");
			sprintf(temp, "%f%s", rct->y, rct->units);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("width"));
			strcpy(name, "width");
			sprintf(temp, "%f%s", rct->width, rct->units);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("height"));
			strcpy(name, "height");
			sprintf(temp, "%f%s", rct->height, rct->units);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);
		} else {
			sprintf(temp, "%f", rct->x);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("y"));
			strcpy(name, "y");
			sprintf(temp, "%f", rct->y);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("width"));
			strcpy(name, "width");
			sprintf(temp, "%f", rct->width);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("height"));
			strcpy(name, "height");
			sprintf(temp, "%f", rct->height);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);
		}

		//add any other attributes in order after
		if(!writeSVGAttr(rct->otherAttributes, tmpNode)) {
			free(name);
			free(value);
			return false;
		}
	}

	free(name);
	free(value);
	return true;
}

//creates xml circle, children node of parent(root/group)
bool writeSVGCirc(List *circ, xmlNodePtr parent) {
	ListIterator itr;
	Circle* crc= NULL;
	xmlNodePtr tmpNode= NULL;
	char temp[1100]; //in theory at max doubles go up to 1030 or something digits long + 50 for units so this should never break
	char* value =malloc(1);
	char* name =malloc(1);
	if(!circ || !parent) {
		free(name);
		free(value);
		return false;
	}
	itr =createIterator(circ);
	for(crc=nextElement(&itr); crc; crc= nextElement(&itr)) {
		//assumed that units are a valid string (has a null terminator)(no way to check otherwise without memory errors)
		//makes sure the other numbers are valid
		if(crc->cx<0 || crc->cy<0 || crc->r<0) {
			free(name);
			free(value);
			return false;
		}
		//make a rect child of root/group for each rect in the list
		tmpNode =xmlNewChild(parent, NULL, BAD_CAST "circle", NULL);

		//add all the standard attributes to the new node
		name =realloc(name, sizeof("cx")); //dont use realloc b/c freeDoc will free these char* ptrs (right?)
		strcpy(name, "cx");
		if(strlen(crc->units) >0) {
			sprintf(temp, "%f%s", crc->cx, crc->units);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("cy"));
			strcpy(name, "cy");
			sprintf(temp, "%f%s", crc->cy, crc->units);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("r"));
			strcpy(name, "r");
			sprintf(temp, "%f%s", crc->r, crc->units);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);
		} else {
			sprintf(temp, "%f", crc->cx);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("cy"));
			strcpy(name, "cy");
			sprintf(temp, "%f", crc->cy);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

			name =realloc(name, sizeof("r"));
			strcpy(name, "r");
			sprintf(temp, "%f", crc->r);
			value =realloc(value, strlen(temp) +1);
			strcpy(value, temp);
			xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);
		}
		//add any other attributes in order after
		if(!writeSVGAttr(crc->otherAttributes, tmpNode)) {
			free(name);
			free(value);
			return false;
		}
	}

	free(name);
	free(value);
	return true;
}

//creates xml path, children node of parent(root/group)
bool writeSVGPath(List *path, xmlNodePtr parent) {
	ListIterator itr;
	Path* pth= NULL;
	xmlNodePtr tmpNode= NULL;
	char* value =malloc(1);
	char* name =malloc(1);
	if(!path || !parent) {
		free(name);
		free(value);
		return false;
	}
	itr =createIterator(path);
	for(pth=nextElement(&itr); pth; pth= nextElement(&itr)) {
		if(!(pth->data)) {
			free(name);
			free(value);
			return false;
		}
		//make a rect child of root/group for each rect in the list
		tmpNode =xmlNewChild(parent, NULL, BAD_CAST "path", NULL);

		//add all the standard attributes to the new node
		name =realloc(name, sizeof("d")); //dont use realloc b/c freeDoc will free these char* ptrs (right?)
		strcpy(name, "d");
		value =realloc(value, strlen(pth->data) +1);
		strcpy(value, pth->data);
		xmlNewProp(tmpNode, BAD_CAST name, BAD_CAST value);

		//add any other attributes in order after
		if(!writeSVGAttr(pth->otherAttributes, tmpNode)) {
			free(name);
			free(value);
			return false;
		}
	}

	free(name);
	free(value);
	return true;
}

//turns imgage into xml doc then checks it against the schema
bool validateSVGimage(SVGimage* doc, char* schemaFile) {
	xmlDocPtr imgDoc =NULL;
	xmlDocPtr schemaDoc = NULL;
	xmlSchemaPtr schema =NULL;
	xmlSchemaParserCtxtPtr ctxt =NULL;
	xmlSchemaValidCtxtPtr ctxtV =NULL;

	if(!doc || !schemaFile) {
		return false;
	}

	schemaDoc = xmlReadFile(schemaFile, NULL, 0);
	if(!schemaDoc) {
		return false;
	}

	//checks if schema file is valid
	ctxt = xmlSchemaNewDocParserCtxt(schemaDoc);
	if(!ctxt) {
		xmlFreeDoc(schemaDoc);
		xmlSchemaCleanupTypes();
		xmlCleanupParser();
		return false;
	}
	schema =xmlSchemaParse(ctxt);
	xmlSchemaFreeParserCtxt(ctxt);
	if(!schema) {
		xmlFreeDoc(schemaDoc);
		xmlSchemaCleanupTypes();
		xmlCleanupParser();
		return false;
	}

	ctxtV =xmlSchemaNewValidCtxt(schema);
	if(!ctxtV) {
		xmlSchemaFree(schema);
		xmlFreeDoc(schemaDoc);
		xmlSchemaCleanupTypes();
		xmlCleanupParser();
		return false;
	}

	imgDoc =SVGToXml(doc);
	if(!imgDoc) {
		xmlSchemaFree(schema);
		xmlSchemaFreeValidCtxt(ctxtV);
		xmlFreeDoc(schemaDoc);
		xmlSchemaCleanupTypes();
		xmlCleanupParser();
		return false;
	}
	
	//checks if the doc is in valid formatting (if its valid itll return 0)
	if((xmlSchemaValidateDoc(ctxtV, imgDoc)) !=0) {
		xmlSchemaFree(schema);
		xmlSchemaFreeValidCtxt(ctxtV);
		xmlFreeDoc(schemaDoc);
		xmlFreeDoc(imgDoc);
		xmlSchemaCleanupTypes();
		xmlCleanupParser();
		return false;
	}

	xmlSchemaFree(schema);
	xmlSchemaFreeValidCtxt(ctxtV);
	xmlFreeDoc(schemaDoc);
	xmlFreeDoc(imgDoc);
	xmlSchemaCleanupTypes();
	xmlCleanupParser();
	return true;
}

//finds all children of the svg image and pareses them into the correct format then adds them to the appropreate list
void addChildrenToSVG(SVGimage* img, xmlNode* root) {
	xmlNode *tempEle = NULL;
	Rectangle *rec;
	Circle *circ;
	Path *path;
	Group *group;


	/*for (attr = root->properties; attr != NULL; attr = attr->next) template dont use !!!!!!!!!
	{
		 xmlNode *value = attr->children;
		 char *attrName = (char *)attr->name;
		 char *cont = (char *)(value->content);
		 printf("\tattribute name: %s, attribute value = %s\n", attrName, cont);
	}*/
	
	img->otherAttributes = parseAllAttributes(root);

	//adds elements in the root to lists
	for(tempEle = xmlFirstElementChild(root); tempEle; tempEle = xmlNextElementSibling(tempEle)) {
		if(!strcmp((char*) tempEle->name, "title")) {
			strncpy(img->title, (char*) tempEle->children->content, 255);
			img->title[255] = '\0';
		}
		else if(!strcmp((char*) tempEle->name, "desc")) {
			strncpy(img->description, (char*) tempEle->children->content, 255);
			img->description[255] = '\0';
		}
		else if(!strcmp((char*) tempEle->name, "rect")) {
			rec = parseRect(tempEle);
			//add test to see if rectangle parsed correctly here
			insertBack(img->rectangles, rec);
		}
		else if(!strcmp((char*) tempEle->name, "circle")) {
			circ = parseCirc(tempEle);
			//add test to see if circle parsed correctly here
			insertBack(img->circles, circ);
		}
		else if(!strcmp((char*) tempEle->name, "path")) {
			path = parsePath(tempEle);
			//add test to see if path parsed correctly here
			insertBack(img->paths, path);
		}
		else if(!strcmp((char*) tempEle->name, "g")) {
			group = addChildrenToGroup(tempEle);
			//add test to see if group parsed correctly here
			insertBack(img->groups, group);
		}
	}
}

//deletes all elements in the svgimage
void deleteSVGimage(SVGimage* img) {
	if(!img){
		return;
	}
	freeList(img->rectangles);
	freeList(img->circles);
	freeList(img->paths);
	freeList(img->groups);
	freeList(img->otherAttributes);
	free(img);
}

//converts the svg image into some type of mish mash of types of string need to fix
char* SVGimageToString(SVGimage* img) { //TODO change this and rect from xml to json formatting
	if(!img) {
		return NULL;
	}
	char* mainStr = malloc(sizeof("<svg"));
	sprintf(mainStr, "<svg");
	mainStr=addNodeDescToString(mainStr, img->otherAttributes);
	mainStr =realloc(mainStr, strlen(mainStr) + sizeof(">"));
	strcat(mainStr, ">");

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n"));
	strcat(mainStr, "\n");
	mainStr=addNodeDescToString(mainStr, img->rectangles);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nCircles:"));
	strcat(mainStr, "\n\nCircles:");
	mainStr=addNodeDescToString(mainStr, img->circles);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nPaths:"));
	strcat(mainStr, "\n\nPaths:");
	mainStr=addNodeDescToString(mainStr, img->paths);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nGroups:"));
	strcat(mainStr, "\n\nGroups:");
	mainStr=addNodeDescToString(mainStr, img->groups);
	return mainStr;
}

/*char* SVGimageToString(SVGimage* img) {
	if(!img) {
		return NULL;
	}
	char* mainStr = malloc(sizeof("namespace: \ntitle: \ndescription: \nAttributes:") + strlen(img->namespace) + strlen(img->title) + strlen(img->description));
	sprintf(mainStr, "namespace: %s\ntitle: %s\ndescription: %s\nAttributes:", img->namespace, img->title, img->description);
	mainStr=addNodeDescToString(mainStr, img->otherAttributes);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nRectangles:"));
	strcat(mainStr, "\n\nRectangles:");
	mainStr=addNodeDescToString(mainStr, img->rectangles);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nCircles:"));
	strcat(mainStr, "\n\nCircles:");
	mainStr=addNodeDescToString(mainStr, img->circles);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nPaths:"));
	strcat(mainStr, "\n\nPaths:");
	mainStr=addNodeDescToString(mainStr, img->paths);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nGroups:"));
	strcat(mainStr, "\n\nGroups:");
	mainStr=addNodeDescToString(mainStr, img->groups);
	return mainStr;
}*/

//use to add all list stuff to main description string
char* addNodeDescToString(char* mainStr, List* toAdd) {
	if(!toAdd || getLength(toAdd)<=0) {
		return mainStr;
	}
	char* tempStr = toString(toAdd);
	mainStr=realloc(mainStr, strlen(mainStr)+ strlen(tempStr) +1);
	strcat(mainStr, tempStr);
	free(tempStr);
	return mainStr;
}

//***************************element parsing***************************
Group* addChildrenToGroup(xmlNode* root) {
	xmlNode *tempEle = NULL;
	Group* parentGroup = malloc(sizeof(Group));
	Rectangle *rec;
	Circle *circ;
	Path *path;
	Group *group;
	
	parentGroup->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
	parentGroup->circles = initializeList(circleToString, deleteCircle, compareCircles);
	parentGroup->paths = initializeList(pathToString, deletePath, comparePaths);
	parentGroup->groups = initializeList(groupToString, deleteGroup, compareGroups);

	parentGroup->otherAttributes = parseAllAttributes(root);

	//adds elements in the root to lists
	for(tempEle = xmlFirstElementChild(root); tempEle; tempEle = xmlNextElementSibling(tempEle)) {
		if(!strcmp((char*) tempEle->name, "rect")) {
			rec = parseRect(tempEle);
			//add test to see if rectangle parsed correctly here
			insertBack(parentGroup->rectangles, rec);
		}
		else if(!strcmp((char*) tempEle->name, "circle")) {
			circ = parseCirc(tempEle);
			//add test to see if circle parsed correctly here
			insertBack(parentGroup->circles, circ);
		}
		else if(!strcmp((char*) tempEle->name, "path")) {
			path = parsePath(tempEle);
			//add test to see if path parsed correctly here
			insertBack(parentGroup->paths, path);
		}
		else if(!strcmp((char*) tempEle->name, "g")) {
			group = addChildrenToGroup(tempEle);
			//add test to see if group parsed correctly here
			insertBack(parentGroup->groups, group);
		}
	}
	return parentGroup;
}

//converts a xml rectangle into Rectangle formatting
Rectangle* parseRect(xmlNode* rect) {
	xmlAttr* attr = NULL;
	Rectangle* rec = malloc(sizeof(Rectangle));
	Attribute* attrib;
	char* ptr;
	rec->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
	//default x & y settings
	rec->x =0.0;
	rec->y =0.0;
	strcpy(rec->units,"");

	//find as many attributes as possible
	for (attr = rect->properties; attr != NULL; attr = attr->next)
	{
		if(!strcmp((char*) attr->name, "x")) {
			sscanf((char*)attr->children->content, "%f", &(rec->x));
		}
		else if(!strcmp((char*) attr->name, "y")) {
			sscanf((char*)attr->children->content, "%f", &(rec->y));
		}
		else if(!strcmp((char*) attr->name, "width")) {
			sscanf((char*)attr->children->content, "%f", &(rec->width));//later add stuff that checks to make sure units are max 50 characters
			ptr =(char*)attr->children->content;
			for(int i=0; ptr[i]; i++) {
				if(ptr[i]>='A' && ptr[i] <= 'z') {
					strncpy(rec->units, &(ptr[i]), 50);
					break;
				}
			}
		}
		else if(!strcmp((char*) attr->name, "height")) {
			sscanf((char*)attr->children->content, "%f%s", &(rec->height), rec->units);//later add stuff that checks to make sure units are max 50 characters
		}
		else{ //non standard attributes are put in their own list
			attrib = parseAttribute(attr);

			//add attrib to attributes list
			insertBack(rec->otherAttributes, attrib);
		}
	}
	//TODO add a test before return to check if its a valid svg (floats>=0 && units<50 characters)
	return rec;
}

//converts a xml circle into circle formatting
Circle* parseCirc(xmlNode* circ) {
	xmlAttr* attr = NULL;
	Circle* cir = malloc(sizeof(Circle));
	Attribute* attrib;
	cir->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
	char* ptr;
	//default x & y settings
	cir->cx =0.0;
	cir->cy =0.0;
	strcpy(cir->units,"");

	//find as many attributes as possible
	for (attr = circ->properties; attr != NULL; attr = attr->next)
	{
		if(!strcmp((char*) attr->name, "cx")) {
			sscanf((char*)attr->children->content, "%f", &(cir->cx));//later add stuff that checks to make sure units are max 50 characters
		}
		else if(!strcmp((char*) attr->name, "cy")) {
			sscanf((char*)attr->children->content, "%f", &(cir->cy));//later add stuff that checks to make sure units are max 50 characters
		}
		else if(!strcmp((char*) attr->name, "r")) {
			sscanf((char*)attr->children->content, "%f", &(cir->r));//later add stuff that checks to make sure units are max 50 characters
			ptr =(char*)attr->children->content;
			for(int i=0; ptr[i]; i++) {
				if(ptr[i]>='A' && ptr[i] <= 'z') {
					strncpy(cir->units, &(ptr[i]), 50);
					break;
				}
			}
		}
		else{ //non standard attributes are put in their own list
			attrib = parseAttribute(attr);

			//add attrib to attributes list
			insertBack(cir->otherAttributes, attrib);
		}
	}
	//TODO add a test before return to check if its a valid svg node (floats>=0 && units<50 characters)
	return cir;
}

//converts a xml path into Path formatting
Path* parsePath(xmlNode* path) {
	xmlAttr* attr = NULL;
	Path* pth = malloc(sizeof(Path));
	Attribute* attrib;
	pth->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);

	//find as many attributes as possible
	for (attr = path->properties; attr != NULL; attr = attr->next)
	{
		if(!strcmp((char*) attr->name, "d")) {
			pth->data = malloc(strlen((char*)(attr->children->content)) +1);
			strcpy(pth->data, (char*)(attr->children->content));
		}
		else{ //non standard attributes are put in their own list
			attrib = parseAttribute(attr);

			//add attrib to attributes list
			insertBack(pth->otherAttributes, attrib);
		}
	}
	//TODO add a test before return to check if its a valid svg node (floats>=0 && units<50 characters)
	return pth;
}

//parses all attributes of a node (usually root or group)
List* parseAllAttributes(xmlNode* node) {
	List* attributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
	xmlAttr* attr;
	Attribute* attrib;

	for (attr = node->properties; attr != NULL; attr = attr->next)
	{
		attrib = parseAttribute(attr);

		//add attrib to attributes list
		insertBack(attributes, attrib);
	}

	return attributes;
}

//parse specific attributes that arent essential to the node (usually rect, circle, or path)
Attribute* parseAttribute(xmlAttr* attr) {
	Attribute* attrib;

	attrib = malloc(sizeof(Attribute));
	attrib->name = malloc(strlen((char *)attr->name) + 1);
	strcpy(attrib->name, (char *)attr->name);

	attrib->value = malloc(strlen((char *)(attr->children->content)) + 1);
	strcpy(attrib->value, (char *)(attr->children->content));

	return attrib;
}

//used to shorten adding strings of defined length to character pointer
char* addStr(char* str, char* toAdd) {
	str=realloc(str, strlen(toAdd) +1);
	strcpy(str, toAdd);
	return str;
}

//***************************attribute***************************
void deleteAttribute(void* data) {
	Attribute* attrib = (Attribute*)data;
	if(!data) {
		return;
	}
	free(attrib->name);
	free(attrib->value);
	free(attrib);
}

char* attributeToString( void* data) {
	if(!data) {
		return NULL;
	}
	Attribute* attrib = (Attribute*)data;
	char* tempStr = malloc(sizeof(" =\"\"") + strlen(attrib->name) + strlen(attrib->value));
	sprintf(tempStr, " %s=\"%s\"", attrib->name, attrib->value);
	return tempStr;
}

int compareAttributes(const void *first, const void *second) {
	int same =0;
	Attribute* a = (Attribute*) first;
	Attribute* b = (Attribute*) second;

	if(!a || !b) {
		return 0;
	}

	same = strcmp(a->name, b->name);
	return same;
}

bool cmpAttributes(const void *first, const void *second) {
	Attribute* a = (Attribute*) first;
	Attribute* b = (Attribute*) second;

	if(!a || !b) {
		return 0;
	}

	if(strcmp(a->name, b->name) !=0) {
		return false;
	}
	return true;
}

//***************************group***************************
void deleteGroup(void* data) {
	Group* parentGroup = (Group*) data;
	freeList(parentGroup->rectangles);
	freeList(parentGroup->circles);
	freeList(parentGroup->paths);
	freeList(parentGroup->otherAttributes);
	freeList(parentGroup->groups);
	free(parentGroup);
}

char* groupToString( void* data) {
	if(!data) {
		return NULL;
	}
	Group* parentGroup = (Group*) data;
	char* mainStr = malloc(sizeof("Group{\nAttributes:"));
	strcpy(mainStr, "Group{\nAttributes:");
	mainStr=addNodeDescToString(mainStr, parentGroup->otherAttributes);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nRectangles:"));
	strcat(mainStr, "\n\nRectangles:");
	mainStr=addNodeDescToString(mainStr, parentGroup->rectangles);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nCircles:"));
	strcat(mainStr, "\n\nCircles:");
	mainStr=addNodeDescToString(mainStr, parentGroup->circles);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nPaths:"));
	strcat(mainStr, "\n\nPaths:");
	mainStr=addNodeDescToString(mainStr, parentGroup->paths);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n\nGroups:"));
	strcat(mainStr, "\n\nGroups:");
	mainStr=addNodeDescToString(mainStr, parentGroup->groups);

	mainStr=realloc(mainStr,strlen(mainStr) + sizeof("\n}"));
	strcat(mainStr, "\n}");
	return mainStr;
}

int compareGroups(const void *first, const void *second) {
	int same =0;
	Group* a = (Group*) first;
	Group* b = (Group*) second;

	if(!a || !b) {
		return 0;
	}

	same = strcmp(groupToString(a), groupToString(b));
	return same;
}

//***************************rectangle***************************
void deleteRectangle(void* data) {
	if(!data) {
		return;
	}
	Rectangle* rect = (Rectangle*) data;
	freeList(rect->otherAttributes);
	free(rect);
}

char* rectangleToString( void* data) {
	char temp[100];
	int lengths[4];
	if(!data) {
		return NULL;
	}
	Rectangle* rect = (Rectangle*) data;
	sprintf(temp, "%f", rect->x);
	lengths[0] = strlen(temp);
	sprintf(temp, "%f", rect->y);
	lengths[1] = strlen(temp);
	sprintf(temp, "%f", rect->width);
	lengths[2] = strlen(temp);
	sprintf(temp, "%f", rect->height);
	lengths[3] = strlen(temp);
	char* tempStr = malloc(sizeof("\t<rect x=\"\" y=\"\" width=\"\" height=\"\" ") + 4*(strlen(rect->units)) + lengths[0] + lengths[1] + lengths[2] + lengths[3] + 1);
	sprintf(tempStr, "\t<rect x=\"%f%s\" y=\"%f%s\" width=\"%f%s\" height=\"%f%s\" ", rect->x, rect->units, rect->y, rect->units, rect->width, rect->units, rect->height, rect->units);
	tempStr= addNodeDescToString(tempStr, rect->otherAttributes);
	tempStr = realloc(tempStr, strlen(tempStr) + sizeof("/>\n"));
	strcat(tempStr, "/>\n");
	return tempStr;
}

int compareRectangles(const void *first, const void *second) {
	int same =0;
	char* rect1;
	char* rect2;
	Rectangle* a = (Rectangle*) first;
	Rectangle* b = (Rectangle*) second;

	if(!a || !b) {
		return 0;
	}
	rect1=rectangleToString(a);
	rect2=rectangleToString(b);

	same = strcmp(rect1, rect2);
	free(rect1);
	free(rect2);
	return same;
}

//***************************circle***************************
void deleteCircle(void* data) {
	if(!data) {
		return;
	}
	Circle* circ = (Circle*) data;
	freeList(circ->otherAttributes);
	free(circ);
}

char* circleToString( void* data) {
	if(!data) {
		return "";
	}
	Circle* circ = (Circle*) data;
	char* tempStr = malloc(sizeof("cx=\ncy=\nradius=") + 3*(13*sizeof(char) + strlen(circ->units)) + 1);
	sprintf(tempStr, "cx=%10.2f%s\ncy=%10.2f%s\nradius=%10.2f%s", circ->cx, circ->units, circ->cy, circ->units, circ->r, circ->units);
	tempStr= addNodeDescToString(tempStr, circ->otherAttributes);
	return tempStr;
}

int compareCircles(const void *first, const void *second) {
	int same =0;
	Circle* a = (Circle*) first;
	Circle* b = (Circle*) second;

	if(!a || !b) {
		return 0;
	}

	same = strcmp(circleToString(a), circleToString(b));
	return same;
}

//***************************path***************************
void deletePath(void* data) {
	if(!data) {
		return;
	}
	Path* path = (Path*) data;
	freeList(path->otherAttributes);
	free(path->data);
	free(path);
}

char* pathToString( void* data) {
	if(!data) {
		return NULL;
	}
	Path* path = (Path*) data;
	char* tempStr = malloc(strlen(path->data) + 1);
	strcpy(tempStr, path->data);
	tempStr= addNodeDescToString(tempStr, path->otherAttributes);
	return tempStr;
}

int comparePaths(const void *first, const void *second) {
	int same =0;
	Path* a = (Path*) first;
	Path* b = (Path*) second;

	if(!a || !b) {
		return 0;
	}

	same = strcmp(pathToString(a), pathToString(b));
	return same;
}

//***************************getters***************************
//used for getter lists so original data isnt destroyed
void dontDelete(void* data) {
}

List* getGroupRects(Group* grp) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Rectangle* rect;
	Group* grps;
	lst = initializeList(rectangleToString, dontDelete, compareRectangles);
	itr = createIterator(grp->rectangles);

	for(rect = nextElement(&itr); rect; rect = nextElement(&itr)) {
		insertBack(lst, rect);
	}

	itr2 = createIterator(grp->groups);
	for(grps = nextElement(&itr2); grps; grps = nextElement(&itr2)) {
		tmpList = getGroupRects(grps);
		itr = createIterator(tmpList);
		for(rect = nextElement(&itr); rect; rect = nextElement(&itr)) {
			insertBack(lst, rect);
		}
		freeList(tmpList);
	}
	return lst;
}

//adds all rectangles from svg image to a list and then calls getgrouprects to get all rectangles from the groups
List* getRects(SVGimage* img) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Rectangle* rect;
	Group* grp;
	if(!img) {
		return NULL;
	}
	lst = initializeList(rectangleToString, dontDelete, compareRectangles);
	itr = createIterator(img->rectangles);

	for(rect = nextElement(&itr); rect; rect = nextElement(&itr)) {
		insertBack(lst, rect);
	}

	itr2 = createIterator(img->groups);
	for(grp = nextElement(&itr2); grp; grp = nextElement(&itr2)) {
		tmpList = getGroupRects(grp);
		itr = createIterator(tmpList);
		for(rect = nextElement(&itr); rect; rect = nextElement(&itr)) {
			insertBack(lst, rect);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getConstRects(const SVGimage* img) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Rectangle* rect;
	Group* grp;
	if(!img) {
		return NULL;
	}
	lst = initializeList(rectangleToString, dontDelete, compareRectangles);
	itr = createIterator(img->rectangles);

	for(rect = nextElement(&itr); rect; rect = nextElement(&itr)) {
		insertBack(lst, rect);
	}

	itr2 = createIterator(img->groups);
	for(grp = nextElement(&itr2); grp; grp = nextElement(&itr2)) {
		tmpList = getGroupRects(grp);
		itr = createIterator(tmpList);
		for(rect = nextElement(&itr); rect; rect = nextElement(&itr)) {
			insertBack(lst, rect);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getGroupCircs(Group* grp) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Circle* circ;
	Group* grps;
	lst = initializeList(circleToString, dontDelete, compareCircles);
	itr = createIterator(grp->circles);

	for(circ = nextElement(&itr); circ; circ = nextElement(&itr)) {
		insertBack(lst, circ);
	}

	itr2 = createIterator(grp->groups);
	for(grps = nextElement(&itr2); grps; grps = nextElement(&itr2)) {
		tmpList = getGroupCircs(grps);
		itr = createIterator(tmpList);
		for(circ = nextElement(&itr); circ; circ = nextElement(&itr)) {
			insertBack(lst, circ);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getCircles(SVGimage* img) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Circle* circ;
	Group* grp;
	if(!img) {
		return NULL;
	}
	lst = initializeList(circleToString, dontDelete, compareCircles);
	itr = createIterator(img->circles);

	for(circ = nextElement(&itr); circ; circ = nextElement(&itr)) {
		insertBack(lst, circ);
	}

	itr2 = createIterator(img->groups);
	for(grp = nextElement(&itr2); grp; grp = nextElement(&itr2)) {
		tmpList = getGroupCircs(grp);
		itr = createIterator(tmpList);
		for(circ = nextElement(&itr); circ; circ = nextElement(&itr)) {
			insertBack(lst, circ);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getConstCircles(const SVGimage* img) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Circle* circ;
	Group* grp;
	if(!img) {
		return NULL;
	}
	lst = initializeList(circleToString, dontDelete, compareCircles);
	itr = createIterator(img->circles);

	for(circ = nextElement(&itr); circ; circ = nextElement(&itr)) {
		insertBack(lst, circ);
	}

	itr2 = createIterator(img->groups);
	for(grp = nextElement(&itr2); grp; grp = nextElement(&itr2)) {
		tmpList = getGroupCircs(grp);
		itr = createIterator(tmpList);
		for(circ = nextElement(&itr); circ; circ = nextElement(&itr)) {
			insertBack(lst, circ);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getGroupPaths(Group* grp) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Path* path;
	Group* grps;
	lst = initializeList(pathToString, dontDelete, comparePaths);
	itr = createIterator(grp->paths);

	for(path = nextElement(&itr); path; path = nextElement(&itr)) {
		insertBack(lst, path);
	}

	itr2 = createIterator(grp->groups);
	for(grps = nextElement(&itr2); grps; grps = nextElement(&itr2)) {
		tmpList = getGroupPaths(grps);
		itr = createIterator(tmpList);
		for(path = nextElement(&itr); path; path = nextElement(&itr)) {
			insertBack(lst, path);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getPaths(SVGimage* img) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Path* path;
	Group* grp;
	if(!img) {
		return NULL;
	}
	lst = initializeList(pathToString, dontDelete, comparePaths);
	itr = createIterator(img->paths);

	for(path = nextElement(&itr); path; path = nextElement(&itr)) {
		insertBack(lst, path);
	}

	itr2 = createIterator(img->groups);
	for(grp = nextElement(&itr2); grp; grp = nextElement(&itr2)) {
		tmpList = getGroupPaths(grp);
		itr = createIterator(tmpList);
		for(path = nextElement(&itr); path; path = nextElement(&itr)) {
			insertBack(lst, path);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getConstPaths(const SVGimage* img) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Path* path;
	Group* grp;
	if(!img) {
		return NULL;
	}
	lst = initializeList(pathToString, dontDelete, comparePaths);
	itr = createIterator(img->paths);

	for(path = nextElement(&itr); path; path = nextElement(&itr)) {
		insertBack(lst, path);
	}

	itr2 = createIterator(img->groups);
	for(grp = nextElement(&itr2); grp; grp = nextElement(&itr2)) {
		tmpList = getGroupPaths(grp);
		itr = createIterator(tmpList);
		for(path = nextElement(&itr); path; path = nextElement(&itr)) {
			insertBack(lst, path);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getGroupGroups(Group* grp) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Group* groups;
	Group* grps;
	lst = initializeList(pathToString, dontDelete, comparePaths);
	itr = createIterator(grp->groups);

	for(groups = nextElement(&itr); groups; groups = nextElement(&itr)) {
		insertBack(lst, groups);
	}

	itr2 = createIterator(grp->groups);
	for(grps = nextElement(&itr2); grps; grps = nextElement(&itr2)) {
		tmpList = getGroupGroups(grps);
		itr = createIterator(tmpList);
		for(groups = nextElement(&itr); groups; groups = nextElement(&itr)) {
			insertBack(lst, groups);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getGroups(SVGimage* img) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Group* groups;
	Group* grp;
	if(!img) {
		return NULL;
	}
	lst = initializeList(groupToString, dontDelete, compareGroups);
	itr = createIterator(img->groups);

	for(groups = nextElement(&itr); groups; groups = nextElement(&itr)) {
		insertBack(lst, groups);
	}

	itr2 = createIterator(img->groups);
	for(grp = nextElement(&itr2); grp; grp = nextElement(&itr2)) {
		tmpList = getGroupGroups(grp);
		itr = createIterator(tmpList);
		for(groups = nextElement(&itr); groups; groups = nextElement(&itr)) {
			insertBack(lst, groups);
		}
		freeList(tmpList);
	}
	return lst;
}

List* getConstGroups(const SVGimage* img) {
	List* lst;
	List* tmpList;
	ListIterator itr;
	ListIterator itr2;
	Group* groups;
	Group* grp;
	if(!img) {
		return NULL;
	}
	lst = initializeList(groupToString, dontDelete, compareGroups);
	itr = createIterator(img->groups);

	for(groups = nextElement(&itr); groups; groups = nextElement(&itr)) {
		insertBack(lst, groups);
	}

	itr2 = createIterator(img->groups);
	for(grp = nextElement(&itr2); grp; grp = nextElement(&itr2)) {
		tmpList = getGroupGroups(grp);
		itr = createIterator(tmpList);
		for(groups = nextElement(&itr); groups; groups = nextElement(&itr)) {
			insertBack(lst, groups);
		}
		freeList(tmpList);
	}
	return lst;
}
/*there was way too much copy pasting here theres gotta be a better way of doing this*/

//***************************finders***************************
//checks the type and returns the appropreate list from svg image
List* getTypeImg(SVGimage* img, char* type) {
	List* data = NULL;
	if(!strcmp(type, "Rect")) {
		data = img->rectangles;
	}
	else if(!strcmp(type, "Circ")) {
		data = img->circles;
	}
	else if(!strcmp(type, "Path")) {
		data = img->paths;
	}
	else if(!strcmp(type, "Attr")) {
		data = img->otherAttributes;
	}
	else if(!strcmp(type, "Group")) {
		data = img->groups;
	}

	return data;
}

//checks the type and returns the appropreate list from group
List* getTypeGrp(Group* grp, char* type) {
	List* data = NULL;
	if(!strcmp(type, "Rect")) {
		data = grp->rectangles;
	}
	else if(!strcmp(type, "Circ")) {
		data = grp->circles;
	}
	else if(!strcmp(type, "Path")) {
		data = grp->paths;
	}
	else if(!strcmp(type, "Attr")) {
		data = grp->otherAttributes;
	}
	else if(!strcmp(type, "Group")) {
		data = grp->groups;
	}

	return data;
}

//generic search function for group (requires type to compare against searchfor)
int findNumElementsGrp(Group* grp, bool (*customCompare)(const void* first,const void* second), const void* searchFor, char* type){
	int count =0;
	List* list;
	if (!customCompare || !grp || !searchFor || !type)
		return 0;

	list = getTypeGrp(grp, type);
	ListIterator itr = createIterator(list);

	

	void* data = nextElement(&itr);
	while (data != NULL)
	{
		if (customCompare(data, searchFor))
			count++;

		data = nextElement(&itr);
	}

	itr = createIterator(grp->groups);
	//goes into each group and adds any elements that fit the criteria to the count
	for(data= nextElement(&itr); data; data= nextElement(&itr)) {
		count += findNumElementsGrp(data, customCompare, searchFor, type);
	}

	return count;
}

//generic search function for svg image (requires type to compare against searchfor)
int findNumElementsImg(SVGimage* img, bool (*customCompare)(const void* first,const void* second), const void* searchFor, char* type){
	int count =0;
	List* list;
	ListIterator itr;
	if (!customCompare || !img || !searchFor || !type)
		return 0;

	list = getTypeImg(img, type);

	itr = createIterator(list);

	void* data = nextElement(&itr);
	while (data != NULL)
	{
		if (customCompare(data, searchFor))
			count++;

		data = nextElement(&itr);
	}

	itr = createIterator(img->groups);
	//goes into each group and adds any elements that fit the criteria to the count
	for(data= nextElement(&itr); data; data= nextElement(&itr)) {
		count += findNumElementsGrp(data, customCompare, searchFor, type);
	}

	return count;
}

//compares rectangles area vs a constant area (rounded up)
bool cmpRectArea(const void* first, const void* second) {
	float areaA;
	Rectangle* a =(Rectangle*) first;
	float* areaB =(float*) second;

	areaA = ((a->width) * (a->height));

	if(ceil(areaA) != ceil(*areaB)) {
		return false;
	}
	return true;
}

//compares circles area vs a constant area (rounded up)
bool cmpCirclesArea(const void* first, const void* second) {
	float areaA;
	Circle* a =(Circle*) first;
	float* areaB = (float*) second;

	areaA = ((a->r) * (a->r) *  M_PI);

	if(ceil(areaA) != ceil(*areaB)) {
		return false;
	}
	return true;
}

//compares path data string to a constant string
bool cmpPathData(const void* first, const void* second) {
	Path* a =(Path*) first;
	char* data =(char*) second;

	if(strcmp(a->data, data)) {
		return false;
	}
	return true;
}

//compares the length of a group vs a constant
bool cmpGroupLen(const void* first, const void* second) {
	int lengthA =0;
	Group* a =(Group*) first;
	int* lengthB =(int*) second;

	lengthA = getLength(a->rectangles);
	lengthA += getLength(a->circles);
	lengthA += getLength(a->paths);
	lengthA += getLength(a->groups);

	if(lengthA != *lengthB) {
		return false;
	}
	return true;
}

//recursively goes into groups adding the total number of attributes and returning it
int getAttrLen(Group* grp) {
	int sum =0;
	ListIterator itr;

	itr = createIterator(grp->rectangles);
	for(Rectangle* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getLength(data->otherAttributes);
	}

	itr = createIterator(grp->circles);
	for(Circle* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getLength(data->otherAttributes);
	}
	
	itr = createIterator(grp->paths);
	for(Path* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getLength(data->otherAttributes);
	}
	
	itr = createIterator(grp->groups);
	for(Group* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getLength(data->otherAttributes);
	}

	itr = createIterator(grp->groups);
	for(Group* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getAttrLen(data);
	}
	
	return sum;
}

//***************************required finders***************************
//gets number of rectangles with same area (rounded up to nearest int)
int numRectsWithArea(SVGimage* img, float area) {
	int sum =0;
	if(!img) {
		return 0;
	}
	char* type = malloc(sizeof("Rect"));
	strcpy(type, "Rect");
	sum = findNumElementsImg(img, cmpRectArea, &area, type);
	free(type);
	return sum;
}

//gets number of circles with same area (rounded up to nearest int)
int numCirclesWithArea(SVGimage* img, float area) {
	int sum =0;
	if(!img) {
		return 0;
	}
	char* type = malloc(sizeof("Circ"));
	strcpy(type, "Circ");
	
	sum = findNumElementsImg(img, cmpCirclesArea, &area, type);
	free(type);
	return sum;
}

//gets number of paths with same string as data
int numPathsWithdata(SVGimage* img, char* data) {
	int sum =0;
	if(!img) {
		return 0;
	}
	char* type = malloc(sizeof("Path"));
	strcpy(type, "Path");
	
	sum = findNumElementsImg(img, cmpPathData, data, type);
	free(type);
	return sum;
}

//gets number of groups with same length as len
int numGroupsWithLen(SVGimage* img, int len) {
	int sum =0;
	if(!img) {
		return 0;
	}
	char* type = malloc(sizeof("Group"));
	strcpy(type, "Group");
	
	sum = findNumElementsImg(img, cmpGroupLen, &len, type);
	free(type);
	return sum;
}

//gets total number of attributes in image
int numAttr(SVGimage* img) {
	int sum =0;
	ListIterator itr;
	if(!img) {
		return 0;
	}

	sum += getLength(img->otherAttributes);

	itr = createIterator(img->rectangles);
	for(Rectangle* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getLength(data->otherAttributes);
	}

	itr = createIterator(img->circles);
	for(Circle* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getLength(data->otherAttributes);
	}
	
	itr = createIterator(img->paths);
	for(Path* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getLength(data->otherAttributes);
	}
	
	itr = createIterator(img->groups);
	for(Group* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getLength(data->otherAttributes);
	}

	itr = createIterator(img->groups);
	for(Group* data = nextElement(&itr); data; data = nextElement(&itr))
	{
		sum += getAttrLen(data);
	}
	
	return sum;
}

//***************************a2 mod 2+ stuff***************************
void setAttribute(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
	ListIterator itr;
	if(!image || !newAttribute) {
		return;
	}
	if(elemType >4 || elemType <0) {
		return;
	} else if(elemType == SVG_IMAGE) {
		if(!strcmp(newAttribute->name, "title")) {
			strncpy(image->title, newAttribute->value, 255);
		} else if(!strcmp(newAttribute->name, "namespace")) {
			strncpy(image->namespace, newAttribute->value, 255);
		} else if(!strcmp(newAttribute->name, "description")) {
			strncpy(image->description, newAttribute->value, 255);
		} else {
			setOtherAttr(image->otherAttributes, newAttribute);
			return;
		}
	} else if(elemType == RECT) {
		Rectangle* rect;
		if(getLength(image->rectangles)-1 < elemIndex) {
			return;
		}
		itr = createIterator(image->rectangles);
		for(int i=0; i<=elemIndex; i++) {
			rect =nextElement(&itr);
		}
		if(!strcmp(newAttribute->name, "x")) {
			sscanf(newAttribute->value, "%f", &(rect->x));
		} else if(!strcmp(newAttribute->name, "y")) {
			sscanf(newAttribute->value, "%f", &(rect->y));
		} else if(!strcmp(newAttribute->name, "width")) {
			sscanf(newAttribute->value, "%f", &(rect->width));
		} else if(!strcmp(newAttribute->name, "height")) {
			sscanf(newAttribute->value, "%f", &(rect->height));
		} else if(!strcmp(newAttribute->name, "units")) {
			strncpy(rect->units, newAttribute->value, 50);
		} else {
			setOtherAttr(rect->otherAttributes, newAttribute);
			return;
		}
	} else if(elemType == CIRC) {
		Circle* circ;
		if(getLength(image->circles)-1 < elemIndex) {
			return;
		}
		itr = createIterator(image->circles);
		for(int i=0; i<=elemIndex; i++) {
			circ =nextElement(&itr);
		}
		if(!strcmp(newAttribute->name, "cx")) {
			sscanf(newAttribute->value, "%f", &(circ->cx));
		} else if(!strcmp(newAttribute->name, "cy")) {
			sscanf(newAttribute->value, "%f", &(circ->cy));
		} else if(!strcmp(newAttribute->name, "r")) {
			sscanf(newAttribute->value, "%f", &(circ->r));
		} else if(!strcmp(newAttribute->name, "units")) {
			strncpy(circ->units, newAttribute->value, 50);
		} else {
			setOtherAttr(circ->otherAttributes, newAttribute);
			return;
		}
	} else if(elemType == PATH) {
		Path* path;
		if(getLength(image->paths)-1 < elemIndex) {
			return;
		}
		itr = createIterator(image->paths);
		for(int i=0; i<=elemIndex; i++) {
			path =nextElement(&itr);
		}
		if(!strcmp(newAttribute->name, "d")) {
			path->data = realloc(path->data, strlen(newAttribute->value) +1);
			strcpy(path->data, newAttribute->value);
		}else {
			setOtherAttr(path->otherAttributes, newAttribute);
			return;
		}
	} else if(elemType == GROUP) {
		Group* grp;
		if(getLength(image->groups)-1 < elemIndex) {
			return;
		}
		itr = createIterator(image->groups);
		for(int i=0; i<=elemIndex; i++) {
			grp =nextElement(&itr);
		}
		setOtherAttr(grp->otherAttributes, newAttribute);
		return;
	} else {
	}

	//everything given should be freed here
	free(newAttribute->name);
	free(newAttribute->value);
	free(newAttribute);
}

//sets attributes other than the prime ones (x,y,height,width,cx,xy,r,data)
void setOtherAttr(List* list, Attribute* newAttribute) {
	Attribute* attr;

	attr = findElement(list, cmpAttributes, newAttribute);
	if(attr) {
		attr->value = realloc(attr->value, strlen(newAttribute->value) +1);
		strcpy(attr->value, newAttribute->value);
		free(newAttribute->name);
		free(newAttribute->value);
		free(newAttribute);
		return;
	}
	insertBack(list, newAttribute);
	
	/*itr =createIterator(list);
	for(attr= nextElement(&itr); attr; attr=nextElement(&itr)) {
		if(!strcmp(attr->name, newAttribute->name)) {
			attr->value = realloc(attr->value, strlen(newAttribute->value) +1);
			strcpy(attr->value, newAttribute->value);
			free(newAttribute->value);
			free(newAttribute->name);
			free(newAttribute);
			return;
		}
	}*/

}

//adds individual svg components and tests if theyre valid
void addComponent(SVGimage* image, elementType elemType, void* newComponent) {
	xmlNodePtr node;
	if(!image || !newComponent) {
		return;
	}
	node =xmlNewNode(NULL, BAD_CAST "root");
	if(elemType<1 || elemType>4) {
		return;
	} else if(elemType == RECT) {
		insertBack(image->rectangles, newComponent);
		//checks validity of the rectangle
		if(!writeSVGRect(image->rectangles, node)) {
			//if not vaild remove it
			deleteDataFromList(image->rectangles, newComponent);
		}
	} else if(elemType == CIRC) {
		insertBack(image->circles, newComponent);
		//checks validity of the circle
		if(!writeSVGCirc(image->circles, node)) {
			//if not vaild remove it
			deleteDataFromList(image->circles, newComponent);
		}
	} else if(elemType == PATH) {
		insertBack(image->paths, newComponent);
		//checks validity of the path
		if(!writeSVGPath(image->paths, node)) {
			//if not vaild remove it
			deleteDataFromList(image->paths, newComponent);
		}
		//just checked specs and this shouldnt be allowed (insterting groups) but im gonna leave it anyways
		//i already did it so theres no point in removing it(its not like theyll doc marks for additional functionality)
	} else if(elemType == GROUP) {
		insertBack(image->groups, newComponent);
		//checks validity of the group
		if(!writeSVGGroup(image->groups, node)) {
			//if not vaild remove it
			deleteDataFromList(image->groups, newComponent);
		}
	} else{
		return;
	}
	xmlFreeNode(node);
}

//***************************to json functions***************************
char* attrToJSON(const Attribute *a) {
	char* tempStr= NULL;
	if(!a) {
		tempStr =addStr(tempStr, "{}");
		return tempStr;
	}
	tempStr = malloc(sizeof("{\"name\":\"\",\"value\":\"\"}") + strlen(a->name) + strlen(a->value));
	sprintf(tempStr, "{\"name\":\"%s\",\"value\":\"%s\"}", a->name, a->value);
	return tempStr;
}

char* circleToJSON(const Circle *c) {
	char* tempStr= NULL;
	char tmp[1000];
	int attrLen=0;
	int sizes[5];
	if(!c) {
		tempStr =addStr(tempStr, "{}");
		return tempStr;
	}
	sprintf(tmp, "%.2lf", c->cx);
	sizes[0]=strlen(tmp);
	sprintf(tmp, "%.2lf", c->cy);
	sizes[1]=strlen(tmp);
	sprintf(tmp, "%.2lf", c->r);
	sizes[2]=strlen(tmp);
	if(c->otherAttributes) {
		attrLen=getLength(c->otherAttributes);
	}
	sprintf(tmp, "%d", attrLen);
	sizes[3]=strlen(tmp);
	sizes[4]=strlen(c->units);
	tempStr = malloc(sizeof("{\"cx\":,\"cy\":,\"r\":,\"numAttr\":,\"units\":\"\"}") + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4]);
	sprintf(tempStr, "{\"cx\":%.2lf,\"cy\":%.2lf,\"r\":%.2lf,\"numAttr\":%d,\"units\":\"%s\"}", c->cx, 
																								c->cy, 
																								c->r, 
																								getLength(c->otherAttributes), 
																								c->units);
	return tempStr;
}

char* rectToJSON(const Rectangle *r) {
	char* tempStr= NULL;
	char tmp[1000];
	int attrLen=0;
	int sizes[6];
	if(!r) {
		tempStr =addStr(tempStr, "{}");
		return tempStr;
	}
	sprintf(tmp, "%.2lf", r->x);
	sizes[0]=strlen(tmp);
	sprintf(tmp, "%.2lf", r->y);
	sizes[1]=strlen(tmp);
	sprintf(tmp, "%.2lf", r->width);
	sizes[2]=strlen(tmp);
	sprintf(tmp, "%.2lf", r->height);
	sizes[3]=strlen(tmp);
	if(r->otherAttributes) {
		attrLen=getLength(r->otherAttributes);
	}
	sprintf(tmp, "%d", attrLen);
	sizes[4]=strlen(tmp);
	sizes[5]=strlen(r->units);
	tempStr = malloc(sizeof("{\"x\":,\"y\":,\"w\":,\"h\":,\"numAttr\":,\"units\":\"\"}") + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] +sizes[5]);
	sprintf(tempStr, "{\"x\":%.2lf,\"y\":%.2lf,\"w\":%.2lf,\"h\":%.2lf,\"numAttr\":%d,\"units\":\"%s\"}", 	r->x, 
																											r->y, 
																											r->width, 
																											r->height, 
																											getLength(r->otherAttributes), 
																											r->units);
	return tempStr;
}

char* pathToJSON(const Path *p) {
	char* tempStr= NULL;
	char tmp[1000];
	int attrLen=0;
	int sizes[2];
	if(!p) {
		tempStr =addStr(tempStr, "{}");
		return tempStr;
	}
	if(p->otherAttributes) {
		attrLen=getLength(p->otherAttributes);
	}
	sprintf(tmp, "%d", attrLen);
	sizes[1]=strlen(tmp);
	if(p->data) {
		strncpy(tmp, p->data, 64);
		tmp[64] ='\0';
		sizes[0]=strlen(tmp);
	} else {
		strcpy(tmp, "");
	}
	tempStr = malloc(sizeof("{\"d\":\"\",\"numAttr\":}") + sizes[0] + sizes[1]);
	sprintf(tempStr, "{\"d\":\"%s\",\"numAttr\":%d}", tmp, attrLen);
	return tempStr;
}

char* groupToJSON(const Group *g) {
	char* tempStr= NULL;
	char tmp[1000];
	int sizes[2];
	int length =0;
	int attrLen =0;
	if(!g) {
		tempStr =addStr(tempStr, "{}");
		return tempStr;
	}

	//if any lists are null it sets their length to 0
	if(g->rectangles) {
		length+=getLength(g->rectangles);
	}
	if(g->circles) {
		length+=getLength(g->circles);
	}
	if(g->paths) {
		length+=getLength(g->paths);
	}
	if(g->groups) {
		length+=getLength(g->groups);
	}
	if(g->otherAttributes) {
		attrLen=getLength(g->otherAttributes);
	}
	sprintf(tmp, "%d", length);
	sizes[0]=strlen(tmp);
	sprintf(tmp, "%d", attrLen);
	sizes[1]=strlen(tmp);
	tempStr = malloc(sizeof("{\"children\":,\"numAttr\":}") + sizes[0] + sizes[1]);
	sprintf(tempStr, "{\"children\":%d,\"numAttr\":%d}", length, attrLen);
	return tempStr;
}

char* attrListToJSON(const List *list) {
	Attribute* attr;
	char* mainStr =NULL;
	char* tmp =NULL;
	if(!list || list->length ==0) {
		mainStr = addStr(mainStr, "[]");
		return mainStr;
	}
	mainStr =addStr(mainStr, "[");
	//did this instead of calling create iterator because it threw a warning at me for converting from const to non const
	ListIterator iter;
	iter.current = list->head;
	//convert every element into a json and add it to the main string
	for(attr=nextElement(&iter); attr; attr=nextElement(&iter)) {
		tmp =attrToJSON(attr);
		mainStr = realloc(mainStr, strlen(mainStr) + strlen(tmp) + sizeof(","));
		strcat(mainStr, tmp);
		strcat(mainStr, ",");
		free(tmp);
	}
	mainStr[strlen(mainStr) -1] =']';

	return mainStr;
}

char* circListToJSON(const List *list) {
	Circle* crc;
	char* mainStr =NULL;
	char* tmp =NULL;
	if(!list || list->length ==0) {
		mainStr = addStr(mainStr, "[]");
		return mainStr;
	}
	mainStr =addStr(mainStr, "[");
	//did this instead of calling create iterator because it threw a warning at me for converting from const to non const
	ListIterator iter;
	iter.current = list->head;
	//convert every element into a json and add it to the main string
	for(crc=nextElement(&iter); crc; crc=nextElement(&iter)) {
		tmp =circleToJSON(crc);
		mainStr = realloc(mainStr, strlen(mainStr) + strlen(tmp) + sizeof(","));
		strcat(mainStr, tmp);
		strcat(mainStr, ",");
		free(tmp);
	}
	mainStr[strlen(mainStr) -1] =']';

	return mainStr;
}

char* rectListToJSON(const List *list) {
	Rectangle* rect;
	char* mainStr =NULL;
	char* tmp =NULL;
	if(!list || list->length ==0) {
		mainStr = addStr(mainStr, "[]");
		return mainStr;
	}
	mainStr =addStr(mainStr, "[");
	//did this instead of calling create iterator because it threw a warning at me for converting from const to non const
	ListIterator iter;
	iter.current = list->head;
	//convert every element into a json and add it to the main string
	for(rect=nextElement(&iter); rect; rect=nextElement(&iter)) {
		tmp =rectToJSON(rect);
		mainStr = realloc(mainStr, strlen(mainStr) + strlen(tmp) + sizeof(","));
		strcat(mainStr, tmp);
		strcat(mainStr, ",");
		free(tmp);
	}
	mainStr[strlen(mainStr) -1] =']';

	return mainStr;
}

char* pathListToJSON(const List *list) {
	Path* path;
	char* mainStr =NULL;
	char* tmp =NULL;
	if(!list || list->length ==0) {
		mainStr = addStr(mainStr, "[]");
		return mainStr;
	}
	mainStr =addStr(mainStr, "[");
	//did this instead of calling create iterator because it threw a warning at me for converting from const to non const
	ListIterator iter;
	iter.current = list->head;
	//convert every element into a json and add it to the main string
	for(path=nextElement(&iter); path; path=nextElement(&iter)) {
		tmp =pathToJSON(path);
		mainStr = realloc(mainStr, strlen(mainStr) + strlen(tmp) + sizeof(","));
		strcat(mainStr, tmp);
		strcat(mainStr, ",");
		free(tmp);
	}
	mainStr[strlen(mainStr) -1] =']';

	return mainStr;
}

char* groupListToJSON(const List *list) {
	Group* grp;
	char* mainStr =NULL;
	char* tmp =NULL;
	if(!list || list->length ==0) {
		mainStr = addStr(mainStr, "[]");
		return mainStr;
	}
	mainStr =addStr(mainStr, "[");
	//did this instead of calling create iterator because it threw a warning at me for converting from const to non const
	ListIterator iter;
	iter.current = list->head;
	//convert every element into a json and add it to the main string
	for(grp=nextElement(&iter); grp; grp=nextElement(&iter)) {
		tmp =groupToJSON(grp);
		mainStr = realloc(mainStr, strlen(mainStr) + strlen(tmp) + sizeof(","));
		strcat(mainStr, tmp);
		strcat(mainStr, ",");
		free(tmp);
	}
	mainStr[strlen(mainStr) -1] =']';

	return mainStr;
}

char* SVGtoJSON(const SVGimage* imge) {
	List* rects;
	List* circles;
	List* paths;
	List* groups;

	char* tempStr= NULL;
	char tmp[1000];
	int sizes[4];
	if(!imge) {
		tempStr =addStr(tempStr, "{}");
		return tempStr;
	}

	rects =getConstRects(imge);
	circles =getConstCircles(imge);
	paths =getConstPaths(imge);
	groups =getConstGroups(imge);

	sprintf(tmp, "%d", getLength(rects));
	sizes[0]=strlen(tmp);
	sprintf(tmp, "%d", getLength(circles));
	sizes[1]=strlen(tmp);
	sprintf(tmp, "%d", getLength(paths));
	sizes[2]=strlen(tmp);
	sprintf(tmp, "%d", getLength(groups));
	sizes[3]=strlen(tmp);
	tempStr = malloc(sizeof("{\"rects\":,\"circs\":,\"paths\":,\"groups\":}") + sizes[0] + sizes[1] + sizes[2] + sizes[3]);
	sprintf(tempStr, "{\"rects\":%d,\"circs\":%d,\"paths\":%d,\"groups\":%d}", 				getLength(rects),
																							getLength(circles),
																							getLength(paths),
																							getLength(groups));

	freeList(rects);
	freeList(circles);
	freeList(paths);
	freeList(groups);
	return tempStr;
}

//not required
SVGimage* JSONtoSVG(const char* svgString) {
	char* tmp;
	int initLen;
	int i;
	SVGimage* img;
	if(!svgString) {
		return NULL;
	}
	//initialize everything
	img =malloc(sizeof(SVGimage));
	strcpy(img->namespace, "http://www.w3.org/2000/svg");
	img->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
	img->circles = initializeList(circleToString, deleteCircle, compareCircles);
	img->paths = initializeList(pathToString, deletePath, comparePaths);
	img->groups = initializeList(groupToString, deleteGroup, compareGroups);
	img->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
	
	tmp = malloc(strlen(svgString)+1);
	//find starting point for title
	initLen =strlen("{\"title\":\"");
	//copy everything after the filler at the beginning
	strcpy(tmp, svgString+initLen);
	//find ending point for title
	for(i =initLen; i <strlen(svgString); i++) {
		if(svgString[i] == '\"' && svgString[i+1] == ',') {
			break;
		}
	}
	/*if(i -initLen >49) {
		i =49 +initLen;
		img->title[49] ='\0';
	}*/
	//end the string at the ending point
	tmp[i-initLen] ='\0';
	//copy up till ending point/ 255 characters (whichever comes first)
	strncpy(img->title, tmp, 255);
	//add null terminator just in case
	img->title[255] ='\0';

	//find the next element in the list
	for(i =initLen; i <strlen(svgString); i++) {
		if(svgString[i] == ',') {
			break;
		}
	}
	initLen = i;
	//find the location of the next value
	for(i =initLen; i <strlen(svgString); i++) {
		if(svgString[i] == ':') {
			break;
		}
	}
	//set initlen to the position of the start of the string
	initLen = i+2;
	//find end point of string
	for(i =initLen; i <strlen(svgString); i++) {
		if(svgString[i] == '\"' && svgString[i+1] == '}') {
			break;
		}
	}
	//copy everything after start point
	strcpy(tmp, svgString +initLen);
	//add null terminator after end point
	tmp[strlen(tmp)-2] ='\0';
	//copy up till ending point/ 255 characters (whichever comes first)
	strncpy(img->description, tmp, 255);
	//add null terminator just in case
	img->description[255] ='\0';

	//start after quotation mark before actual title data (too complex)
	/*for(int i=strlen("{\"title\":\"") -1;i <strlen(svgString); i++) {
		for(int j=i+1; j<strlen(svgString); i++) {
			if(svgString[j] == '\"') {
				if((j-i)>255) {
					for(int k=i+1; k<i+256; k++) {
						tmp[k-i-1] = svgString[k];
					}
					tmp[256] ='\0';
				} else {
					for(int k=i+1; k<j; k++) {
						tmp[k-i-1] = svgString[k];
					}
					tmp[j] ='\0';
				}
				if(i<11) {
					strcpy(img->title, tmp);
					i = j+strlen(",\"descr\":\"") -1;
				} else {
					strcpy(img->description, tmp);
					return img;
				}
			}
		}
	} //end of reading in title and desc loop*/
	free(tmp);
	return img;
}

Rectangle* JSONtoRect(const char* svgString) {
	char* tmp;
	int i;
	Rectangle* rect;
	if(!svgString) {
		return NULL;
	}
	rect=malloc(sizeof(Rectangle));
	rect->x =0;
	rect->y =0;
	rect->width =0;
	rect->height =0;
	strcpy(rect->units, "");
	rect->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
	sscanf(svgString, "{\"x\":%f,\"y\":%f,\"w\":%f,\"h\":%f", &rect->x, &rect->y, &rect->width, &rect->height);
	//if i start at the end then i can look for the first : and that will denote when the units are 2 chars back
	for(i=strlen(svgString);i >0; i--) {
		if(svgString[i] == ':') {
			break;
		}
	}
	i +=2;
	tmp = malloc(strlen(svgString) +1);
	strcpy(tmp, svgString +i);
	tmp[strlen(tmp)-2] = '\0';
	strncpy(rect->units, tmp, 49);
	rect->units[49] ='\0';
	free(tmp);
	/*for(int i=strlen("{\"x\":,\"y\":,\"w\":,\"h\":,\"") -1;i <strlen(svgString); i++) {
		if(svgString[i] != '\"' || svgString[i+1] != 'u') {

		} else {
			for(int j=i+1; j<strlen(svgString); i++) {
				if(svgString[j] == '\"') {
					if((j-i)>255) {
						for(int k=i+1; k<i+256; k++) {
							tmp[k-i-1] = svgString[k];
						}
						tmp[256] ='\0';
					} else {
						for(int k=i+1; k<j; k++) {
							tmp[k-i-1] = svgString[k];
						}
						tmp[j] ='\0';
					}
					if(i<11) {
						strcpy(img->title, tmp);
						i = j+strlen(",\"descr\":\"") -1;
					} else {
						strcpy(img->description, tmp);
						return rect;
					}
				}
			}//end of checking for 
		}//end of else statement
	} //end of reading in units loop*/
	return rect;
}
Circle* JSONtoCircle(const char* svgString) {
	char* tmp;
	int i;
	Circle* circ;
	if(!svgString) {
		return NULL;
	}
	circ=malloc(sizeof(Circle));
	circ->cx =0;
	circ->cy =0;
	circ->r =0;
	strcpy(circ->units, "");
	circ->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
	//get all float attributes
	sscanf(svgString, "{\"cx\":%f,\"cy\":%f,\"r\":%f", &circ->cx, &circ->cy, &circ->r);
	//if i start at the end then i can look for the first : and that will denote when the units are 2 chars back
	for(i=strlen(svgString);i >0; i--) {
		if(svgString[i] == ':') {
			break;
		}
	}
	i +=2;
	tmp = malloc(strlen(svgString) +1);
	strcpy(tmp, svgString +i);
	tmp[strlen(tmp)-2] = '\0';
	strncpy(circ->units, tmp, 49);
	circ->units[49] ='\0';

	free(tmp);
	return circ;
}

//wrapper functions (any function that doesnt return a json returns a string indicating success/ failure)
//turns filename into format readable by javascript /svglist
char* convSVGToJSON(char* fileName) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	json = SVGtoJSON(img);
	if(!json) {
		errors = addStr(errors, "file couldnt be parsed");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//finds the title of filenames svg and returns it as a string /titledesc
char* titleToJSON(char* fileName) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	json = malloc(strlen(img->title) +1);
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	strcpy(json, img->title);
	deleteSVGimage(img);
	free(errors);
	return json;
}

//finds the description of filenames svg and returns it as a string /titledesc
char* descToJSON(char* fileName) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	json = malloc(strlen(img->description) +1);
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	strcpy(json, img->description);
	deleteSVGimage(img);
	free(errors);
	return json;
}

//gets the list of rects from filename and returns it as a json string /components
char* rectsToJSON(char* fileName) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	json = rectListToJSON(img->rectangles);
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//gets the list of circs from filename and returns it as a json string /components
char* circsToJSON(char* fileName) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	json = circListToJSON(img->circles);
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//gets the list of paths from filename and returns it as a json string /components
char* pathsToJSON(char* fileName) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	json = pathListToJSON(img->paths);
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//gets the list of groups from filename and returns it as a json string /components
char* groupsToJSON(char* fileName) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	json = groupListToJSON(img->groups);
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//gets the list of attributes from shape at index in fileNames image /attributes
char* attributeToJSON(char* fileName, char* type, int index) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	if(strcmp(type, fileName+10) ==0) {
		json = attrListToJSON(img->otherAttributes);
	} else if(strcmp(type, "Rectangle ") ==0 || strcmp(type, "Rectangle") ==0) {
		ListIterator itr;
		List* lst =getRects(img);
		Rectangle* rect;
		itr =createIterator(lst);
		for(int i=0; i< index; i++) {
			rect= nextElement(&itr);
			if(!rect){
				errors = addStr(errors, "invalid index");
				return errors;
			}
		}
		json = attrListToJSON(rect->otherAttributes);
	} else if(strcmp(type, "Circle ")==0 || strcmp(type, "Circle")==0) {
		ListIterator itr;
		List* lst =getCircles(img);
		Circle* rect;
		itr =createIterator(lst);
		for(int i=0; i< index; i++) {
			rect= nextElement(&itr);
			if(!rect){
				errors = addStr(errors, "invalid index");
				return errors;
			}
		}
		json = attrListToJSON(rect->otherAttributes);
	} else if(strcmp(type, "Path ")==0 || strcmp(type, "Path")==0) {
		ListIterator itr;
		List* lst =getPaths(img);
		Path* rect;
		itr =createIterator(lst);
		for(int i=0; i< index; i++) {
			rect= nextElement(&itr);
			if(!rect){
				errors = addStr(errors, "invalid index");
				return errors;
			}
		}
		json = attrListToJSON(rect->otherAttributes);
	} else if(strcmp(type, "Group ")==0 || strcmp(type, "Group")==0) {
		ListIterator itr;
		List* lst =getGroups(img);
		Group* rect;
		itr =createIterator(lst);
		for(int i=0; i< index; i++) {
			rect= nextElement(&itr);
			if(!rect){
				errors = addStr(errors, "invalid index");
				return errors;
			}
		}
		json = attrListToJSON(rect->otherAttributes);
	} else {
		errors = addStr(errors, "invaild shape");
		return errors;
	}
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//changes title of filename /title
char* updateTitle(char* fileName, char* title) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	if(strlen(title) > 256) {
		errors = addStr(errors, "title is too long");
		return errors;
	}
	//add copy to title
	strcpy(img->title, title);
	if(!writeValidSVGimage(img, fileName, "./parser/resources/svg.xsd")) {
		errors = addStr(errors, "could not save file after modifications");
		return errors;
	}
	json = addStr(json, "success");
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//changes description of filename /desc
char* updateDesc(char* fileName, char* desc) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	if(strlen(desc) > 256) {
		errors = addStr(errors, "desc is too long");
		return errors;
	}
	//add copy to desc
	strcpy(img->description, desc);
	if(!writeValidSVGimage(img, fileName, "./parser/resources/svg.xsd")) {
		errors = addStr(errors, "could not save file after modifications");
		return errors;
	}
	json = addStr(json, "success");
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//add/change attribute types value to attrVal of shape at index of filename /attribute
char* addAttr(char* fileName, char* shape, int index, char* attrType, char* attrVal) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	elementType type;
	Attribute* attr;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	char* tmpType =malloc(strlen(attrType) +1);
	strcpy(tmpType, attrType);
	char* tmpval =malloc(strlen(attrVal) +1);
	strcpy(tmpval, attrVal);
	attr = malloc(sizeof(Attribute));
	attr->name =tmpType;
	attr->value=tmpval;
	index--;
	if(strcmp(shape, fileName+10) ==0) {
		type =SVG_IMAGE;
		setAttribute(img, type, index, attr);
	} else if(strcmp(shape, "Rectangle ") ==0 || strcmp(shape, "Rectangle") ==0) {
		type= RECT;
		setAttribute(img, type, index, attr);
	} else if(strcmp(shape, "Circle ")==0 || strcmp(shape, "Circle")==0) {
		type= CIRC;
		setAttribute(img, type, index, attr);
	} else if(strcmp(shape, "Path ")==0 || strcmp(shape, "Path")==0) {
		type= PATH;
		setAttribute(img, type, index, attr);
	} else if(strcmp(shape, "Group ")==0 || strcmp(shape, "Group")==0) {
		type= GROUP;
		setAttribute(img, type, index, attr);
	} else {
		errors = addStr(errors, "invaild shape");
		return errors;
	}
	if(!writeValidSVGimage(img, fileName, "./parser/resources/svg.xsd")) {
		errors = addStr(errors, "could not save file after modifications");
		return errors;
	}
	json = addStr(json, "Success");
	deleteSVGimage(img);
	free(errors);
	return json;
}

//add an attribute to shape
Attribute* addAttribute(char** tmp) {
	Attribute* attr;
	int i=0;
	char* type;
	char* val;
	//read the string untill its no longer the type
	while(isalpha(*tmp[i])) {
		i++;
	}
	i--;
	type =malloc(sizeof(char) * (i+1));
	//copy the type
	strncpy(*tmp, type, i);
	type[i+1]='\0';
	*tmp +=i;
	i=0;
	//read the string untill you reach the value
	while(!isalnum(*tmp[i])) {
		i++;
	}
	i--;
	*tmp +=i;
	i=0;
	while(isalnum(*tmp[i])) {
		i++;
	}
	i--;
	val =malloc(sizeof(char) *(i+1));
	strncpy(*tmp, val, i);
	val[i+1]='\0';
	*tmp +=i;
	attr= malloc(sizeof(Attribute));
	attr->name =type;
	attr->value =val;
	return attr;
}

//add a shape to filename with attributes /addshape
char* addShape(char* fileName, char* shape, char* attributes) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	if(strcmp(shape, "Rectangle ") ==0 || strcmp(shape, "Rectangle") ==0) {
		Rectangle* rec =malloc(sizeof(Rectangle));
		rec->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
		//default x & y settings
		rec->x =0.0;
		rec->y =0.0;
		rec->width =0.0;
		rec->height =0.0;
		strcpy(rec->units,"");
		char* tmp =malloc(strlen(attributes) +1);
		strcpy(tmp, attributes);
		char* tmp2 =tmp;
		char compare[256];
		char compare2[256];
		//find as many attributes as possible
		while(tmp2[0]!= '\0') {
			strncpy(compare, tmp2, 5);
			compare[5]='\0';
			strncpy(compare2, tmp2, 6);
			compare2[6]='\0';
			if(tmp2[0] == 'x') {
				//find the number
				while(!isdigit(tmp2[0])) {
					tmp2++;
				}
				//copy it
				sscanf(tmp2, "%f", &(rec->x));
				//move till the next value
				while(!isalpha(tmp2[0]) && tmp2[0] != '\0') {
					tmp2++;
				}
			} else if(tmp2[0] == 'y') {
				while(!isdigit(tmp2[0])) {
					tmp2++;
				}
				sscanf(tmp2, "%f", &(rec->y));
				//move till the next value
				while(!isalpha(tmp2[0]) && tmp2[0] != '\0') {
					tmp2++;
				}
			} else if(!strcmp(compare, "width") || !strcmp(compare, "Width")) {
				while(!isdigit(tmp2[0])) {
					tmp2++;
				}
				sscanf(tmp2, "%f", &(rec->width));
				//move till the next value
				while(!isalpha(tmp2[0]) && tmp2[0] != '\0') {
					tmp2++;
				}
			} else if(!strcmp(compare2, "height") || !strcmp(compare2, "Height")) {
				while(!isdigit(tmp2[0])) {
					tmp2++;
				}
				sscanf(tmp2, "%f", &(rec->height));
				//move till the next value
				while(!isalpha(tmp2[0]) && tmp2[0] != '\0') {
					tmp2++;
				}
			} else if (!strcmp(compare, "units") || !strcmp(compare, "Units")){ //non standard attributes are put in their own list
				while(tmp2[0] != '=') {
					tmp2++;
				}
				while(!isalpha(tmp2[0])){
					tmp2++;
				}
				char* tmpStr = malloc(strlen(tmp2) +1);
				sscanf(tmp2, "%s", tmpStr);//later add stuff that checks to make sure units are max 50 characters
				strncpy(rec->units, tmpStr, 50);
				rec->units[50] ='\0';
				free(tmpStr);
			} else {
				tmp2++;
				//attr = addAttribute(&tmp2);

				//add attrib to attributes list
				//insertBack(rec->otherAttributes, attr);
			}
		}
		insertBack(img->rectangles, rec);
		free(tmp);
	} else if(strcmp(shape, "Circle ")==0 || strcmp(shape, "Circle")==0) {
		Circle* rec =malloc(sizeof(Circle));
		rec->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
		//default x & y settings
		rec->cx =0.0;
		rec->cy =0.0;
		rec->r =0.0;
		strcpy(rec->units,"");
		char* tmp =malloc(strlen(attributes) +1);
		strcpy(tmp, attributes);
		char* tmp2 =tmp;
		char compare[256];
		char compare2[256];
		//find as many attributes as possible
		while(tmp2[0]!= '\0') {
			strncpy(compare, tmp2, 5);
			compare[5]='\0';
			strncpy(compare2, tmp2, 2);
			compare2[2]='\0';
			if(!strcmp(compare2, "cx")) {
				//find the number
				while(!isdigit(tmp2[0])) {
					tmp2++;
				}
				//copy it
				sscanf(tmp2, "%f", &(rec->cx));
				//move till the next value
				while(!isalpha(tmp2[0]) && tmp2[0] != '\0') {
					tmp2++;
				}
			} else if(!strcmp(compare2, "cy")) {
				while(!isdigit(tmp2[0])) {
					tmp2++;
				}
				sscanf(tmp2, "%f", &(rec->cy));
				//move till the next value
				while(!isalpha(tmp2[0]) && tmp2[0] != '\0') {
					tmp2++;
				}
			} else if(tmp2[0] == 'r') {
				while(!isdigit(tmp2[0])) {
					tmp2++;
				}
				sscanf(tmp2, "%f", &(rec->r));
				//move till the next value
				while(!isalpha(tmp2[0]) && tmp2[0] != '\0') {
					tmp2++;
				}
			} else if (!strcmp(compare, "units") || !strcmp(compare, "Units")){ //non standard attributes are put in their own list
				while(tmp2[0] != '=') {
					tmp2++;
				}
				while(!isalpha(tmp2[0])){
					tmp2++;
				}
				char* tmpStr = malloc(strlen(tmp2) +1);
				sscanf(tmp2, "%s", tmpStr);//later add stuff that checks to make sure units are max 50 characters
				strncpy(rec->units, tmpStr, 50);
				rec->units[50] ='\0';
				free(tmpStr);
			} else {
				tmp2++;
				//attr = addAttribute(&tmp2);

				//add attrib to attributes list
				//insertBack(rec->otherAttributes, attr);
			}
		}
		insertBack(img->circles, rec);
		free(tmp);
	} else {
		errors = addStr(errors, "invaild shape");
		return errors;
	}
	if(!writeValidSVGimage(img, fileName, "./parser/resources/svg.xsd")) {
		errors = addStr(errors, "could not save file after modifications");
		return errors;
	}
	json = addStr(json, "Success");
	deleteSVGimage(img);
	free(errors);
	return json;
}

//scale all shapes of fileName by factor /scaleshape
char* scaleShapes(char* fileName, char* shape, double factor) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img = createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(!img) {
		errors = addStr(errors, "file was invalid or not found");
		return errors;
	}
	if(strcmp(shape, "Rectangle ") ==0 || strcmp(shape, "Rectangle") ==0) {
		ListIterator itr;
		List* lst =getRects(img);
		Rectangle* rect;
		itr =createIterator(lst);
		for(rect =nextElement(&itr); rect; rect=nextElement(&itr)) {
			rect->height = rect->height *factor;
			rect->width = rect->width *factor;
		}
	} else if(strcmp(shape, "Circle ")==0 || strcmp(shape, "Circle")==0) {
		ListIterator itr;
		List* lst =getCircles(img);
		Circle* rect;
		itr =createIterator(lst);
		for(rect =nextElement(&itr); rect; rect=nextElement(&itr)) {
			rect->r = rect->r *factor;
		}
	} else {
		errors = addStr(errors, "invaild shape");
		return errors;
	}
	if(!writeValidSVGimage(img, fileName, "./parser/resources/svg.xsd")) {
		errors = addStr(errors, "could not save file after modifications");
		return errors;
	}
	json= addStr(json, "Success");
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}

//creates an empty svg file on the server
char* createXML(char* fileName) {
	char* errors = malloc(sizeof("empty"));
	char* json = NULL;
	strcpy(errors, "empty");
	SVGimage* img;
	img =createValidSVGimage(fileName, "./parser/resources/svg.xsd");
	if(img) {
		deleteSVGimage(img);
		errors =addStr(errors, "image exists");
		return errors;
	}
	//initialize everything
	img =malloc(sizeof(SVGimage));
	strcpy(img->namespace, "http://www.w3.org/2000/svg");
	img->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
	img->circles = initializeList(circleToString, deleteCircle, compareCircles);
	img->paths = initializeList(pathToString, deletePath, comparePaths);
	img->groups = initializeList(groupToString, deleteGroup, compareGroups);
	img->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
	strcpy(img->title, "");
	strcpy(img->description, "");

	if(!writeValidSVGimage(img, fileName, "./parser/resources/svg.xsd")) {
		errors = addStr(errors, "could not save file after modifications");
		return errors;
	}
	json= addStr(json, "Success");
	if(!json) {
		errors = addStr(errors, "no memory");
		return errors;
	}
	deleteSVGimage(img);
	free(errors);
	return json;
}
