class A inherits B {

} ; 

class B inherits C {
    
} ;

class C inherits A {

} ;

class Main inherits IO {
  main() : Object { 
    out_string("Hello, world.\n") 
  } ;
} ; 
