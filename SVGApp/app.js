'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

let clib =ffi.Library('./libsvgparse.so', {
  'convSVGToJSON': ['string', ['string']],
  'titleToJSON': ['string', ['string']],
  'descToJSON': ['string', ['string']],
  'rectsToJSON': ['string', ['string']],
  'circsToJSON': ['string', ['string']],
  'pathsToJSON': ['string', ['string']],
  'groupsToJSON': ['string', ['string']],
  'attributeToJSON': ['string', ['string', 'string', 'int']],
  'updateTitle': ['string', ['string', 'string']],
  'updateDesc': ['string', ['string', 'string']],
  'addAttr': ['string', ['string', 'string', 'int', 'string', 'string']],
  'addShape': ['string', ['string', 'string', 'string']],
  'scaleShapes': ['string', ['string', 'string', 'double']],
  'createXML': ['string', ['string']]
});

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  console.log('upload request');
  if(!req.files || req.files.uploadFile == undefined) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
  let valid = uploadFile.name.endsWith(".svg");
  console.log("vaildity is " + valid);
  if(!valid) {
    return;
  }

  let exists =0;
  let files =fs.readdirSync("./uploads/");
  files.forEach(element => {
    if(element ==uploadFile.name) {
      exists =1;
      return;
    }
  });
  // Use the mv() method to place the file somewhere on your server
  if(exists ==0) {
    uploadFile.mv('uploads/' + uploadFile.name, function(err) {
      if(err) {
        return res.status(500).send(err);
      }
      console.log(uploadFile.name + ' was added');

    });
  } else {
    console.log('file by ' + uploadFile.name + ' already exists');
  }
  res.redirect('/');
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

//Sample endpoint
/*app.get('/someendpoint', function(req , res){
  let retStr = req.query.name1 + " " + req.query.name2;
  res.send({
    foo: retStr
  });
});*/

app.get('/svglist', function(req , res){
  let invalid =[]
  let list =[];
  let item =[];
  let imgComp =[]
  let compArr = {rects: -1, circs: -1, paths: -1, groups: -1};
  list =fs.readdirSync("./uploads/")
  let i=0;
  list.forEach(element => {
      
      //c library call to get number of rectangels circles etc and check if the file is valid
      let temp =clib.convSVGToJSON("./uploads/" + element);
      //if file is invalid flag it and keep it in untill all other files are uploaded
      if(temp == 'file was invalid or not found' || temp == 'file couldnt be parsed') {
        console.log(list[i] + ' ' + temp);
        invalid.push(i);
        imgComp[i] =compArr;
      } else {
        imgComp[i] =JSON.parse(temp);
      }
      item[i] =fs.statSync("./uploads/" + element)["size"];
      
      i++;
  });

  //remove flagged files
  invalid.forEach(element => {
    list.splice(element,1);
    item.splice(element,1);
    imgComp.splice(element,1);
  });
  
  res.send({
    lst: list,
    sizes: item,
    aspects: imgComp
  });
  console.log("sent svglist");
});

//gets title and description for a given image
app.get('/titledesc', function(req , res){
  let img = req.query.name;
  //c library call to get title and desc
  let title = clib.titleToJSON("./uploads/" + img);
  if(title == 'file was invalid or not found' || title == 'no memory') {
    console.log('title error ' + title);
    title = '';
  }

  let desc = clib.descToJSON("./uploads/" + img);
  if(desc == 'file was invalid or not found' || desc == 'no memory') {
    console.log('description error ' + desc);
    desc = '';
  }
  console.log('image= '+ img);
  console.log('title= '+ title);
  console.log('desc= '+ desc);
  res.send({
    title: title,
    desc: desc
  });
});

//returns summary of components of an image
app.get('/components', function(req , res){
  let img = req.query.name;
  let rects =[];
  let circs =[];
  let paths =[];
  let groups =[];
  //each component contains a summary of it + number of other attributes
  let compArr = {rects: rects, circs: circs, paths: paths, groups: groups};

  //function call here to c lib to find the file
  let tmp =clib.rectsToJSON('./uploads/' + img);
  if(tmp =="file was invalid or not found" || tmp =="no memory") {
    console.log(tmp);
  } else {
    let i=0;
    let tempRect =JSON.parse(tmp);
    tempRect.forEach(element => {
      rects[i] = {sum: "Upper left corner: x = "+element.x+element.units+", y = "+element.y+element.units+" Width: "+element.w+element.units+", Height: "+element.h+element.units, other: element.numAttr};
      i++;
    });
  }

  tmp =clib.circsToJSON('./uploads/' + img);
  if(tmp =="file was invalid or not found" || tmp =="no memory") {
    console.log(tmp);
  } else {
    let i=0;
    let tempCirc =JSON.parse(tmp);
    tempCirc.forEach(element => {
      circs[i] = {sum: "Center: x = "+element.cx+element.units+", y = "+element.cy+element.units+", radius: "+element.r+element.units, other: element.numAttr};
      i++;
    });
  }

  tmp =clib.pathsToJSON('./uploads/' + img);
  if(tmp =="file was invalid or not found" || tmp =="no memory") {
    console.log(tmp);
  } else {
    let i=0;
    let tempPath =JSON.parse(tmp);
    tempPath.forEach(element => {
      paths[i] = {sum: element.d, other: element.numAttr};
      i++;
    });
  }

  tmp =clib.groupsToJSON('./uploads/' + img);
  if(tmp =="file was invalid or not found" || tmp =="no memory") {
    console.log(tmp);
  } else {
    let i=0;
    let tempgroup =JSON.parse(tmp);
    tempgroup.forEach(element => {
      groups[i] = {sum: element.children + " child elements", other: element.numAttr};
      i++;
    });
  }

  console.log(img + "'s components were sent");
  res.send({
    svg: compArr
  });
});

//return attributes of an element
app.get('/attributes', function(req , res){
  let img = req.query.svg;
  let type = req.query.type;
  let index = req.query.index;
  //c library function call
  let attributestxt =clib.attributeToJSON('./uploads/' + img, type, index);
  let attributes;
  if(attributestxt == "file was invalid or not found" || attributestxt == "invalid index" || attributestxt == "no memory") {
    console.log(img+ ' '+ attributestxt)
    attributes =[];
  } else {
    attributes = JSON.parse(attributestxt);
  }
  console.log('attr returned');

  res.send({
    attr: attributes
  });
});

//update title
app.get('/title', function(req , res){
  let image = req.query.name;
  let title = req.query.title;
  //c library function call
  let val = clib.updateTitle("./uploads/" + image, title);
  console.log(val);
  if(val =='Success'){
    console.log("updated " + image + " title to " + title);
  }
  res.send({
  });
});

//update description
app.get('/desc', function(req , res){
  let image = req.query.name;
  let desc = req.query.desc;
  //c library function call
  let val = clib.updateDesc("./uploads/" + image, desc);
  console.log(val);
  if(val =='Success'){
    console.log("updated " + image + " desc to " + desc);
  }
  res.send({
  });
});

//add/change attributes of a shape
app.get('/attribute', function(req , res){
  let image = req.query.name;
  let shape = req.query.shape;
  let index = req.query.index;
  let attrType = req.query.type;
  let attrVal = req.query.val;
  //c library function call
  let val = clib.addAttr("./uploads/" + image, shape, index, attrType, attrVal);
  console.log(val);
  if(val == 'Success'){
    console.log("updated " + image + " shape " + shape + " at index " + index + " "+ attrType + " to " + attrVal);
  }
  res.send({
  });
});

//adds a shape to an image
app.get('/addshape', function(req , res){
  let image = req.query.img;
  let shape = req.query.shape;
  let attr = req.query.attr;
  //c library function call
  let val =clib.addShape('./uploads/' + image, shape, attr);
  console.log(val);
  if(val =='Success'){
    console.log("added " + shape + " to " + image + " with attributes " + attr);
  }
  res.send({
  });
});

//multiplies all given shapes by scale factor
app.get('/scaleshape', function(req , res){
  let image = req.query.img;
  let shape = req.query.shape;
  let factor = req.query.factor;
  //c library function call
  let val =clib.scaleShapes('./uploads/'+image, shape, factor);
  console.log(val);
  if(val =='Success'){
    console.log("scaled " + shape + "s in " + image + " to " + factor + "X");
  }
  res.send({
  });
});

//add new svg file
app.get('/newsvg', function(req , res){
  let image = req.query.name;
  //c library function call
  let exists =0;
  let files =fs.readdirSync("./uploads/");
  files.forEach(element => {
    if(element ==image) {
      exists =1;
      return;
    }
  });
  let val;
  if(exists ==0) {
    val =clib.createXML('./uploads/' +image);
  } else {
    val ='image exists';
  }
  
  console.log(val);
  if(val =='Success'){
    console.log('created '+ image);
  }
  res.send({
    result: val
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);