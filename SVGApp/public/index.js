// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    /*$.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/someendpoint',   //The server endpoint we are connecting to
        data: {
            name1: "Value 1",
            name2: "Value 2"
        },
        success: function (data) {
              Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            
            $('#blah').html("On page load, received string '"+data.foo+"' from server");
            //We write the object to the console to show that the request was successful
            console.log(data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("On page load, received error from server");
            console.log(error); 
        }
    });*/

    // On page-load list of svgs
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/svglist',   //The server endpoint we are connecting to
        data: {
        },
        success: function (data) {
            console.log("data " + data.lst);
            if(data.lst =="") {
                console.log("empty");
                let node = document.getElementById("table").insertRow();
                let noFile = document.createTextNode("No files");
                let image = node.insertCell(0);
                image.appendChild(noFile);
                return
            }
            console.log("sizes " + data.sizes);
            console.log("components " + data.aspects[0].rects)
            
            data.sizes.forEach(function(element, index){
                element = element/1000;
                data.sizes[index] = Math.round(element);
            });
            let array =data.lst;

            //update possible choices for view
            let node = document.getElementById("viewSelection");
            data.lst.forEach(element =>{
                let option =document.createElement("option");
                option.text =element;
                node.add(option);
            });

            //update shape adder images
            node =document.getElementById("shapeImageSelection");
            data.lst.forEach(element =>{
                let option =document.createElement("option");
                option.text =element;
                node.add(option);
            });
            
            let i=0;
            //add svg images to view panel
            array.forEach(element => {
                let node = document.getElementById("table").insertRow();

                let img = document.createElement("img");
                img.src = element;
                if(img.width >0 && img.height >0) {
                    let ratio =img.width/200;
                    img.width= img.width/ratio;
                    img.height =img.height/ratio;
                } else {
                    img.width =200;
                    img.height =200;
                }
                let link = document.createElement("A");
                link.href = "/uploads/" +element;
                link.appendChild(img);
                let image = node.insertCell(0);
                image.appendChild(link);

                let nametxt =document.createTextNode(element);
                let nameLink = document.createElement("A");
                nameLink.href ="/uploads/" +element;
                nameLink.appendChild(nametxt)
                let name =node.insertCell(1);
                name.appendChild(nameLink);

                let sizetxt = document.createTextNode(data.sizes[i] + "KB");
                let size =node.insertCell(2);
                size.appendChild(sizetxt);

                let recttxt = document.createTextNode(data.aspects[i].rects);
                let rect =node.insertCell(3);
                rect.appendChild(recttxt);

                let circtxt = document.createTextNode(data.aspects[i].circs);
                let circ =node.insertCell(4);
                circ.appendChild(circtxt);

                let pathtxt = document.createTextNode(data.aspects[i].paths);
                let path =node.insertCell(5);
                path.appendChild(pathtxt);

                let grouptxt = document.createTextNode(data.aspects[i].groups);
                let group =node.insertCell(6);
                group.appendChild(grouptxt);
                i++;
            });
            getViewpic();
            getTitleDesc(); //here when c lib is implimented
            getComp() //here when c lib is implimented
            //We write the object to the console to show that the request was successful
            console.log("just data: " + data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    //redunant ajax call
    /*$.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/imagenames',   //The server endpoint we are connecting to
        data: {
        },
        success: function (data) {
            //update view images
            let node = document.getElementById("viewSelection");
            data.lst.forEach(element =>{
                let option =document.createElement("option");
                option.text =element;
                node.add(option);
            });
            //update shape adder images
            node =document.getElementById("shapeImageSelection");
            data.lst.forEach(element =>{
                let option =document.createElement("option");
                option.text =element;
                node.add(option);
            });
            //We write the object to the console to show that the request was successful
            console.log(data);

            getViewpic();
            //add gettitledesc() here when c lib is implimented
            //add getcomp() here when c lib is implimented

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });*/

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    /*$('#someform').submit(function(e){
        $('#blah').html("Form has data: "+$('#entryBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });*/

    //when user changes the image they wish to view update everything
    $('#viewSelection').change (function (e){
        e.preventDefault();
        getViewpic();
        getTitleDesc();
        getComp();
    });

    //when user changes the title change it in backend then refresh everything
    $('#titleUpdate').submit(function(e){
        e.preventDefault();
        let img = document.getElementById('viewSelection').value;
        if(img == '') {
            alert("no files are available to update the title for");
            return;
        }
        let title = document.getElementById("addTitle").value;
        //shorten the string to the correct length
        if(title.length > 256) {
            title.slice(0, 256);
        }
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/title',
            data: {
                name: img,
                title: title
            },
            success: function (data) {
                getTitleDesc(); //here when c library is implimented
                //document.getElementById('viewTitle').innerHTML = title;
                console.log(title);
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
    });

    //when user changes the title change it in backend then refresh everything
    $('#descUpdate').submit(function(e){
        e.preventDefault();
        let img = document.getElementById('viewSelection').value;
        if(img == '') {
            alert("no files are available to update the description for");
            return;
        }
        let desc = document.getElementById("addDesc").value;
        //shorten the string to the correct length
        if(desc.length > 256) {
            desc.slice(0, 256);
        }
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/desc',
            data: {
                name: img,
                desc: desc
            },
            success: function (data) {
                getTitleDesc(); //here when c library is implimented
                document.getElementById('viewDesc').innerHTML = desc;
                console.log(desc);
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
    });

    //add/replace attribute submitted
    $('#attrUpdate').submit(function(e){
        e.preventDefault();
        let choice = document.getElementById('viewSelection').value; //gets image
        let shapetxt =document.getElementById('compList').value;
        let type = document.getElementById("addType").value; //gets attribute type
        let value = document.getElementById("addValue").value; //gets attribute value
        if(shapetxt =='') {
            alert("no shapes are available to update attributes on in this file");
            return;
        }
        let indexes =choice.match(/(\d+)/);
        let index;
        if(indexes != null) {
            index = indexes[0];
        } else {
            index = 0;
        }
        let shape =shapetxt.replace(/[0-9]/g, ""); //gets shape
        if(choice =='') {
            alert("no files are available to update attributes on");
            return;
        }
        
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/attribute',
            data: {
                name: choice,
                shape: shape,
                index: index,
                type: type,
                val: value
            },
            success: function (data) {
                getViewpic(); //use this when c library implimented
                getComp(); //use this when c library implimented
                location.reload(); //because i cant get the svg image to update dynamically
                console.log(type + " " + value);
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
    });
    
    //show attribute selected
    $('#complist').change (function (e){
        console.log('complist listener triggered');
        getattr();
    });
    //show attribute selected
    /*$('#complist').change (function(e){
        e.preventDefault();
        console.log('complist listener triggered');
        getattr();
    });*/

    //adds a shape to a image
    $('#addShape').submit(function(e){
        e.preventDefault();
        let img = document.getElementById("shapeImageSelection").value; //gets the image
        if(img == '') {
            alert("no files are available to add shapes to");
            return;
        }
        let shapetxt =document.getElementById('shapeSelection').value; //gets the shape
        let attrList = document.getElementById("shapeAttr").value; //gets attributes
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/addshape',
            data: {
                img: img,
                shape: shapetxt,
                attr: attrList
            },
            success: function (data) {
                getComp() //when c lib is implimented
                location.reload(); //because i cant get the svg image to update dynamically
                console.log(attrList);
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
    });

    //adds a shape to a image
    $('#scaleShape').submit(function(e){
        e.preventDefault();
        let img = document.getElementById("shapeImageSelection").value; //gets the image
        if(img == '') {
            alert("no files are available to scale the shapes of");
            return;
        }
        let shapetxt =document.getElementById('shapeScaleSelection').value; //gets the shape
        let factor = parseFloat(document.getElementById("scaleShapeFactor").value); //gets attributes
        if(isNaN(factor) || factor <0) {
            alert('invalid number');
            console.log("invalid number");
            return
        }
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/scaleshape',
            data: {
                img: img,
                shape: shapetxt,
                factor: factor
            },
            success: function (data) {
                getViewpic(); //when c lib is implimented
                getComp(); //when c lib is implimented
                location.reload(); //because i cant get the svg image to update dynamically
                console.log("pretend every number is multiplied by " + factor);
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
    });

    $('#createSvg').submit (function (e){
        e.preventDefault();
        let fileName = document.getElementById('newFileName').value;
        if(fileName == undefined || fileName == '') {
            alert("invalid file name");
            return
        }
        if(!fileName.endsWith(".svg")) {
            fileName = fileName.concat(".svg");
        }
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/newsvg',   //The server endpoint we are connecting to
            data: {
                name: fileName,
            },
            success: function (data) {
                //location.reload(); //just reload the entire page im not recoding an entire ajax call for one case
                if(data.result == 'image exists') {
                    alert("image " + fileName + " already exists");
                }
                console.log(fileName + ' was created'); 
    
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
    });
});

//get picture of image
function getViewpic() {
    let choice = document.getElementById('viewSelection').value;
    console.log("pic choice " + choice);
    if(choice == '') {
        return
    }
    //clear any initial html writing
    let img = document.getElementById('viewimage');
    img.innerHTML ="";
    //put a picture there
    let image =document.createElement("img");
    image.src = "/uploads/" + choice;
        
    //scale it to proper proportions
    if(image.width >0 && image.height >0) {
        let ratio =image.width/800;
        image.width= image.width/ratio;
        image.height =image.height/ratio;
    } else {
        image.width =800;
        image.height =800;
    }
    img.appendChild(image);
    console.log("appended img");
}

//get title and description of image
function getTitleDesc() {
    let choice = document.getElementById('viewSelection').value;
    if(choice == '') {
        return
    }
    let title = document.getElementById('viewTitle');
    let desc = document.getElementById('viewDesc');
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/titledesc',   //The server endpoint we are connecting to
        data: {
            name: choice,
        },
        success: function (data) {
            
            title.innerHTML = data.title;
            desc.innerHTML = data.desc;
            console.log(data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });
}

//update components of image
function getComp() {
    let choice = document.getElementById('viewSelection').value;
    console.log('choice '+choice);
    if(choice == '') {
        return
    }
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/components',   //The server endpoint we are connecting to
        data: {
            name: choice,
        },
        success: function (data) {
            let i=0;
            let reset = document.getElementById('comp');
            let rows = reset.rows;
            i = rows.length -1;
            //remove previous rows except header row
            
            while(i > 0) {
                reset.deleteRow(i);
                i--;
            }
            reset =document.getElementById("compList");
            rows = reset.options;
            i= rows.length -1;

            while(i >= 0) {
                reset.remove(i);
                i--;
            }
            
            let imageOption = document.createElement('option');
            imageOption.value =choice;
            imageOption.text =choice;

            document.getElementById("compList").add(imageOption);

            i=1
            data.svg.rects.forEach(element => {
                let recttxt = document.createTextNode("Rectangle " +i);
                let rectOption = document.createElement('option');
                rectOption.value ="Rectangle " +i;
                rectOption.text ="Rectangle " +i;

                document.getElementById("compList").add(rectOption);

                let node = document.getElementById("comp").insertRow();
                let rect =node.insertCell(0);
                rect.appendChild(recttxt);

                let rectSum = document.createTextNode("" + element.sum);
                let sum =node.insertCell(1);
                sum.appendChild(rectSum);

                let rectOther = document.createTextNode("" + element.other);
                let other =node.insertCell(2);
                other.appendChild(rectOther);
                i++;
            });

            i=1;
            data.svg.circs.forEach(element => {
                let circtxt = document.createTextNode("Circle " +i);
                let circOption = document.createElement('option');
                circOption.value ="Circle " +i;
                circOption.text ="Circle " +i;

                document.getElementById("compList").add(circOption);
                let node = document.getElementById("comp").insertRow();

                let circ =node.insertCell(0);
                circ.appendChild(circtxt);

                let circSum = document.createTextNode("" + element.sum);
                let sum =node.insertCell(1);
                sum.appendChild(circSum);

                let circOther = document.createTextNode("" + element.other);
                let other =node.insertCell(2);
                other.appendChild(circOther);
                i++;
            });

            i=1;
            data.svg.paths.forEach(element => {
                let pathtxt = document.createTextNode("Path " +i);
                let pathOption = document.createElement('option');
                pathOption.value ="Path " +i;
                pathOption.text ="Path " +i;

                document.getElementById("compList").add(pathOption);
                let node = document.getElementById("comp").insertRow();

                let path =node.insertCell(0);
                path.appendChild(pathtxt);

                let pathSum = document.createTextNode("" + element.sum);
                let sum =node.insertCell(1);
                sum.appendChild(pathSum);

                let pathOther = document.createTextNode("" + element.other);
                let other =node.insertCell(2);
                other.appendChild(pathOther);
                i++;
            });

            i=1;
            data.svg.groups.forEach(element => {
                let grouptxt = document.createTextNode("Group " +i);
                let groupOption = document.createElement('option');
                groupOption.value ="Group " +i;
                groupOption.text ="Group " +i;

                document.getElementById("compList").add(groupOption);
                let node = document.getElementById("comp").insertRow();


                let group =node.insertCell(0);
                group.appendChild(grouptxt);

                let groupSum = document.createTextNode("" + element.sum);
                let sum =node.insertCell(1);
                sum.appendChild(groupSum);

                let groupOther = document.createTextNode("" + element.other);
                let other =node.insertCell(2);
                other.appendChild(groupOther);
                i++;
            });
            getattr();
            console.log(data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error);
        }
    });
}

//update displayed other attributes list of component
function getattr() {
    let img = document.getElementById('viewSelection').value;
    let choice =document.getElementById('compList').value;
    if(choice == '') {
        let reset1 =document.getElementById("attributes");
        let rows1 = reset1.rows;
        let i1= rows1.length -1;

        while(i1 > 0) {
            reset1.deleteRow(i1);
            i1--;
        }
        return
    }
    console.log('about to check');
    let indexes =choice.match(/(\d+)/);
    let index;
    if(indexes != null) {
        index = indexes[0];
    } else {
        index = 0;
    }
    let type = choice.replace(/[0-9]/g, "");
    console.log("string = <" + type + ">");
    console.log("index =" + index);
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/attributes',   //The server endpoint we are connecting to
        data: {
            svg: img,
            type: type,
            index: index
        },
        success: function (data) {
            let attributes = document.getElementById('attributes');
            let rows = attributes.rows;
            let i = rows.length -1;
            //remove previous rows except header row
            
            while(i > 0) {
                attributes.deleteRow(i);
                i--;
            }

            data.attr.forEach( element => {
                let typetxt = document.createTextNode(element.name);
                let node =document.getElementById('attributes').insertRow();
                
                let cell1 =node.insertCell(0);
                cell1.appendChild(typetxt);

                let valtxt = document.createTextNode(element.value);
                let cell2 =node.insertCell(1);
                cell2.appendChild(valtxt);
            });
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error);
        }
    });
}