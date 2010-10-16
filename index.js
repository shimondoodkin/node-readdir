var readdir=require(__dirname+'/build/default/readdir').ReadDir;
var x=new readdir();
x.open("/var",function(ok1){
 console.log("ok1:"+ok1);
 x.read(2,function(arr1,eof1){
  console.log("arr1:"+arr1,"eof1:"+eof1);
  x.close(function(cok1){
   console.log("cok1:"+cok1);
/*
x.open("/var",function(ok2){
 console.log("ok2:"+ok2);
 x.read(2,function(arr2,eof2){
  console.log("arr2:"+arr2,"eof2:"+eof2);
  x.close(function(cok2){
   console.log("cok2:"+cok2);
  });
 });
});
*/

  });
 });
});

var z=new readdir();
z.open("/var",function(ok3){
 console.log("ok3:"+ok3);
 z.read(2,function(arr3,eof3){
  console.log("arr3:"+arr3,"eof3:"+eof3);
  z.close(function(cok3){
   console.log("cok3:"+cok3);
  });
 });
});


